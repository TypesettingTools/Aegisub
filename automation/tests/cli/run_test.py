#!/usr/bin/env python3
"""End-to-end tests for Aegisub's --cli mode.

Runs a deterministic automation macro on fixture files in a throwaway HOME
(so the user's real config is untouched) and checks the results, plus the
error paths for bad command lines and unloadable input files. Must work
without a display server; DISPLAY/WAYLAND_DISPLAY are stripped from the
environment to make sure of that.
"""

import json
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

def test_resample(aegisub, srcdir, env, tmp):
    # The built-in resampler has no UI in CLI mode and instead resamples to
    # the video's resolution, which is what muxtools relies on
    out_file = os.path.join(tmp, "out-resample.ass")
    proc = run_cli(aegisub, [
        os.path.join(srcdir, "input.ass"),
        out_file,
        "tool/resampleres",
        "--video", "?dummy:24:48:1920:1080:0:0:0:",
    ], env)
    if proc.returncode != 0:
        return fail(f"resample: aegisub --cli exited with {proc.returncode}")

    with open(out_file, encoding="utf-8-sig") as f:
        out = f.read()

    if "PlayResX: 1920" not in out or "PlayResY: 1080" not in out:
        return fail("resample: PlayRes was not changed to the video's resolution", out)
    # 1280x720 -> 1920x1080 scales sizes by 1.5
    if "Style: Default,Arial,72," not in out:
        return fail("resample: the style was not resampled", out)
    return 0

def test_resample_without_video(aegisub, srcdir, env, tmp):
    out_file = os.path.join(tmp, "out-resample-novideo.ass")
    proc = run_cli(aegisub, [
        os.path.join(srcdir, "input.ass"),
        out_file,
        "tool/resampleres",
    ], env)
    if proc.returncode != 1:
        return fail(f"resample without video: expected exit code 1, got {proc.returncode}")
    if b"requires --video" not in proc.stdout:
        return fail("resample without video: no diagnostic was printed")
    if os.path.exists(out_file):
        return fail("resample without video: an output file was written anyway")
    return 0

def test_lua_error(aegisub, srcdir, env, tmp):
    # A script error has to fail the run and say what went wrong
    out_file = os.path.join(tmp, "out-error.ass")
    proc = run_cli(aegisub, [
        os.path.join(srcdir, "input.ass"),
        out_file,
        "CLI Test/Fail",
        "--automation", os.path.join(srcdir, "cli-test-macro.lua"),
    ], env)
    if proc.returncode != 1:
        return fail(f"lua error: expected exit code 1, got {proc.returncode}")
    if b"cli-test-failure" not in proc.stdout:
        return fail("lua error: the script's error was not printed")
    if os.path.exists(out_file):
        return fail("lua error: an output file was written anyway")
    return 0

def test_builtin_command(aegisub, srcdir, env, tmp):
    # Built-in commands which work without the GUI can be run directly
    out_file = os.path.join(tmp, "out-delete.ass")
    proc = run_cli(aegisub, [
        os.path.join(srcdir, "input.ass"),
        out_file,
        "edit/line/delete",
        "--selected-lines", "0",
    ], env)
    if proc.returncode != 0:
        return fail(f"built-in command: aegisub --cli exited with {proc.returncode}")

    with open(out_file, encoding="utf-8-sig") as f:
        out = f.read()

    if "First line" in out or "Second line" not in out:
        return fail("built-in command: the selected line was not deleted", out)
    return 0

def test_gui_command(aegisub, srcdir, env, tmp):
    # Commands which need the GUI have to fail cleanly rather than hang or crash
    out_file = os.path.join(tmp, "out-gui.ass")
    proc = run_cli(aegisub, [
        os.path.join(srcdir, "input.ass"),
        out_file,
        "video/play",
    ], env)
    if proc.returncode != 1:
        return fail(f"GUI command: expected exit code 1, got {proc.returncode}")
    if b"can't be run in CLI mode" not in proc.stdout:
        return fail("GUI command: no diagnostic was printed")
    if os.path.exists(out_file):
        return fail("GUI command: an output file was written anyway")
    return 0

