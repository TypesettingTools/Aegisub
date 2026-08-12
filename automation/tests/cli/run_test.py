#!/usr/bin/env python3
"""Smoke test for Aegisub's --cli mode.

Runs a deterministic automation macro on a fixture file in a throwaway HOME
(so the user's real config is untouched) and checks the modified lines were
written out. Must work without a display server; DISPLAY/WAYLAND_DISPLAY are
stripped from the environment to make sure of that.
"""

import os
import subprocess
import sys
import tempfile

def main():
    aegisub, srcdir = sys.argv[1], sys.argv[2]

    env = dict(os.environ)
    env.pop("DISPLAY", None)
    env.pop("WAYLAND_DISPLAY", None)

    with tempfile.TemporaryDirectory() as tmp:
        env["HOME"] = tmp
        env["XDG_CONFIG_HOME"] = os.path.join(tmp, ".config")
        env["XDG_CACHE_HOME"] = os.path.join(tmp, ".cache")
        env["XDG_DATA_HOME"] = os.path.join(tmp, ".local", "share")
        out_file = os.path.join(tmp, "out.ass")

        # ?data points at the install prefix, which doesn't exist for an
        # uninstalled build, so make ?user/automation point at the source
        # tree to give scripts access to the include files (moonscript etc.)
        automation_dir = os.path.dirname(os.path.dirname(srcdir))
        user_dir = os.path.join(tmp, ".aegisub")
        os.makedirs(user_dir)
        os.symlink(automation_dir, os.path.join(user_dir, "automation"))

        cmd = [
            aegisub, "--cli",
            os.path.join(srcdir, "input.ass"),
            out_file,
            "CLI Test/Append marker",
            "--automation", os.path.join(srcdir, "cli-test-macro.lua"),
        ]
        proc = subprocess.run(cmd, env=env, timeout=240,
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        sys.stdout.buffer.write(proc.stdout)
        if proc.returncode != 0:
            print(f"aegisub --cli exited with {proc.returncode}")
            return 1

        if not os.path.exists(out_file):
            print("output file was not written")
            return 1

        with open(out_file, encoding="utf-8-sig") as f:
            out = f.read()

        marked = out.count("\\N{cli-test}")
        if marked != 3:
            print(f"expected 3 marked dialogue lines, found {marked}")
            print("---- output file ----")
            print(out)
            return 1

        if "A comment line" not in out:
            print("comment line went missing from the output")
            print("---- output file ----")
            print(out)
            return 1

        print("ok")
        return 0

if __name__ == "__main__":
    sys.exit(main())
