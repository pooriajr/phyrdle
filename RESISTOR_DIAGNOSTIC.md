# Resistor diagnostic

Run `./upload_diagnostic_usbasp.sh` in this directory to choose a profile and
upload via USBasp. Enter accepts the last successful diagnostic profile
(initially `yageo-10k`). Profiles are discovered from `profiles/*.h`. Use `--list` to list them,
pass a name such as `yageo-linear-10k` (or `yageo_linear_10k.h`),
or use `--last`. The diagnostic remembers its choice separately from the game.

The uploader compiles for the Nano at 8 MHz and verifies the uploaded flash.
It replaces the game firmware; use `./upload_usbasp.sh` to restore the game.

Each LED corresponds to its slot, using A0, A1, A2, A3, A6 and the game's
APA102 strip on data pin 11 / clock pin 13. ADC reference is Vcc.
Every slot gets one reading every 5 ms, passed through the same eight-sample moving average
as the game (about 40 ms to fully respond). A rolling window holds the most
recent 16 averaged readings (about 80 ms); LEDs refresh every 25 ms.
A settled diagnostic color normally follows within about 145 ms including
averaging, stability history, and LED refresh. This is not a hardware timing
guarantee; long processing pauses reset the average rather than retain stale samples.

- **Red:** readings in the window map to different ranges (including empty),
  or a reading is unrecognized. Crossings remain visible until they leave
  the window, so insertion/removal can briefly show red.
- **Yellow:** all readings identify the same letter, but some are outside
  the central 50% of that letter's detection range.
- **Green:** every reading identifies the same letter and lies within the
  central 50% of its range.
- **Dim white:** every reading is below or at the profile's empty threshold.
- **Blue:** collecting the initial window after startup.

A stable wrong letter can still show green: this checks detection stability
and boundary clearance, not the identity printed on the physical tile.
The diagnostic uses the selected profile's actual inclusive detection ranges.

## Serial readings

Open Serial Monitor at **9600 baud**. Once per second, the diagnostic logs
all five slots using this format:

```text
slot1=410(A,G,406,414) | slot2=512(H,Y,490,515) | ...
```

The fields are latest averaged ADC, detected letter, status, minimum averaged ADC, maximum averaged ADC.
Each log line ends with `filter=mean8`. Raw and averaged measurements must not be mixed.
Status codes are G=green, Y=yellow, R=red, E=empty, B=warming up.
`-` means empty; `?` means unrecognized. The logged extrema and status cover
all samples since the previous log (about one second), so fluctuations between
log messages are retained. LEDs still use their rolling 80 ms window.
Output is buffered so serial logging does not block ADC sampling.

USBasp does not provide serial monitoring; use the Nano USB serial connection
or a USB-to-serial adapter. Close Serial Monitor before starting calibration.

## Automatic profile calibration

First upload this updated diagnostic with the profile you want to calibrate:

```sh
./upload_diagnostic_usbasp.sh yageo-10k
./calibrate_profile.py yageo-10k --port /dev/cu.usbserial-10
```

Replace the port with your serial device. With no profile argument, the CLI
lists profiles; with no port, it shows discovered USB serial ports in a numbered menu.
Press Enter for the first port, R to rescan after connecting a cable, or Q to quit. The CLI uses Python 3.9+ and the standard library
on macOS/Linux; no pip packages are needed.

Insert tiles into any of the five slots. After three consecutive windows,
green/yellow letters are identified automatically. Yellow/red (or green/red) flicker keeps the last green/yellow identity, and
red windows are included in that letter’s measured envelope. Only persistent
red slots without a green/yellow identity prompt for a physical letter. Enter skips that slot until it becomes empty.
A confirmed red label stays associated with its slot until any sample reaches
the empty threshold. Mixed tile/empty windows are reported as removal/contact
events and excluded from calibration, even if their logged color is red. If
this happens without removing the tile, inspect its contacts. Old sessions
containing empty-level minima automatically archive those measurements and
request only the affected letters again.
**Remove a tile and wait for an empty window before putting another tile in.**
The tool cannot recognize a consistently wrong letter that appears green or
yellow, so verify you physically tested A–Z. It does not infer identities from
ADC ordering.

The default is five accepted one-second windows per letter. Measurements from
multiple slots are combined. Test the same tile across all slots for broader
coverage; the CLI can only promise separation for the measurements collected.
It automatically finishes when all 26 letters reach the requested window count.

- GOOD: measurements lie in the central half of the current range with at least
  the requested extra clearance (`--padding`, default 3 ADC counts).
- RISKY: inside the current range, but close to an edge.
- NEEDS CHANGE: observed measurements extend outside the assigned range.

Ctrl-C saves progress to `<profile>_mean8.calibration.json`. Rerun to resume; the tool
checks the source profile hash. To discard a letter's old measurements before
resuming, add `--reset-letter A`. Use `--session new.json` for a fresh run.

After coverage is complete, the tool writes `<profile>_mean8_calibrated.h`. It keeps
the original empty threshold, divides each gap between measured envelopes at
its midpoint, and extends the outer letters to the empty threshold and 1023.
This maximizes the smaller error clearance on each neighboring boundary.
Observed envelopes are never silently trimmed. Overlapping envelopes or a
letter reaching the empty threshold block export and identify the conflict.
A separable result with less than the requested reserve is exported with an
explicit warning. An updated boundary may fix a NEEDS CHANGE classification;
overlapping physical measurements require better hardware separation or noise
control instead.

These are measured calibration ranges, not worst-case resistor tolerance
bounds for unmeasured replacement tiles. Validate the resulting profile on the
assembled device. The original file is never overwritten; use `--output` to
choose a fresh destination. Profiles written to `profiles/` appear in both
USBasp upload menus. Nothing is uploaded automatically.

Averaging calibration starts a fresh session automatically. Previous raw-reading
sessions stay intact; upload the updated diagnostic and collect the letters again.
The game firmware must also be re-uploaded to use the same averaging.

The calibration CLI remembers the selected profile, last working serial port,
window count, and padding. Run `./calibrate_profile.py` and press Enter at the
menus to reuse them. Missing profiles/ports fall back to available choices.
Explicit command-line options override and update these defaults.
