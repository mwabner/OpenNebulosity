# ARM64 validation — 2026-11-12

## Environment and reproducibility

- Apple Silicon M1; native ARM64 process environment.
- macOS 27.0.1, Xcode 27.2 (27B52), macOS SDK 27.2.
- Dependency versions: [tested environment snapshot](../nebulosity4/arm64/dependencies-tested.json).
- Application deployment target: macOS 27.0; unsigned developer build.

The complete proposed Git index was exported to a separate source directory whose path contained spaces. This export had no `.git` directory, personal Xcode settings, private test pictures or inherited build products. The normal `build_arm64.py` command completed a fresh **Development** build from those files, including the isolated FreeImage source build. This demonstrates that the local working copy's untracked files/build residue are not required. It was tested on the same physical Mac; a second physical developer machine has not yet been tested.

The new setup check validated the selected SDK, dependency presence and ARM64 library architecture. Build flags were derived from the installed wx-config. Xcode user-script protection stayed enabled. Only ARM64 project settings were adjusted during publication preparation; Intel project objects were compared with the saved starting state and remained unchanged.

## Regression results

The checked-in `test_arm64.py` command completed successfully against the fresh build:

| Check | Result |
|---|---|
| Eight legacy language choices migrated to current wxWidgets IDs | PASS |
| Stable language-code round-trip and preservation of the Intel preference key | PASS |
| English locale and bundled German translation | PASS |
| Capture without connection, failed connection cleanup and cancellation error paths | PASS |
| Optional Sony ARW → application FITS sensor-pixel comparison | PASS — 24,240,576 samples, zero direct-order differences |
| Staged whitespace/error check | PASS |

The optional RAW comparison used the locally retained Alpha 6000 test exposure and its application FITS export. Those private images are not included in Git. Tests compile their executables and configuration files under the ignored build directory and do not require an attached camera for the standard checks.

## Hardware evidence and limits

Earlier in this session the Sony ILCE-6000 was detected in PC Remote mode. gphoto2 successfully captured and downloaded a 6000 × 4000 JPEG and an ARW. RAW was temporarily selected for one test; the original JPEG Fine setting was restored and read back. The application imported that ARW and exported a pixel-exact FITS file after removal of the conflicting legacy LibRaw copy from FreeImage.

This is not a successful end-to-end hardware test of the newly written camera-dialog capture button. That positive capture path, cancellation during a physical exposure, cable loss, Bulb/series integration and Nikon/Canon devices remain open work. The default Development build is the validation baseline; optimized configurations and legacy Intel/Windows builds have not been tested here. The application still uses local Homebrew runtime libraries and includes some unvalidated legacy helper programs.
