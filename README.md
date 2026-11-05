# Lunchbox303

Independent **TB-303–style** bass synthesizer plugin (VST3 / AU / Standalone), inspired by the Reaktor ensemble **LUNCHBOX-303 v7.2** by **Eige Kitamura** (2002–2005).

This is a **from-scratch reimplementation**. It is **not** Native Instruments or Roland code, and it does **not** contain the original Reaktor ensemble binary or DSP graph. Factory preset *names* (`init`, `me acid 1`…`9`) match the ensemble; numeric preset values are new defaults (the `.ens` snapshot values could not be decoded).

See [ANALYSIS.md](ANALYSIS.md) for what was extracted from the `.ens` vs. what was assumed.

## Features

- SAW / PULSE oscillator (band-limited)
- 4-pole resonant lowpass (diode-ladder–style)
- ENV MOD, DECAY, ACCENT (with transient click), TUNING (±4 semitones), CUT OFF, RESO, VOLUME, BPM
- Internal 16-step sequencer (note / gate / accent / slide), optional host tempo sync
- MIDI note input
- Factory programs: `init`, `me acid 1`–`me acid 9`

## macOS install (primary)

CI builds a **universal** binary (`arm64` + `x86_64`) as **VST3** and **AU**.

1. Download the `Lunchbox303-macOS-universal` artifact from GitHub Actions (or build locally).
2. Copy the plugins:

```bash
# VST3
cp -R Lunchbox303.vst3 ~/Library/Audio/Plug-Ins/VST3/

# Audio Unit
cp -R Lunchbox303.component ~/Library/Audio/Plug-Ins/Components/
```

3. Clear quarantine / ad-hoc unsigned warnings (common for CI or local builds):

```bash
xattr -cr ~/Library/Audio/Plug-Ins/VST3/Lunchbox303.vst3
xattr -cr ~/Library/Audio/Plug-Ins/Components/Lunchbox303.component
```

4. Rescan plugins in your DAW (Logic: restart / Plug-in Manager; Ableton, Bitwig, etc.: rescan).

### Build on macOS

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13
cmake --build build --config Release --parallel
```

Artifacts appear under `build/Lunchbox303_artefacts/Release/` (VST3, AU, Standalone).

## Linux / Windows (optional)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

Linux needs typical JUCE deps (`libasound2-dev`, `libfreetype6-dev`, X11/GL, etc.). Windows produces VST3 via the optional Actions job.

## Headless smoke test

```bash
cmake --build build --target lunchbox303_render
./build/lunchbox303_render sample_pattern.wav
```

## Credits & legal

- Original Reaktor ensemble concept/UI labels: **Eige Kitamura** — *LUNCHBOX-303 v7.2*
- This plugin: independent clean-room–style model based on public TB-303 behaviour and decoded ensemble *strings/labels*, not a port of NI Reaktor or Roland firmware
- Built with [JUCE](https://juce.com/)



## Package contents

- `Source/` — JUCE plugin + headless renderer
- `CMakeLists.txt` — fetches JUCE 8; on Apple enables **AU + VST3 + Standalone** and defaults to **universal** `arm64;x86_64`
- `.github/workflows/build.yml` — **macOS universal** job first (VST3+AU artifacts); Linux smoke; optional Windows VST3
- `ANALYSIS.md` — Reaktor `.ens` string/label findings
- `dist/linux/` — Linux VST3 + Standalone + `sample_pattern.wav` from this build host (macOS plugins come from Actions)
