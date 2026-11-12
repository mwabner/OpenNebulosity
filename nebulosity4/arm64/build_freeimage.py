#!/usr/bin/env python3
"""Build the bundled FreeImage sources for ARM64 without touching Intel artifacts."""
from pathlib import Path
import shutil
import subprocess
import sys

here = Path(__file__).resolve().parent
source = here.parent.parent / "Mac_Libraries" / "FreeImage_3_18"
build = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else here / "build" / "freeimage"
shutil.copytree(source / "Source", build / "Source", dirs_exist_ok=True,
                ignore=shutil.ignore_patterns("*.o", "*.a", "*.dylib"))
shutil.copy2(source / "Makefile.srcs", build / "Makefile.srcs")

# The bundled PNG/zlib predate modern macOS SDKs.
patches = [
    ("Source/FreeImage/Plugin.cpp", b"s_plugins->AddNode(InitRAW);",
     b"/* RAW decoding is provided by the application LibRaw backend. */"),
    ("Source/LibPNG/pngpriv.h", b"defined(TARGET_OS_MAC)",
     b"(defined(TARGET_OS_MAC) && !defined(__APPLE__))"),
    ("Source/ZLib/zutil.h", b"#        define fdopen(fd,mode) NULL /* No fdopen() */",
     b"/* Modern macOS provides fdopen. */"),
]
for name, old, new in patches:
    path = build / name
    data = path.read_bytes()
    if old not in data:
        raise RuntimeError("Unexpected bundled source: " + name)
    path.write_bytes(data.replace(old, new))

makefile = 'include Makefile.srcs\n# RAW decoding uses the current system LibRaw, never the bundled legacy copy.\nSRCS := $(filter-out ./Source/FreeImage/PluginRAW.cpp Source/LibRawLite/% ./Source/LibRawLite/%,$(SRCS))\nMODULES = $(SRCS:.c=.o)\nMODULES := $(MODULES:.cpp=.o)\nCC = clang\nCXX = clang++\nCFLAGS = -DPNG_ARM_NEON_OPT=0 -O2 -arch arm64 -mmacosx-version-min=14.0 -fPIC -fexceptions -DOPJ_STATIC -DNO_LCMS -DDISABLE_PERF_MEASUREMENT -D__ANSI__ -Wno-error=implicit-function-declaration $(INCLUDE)\nCXXFLAGS = -O2 -arch arm64 -mmacosx-version-min=14.0 -std=c++11 -fPIC -fexceptions -D__ANSI__ $(INCLUDE)\nall: libfreeimage.a\nlibfreeimage.a: $(MODULES)\n\trm -f $@\n\tar rcs $@ $(MODULES)\n.c.o:\n\t$(CC) $(CFLAGS) -c $< -o $@\n.cpp.o:\n\t$(CXX) $(CXXFLAGS) -c $< -o $@\n'
path = build / "Makefile.arm64"
if not path.exists() or path.read_text() != makefile:
    # Compiler flags changed: discard only this isolated ARM64 build's objects.
    for obj in (build / "Source").rglob("*.o"):
        obj.unlink()
    archive = build / "libfreeimage.a"
    if archive.exists():
        archive.unlink()
    path.write_text(makefile)
subprocess.run(["make", "-f", "Makefile.arm64", "-j4"], cwd=build, check=True)
