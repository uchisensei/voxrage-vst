# VØX//RAGE — Aggressive Vocal Mix VST3

A macOS-first VST3 vocal channel strip inspired by the *workflow* of modern all-in-one vocal processors: fast controls, aggressive presets, saturation, de-essing, compression, tone shaping and limiting.

> This is an original implementation and UI. It does not copy any third-party plugin's code, assets, branding, or GUI.

## Features

- VST3 + AU + Standalone targets
- Apple Silicon + Intel macOS (`arm64;x86_64`)
- CMake + JUCE 9.0.3 fetched automatically
- Custom dark/purple GUI
- Real-time input/output meters
- Vocal analyzer-style animated display
- HPF
- 3-band musical EQ
- Dynamic de-esser
- Compressor
- Aggressive saturation / drive
- Parallel punch
- Width
- Air
- Output limiter
- Dry/Wet mix
- Factory presets:
  - TRAP BITE
  - RAP FORWARD
  - RADIO AGGRO
  - HYPER POP
  - DARK VOCAL
  - CLEAN PUNCH

## Build on macOS

Install Xcode Command Line Tools and CMake 3.22+.

```bash
xcode-select --install
brew install cmake ninja
```

Then:

```bash
git clone https://github.com/YOUR_USERNAME/VoxRageVST3.git
cd VoxRageVST3
cmake --preset macos
cmake --build --preset macos-release
```

The VST3 bundle is produced under:

```text
build/VoxRage_artefacts/Release/VST3/
```

For a local Xcode project:

```bash
cmake --preset macos-xcode
cmake --build --preset macos-xcode-release
```

## Install locally

Copy `VoxRage.vst3` to:

```text
~/Library/Audio/Plug-Ins/VST3/
```

For AU, copy `VoxRage.component` to:

```text
~/Library/Audio/Plug-Ins/Components/
```

Then rescan plugins in your DAW.

## GitHub Actions

The included workflow builds the VST3/AU/Standalone targets on macOS for Apple Silicon and Intel.

## Preset philosophy

The factory presets deliberately push the vocal chain harder than a transparent channel strip. They are intended as starting points for rap, trap, hyper-pop and modern pop vocals.

## License

The application code in this repository is provided under the MIT license.

JUCE itself is separately licensed by the JUCE project. This project fetches JUCE from its official repository at build time. Check the JUCE licensing terms before distributing a closed-source commercial binary.
