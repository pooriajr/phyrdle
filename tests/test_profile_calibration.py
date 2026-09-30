import importlib.util
import pathlib
import tempfile
import unittest
from unittest.mock import patch
import contextlib
import io
import json

ROOT = pathlib.Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('calibrate', ROOT/'calibrate_profile.py')
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


class CalibrationTests(unittest.TestCase):
    def setUp(self):
        self.defaults_temp=tempfile.TemporaryDirectory()
        self.addCleanup(self.defaults_temp.cleanup)
        patcher=patch.object(m,'DEFAULTS_FILE',pathlib.Path(self.defaults_temp.name)/'defaults.json')
        patcher.start();self.addCleanup(patcher.stop)

    def test_remembered_defaults(self):
        m.save_defaults(dict(profile='example',port='/dev/cu.usbserial-B',windows=20,padding=4))
        self.assertEqual(m.load_defaults()['windows'],20)
        with patch.object(m.glob,'glob',return_value=['/dev/cu.usbserial-A','/dev/cu.usbserial-B']),patch('builtins.input',return_value=''),contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(m.choose_port('/dev/cu.usbserial-B'),'/dev/cu.usbserial-B')
            self.assertEqual(m.choose_port('/dev/cu.missing'),'/dev/cu.usbserial-A')
        with tempfile.TemporaryDirectory() as tmp:
            folder=pathlib.Path(tmp).resolve();(folder/'a.h').touch();(folder/'b.h').touch()
            with patch.object(m,'PROFILES',folder),patch('builtins.input',return_value=''),contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(m.choose_profile(None,str(folder/'b.h')),folder/'b.h')
                self.assertEqual(m.choose_profile(None,str(folder/'missing.h')),folder/'a.h')
        m.DEFAULTS_FILE.write_text('broken json')
        self.assertEqual(m.load_defaults(),{})

    def data(self):
        return {letter: dict(lo=30+i*36, hi=34+i*36, windows=10, slots=[1], verified=False)
                for i, letter in enumerate(m.LETTERS)}

    def test_serial_resynchronizes_after_prompt(self):
        reader=m.SerialLog.__new__(m.SerialLog)
        reader.fd=123
        full=' | '.join(f'slot{i}=400(A,G,398,402)' for i in range(1,6))
        reader.buffer=b'old data'
        with patch.object(m.termios,'tcflush'):
            reader.flush()
        reader.buffer=('ot4=970(I,R,963,970) | slot5=0(-,E,0,0)\n'+full+'\n').encode()
        self.assertEqual(reader.readline(),'')
        self.assertEqual(len(m.parse_line(reader.readline())),5)
        with patch.object(m.termios,'tcflush'):
            reader.flush()
        # Bytes may arrive in chunks and only finish a fragment on a later read.
        with patch.object(m.select,'select',return_value=([123],[],[])), patch.object(m.os,'read',side_effect=[b'partial', (' tail\n'+full+'\n').encode()]):
            self.assertEqual(reader.readline(),'')
            self.assertEqual(reader.readline(),full)

    def test_contaminated_session_resumes_only_bad_letters(self):
        with tempfile.TemporaryDirectory() as tmp:
            tmp=pathlib.Path(tmp);source=tmp/'source.h';session=tmp/'session.json'
            data=self.data();ranges,room=m.optimize(data,20,3)
            m.write_profile(source,source,data,ranges,20,room,3)
            data['I']['lo']=0
            session.write_text(json.dumps(dict(profile_sha256=m.hashlib.sha256(source.read_bytes()).hexdigest(),filter='mean8',letters=data)))
            class FakeSerial:
                def __init__(self,port):pass
                def readline(self):raise KeyboardInterrupt
                def close(self):pass
            args=['calibrate',str(source),'--port','fake','--session',str(session)]
            with patch.object(m,'SerialLog',FakeSerial),patch('sys.argv',args),contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(m.main(),130)
            saved=json.loads(session.read_text())
            self.assertEqual(len(saved['letters']),25)
            self.assertNotIn('I',saved['letters'])
            self.assertEqual(saved['excluded_empty_measurements']['I']['lo'],0)

    def test_port_picker(self):
        def discover(pattern):
            return ['/dev/cu.usbserial-B', '/dev/cu.usbserial-A'] if pattern == '/dev/cu.usb*' else []
        with patch.object(m.glob, 'glob', side_effect=discover), patch('builtins.input', side_effect=['bad', '3', '2']), contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(m.choose_port(), '/dev/cu.usbserial-B')
        with patch.object(m.glob, 'glob', side_effect=discover), patch('builtins.input', return_value=''), contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(m.choose_port(), '/dev/cu.usbserial-A')
        with patch.object(m.glob, 'glob', return_value=[]), patch('builtins.input', return_value='q'), contextlib.redirect_stdout(io.StringIO()):
            with self.assertRaisesRegex(ValueError, 'No serial port selected'):
                m.choose_port()

    def test_yellow_identity_survives_red_flicker(self):
        inferred={}; confirmed={}
        row=dict(slot=1,letter='X',status='Y')
        self.assertEqual(m.observation_identity(row,confirmed,inferred),'X')
        row.update(letter='Y',status='R')
        self.assertEqual(m.observation_identity(row,confirmed,inferred),'X')
        row.update(letter='X',status='Y')
        self.assertEqual(m.observation_identity(row,confirmed,inferred),'X')
        confirmed[1]='I'
        self.assertEqual(m.observation_identity(row,confirmed,inferred),'I')
        confirmed[1]=''
        self.assertEqual(m.observation_identity(row,confirmed,inferred),'')
        confirmed.clear();inferred.clear();row['status']='R'
        self.assertEqual(m.observation_identity(row,confirmed,inferred),'RED')

    def test_log(self):
        line = ' | '.join(f'slot{i}=400(A,G,398,402)' for i in range(1, 6))
        rows = m.parse_line(line)
        self.assertEqual(len(rows), 5)
        self.assertEqual(rows[0]['hi'], 402)
        with self.assertRaises(ValueError):
            m.parse_line('slot1=400(A,G,398,402)')
        with self.assertRaises(ValueError):
            m.parse_line(line.replace('398,402', '401,402'))
        self.assertEqual(m.parse_line('startup'), [])

    def test_assessment(self):
        self.assertEqual(m.classify(dict(lo=140,hi=160),(100,200),3)[0], 'GOOD')
        self.assertEqual(m.classify(dict(lo=101,hi=105),(100,200),3)[0], 'RISKY')
        self.assertEqual(m.classify(dict(lo=99,hi=105),(100,200),3)[0], 'NEEDS CHANGE')

    def test_midpoints_maximize_clearance(self):
        data = self.data()
        ranges, room = m.optimize(data, 20, 3)
        for adc in range(1024):
            self.assertEqual(sum(lo<=adc<=hi for lo,hi in ranges.values()), int(adc>20))
        for a,b in zip(m.LETTERS,m.LETTERS[1:]):
            cut = ranges[a][1]
            objective = lambda c: min(c-data[a]['hi'],data[b]['lo']-c-1)
            best = max(objective(c) for c in range(data[a]['hi'],data[b]['lo']))
            self.assertEqual(objective(cut),best)
            self.assertEqual(ranges[b][0],cut+1)
        self.assertEqual(room,9)

    def test_overlaps_empty_missing(self):
        data=self.data();data['B']['lo']=data['A']['hi']
        with self.assertRaisesRegex(ValueError,'overlaps'):m.optimize(data,20,3)
        data=self.data();data['A']['lo']=20
        with self.assertRaisesRegex(ValueError,'empty'):m.optimize(data,20,3)
        data=self.data();del data['Z']
        with self.assertRaises(ValueError):m.optimize(data,20,3)

    def test_capture_and_export(self):
        data={}
        m.record(data,'Q',dict(lo=100,hi=105,slot=1),True)
        m.record(data,'Q',dict(lo=99,hi=106,slot=2),False)
        self.assertEqual(data['Q']['lo'],99)
        self.assertEqual(data['Q']['slots'],[1,2])
        data=self.data();ranges,room=m.optimize(data,20,3)
        with tempfile.TemporaryDirectory() as tmp:
            output=pathlib.Path(tmp)/'calibrated.h'
            m.write_profile(output,pathlib.Path('source.h'),data,ranges,20,room,3)
            read,empty,name=m.read_profile(output)
            self.assertEqual(read,ranges);self.assertEqual(empty,20)
            with self.assertRaises(FileExistsError):
                m.write_profile(output,pathlib.Path('source.h'),data,ranges,20,room,3)


    def test_end_to_end_red_prompt_and_auto_letters(self):
        with tempfile.TemporaryDirectory() as tmp:
            tmp=pathlib.Path(tmp)
            source=tmp/'source.h';output=tmp/'result.h';session=tmp/'session.json'
            data=self.data();ranges,room=m.optimize(data,20,3)
            m.write_profile(source,source,data,ranges,20,room,3)
            logs=[]
            for letter in m.LETTERS:
                lo,hi=ranges[letter]
                raw=(lo+hi)//2
                if letter=='A':
                    # True A crosses the old A/B boundary; identify manually.
                    low,high=hi-2,hi+2
                    detected='B';raw=high;state='R'
                else:
                    low,high=raw-1,raw+1;detected=letter;state='G'
                line=f'slot1={raw}({detected},{state},{low},{high})' + ''.join(f' | slot{i}=0(-,E,0,0)' for i in range(2,6))
                logs.extend([line]*9)
                if letter=='A':
                    logs.append(f'slot1=0(-,R,0,{high})' + ''.join(f' | slot{i}=0(-,E,0,0)' for i in range(2,6)))
                    logs.insert(4, 'slot4=970(I,R,963,970) | slot5=0(-,E,0,0)')
                logs.append(' | '.join(f'slot{i}=0(-,E,0,0)' for i in range(1,6)))
            class FakeSerial:
                def __init__(self,port):self.lines=iter(line + ' filter=mean8' for line in logs)
                def readline(self):return next(self.lines)
                def flush(self):pass
                def close(self):pass
            args=['calibrate',str(source),'--port','fake','--windows','2','--session',str(session),'--output',str(output)]
            with patch.object(m,'SerialLog',FakeSerial),patch('sys.argv',args),patch('builtins.input',return_value='A') as prompt,contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(m.main(),0)
                self.assertEqual(prompt.call_count,1)
            captured=json.loads(session.read_text())['letters']
            self.assertEqual(set(captured),set(m.LETTERS))
            self.assertTrue(captured['A']['verified'])
            self.assertGreater(captured['A']['lo'],20)
            updated,_,_=m.read_profile(output)
            self.assertGreaterEqual(updated['A'][1],captured['A']['hi'])


if __name__=='__main__':unittest.main()
