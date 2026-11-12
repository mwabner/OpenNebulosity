# OpenNebulosity 4 — Apple Silicon development

Development fork of [Craig Stark's OpenNebulosity](https://github.com/celstark/OpenNebulosity), an astrophotography capture and image-processing application originally developed at Stark Labs.

**Active development branch: [`feature/macos-arm64`](https://github.com/mwabner/OpenNebulosity/tree/feature/macos-arm64).** This is an experimental native macOS ARM64 port, not a production release or a claim of complete camera compatibility. The historical Intel and Windows sources and targets remain available; they are not covered by the current ARM64 validation.

## Long-term development goal

Apple Silicon ARM64 is the primary development and validation target. The goal is a maintainable native macOS application with a shared image-processing core and clearly separated camera backends for Sony, Nikon and Canon, with compatibility verified per model and function.

Intel and Windows code remain as legacy references while shared code and camera dependencies are being separated. Before removing legacy targets or libraries from the active development branch, preserve their last documented state in a dedicated legacy branch or tag and confirm that the ARM64 build no longer needs them. The intended end state is a lean ARM64-focused development branch, with historical platform support retained separately. Legacy removal is a later, reviewed migration step, not a deletion planned for the current preview.

## Current status

| Area | Status |
|---|---|
| Native ARM64 application | Development build and application launch verified on an M1 Mac |
| FITS processing | Open, rotate and save tested; synthetic FITS round-trip checked |
| Bitmap export | BMP and JPEG export tested |
| Simulator | Connection and 800 × 600 preview acquisition tested |
| Sony Alpha 6000 | USB recognition, JPEG and ARW capture/download verified using gphoto2 |
| Sony ARW → FITS | App import/export verified against all 24,240,576 decoded sensor pixels: zero differences |
| New USB camera dialog | Discovery, persistent-session capture/download code and error tests implemented; positive end-to-end capture from its new button still needs a hardware test |
| Nikon and Canon | Shared libgphoto2 adapter prepared; no physical camera validation yet |
| Bulb, capture series and focus | Not yet connected to the new camera adapter; Alpha 6000 live view is not advertised by its installed driver |
| Legacy camera SDKs | Intel-only implementations are replaced or disabled in the ARM64 preview |

The camera test is available under **Help → Sony / Nikon / Canon camera test**. It uses the exposure and file format selected on the camera. The normal camera selector still offers the simulator; it does not yet drive this experimental adapter.

## Build on another Apple Silicon Mac

The currently tested configuration uses **macOS 27.0.1, Xcode 27.2 and native ARM64 Homebrew**. The application's current deployment target is macOS 27.0. Full Xcode with a macOS 27 SDK or newer is required; Command Line Tools alone are insufficient. Older macOS deployment and standalone distribution are separate work items.

```sh
git clone --branch feature/macos-arm64 https://github.com/mwabner/OpenNebulosity.git
cd OpenNebulosity
brew bundle --file nebulosity4/arm64/Brewfile
python3 nebulosity4/arm64/build_arm64.py --check
python3 nebulosity4/arm64/build_arm64.py
python3 nebulosity4/arm64/test_arm64.py
open "build/arm64/products/Development/Nebulosity 4 ARM64.app"
```

Run from a native ARM64 terminal, not Rosetta. Select full Xcode in Xcode's settings or with `xcode-select` before building. The build script reads the installed `wx-config`, derives headers/linker flags and validates the native dependency libraries. It builds only `Nebulosity 4 ARM64`, without requiring a signing certificate, and leaves the Intel target untouched. Xcode's user-script protection remains enabled.

FreeImage is built automatically from the bundled source into Xcode's temporary build directory. Its old embedded LibRaw is excluded to avoid symbol collisions with the current RAW decoder. No separate proprietary camera SDK download is needed for the tested Alpha 6000 route; some legacy headers are still referenced from the bundled libraries.

See [the development guide](docs/DEVELOPMENT.md), [tested dependency versions](nebulosity4/arm64/dependencies-tested.json) and [camera validation details](nebulosity4/arm64/CAMERA_SUPPORT.md).

## Development components

| Component | Role |
|---|---|
| C++ and wxWidgets / Cocoa | Application, image processing and desktop interface |
| Objective-C++ / ImageCaptureCore | Native macOS camera discovery and reported PTP capabilities |
| libgphoto2 | Shared USB discovery and experimental Sony/Nikon/Canon capture sessions |
| LibRaw | Current DSLR/mirrorless RAW decoding, including tested Sony ARW |
| FreeImage 3.18 bundled sources | Bitmap/image codecs, compiled separately for ARM64 |
| CFITSIO | FITS reading and writing |
| Little CMS / libusb | Color-management and USB dependencies |
| Python 3 / Xcode / Clang | Build environment checks, isolated dependency build and regression-test commands |

The [driver capability snapshot](nebulosity4/arm64/driver-model-support.tsv) lists current and historical driver entries and USB modes. It is not a list of cameras proven to work in OpenNebulosity. Future models and firmware need per-function checks rather than assuming that detection guarantees capture, RAW transfer or live view.

## Next development work

1. Validate connection, capture, cancellation and disconnect from the new dialog with the Alpha 6000.
2. Integrate the adapter into the normal capture controls and series workflow.
3. Add bounded Bulb exposures and verify exposure/ISO metadata and RAW behavior.
4. Test representative Nikon and Canon models and cover differences in driver capabilities.
5. Separate legacy backends from the shared processing core before considering target removal.
6. Package ARM64 runtime dependencies and address signing/notarization for distribution.

Generated builds, personal Xcode state and private camera images do not belong in commits. Hardware-independent tests can run without a camera; the optional ARW/FITS regression uses locally supplied test files. Homebrew's Brewfile is a setup recipe, not a permanent version lock; the recorded versions document the verified environment.

## Legacy reference and attribution

This fork retains the upstream history and bundled library notices. [The original build-environment notes](docs/legacy-build-environments.md) describe the historical macOS Intel and Windows setups. Retaining those files does not mean that their old SDKs or toolchains work on current systems. Intel/Windows builds have not been retested during this port.
