# Continuing ARM64 development

Work on `feature/macos-arm64`. The current default branch `main` preserves the historical baseline. This branch's README and the repository About text describe the ARM64 effort. Keep shared application code and legacy targets intact until backend separation is complete.

## Architecture direction

Prioritize native ARM64 development and tests. Separate camera backends from the image-processing core, then retire unnecessary legacy dependencies from the active branch. Preserve the last documented Intel/Windows state in a legacy branch or tag before removal, and verify that clean ARM64 builds and regression tests are independent of the files being retired. Until that migration is explicitly carried out, keep legacy targets and shared sources intact. See the repository README's long-term development goal.

## A complete checkout

Use a normal clone of the development branch. The tracked `Mac_Libraries` and `nebulosity4/cameras` inputs are required even though their Intel binaries are not linked into the new capture adapter. Do not copy a build folder or local Xcode user settings as a substitute for source. Private test photographs are intentionally absent.

Install the recipes in `nebulosity4/arm64/Brewfile` using native Apple Silicon Homebrew. Select full Xcode with a macOS 27 SDK or newer. Python 3.9+ is sufficient for the scripts; Python 3.14.7 is the recorded Homebrew version. The present app targets macOS 27.0; reducing this requires rebuilding dependencies with a suitable deployment target and testing on the older system.

```sh
python3 nebulosity4/arm64/build_arm64.py --check
python3 nebulosity4/arm64/build_arm64.py
python3 nebulosity4/arm64/test_arm64.py
```

The default outputs are `build/arm64/products/Development`, `build/arm64/build.log` and `build/arm64/environment.json`. All are ignored by Git. `--build-dir /absolute/path` selects a separate build directory. `--configuration Deployment` selects the optimized configuration, but the Development configuration is the currently documented validation baseline. Pass the same options to the tests. `NEB_WX_CONFIG` can select a different native wx-config executable.

The helper derives wxWidgets headers, library names and ABI defines from the installed wx-config instead of copying a previous machine's paths. It verifies dependency architecture, disables signing for local development and sets C++14 explicitly. The project can also be opened in Xcode; its checked-in target defaults refer to the tested Homebrew wxWidgets 3.3 layout. Prefer the helper after changing dependency versions or install prefixes.

A Brewfile does not freeze package versions forever. `dependencies-tested.json` records the exact tested versions; compare it with the build's generated environment report before investigating a new compiler failure. Source-level dependency upgrades may still require code changes.

## Tests

The standard test command builds and runs language migration and camera-session error tests. It needs the built app's translation resources, but no attached camera. The test configuration file and executables are created under the ignored build directory, without changing user preferences.

Optional sensor-pixel comparison:

```sh
python3 nebulosity4/arm64/test_arm64.py \
  --raw /path/outside/repository/camera.ARW \
  --fits /path/outside/repository/exported.fit
```

Open the ARW in the built application and save it as FITS without image processing or 15-bit output scaling before comparing. The regression uses a Bayer RAW image, compares decoded sensor samples, and accepts direct order or an exact vertical reversal. It does not validate all camera metadata, non-Bayer sensors or lossy/processed output settings. Do not add private RAW/JPEG/FITS files to the repository.

## Camera adapter

Open **Help → Sony / Nikon / Canon camera test**. Select PC Remote on a supported camera, refresh discovery, connect and select a folder for a test image. Exposure and file format remain camera-controlled. The downloaded image opens in the application when the dialog closes.

`MacCameraDiscovery.mm` owns the user interface and Apple discovery. `GPhotoDiscovery.cpp` enumerates gPhoto USB candidates without opening a capture session. `GPhotoCapture.cpp` owns a persistent session and asynchronous test acquisition. It rejects unbounded Bulb/Time exposure, supports cancellation requests and creates a unique output file rather than overwriting one. The adapter does not delete camera files or silently change exposure/ISO/quality.

If macOS's image importer holds USB access, a dialog asks before releasing that user's importer process. Active imports may be interrupted. The program does not disable services permanently. Positive capture, cancellation and cable-loss testing of the newly written session/UI are still required; the successful physical Alpha 6000 captures were made through the gphoto2 diagnostic program.

ImageCaptureCore and gPhoto may list the same physical camera as two connection routes. The displayed count refers to routes, not unique physical devices. Static driver capabilities are not an end-to-end compatibility guarantee.

## RAW and legacy boundaries

`build_freeimage.py` builds a private FreeImage archive from tracked sources inside the target's temporary directory. Its RAW plugin and bundled LibRawLite are intentionally removed: including them together with current LibRaw produced symbol collisions and incorrect Bayer pixels. The Alpha 6000 ARW/FITS comparison after this fix matched all 24,240,576 samples.

`LegacyCameras_arm64.cpp` preserves metadata and reconstruction routines needed by shared code while unavailable SDK operations fail explicitly. DSUSB is a stub. The original vendor implementations remain associated with the legacy target. The source remains mixed-platform C++; removing an old target or directory does not automatically make shared code portable.

Keep signing/notarization and redistributable dependency packaging separate from the local developer build. Some bundled external tools remain Intel-only and unvalidated; an ARM64 main executable does not imply that every bundled helper is native.

## Publishing changes

Review `git status` and the staged diff. Commit source, the shared Xcode project, scripts, dependency recipes, documentation and tests. Exclude build products, personal `xcuserdata`, local paths, logs and private photographs. Push the development branch normally; avoid rewriting existing commits or the historical main branch. Keep remaining hardware validation visible in documentation rather than marking every listed camera as supported.
