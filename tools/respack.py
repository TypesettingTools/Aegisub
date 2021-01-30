#!/usr/bin/env python3

import sys, os

manifestfile, cppfile, hfile = sys.argv[1:]

with open(manifestfile, 'r') as manifest:
    files = dict((x.strip(), None) for x in manifest.readlines() if x.strip() != '')

sourcepath = os.path.split(manifestfile)[0]
buildpath = os.path.split(cppfile)[0]

for k in files:
    sf = os.path.join(sourcepath, k)
    bf = os.path.join(buildpath, k)

    if os.path.isfile(sf):
        files[k] = sf
    elif os.path.isfile(bf):
        files[k] = bf
    else:
        print("{}: Failed to open '{}'".format(manifestfile, k))
        sys.exit(1)

with open(cppfile, 'w') as cpp:
    cpp.write('#include "libresrc.h"\n')
    with open(hfile, 'w') as h:

        for k in files:
            with open(files[k], 'rb') as f:
                data = [str(int(x)) for x in f.read()]

            datastr = ','.join(data)
            name = os.path.splitext(os.path.basename(k))[0]
            cpp.write('const unsigned char {}[] = {{{}}};\n'.format(name, datastr))
            h.write('extern const unsigned char {}[{}];\n'.format(name, len(data)))
