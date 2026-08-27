#!/usr/bin/env python3
import pathlib,subprocess,sys
program,root=sys.argv[1],pathlib.Path(sys.argv[2]); cases=sorted(root.glob('*.mkv'))
if not cases: raise SystemExit('Matroska fixtures missing; run fixtures/generate.py')
for case in cases:
 result=subprocess.run([program,case],text=True,stdout=subprocess.PIPE)
 got=result.stdout
 expected=case.with_suffix('.behavior').read_text()
 if got!=expected: raise SystemExit(f'{case.name}: Matroska behavior changed\n{got}')
missing=subprocess.run([program,root/'does-not-exist.mkv'],text=True,stdout=subprocess.PIPE)
if missing.returncode != 1 or missing.stdout != 'error\tEBML header not found\n':
 raise SystemExit(f'missing input: unsafe or unexpected result\n{missing.stdout}')
