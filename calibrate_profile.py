#!/usr/bin/env python3
"""Calibrate inclusive ADC ranges from resistor diagnostic logs (macOS/Linux)."""
import argparse
import glob
import hashlib
import json
import os
from pathlib import Path
import re
import select
import string
import sys
import termios
import time

LETTERS = string.ascii_uppercase
PROFILES = Path(__file__).resolve().parent / 'profiles'
DEFAULTS_FILE = Path(__file__).resolve().parent / '.calibration-defaults.json'
LINE = re.compile(r'slot([1-5])=(\d+)\(([A-Z?\-]),([RGYEB]),(\d+),(\d+)\)')


def read_profile(path):
    text = path.read_text()
    empty = re.search(r'EMPTY_SLOT_MAX_ADC\s*=\s*(\d+)', text)
    name = re.search(r'HARDWARE_PROFILE_NAME\[\]\s*=\s*"([^"]+)"', text)
    entries = re.findall(r"\{\s*(\d+)\s*,\s*(\d+)\s*,\s*'([A-Z])'\s*\}", text)
    if not empty or len(entries) != 26 or {r[2] for r in entries} != set(LETTERS):
        raise ValueError('Profile must contain 26 unique A–Z literal ranges and EMPTY_SLOT_MAX_ADC.')
    ranges = {letter: (int(lo), int(hi)) for lo, hi, letter in entries}
    threshold = int(empty[1])
    previous = threshold
    for lo, hi in sorted(ranges.values()):
        if not previous < lo <= hi <= 1023:
            raise ValueError('Profile has overlapping/invalid ranges or conflicts with empty threshold.')
        previous = hi
    return ranges, threshold, name[1] if name else path.stem


class IncompleteDiagnosticLine(ValueError):
    """A recoverable truncated serial record, never calibration data."""


def parse_line(line):
    records = []
    for slot, raw, letter, status, lo, hi in LINE.findall(line):
        raw, lo, hi = int(raw), int(lo), int(hi)
        if not 0 <= lo <= raw <= hi <= 1023:
            raise ValueError('Invalid ADC values in diagnostic log.')
        records.append(dict(slot=int(slot), raw=raw, letter=letter, status=status, lo=lo, hi=hi))
    if records and (len(records) != 5 or len({r['slot'] for r in records}) != 5):
        raise IncompleteDiagnosticLine('Incomplete diagnostic line.')
    return records


def classify(item, bounds, padding):
    low, high = bounds
    clearance = min(item['lo'] - low, high - item['hi'])
    if clearance < 0:
        return 'NEEDS CHANGE', clearance
    width = high - low
    if clearance < padding or clearance * 4 < width:
        return 'RISKY', clearance
    return 'GOOD', clearance


def observation_identity(row, confirmed, inferred):
    slot = row['slot']
    if slot in confirmed:
        return confirmed[slot]  # Includes explicit skip-until-empty.
    if row['status'] in ('G', 'Y') and row['letter'] in LETTERS:
        inferred[slot] = row['letter']
    return inferred.get(slot, 'RED')


def record(data, letter, row, verified):
    item = data.setdefault(letter, dict(lo=row['lo'], hi=row['hi'], windows=0, slots=[], verified=False))
    item['lo'] = min(item['lo'], row['lo'])
    item['hi'] = max(item['hi'], row['hi'])
    item['windows'] += 1
    if row['slot'] not in item['slots']:
        item['slots'].append(row['slot'])
    item['verified'] = item['verified'] or verified


