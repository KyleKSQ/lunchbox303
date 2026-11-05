# LUNCHBOX-303 v7.2 (.ens) analysis

**Source file:** `01M6N2VEWVVR0FF183TT48YPSN.ens` (444 489 bytes)  
**Format:** Native Instruments Reaktor ensemble (binary). Header begins with `KEnsemble` / `Ensemble`.  
**Title (decoded):** `LUNCHBOX` / panel banner `L U N C H B O X - 303`  
**Version / author (decoded):** version **7.2**, by **Eige Kitamura**, **2002–2005**  
**Honesty note:** Reaktor `.ens` stores a compiled instrument graph (modules, wiring, tables, panel). Strings, port names, tooltips, and snapshot *names* are recoverable. The actual DSP graph, table contents, and numeric snapshot/preset *values* are not fully decoded here (proprietary binary; no public schema used). Gaps are filled with a conventional TB-303 model.

## Decoded — identity & I/O

| Item | Evidence |
|------|----------|
| Ensemble name | `LUNCHBOX`, banner text |
| Instrument / voice macros | `Bass Line`, subtitle `Transister Bass` (typo as in file); `Overdrive Unit for Bass Line`; `Lopass` / `Lopass Filter`; `#R66 macro taken from the SteamPipe` |
| External clock in | “Input for the external clock signal… 16th-rate half-width pulse”; port `16th-clock-event (0, 1)` |
| Start/stop in | “Input for start/stop events of the external clock gen.”; `start-stop-event (0, 1)` |
| Audio | `Audio input` / `Audio output` / `Filtered waveform output` / `Master Level` |
| Sync behaviour | “It requires a 1/16 sync clock signal.” “If switched off, LB303 refers to the system clock.” |

## Decoded — panel / parameter labels

| Label | Tooltip / note from file |
|-------|--------------------------|
| **SAW/PULSE** (WAVE) | “Wave Form Switch: sawtooth / pulse” |
| **TUNING** | “Tuning / Range: -4….+4 semitones” |
| **CUT OFF** | “Filter cutoff frequency” |
| **RESO** | “Filter resonance” |
| **ENV MOD** | “Amount of modulation of the filter cutoff frequency by the envelope” |
| **DECAY** | “Envelope decay time” |
| **ACCENT** | “Accent amount (Adds a click to the start of notes)” |
| **VOLUME** | “Master Level” |
| **BPM** | Present on panel (numeric range not recovered) |
| **EXTREME** | “Enables Filter Animation” (XY auto-filter style; not reimplemented as full XY anim) |
| **VCF EG**, **VCA**, **ACID** | Panel section markers / routing labels |
| Sequencer UI | MODE, WRITE, PLAY, TRACK, PATTERN, BANK, TIMING/TIME, ACC, SLIDE, LENGTH, GATE TIME / GATE U / GATE D, TRANSPOSE UP/DOWN, BACK/NEXT, SELECTOR, PITCH / PITCH MODE, ENT |

Tuning range **-4…+4 semitones** is explicitly in the ensemble text. Other knobs: labels and semantics decoded; **min/max/default floats not reliably recovered** (nearby binary floats look like panel layout coords, not audio params).

## Decoded — sequencer model (from embedded help text)

- **32 pattern memory locations** on buttons 1–8 × **4 banks** (A: 1–8, B: 9–16, C: 17–24, D: 25–32).
- Modes: `0 : pattern-play`, `1 : pattern-write` (plus track-write mode).
- Pattern write: enter **16 note pitches**, then per-step **gate/timing** length, **Accent**, **Slide (portamento)**.
- Gate timing uses table logic (`GATE TIME`, GATE U/D); gate-timing-cv described as `(-1, 0, 1)`.
- **7 tracks**, each up to **128 measures**; track built by chaining pattern numbers; BACK marks end (“D.C.” loop).
- Simple keyboard: **3 octave** range via TRANSPOSE; pitch entry vs pattern select via PITCH MODE.
- Accent / slide CV ports: `accent-cv (0, 1)`, `slide-cv (0, 1)`.

## Decoded — factory snapshot / preset **names**

From Bank 1 snapshot name table (names only; values not decoded):

1. `init`
2. `me acid 1` … `me acid 9`

No usable per-preset knob floats were extracted from the binary around those names (entries look like name + flags, not a clear float dump).

## Inferred / assumed for the VST reimplementation

Because the compiled DSP graph and snapshot values are opaque, the plugin uses a **faithful classic TB-303-style** voice driven by the decoded control set:

| Feature | Assumption |
|---------|------------|
| Oscillator | Band-limited saw or square (pulse), switch = SAW/PULSE |
| Filter | 4-pole resonant lowpass, diode-ladder–style (nonlinear cascade) |
| Env → filter | Decaying envelope × ENV MOD into cutoff |
| Amp | Simple decay/gate envelope (VCA) |
| Accent | Raises level, opens cutoff, shortens/brightens envelope; optional transient click |
| Slide | Portamento ~**60 ms** when slide flagged |
| Tuning | ±4 semitones (from ens text) |
| Cutoff / reso / env mod / decay / accent / volume | Normalized 0–1 (or musical mappings) — ranges chosen to feel 303-like |
| Sequencer | Internal **16-step** pattern: note (MIDI), octave shift, accent, slide, gate; tempo BPM; host transport sync |
| MIDI | Note on/off drives the same voice (overrides or parallels sequencer) |
| EXTREME / overdrive / full 32×7 track writer / XY filter anim | Simplified or omitted vs original Reaktor UI |
| Factory presets | Names match ens; parameter values are **new** artistic defaults, not recovered dumps |

## What was *not* recovered

- Exact filter topology coefficients / tables inside Reaktor macros  
- Exact envelope curves, accent click DSP, overdrive curve  
- Numeric values for init / me acid 1–9  
- Full track RAM / pattern table contents  
- Panel graphics (referenced TGA path on author’s Windows box)

