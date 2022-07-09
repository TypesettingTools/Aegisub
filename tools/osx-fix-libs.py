#!/usr/bin/env python3

import re
import sys
import os
import shutil
import stat
import subprocess

is_bad_lib = re.compile(r'(/usr/local|/opt)').match
is_sys_lib = re.compile(r'(/usr|/System)').match
otool_libname_extract = re.compile(r'\s+(/.*?)[(\s:]').search
otool_loader_path_extract = re.compile(r'\s+@loader_path/(.*?)[(\s:]').search
goodlist = []
badlist = []
link_map = {}


def otool(cmdline):
    with subprocess.Popen(['otool'] + cmdline, stdout=subprocess.PIPE,
                          encoding='utf-8') as p:
        return p.stdout.readlines()


def collectlibs(lib, masterlist, targetdir):
    global goodlist, link_map
    liblist = otool(['-L', lib])
    locallist = []

    for l in liblist:
        lr = otool_libname_extract(l)
        if lr:
            l = lr.group(1)
        else:
            lr = otool_loader_path_extract(l)
            if lr:
                l = os.path.join(os.path.dirname(lib), lr.group(1))
            else:
                continue
        if is_bad_lib(l) and l not in badlist:
            badlist.append(l)
        if ((not is_sys_lib(l)) or is_bad_lib(l)) and l not in masterlist:
            locallist.append(l)
            print("found %s:" % l)

            check = l
            link_list = []
            while check:
                basename = os.path.basename(check)
                target = os.path.join(targetdir, basename)

                if os.path.islink(target):
                    # If a library was a symlink to a file with the same name in another directory,
                    # this could otherwise cause a FileNotFoundError
                    os.remove(target)

                if os.path.isfile(check) and not os.path.islink(check):
                    try:
                        shutil.copy(check, target)
                    except PermissionError:
                        print("    FILE %s ... skipped" % check)
                        break
                    print("    FILE %s ... copied to target" % check)
                    if link_list:
                        for link in link_list:
                            link_map[link] = basename
                    break

                if os.path.islink(check):
                    link_dst = os.readlink(check)
                    try:
                        os.symlink(link_dst, target)
                    except FileExistsError:
                        print("    LINK %s ... existed" % check)
                        break
                    print("    LINK %s ... copied to target" % check)
                    link_list.append(basename)
                    check = os.path.join(os.path.dirname(check), link_dst)

        elif l not in goodlist and l not in masterlist:
            goodlist.append(l)
    masterlist.extend(locallist)

    for l in locallist:
        collectlibs(l, masterlist, targetdir)


if __name__ == '__main__':
    binname = sys.argv[1]
    targetdir = os.path.dirname(binname)
    print("Searching for libraries in", binname, "...")
    libs = [binname]
    collectlibs(binname, libs, targetdir)

    print()
    print("System libraries used...")
    goodlist.sort()
    for l in goodlist:
        print(l)

    print()
    print("Fixing library install names...")
    in_tool_cmdline = ['install_name_tool']
    for lib in libs:
        libbase = os.path.basename(lib)
        if libbase in link_map:
            libbase = link_map[libbase]
        in_tool_cmdline = in_tool_cmdline + ['-change', lib,
                                             '@executable_path/' + libbase]
    for lib in libs:
        libbase = os.path.basename(lib)

        if libbase in link_map:
            libbase = link_map[libbase]
            print("%s -> @executable_path/%s (REMAPPED)" % (lib, libbase))
        else:
            print("%s -> @executable_path/%s" % (lib, libbase))

        targetlib = targetdir + '/' + libbase
        orig_permission = os.stat(targetlib).st_mode
        if not(orig_permission & stat.S_IWUSR):
            os.chmod(targetlib, orig_permission | stat.S_IWUSR)
        subprocess.run(in_tool_cmdline + ['-id', '@executable_path/' + libbase,
                                          targetlib])
        if not(orig_permission & stat.S_IWUSR):
            os.chmod(targetlib, orig_permission)

    if badlist:
        print()
        print("WARNING: The following libraries have blacklisted paths:")
        for lib in sorted(badlist):
            print(lib)
        print(
            "These paths normally have files from a package manager, which means that end result may not work if copied to another machine.")

    print()
    print("All done!")