def test_config_options(aegisub, srcdir, env, tmp):
    # --config and --user-dir read the settings from elsewhere, and --no-config
    # ignores them. The default duration shows up as the length of the line
    # which subtitle/insert/after adds after the last one.
    config = json.dumps({"Timing": {"Default Duration": 1234}})
    config_file = os.path.join(tmp, "custom-config.json")
    with open(config_file, "w") as f:
        f.write(config)
    profile = os.path.join(tmp, "profile")
    os.makedirs(profile)
    with open(os.path.join(profile, "config.json"), "w") as f:
        f.write(config)

    out_file = os.path.join(tmp, "out-config.ass")
    def inserted_line_end(*args):
        proc = run_cli(aegisub, [
            os.path.join(srcdir, "input.ass"),
            out_file,
            "subtitle/insert/after",
            "--active-line", "3",
            "--selected-lines", "3",
        ] + list(args), env)
        if proc.returncode != 0:
            return None
        with open(out_file, encoding="utf-8-sig") as f:
            dialogue = [line for line in f if line.startswith("Dialogue:")]
        return dialogue[-1].split(",")[2]

    for args, expected in [((), "0:00:23.00"),
                           (("--config", config_file), "0:00:21.23"),
                           (("--user-dir", profile), "0:00:21.23"),
                           (("--user-dir", profile, "--no-config"), "0:00:23.00")]:
        end = inserted_line_end(*args)
        if end != expected:
            return fail(f"config options: with {' '.join(args) or 'no options'}, the new line ends at {end}, expected {expected}")

    proc = run_cli(aegisub, [
        os.path.join(srcdir, "input.ass"),
        out_file,
        "subtitle/insert/after",
        "--config", os.path.join(tmp, "does-not-exist.json"),
    ], env)
    if proc.returncode != 1:
        return fail(f"config options: a missing --config file gave exit code {proc.returncode}, expected 1")
    return 0

def test_loglevel(aegisub, srcdir, env, tmp):
    out_file = os.path.join(tmp, "out-loglevel.ass")
    def run(*args):
        return run_cli(aegisub, [
            os.path.join(srcdir, "input.ass"),
            out_file,
            "grid/sort/start",
        ] + list(args), env)

    proc = run()
    if proc.returncode != 0 or b"Debug:" in proc.stdout:
        return fail("loglevel: debug messages were logged by default")
    proc = run("--loglevel", "4")
    if proc.returncode != 0 or b"Debug:" not in proc.stdout:
        return fail("loglevel: --loglevel 4 did not log debug messages")
    proc = run("--loglevel", "5")
    if proc.returncode != 1:
        return fail(f"loglevel: --loglevel 5 gave exit code {proc.returncode}, expected 1")
    return 0

def test_aegisub_cli_name(aegisub, srcdir, env, tmp):
    # Tools which can't pass --cli can run Aegisub as aegisub-cli instead
    link = os.path.join(tmp, "aegisub-cli")
    os.symlink(os.path.abspath(aegisub), link)
    out_file = os.path.join(tmp, "out-link.ass")
    proc = subprocess.run([link, os.path.join(srcdir, "input.ass"), out_file, "grid/sort/start"],
                          env=env, timeout=240, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    sys.stdout.buffer.write(proc.stdout)
    if proc.returncode != 0 or not os.path.exists(out_file):
        return fail(f"aegisub-cli: exited with {proc.returncode} without writing the output file")
    return 0

# Caches that CLI mode may write, since they're disposable and only speed up
# later runs. On some platforms ?cache is the same directory as ?user.
CACHE_DIRS = {"auto4cache", "ffms2cache"}

def test_no_user_state(aegisub, srcdir, env, tmp):
    # None of the runs above may have written config, hotkeys, MRU lists,
    # logs etc. into any of the user's directories
    written = []
    for user_dir in (os.path.join(tmp, ".aegisub"),
                     os.path.join(tmp, ".config", "aegisub"),
                     os.path.join(tmp, ".cache", "aegisub"),
                     os.path.join(tmp, ".local", "state", "aegisub"),
                     os.path.join(tmp, "Library", "Application Support", "Aegisub")):
        for root, dirs, files in os.walk(user_dir):
            dirs[:] = [d for d in dirs if d not in CACHE_DIRS]
            written += [os.path.join(root, f) for f in files]
    if written:
        return fail("files were written to the user's config directory:\n" + "\n".join(written))
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
        env["XDG_STATE_HOME"] = os.path.join(tmp, ".local", "state")
        # macOS looks up ?user through Foundation, which ignores HOME
        env["CFFIXED_USER_HOME"] = tmp

        # ?data points at the install prefix, which doesn't exist for an
        # uninstalled build, so point it at the source tree instead to give
        # scripts access to the bundled include files (moonscript etc.)
        env["AEGISUB_DATA_DIR"] = os.path.dirname(os.path.dirname(os.path.dirname(srcdir)))

        tests = [test_macro_runs, test_empty_file, test_missing_input,
                 test_resample, test_resample_without_video,
                 test_lua_error, test_builtin_command, test_gui_command,
                 test_config_options, test_loglevel, test_aegisub_cli_name,
                 test_bad_option_value, test_unknown_option,
                 test_no_user_state]
        for test in tests:
            print(f"== {test.__name__}")
            if test(aegisub, srcdir, env, tmp) != 0:
                return 1

    print("ok")
    return 0

if __name__ == "__main__":
    sys.exit(main())
