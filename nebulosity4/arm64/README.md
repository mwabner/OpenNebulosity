# Apple Silicon development

The active branch is `feature/macos-arm64`. Start with the [repository README](../../README.md) and the [development guide](../../docs/DEVELOPMENT.md).

## Build and test

```sh
brew bundle --file nebulosity4/arm64/Brewfile
python3 nebulosity4/arm64/build_arm64.py --check
python3 nebulosity4/arm64/build_arm64.py
python3 nebulosity4/arm64/test_arm64.py
```

Run these commands at the repository root on a native Apple Silicon Mac with full Xcode and a macOS 27 SDK or newer. The current deployment target is macOS 27.0. The build reads installed dependency paths through Homebrew and wx-config, builds the ARM64 target without signing, and records its environment under the ignored `build/arm64` directory. The tested package versions are recorded in [dependencies-tested.json](dependencies-tested.json); the Brewfile is not a permanent version lock.

FreeImage is compiled from the tracked bundled sources inside Xcode's temporary build directory. Its old RAW plugin and LibRawLite sources are excluded, preventing collisions with the current LibRaw. Do not reintroduce those modules into the same executable.

## Camera implementation

See [CAMERA_SUPPORT.md](CAMERA_SUPPORT.md) for exact hardware evidence and remaining work. The experimental dialog is under **Help → Sony / Nikon / Canon camera test**. Sony Alpha 6000 JPEG/ARW acquisition has been verified through gphoto2; the new session/UI capture path still requires its final physical-camera test. ARW import and FITS export through the application matched all 24,240,576 sensor pixels. Nikon/Canon and the complete model catalogue are not hardware validated.

`LegacyCameras_arm64.cpp` preserves legacy metadata/reconstruction needed by shared code and fails unsupported SDK operations explicitly. `DSUSB_arm64.cpp` is a stub. The ordinary camera selector currently offers no camera and the simulator; the new adapter is not yet integrated into series acquisition or Bulb exposure.

Keep generated libraries, local Xcode user settings, logs and private test images outside Git. Historical Intel targets and libraries remain for reference and were not retested during this ARM64 work.