def optimize(data, empty, padding):
    """Maximize minimum extra clearance locally by splitting measured gaps."""
    if set(data) != set(LETTERS):
        raise ValueError('Measurements for all 26 letters are required.')
    ordered = sorted(data, key=lambda letter: data[letter]['lo'])
    errors = []
    for a, b in zip(ordered, ordered[1:]):
        if data[a]['hi'] >= data[b]['lo']:
            errors.append(f"{a} [{data[a]['lo']},{data[a]['hi']}] overlaps {b} [{data[b]['lo']},{data[b]['hi']}]")
    if data[ordered[0]]['lo'] <= empty:
        errors.append(f'{ordered[0]} reaches the empty threshold ({empty})')
    if errors:
        raise ValueError('Cannot separate the observed readings: ' + '; '.join(errors) + '. Re-measure or change these resistors; ranges alone cannot fix this.')
    cuts = [(data[a]['hi'] + data[b]['lo']) // 2 for a, b in zip(ordered, ordered[1:])]
    ranges = {}
    for i, letter in enumerate(ordered):
        lo = empty + 1 if i == 0 else cuts[i - 1] + 1
        hi = 1023 if i == 25 else cuts[i]
        ranges[letter] = (lo, hi)
    weakest = min(min(data[k]['lo'] - lo, hi - data[k]['hi']) for k, (lo, hi) in ranges.items())
    return ranges, weakest


def report(data, ranges, padding, required):
    print('\nLetter  Assessment     Measured ADC   Current range  Extra room  Windows / slots')
    for letter in LETTERS:
        if letter not in data:
            print(f'{letter:6}  NOT SEEN')
            continue
        d = data[letter]
        status, room = classify(d, ranges[letter], padding)
        if d['windows'] < required:
            status = 'COLLECTING'
        print(f"{letter:6}  {status:13}  {d['lo']:4}–{d['hi']:<4}      {ranges[letter][0]:4}–{ranges[letter][1]:<4}      {room:4}      {d['windows']} / {','.join(map(str,d['slots']))}")


def write_profile(path, source, data, ranges, empty, weakest, padding):
    lines = ['#ifndef PROFILE_MEASURED_CALIBRATION_H', '#define PROFILE_MEASURED_CALIBRATION_H', '',
             f'// Calibrated from {source.name}; measured envelopes, not resistor tolerance bounds.',
             '// Requires the same eight-sample moving average (mean8) used during calibration.',
             '// Inclusive midpoint boundaries maximize clearance between measured envelopes.',
             '// Assumes stable readings identify the physical letter correctly unless manually labeled.',
             '// Applies only to the measured tiles/slots/conditions; validate before normal use.',
             f'// Minimum observed extra clearance: {weakest} counts; requested reserve: {padding}.',
             f'const char HARDWARE_PROFILE_NAME[] = "{path.stem}";',
             f'const uint16_t EMPTY_SLOT_MAX_ADC = {empty};', '',
             'const LetterAdcRange LETTER_ADC_RANGES[] PROGMEM = {']
    for letter in LETTERS:
        d = data[letter]
        lines.append(f"  {{{ranges[letter][0]}, {ranges[letter][1]}, '{letter}'}}, // measured {d['lo']}..{d['hi']}; {d['windows']} windows; slots {','.join(map(str,d['slots']))}")
    lines += ['};', 'const uint8_t LETTER_ADC_RANGE_COUNT = sizeof(LETTER_ADC_RANGES) / sizeof(LETTER_ADC_RANGES[0]);', '', '#endif', '']
    # Never overwrite an existing profile, including the source.
    with path.open('x') as f:
        f.write('\n'.join(lines))


class SerialLog:
    def __init__(self, port):
        self.fd = os.open(port, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
        self.old = termios.tcgetattr(self.fd)
        attrs = termios.tcgetattr(self.fd)
        attrs[0] = attrs[1] = attrs[3] = 0
        attrs[2] = termios.CS8 | termios.CREAD | termios.CLOCAL
        attrs[4] = attrs[5] = termios.B9600
        attrs[6][termios.VMIN] = 0
        attrs[6][termios.VTIME] = 0
        termios.tcsetattr(self.fd, termios.TCSANOW, attrs)
        self.buffer = b''
        self.flush()

    def flush(self):
        termios.tcflush(self.fd, termios.TCIFLUSH)
        self.buffer = b''
        # The device may already be transmitting. Drop through the next newline
        # before accepting a record, even if the first fragment looks parseable.
        self.resync = True

    def readline(self):
        while b'\n' not in self.buffer:
            if not select.select([self.fd], [], [], 5)[0]:
                return ''
            chunk = os.read(self.fd, 4096)
            if not chunk:
                raise OSError('Serial device disconnected.')
            self.buffer += chunk
            if len(self.buffer) > 16384:
                raise ValueError('Unexpected serial stream; check baud rate/diagnostic firmware.')
        line, self.buffer = self.buffer.split(b'\n', 1)
        if self.resync:
            self.resync = False
            return ''
        return line.decode('ascii', errors='replace').strip()

    def close(self):
        try:
            termios.tcsetattr(self.fd, termios.TCSANOW, self.old)
        finally:
            os.close(self.fd)


def load_defaults():
    try:
        data = json.loads(DEFAULTS_FILE.read_text())
        return data if isinstance(data, dict) else {}
    except (OSError, ValueError):
        return {}


def save_defaults(data):
    temp = DEFAULTS_FILE.with_suffix('.tmp')
    temp.write_text(json.dumps(data, indent=2))
    temp.replace(DEFAULTS_FILE)


def choose_profile(value, remembered=None):
    paths = sorted(p.resolve() for p in PROFILES.glob('*.h'))
    if value:
        path = Path(value)
        if path.is_file():
            return path.resolve()
        matches = [p for p in paths if value.removesuffix('.h').replace('_', '-') == p.stem.replace('_', '-')]
        if len(matches) == 1:
            return matches[0]
        raise ValueError(f'Profile not found: {value}')
    if not paths:
        raise ValueError('No profiles found.')
    remembered_path = Path(remembered).resolve() if isinstance(remembered, str) else None
    if remembered_path and remembered_path.is_file() and remembered_path not in paths:
        paths.append(remembered_path)
    default = paths.index(remembered_path) + 1 if remembered_path in paths else 1
    for i, p in enumerate(paths, 1):
        print(f'{i}) {p.stem}' + (' [default]' if i == default else ''))
    while True:
        answer = input(f'Profile number [{default}]: ').strip() or str(default)
        if answer.isdigit() and 1 <= int(answer) <= len(paths):
            return paths[int(answer) - 1]
        print('Choose one of the listed numbers.')


def choose_port(remembered=None):
    patterns = ('/dev/cu.usb*', '/dev/cu.wch*', '/dev/cu.SLAB*',
                '/dev/ttyUSB*', '/dev/ttyACM*')
    while True:
        ports = sorted({port for pattern in patterns for port in glob.glob(pattern)})
        if ports:
            default = ports.index(remembered) + 1 if remembered in ports else 1
            print('\nDetected USB serial ports:')
            for i, port in enumerate(ports, 1):
                print(f'  {i}) {port}' + (' [default]' if i == default else ''))
            prompt = f'Choose a number [{default}], R to rescan, or Q to quit: '
        else:
            print('\nNo USB serial ports found. Connect the Nano USB cable or a USB-to-serial adapter.')
            print('USBasp is a programmer and will not appear as a serial port.')
            prompt = 'Press Enter to rescan, or Q to quit: '
        answer = input(prompt).strip().lower()
        if answer == 'q':
            raise ValueError('No serial port selected.')
        if answer == 'r' or (not ports and not answer):
            continue
        if ports and not answer:
            answer = str(default)
        if answer.isdigit() and 1 <= int(answer) <= len(ports):
            port = ports[int(answer) - 1]
            print(f'Using {port}')
            return port
        print('Choose one of the listed numbers, R to rescan, or Q to quit.')


def main():
    defaults = load_defaults()
    windows = defaults.get('windows', 5)
    padding = defaults.get('padding', 3)
    if not isinstance(windows, int) or windows < 2: windows = 5
    if not isinstance(padding, int) or not 0 <= padding <= 1023: padding = 3
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('profile', nargs='?', help='Profile name or .h path')
    parser.add_argument('--port', help='Serial port (9600 baud, not USBasp)')
    parser.add_argument('--windows', type=int, default=windows, help=f'One-second measurement windows per letter (remembered default {windows})')
    parser.add_argument('--padding', type=int, default=padding, help=f'Desired extra ADC error reserve (remembered default {padding} counts)')
    parser.add_argument('--session', type=Path, help='Save/resume measurement JSON')
    parser.add_argument('--output', type=Path, help='New .h profile; never overwrites existing files')
    parser.add_argument('--reset-letter', choices=list(LETTERS), help='Discard this letter from a resumed session before collecting')
    args = parser.parse_args()
    if args.windows < 2 or not 0 <= args.padding <= 1023:
        parser.error('--windows must be >=2; --padding must be 0–1023.')
    source = choose_profile(args.profile, defaults.get('profile'))
    ranges, empty, profile_name = read_profile(source)
    fingerprint = hashlib.sha256(source.read_bytes()).hexdigest()
    session = args.session or source.with_name(source.stem + '_mean8.calibration.json')
    output = args.output or source.with_name(source.stem + '_mean8_calibrated.h')
    if output.exists():
        raise ValueError(f'Output exists: {output}. Choose another --output filename.')
    if not re.fullmatch(r'[A-Za-z0-9_][A-Za-z0-9_-]*\.h', output.name):
        raise ValueError('Output must be an upload-compatible .h filename (letters/numbers/_/-).')
    if session.resolve() in (source.resolve(), output.resolve()):
        raise ValueError('Session path must differ from profile paths.')
    data = {}
    excluded = {}
    if session.exists():
        saved = json.loads(session.read_text())
        if saved.get('profile_sha256') != fingerprint:
            raise ValueError('Session belongs to a different or modified profile. Choose a new --session.')
        if saved.get('filter') != 'mean8':
            raise ValueError('This session uses raw or different filtering. Start a new session for mean8 measurements.')
        data = saved['letters']
        excluded = saved.get('excluded_empty_measurements', {})
        if not isinstance(data, dict):
            raise ValueError('Invalid session measurements.')
        for letter, item in data.items():
            if len(letter) != 1 or letter not in LETTERS or not 0 <= item['lo'] <= item['hi'] <= 1023 or item['windows'] < 1:
                raise ValueError('Invalid session measurements.')
    contaminated = [letter for letter, item in data.items() if item['lo'] <= empty]
    for letter in contaminated:
        excluded[letter] = data.pop(letter)
    if contaminated:
        print('Excluded previous measurements that reached empty: ' + ', '.join(sorted(contaminated)))
        print('Their original data is archived in the session; only these letters need to be measured again.')
    if args.reset_letter:
        data.pop(args.reset_letter, None)

    def save():
        temp = session.with_name(session.name + '.tmp')
        temp.write_text(json.dumps(dict(profile=str(source), profile_sha256=fingerprint, filter='mean8', letters=data, excluded_empty_measurements=excluded), indent=2))
        temp.replace(session)

    def complete():
        return all(data.get(k, {}).get('windows', 0) >= args.windows for k in LETTERS)

    print(f'Profile: {profile_name}; baud: 9600; empty threshold: {empty}')
    print('Green/yellow identifies the tile; red flicker keeps that identity until empty. Red without an identity prompts.')
    print('Remove a tile until its slot is empty before changing tiles or clearing a red label.')
    print('Stable-but-wrong identities cannot be detected automatically. Test each physical letter.')
    print(f'Collecting {args.windows} measurement windows per letter. Ctrl-C saves progress; rerun to resume.')
    save()
    defaults.update(profile=str(source.resolve()), windows=args.windows, padding=args.padding)
    save_defaults(defaults)
    if not complete():
        port = args.port or choose_port(defaults.get('port'))
        serial = SerialLog(port)
        identities = {}  # User-confirmed red labels persist until an empty window.
        streaks = {}
        inferred = {}  # Carry green/yellow identity through red flicker, until empty.
        last_valid = time.monotonic()
        print(f'Listening on {port}. Close any other Serial Monitor. Insert tiles and leave them still.')
        try:
            while not complete():
                line = serial.readline()
                if line.startswith('Resistor diagnostic:'):
                    if line.split(':', 1)[1].strip() != profile_name:
                        raise ValueError('Firmware reports a different profile. Upload the selected diagnostic profile first.')
                    identities.clear(); streaks.clear(); inferred.clear()
                try:
                    rows = parse_line(line)
                except IncompleteDiagnosticLine:
                    if time.monotonic() - last_valid > 20:
                        raise ValueError('No complete diagnostic lines for 20 seconds. Check the serial connection.')
                    continue  # Partial records must never enter the measurements.
                if not rows:
                    if 'slot1=' in line:
                        raise ValueError('Old diagnostic log format. Re-upload using upload_diagnostic_usbasp.sh.')
                    if time.monotonic() - last_valid > 20:
                        raise ValueError('No valid logs for 20 seconds. Check serial port and diagnostic firmware.')
                    continue
                if 'filter=mean8' not in line:
                    raise ValueError('Upload the updated averaging diagnostic before calibrating. Expected filter=mean8.')
                last_valid = time.monotonic()
                # Compare each raw reading against the selected profile, even if startup banner was missed.
                for r in rows:
                    expected = '-' if r['raw'] <= empty else next((k for k, (lo, hi) in ranges.items() if lo <= r['raw'] <= hi), '?')
                    if expected != r['letter']:
                        raise ValueError('Log classification disagrees with selected profile. Re-upload that diagnostic profile.')
                if defaults.get('port') != port:
                    defaults['port'] = port
                    save_defaults(defaults)
                prompted = False
                for r in rows:
                    slot = r['slot']
                    if r['lo'] <= empty:
                        # A log window can contain BOTH a tile and its removal.
                        # Clear the label before accepting anything else, even if
                        # firmware reports RED and the latest reading is nonzero.
                        identities.pop(slot, None); streaks.pop(slot, None); inferred.pop(slot, None)
                        if r['hi'] > empty:
                            print(f"slot {slot}: skipped {r['lo']}–{r['hi']} window reaching empty (removal/contact dropout). If the tile stayed inserted, inspect its contacts.")
                        continue
                    if r['status'] == 'B':
                        continue
                    key = observation_identity(r, identities, inferred)
                    oldkey, n = streaks.get(slot, (None, 0))
                    streaks[slot] = (key, n + 1 if oldkey == key else 1)
                    if streaks[slot][1] < 3:
                        continue  # Discard insertion windows; require three consecutive windows.
                    if key == 'RED' and slot not in identities:
                        label = input(f"RED slot {slot}, ADC {r['lo']}–{r['hi']}. Physical letter A–Z (Enter skips until empty): ").strip().upper()
                        while label and (len(label) != 1 or label not in LETTERS):
                            label = input('Enter one letter A–Z, or Enter to skip: ').strip().upper()
                        if len(label) > 1:
                            label = ''
                        identities[slot] = label
                        serial.flush(); streaks[slot] = (label, 0)
                        last_valid = time.monotonic()  # Time spent answering is not a serial timeout.
                        prompted = True
                        break  # Discard stale records while waiting for user input.
                    letter = key
                    if letter not in LETTERS or len(letter) != 1:
                        continue
                    record(data, letter, r, slot in identities)
                    status, room = classify(data[letter], ranges[letter], args.padding)
                    print(f"slot {slot}: {letter} {r['lo']}–{r['hi']} | {status} | extra room {room} | {data[letter]['windows']}/{args.windows} windows")
                save()
                if not prompted:
                    done = ''.join(k for k in LETTERS if data.get(k, {}).get('windows', 0) >= args.windows)
                    missing = ''.join(k for k in LETTERS if k not in done)
                    print(f'Covered {len(done)}/26. Still need: {missing or "none"}', flush=True)
        except KeyboardInterrupt:
            save(); report(data, ranges, args.padding, args.windows)
            print(f'\nSaved {session}. Rerun to resume.')
            return 130
        finally:
            try:
                save()
            finally:
                serial.close()
    report(data, ranges, args.padding, args.windows)
    updated, weakest = optimize(data, empty, args.padding)
    write_profile(output, source, data, updated, empty, weakest, args.padding)
    print(f'\nCreated {output}\nSession: {session}\nMinimum extra clearance after calibration: ±{weakest} counts.')
    if weakest < args.padding:
        print(f'WARNING: requested ±{args.padding} reserve is not achievable with these measurements. These pairs/edges need attention:')
        for letter, (lo, hi) in updated.items():
            room = min(data[letter]['lo'] - lo, hi - data[letter]['hi'])
            if room < args.padding:
                print(f'  {letter}: only ±{room} counts')
    print('The original profile was not changed. Profiles saved in the profiles folder appear in both upload pickers.')
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (ValueError, OSError, KeyError, IndexError) as exc:
        print(f'Error: {exc}', file=sys.stderr)
        sys.exit(1)
