#!/usr/bin/env python3

import plistlib
import re
import subprocess
import sys
from pathlib import Path


MACHO_MAGICS = {
    b"\xca\xfe\xba\xbe",
    b"\xbe\xba\xfe\xca",
    b"\xca\xfe\xba\xbf",
    b"\xbf\xba\xfe\xca",
    b"\xce\xfa\xed\xfe",
    b"\xfe\xed\xfa\xce",
    b"\xcf\xfa\xed\xfe",
    b"\xfe\xed\xfa\xcf",
}


def version_tuple(version: str) -> tuple[int, ...]:
    if not re.fullmatch(r"[0-9]+(?:\.[0-9]+)*", version):
        raise ValueError(f"invalid macOS version: {version}")
    parts = [int(part) for part in version.split(".")]
    while len(parts) > 1 and parts[-1] == 0:
        parts.pop()
    return tuple(parts)


def minimum_versions(binary: Path) -> list[str]:
    output = subprocess.run(
        ["otool", "-l", binary],
        check=True,
        capture_output=True,
        text=True,
    ).stdout

    versions = []
    command = None
    for line in output.splitlines():
        fields = line.split()
        if fields[:1] == ["cmd"]:
            command = fields[1] if len(fields) > 1 else None
        elif command == "LC_BUILD_VERSION" and fields[:1] == ["minos"]:
            versions.append(fields[1])
        elif command == "LC_VERSION_MIN_MACOSX" and fields[:1] == ["version"]:
            versions.append(fields[1])
    return versions


def is_macho(path: Path) -> bool:
    if not path.is_file():
        return False
    with path.open("rb") as file:
        return file.read(4) in MACHO_MAGICS


def main() -> int:
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} AEGISUB_APP", file=sys.stderr)
        return 2

    app = Path(sys.argv[1])
    with (app / "Contents" / "Info.plist").open("rb") as file:
        info = plistlib.load(file)

    declared = info["LSMinimumSystemVersion"]
    declared_tuple = version_tuple(declared)
    executable = app / "Contents" / "MacOS" / info["CFBundleExecutable"]
    binaries = [path for path in app.rglob("*") if is_macho(path)]

    errors = []
    for binary in binaries:
        versions = minimum_versions(binary)
        if not versions:
            errors.append(f"{binary}: no macOS deployment target")
            continue
        for version in versions:
            if version_tuple(version) > declared_tuple:
                errors.append(f"{binary}: targets macOS {version}, above declared {declared}")

    executable_versions = minimum_versions(executable)
    if not executable_versions or any(version_tuple(version) != declared_tuple for version in executable_versions):
        found = ", ".join(executable_versions) if executable_versions else "none"
        errors.append(f"{executable}: expected deployment target {declared}, found {found}")

    if errors:
        print("macOS deployment-target verification failed:", file=sys.stderr)
        for error in errors:
            print(f"  {error}", file=sys.stderr)
        return 1

    print(f"Verified {len(binaries)} Mach-O files target macOS {declared} or older")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
