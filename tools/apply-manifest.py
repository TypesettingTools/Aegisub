#!/usr/bin/env python3

import sys, subprocess, pathlib, os

mt_exe, executable = sys.argv[1:]

subprocess.run([mt_exe, '-manifest', executable + '.manifest', '-outputresource:' + executable + ';1'])
pathlib.Path(os.path.join(os.path.dirname(executable), 'applied_manifest')).touch()
