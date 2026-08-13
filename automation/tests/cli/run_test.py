#!/usr/bin/env python3
"""End-to-end tests for Aegisub's --cli mode.

Runs a deterministic automation macro on fixture files in a throwaway HOME
(so the user's real config is untouched) and checks the results, plus the
error paths for bad command lines and unloadable input files. Must work
without a display server; DISPLAY/WAYLAND_DISPLAY are stripped from the
environment to make sure of that.
"""

import os
import subprocess
import sys
import tempfile

def run_cli(aegisub, args, env):
    proc = subprocess.run([aegisub, "--cli"] + args, env=env, timeout=240,
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    sys.stdout.buffer.write(proc.stdout)
    return proc

def fail(message, output=None):
    print(message)
    if output is not None:
        print("---- output file ----")
        print(output)
    return 1

def test_macro_runs(aegisub, srcdir, env, tmp):
    out_file = os.path.join(tmp, "out.ass")
    proc = run_cli(aegisub, [
        os.path.join(srcdir, "input.ass"),
        out_file,
        "CLI Test/Append marker",
        "--automation", os.path.join(srcdir, "cli-test-macro.lua"),
    ], env)
    if proc.returncode != 0:
        return fail(f"aegisub --cli exited with {proc.returncode}")

    if not os.path.exists(out_file):
        return fail("output file was not written")

    with open(out_file, encoding="utf-8-sig") as f:
        out = f.read()

    marked = out.count("\\N{cli-test}")
    if marked != 3:
        return fail(f"expected 3 marked dialogue lines, found {marked}", out)

    if "A comment line" not in out:
        return fail("comment line went missing from the output", out)
    return 0

def test_empty_file(aegisub, srcdir, env, tmp):
    # A valid file with no dialogue lines gets a blank line inserted on
    # load, the same as when the GUI opens it, and the macro runs on that
    out_file = os.path.join(tmp, "out-empty.ass")
    proc = run_cli(aegisub, [
        os.path.join(srcdir, "empty.ass"),
        out_file,
        "CLI Test/Append marker",
        "--automation", os.path.join(srcdir, "cli-test-macro.lua"),
    ], env)
    if proc.returncode != 0:
        return fail(f"empty input: aegisub --cli exited with {proc.returncode}")

    if not os.path.exists(out_file):
        return fail("empty input: output file was not written")

    with open(out_file, encoding="utf-8-sig") as f:
        out = f.read()

    marked = out.count("\\N{cli-test}")
    if marked != 1:
        return fail(f"empty input: expected 1 marked line, found {marked}", out)
    return 0

def test_missing_input(aegisub, srcdir, env, tmp):
    out_file = os.path.join(tmp, "out-missing.ass")
    proc = run_cli(aegisub, [
        os.path.join(tmp, "does-not-exist.ass"),
        out_file,
        "CLI Test/Append marker",
        "--automation", os.path.join(srcdir, "cli-test-macro.lua"),
    ], env)
    if proc.returncode != 1:
        return fail(f"missing input: expected exit code 1, got {proc.returncode}")
    if b"Failed to load" not in proc.stdout:
        return fail("missing input: no diagnostic was printed")
    if os.path.exists(out_file):
        return fail("missing input: an output file was written anyway")
    return 0

def test_bad_option_value(aegisub, srcdir, env, tmp):
    # A malformed value must produce a clean error, not an uncaught
    # exception (which would show up here as death by SIGABRT)
    proc = run_cli(aegisub, ["--active-line=notanumber"], env)
    if proc.returncode != 1:
        return fail(f"bad option value: expected exit code 1, got {proc.returncode}")
    return 0

def test_unknown_option(aegisub, srcdir, env, tmp):
    proc = run_cli(aegisub, ["--definitely-not-an-option"], env)
    if proc.returncode != 1:
        return fail(f"unknown option: expected exit code 1, got {proc.returncode}")
    if b"unrecognised option" not in proc.stdout:
        return fail("unknown option: no diagnostic was printed")
    return 0

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

        # ?data points at the install prefix, which doesn't exist for an
        # uninstalled build, so make ?user/automation point at the source
        # tree to give scripts access to the include files (moonscript etc.)
        automation_dir = os.path.dirname(os.path.dirname(srcdir))
        user_dir = os.path.join(tmp, ".aegisub")
        os.makedirs(user_dir)
        os.symlink(automation_dir, os.path.join(user_dir, "automation"))

        tests = [test_macro_runs, test_empty_file, test_missing_input,
                 test_bad_option_value, test_unknown_option]
        for test in tests:
            print(f"== {test.__name__}")
            if test(aegisub, srcdir, env, tmp) != 0:
                return 1

    print("ok")
    return 0

if __name__ == "__main__":
    sys.exit(main())
