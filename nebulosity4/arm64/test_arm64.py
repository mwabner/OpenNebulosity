#!/usr/bin/env python3
"""Run camera-independent ARM64 regression tests; optionally compare an ARW/FITS pair."""
import argparse
from pathlib import Path
import subprocess
import sys
from build_arm64 import REPO, environment


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, default=REPO / "build/arm64")
    parser.add_argument("--configuration", default="Development")
    parser.add_argument("--raw", type=Path)
    parser.add_argument("--fits", type=Path)
    args = parser.parse_args()
    if bool(args.raw) != bool(args.fits):
        parser.error("Supply --raw and --fits together; keep private camera images outside Git.")
    env = environment()
    build = args.build_dir.resolve()
    resources = build / "products" / args.configuration / "Nebulosity 4 ARM64.app/Contents/Resources"
    if not resources.is_dir():
        raise RuntimeError("Build the app first with build_arm64.py.")
    tests = build / "tests"
    tests.mkdir(parents=True, exist_ok=True)
    includes = ["-I" + str(path / "include") for path in env["prefixes"].values()]
    libs = ["-L" + str(path / "lib") for path in env["prefixes"].values()]
    wx_flags = env["wx_flags"] + env["wx_libs"]
    source = REPO / "nebulosity4/arm64"
    def run_test(name, sources, flags, arguments):
        exe = tests / name
        subprocess.run(["xcrun", "clang++", "-arch", "arm64", "-std=c++14", *includes,
                        *[str(path) for path in sources], *libs, *flags, "-o", str(exe)], check=True)
        subprocess.run([str(exe), *[str(arg) for arg in arguments]], check=True)
    run_test("language", [source / "tests/language_compat_test.cpp"], wx_flags, [tests / "language.conf", resources])
    run_test("capture-errors", [source / "tests/gphoto_capture_errors.cpp", source / "GPhotoCapture.cpp"],
             ["-lgphoto2", "-lgphoto2_port"], [])
    if args.raw:
        run_test("raw-fits", [source / "tests/raw_fits_roundtrip.cpp"], ["-lraw", "-lcfitsio"],
                 [args.raw.resolve(), args.fits.resolve()])
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (RuntimeError, OSError, subprocess.CalledProcessError) as error:
        print("ARM64 tests failed: " + str(error), file=sys.stderr)
        sys.exit(1)
