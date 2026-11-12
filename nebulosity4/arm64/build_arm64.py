#!/usr/bin/env python3
"""Build only the Apple Silicon target using the installed native dependencies."""
import argparse
import json
import os
from pathlib import Path
import platform
import shlex
import shutil
import subprocess
import sys

REPO = Path(__file__).resolve().parents[2]
FORMULAE = ("wxwidgets", "cfitsio", "libraw", "libusb", "little-cms2", "libgphoto2", "gphoto2")


def output(args):
    return subprocess.check_output([str(arg) for arg in args], text=True).strip()


def quoted(items):
    # Xcode parses build-setting lists after argv parsing; keep spaces in paths.
    return " ".join('"' + str(item).replace('\\', '\\\\').replace('"', '\\"') + '"' for item in items)


def environment():
    if sys.platform != "darwin" or platform.machine() != "arm64":
        raise RuntimeError("Use a native Apple Silicon terminal on macOS (not Rosetta).")
    brew = shutil.which("brew") or "/opt/homebrew/bin/brew"
    if not Path(brew).is_file():
        raise RuntimeError("Install native Homebrew, then run brew bundle with arm64/Brewfile.")
    prefixes = {formula: Path(output([brew, "--prefix", formula])) for formula in FORMULAE}
    wx_config = os.environ.get("NEB_WX_CONFIG") or str(prefixes["wxwidgets"] / "bin/wx-config")
    wx_flags = shlex.split(output([wx_config, "--cxxflags"]))
    wx_libs = shlex.split(output([wx_config, "--libs", "std,aui,propgrid"]))
    wx_version = output([wx_config, "--version"])
    if tuple(int(part) for part in wx_version.split(".")[:2]) < (3, 3):
        raise RuntimeError("This target needs wxWidgets 3.3 or newer; use the documented tested versions.")
    sdk_version = output(["xcrun", "--sdk", "macosx", "--show-sdk-version"])
    if int(sdk_version.split(".")[0]) < 27:
        raise RuntimeError("The current preview targets macOS 27.0 and needs a macOS 27 SDK or newer.")
    required = (
        "Mac_Libraries/FreeImage_3_18/Makefile.srcs",
        "Mac_Libraries/FreeImage_3_18/Source/FreeImage/Plugin.cpp",
        "Mac_Libraries/EDSDK_v131231_Macintosh/EDSDK/Header/EDSDK.h",
        "Mac_Libraries/SBIGUDrv.framework/Headers/sbigudrv.h",
    )
    for name in required:
        if not (REPO / name).is_file():
            raise RuntimeError("Missing bundled build input: " + name + "; use a complete checkout.")
    native_libraries = {
        "cfitsio": ("libcfitsio.dylib",), "libraw": ("libraw.dylib",),
        "libusb": ("libusb-1.0.dylib",), "little-cms2": ("liblcms2.dylib",),
        "libgphoto2": ("libgphoto2.dylib", "libgphoto2_port.dylib"),
    }
    for formula, libraries in native_libraries.items():
        for library in libraries:
            subprocess.run(["lipo", "-verify_arch", "arm64", str(prefixes[formula] / "lib" / library)], check=True)
    return {"brew": brew, "prefixes": prefixes, "wx_config": wx_config,
            "wx_flags": wx_flags, "wx_libs": wx_libs, "wx_version": wx_version,
            "sdk_version": sdk_version, "xcode": output(["xcodebuild", "-version"]),
            "versions": output([brew, "list", "--versions", *FORMULAE])}


def build_arguments(env, build_dir, configuration):
    prefixes = env["prefixes"]
    includes = [token[2:] for token in env["wx_flags"] if token.startswith("-I")]
    includes += [str(path / "include") for path in prefixes.values()]
    includes += [str(prefixes["libraw"] / "include/libraw"),
                 str(REPO / "Mac_Libraries/EDSDK_v131231_Macintosh/EDSDK/Header"),
                 str(REPO / "Mac_Libraries/FreeImage_3_18/Dist"), str(REPO / "nebulosity4/cameras")]
    flags = env["wx_flags"] + ["-DNO_GCC_PRAGMA", "-DNEB2", "-DNEB2IL", "-DN64MAC", "-DNEB_ARM64_PREVIEW=1"]
    libs = env["wx_libs"] + ["-lfreeimage", "-lcfitsio", "-lraw", "-lusb-1.0", "-llcms2", "-lgphoto2", "-lgphoto2_port",
          "-framework", "ImageCaptureCore", "-framework", "WebKit", "-framework", "Accelerate", "-lz", "-lpthread", "-liconv"]
    return ["xcodebuild", "-project", str(REPO / "nebulosity4/OpenNebulosity4_64.xcodeproj"),
            "-target", "Nebulosity 4 ARM64", "-configuration", configuration, "-arch", "arm64",
            "CODE_SIGNING_ALLOWED=NO", "CLANG_CXX_LANGUAGE_STANDARD=c++14",
            "SYMROOT=" + str(build_dir / "products"), "OBJROOT=" + str(build_dir / "intermediates"),
            "CLANG_MODULE_CACHE_PATH=" + str(build_dir / "ModuleCache"),
            "HEADER_SEARCH_PATHS=" + quoted(includes),
            "LIBRARY_SEARCH_PATHS=" + quoted(["$(TARGET_TEMP_DIR)/freeimage", *[str(path / "lib") for path in prefixes.values()]]),
            "OTHER_CFLAGS=" + quoted(flags), "OTHER_CPLUSPLUSFLAGS=$(OTHER_CFLAGS)",
            "OTHER_LDFLAGS=" + quoted(libs), "build"]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="check prerequisites without building")
    parser.add_argument("--configuration", choices=("Development", "Deployment", "Default"), default="Development")
    parser.add_argument("--build-dir", type=Path, default=REPO / "build/arm64")
    args = parser.parse_args()
    env = environment()
    report = {key: env[key] for key in ("wx_version", "sdk_version", "xcode", "versions")}
    print(json.dumps(report, indent=2), flush=True)
    if args.check:
        return 0
    build_dir = args.build_dir.resolve()
    build_dir.mkdir(parents=True, exist_ok=True)
    (build_dir / "environment.json").write_text(json.dumps(report, indent=2) + "\n")
    log_path = build_dir / "build.log"
    print("Building ARM64; log: " + str(log_path), flush=True)
    with log_path.open("w") as log:
        result = subprocess.run(build_arguments(env, build_dir, args.configuration), cwd=REPO, stdout=log, stderr=subprocess.STDOUT)
    if result.returncode:
        for line in log_path.read_text(errors="replace").splitlines():
            if "error:" in line or "BUILD FAILED" in line:
                print(line, file=sys.stderr)
        return result.returncode
    app = build_dir / "products" / args.configuration / "Nebulosity 4 ARM64.app"
    print("Build succeeded: " + str(app))
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (RuntimeError, OSError, subprocess.CalledProcessError) as error:
        print("ARM64 build setup failed: " + str(error), file=sys.stderr)
        sys.exit(1)
