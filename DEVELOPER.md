# HexaCube Project Description

HexaCube is a Windows VST3 sample-playback plugin built around a 16-instrument library captured from a vintage analog drum machine. Each instrument is sampled at 6 velocity layers with 4 round-robin variations, giving 384 samples (24-bit / 44.1 kHz, ~46 MB total) embedded directly into the plugin binary. The library combines five factory voices, five custom voices, and six FX voices (hi-hat, cymbal, etc.). HexaCube is built with JUCE for future cross-platform expansion — Windows first, macOS and Linux planned — and supports per-instrument volume, pan and mute, configurable choke groups, MIDI note remapping, and dual-mode output routing (single stereo bus or 16 stereo outs for per-instrument processing inside the host DAW).

# HexaCube — Developer Documentation

> **Status:** Specification phase. No code written yet. This document captures the agreed design so implementation can start from a single source of truth.

---

## 1. Overview

HexaCube is a standalone VST3 sample-playback plugin that reproduces a 16-voice electronic drum library. The library is composed of five factory voices, five custom voices, and six FX voices (hi-hat, cymbal, etc.) sampled from a vintage analog drum machine.

Each voice was captured at 6 velocity layers with 4 round-robin variations per layer, producing 384 samples in total (~46 MB uncompressed PCM). All samples are embedded directly in the plugin binary — no external sample folder, no disk I/O at runtime.

---

## 2. Technology Stack

| Area | Choice |
|------|--------|
| Framework | JUCE |
| Build system | CMake (JUCE's recommended modern path) |
| Language | C++ (C++17 or later, whatever JUCE's current minimum is) |
| Plugin format | VST3 |
| Primary target OS | Windows |
| Future targets | macOS (Intel + Apple Silicon universal), Linux |

Code must be written cross-platform from day one — no Windows-only APIs, file paths, or conditional compilation that would block later macOS/Linux builds.

---

## 3. Sample Library

### 3.1 File Specification

- Format: PCM S24 LE (signed 24-bit little-endian)
- Sample rate: 44.1 kHz
- Bit depth: 24 bit
- Total count: 384 files
- Total size: ~46 MB uncompressed

### 3.2 Filename Convention

```
snd<aa>-vel<bb>-<cc>.wav
```

| Field | Range | Meaning |
|-------|-------|---------|
| `aa`  | 01–16 | Instrument number |
| `bb`  | 01–06 | Velocity layer (01 = softest, 06 = loudest) |
| `cc`  | 01–04 | Round-robin index |

### 3.3 Embedding Strategy

Samples are embedded as binary resources via JUCE's `juce_add_binary_data` (CMake API). JUCE splits the data into chunks automatically, so the 46 MB total is not a problem — the resulting plugin binary lands in the ~50–60 MB range.

At plugin instantiation, samples are decoded once into RAM-resident `AudioBuffer<float>` objects. All triggering reads from those in-memory buffers; the disk is never touched after load.

---

## 4. Plugin Architecture

### 4.1 Polyphony

- 32 voices, global pool.
- Voice stealing: oldest **of the same instrument**. Avoids a long FX tail being cut by an unrelated kick.
- (Not expected to matter much in practice — drum samples are short — but the rule is in place.)

### 4.2 Velocity-Layer Mapping

Six velocity layers are mapped to incoming MIDI velocity (1–127) using equal-width thresholds:

| Layer | MIDI velocity range |
|-------|---------------------|
| 01    | 1 – 21              |
| 02    | 22 – 42             |
| 03    | 43 – 63             |
| 04    | 64 – 84             |
| 05    | 85 – 105            |
| 06    | 106 – 127           |

To smooth transitions between layers, **amplitude scaling within each layer** is applied based on where the incoming velocity sits inside its band. This avoids audible hard steps at the layer boundaries while keeping the timbral character of each captured layer intact.

### 4.3 Round Robin

- Cyclic, shared counter across all velocity layers of a given instrument.
- No anti-repetition logic (keeps code simple).
- Order: hit *N* plays RR `((N-1) mod 4) + 1` — i.e. first trigger plays RR1, then RR2, RR3, RR4, RR1, …
- Counter is per-instrument, not global.

### 4.4 Choke Groups

- Every instrument chokes itself implicitly (re-triggering an instrument cuts its own previous voices).
- Cross-instrument choke pairings are determined empirically during iteration 1 of the GUI (see §7.1) and then **hardcoded** into iterations 2 and 3.
- Choke action is a **hard cut** — no release envelope.
- Cross-choke pairings follow the **instrument**, not the MIDI note. If the user reassigns an instrument to a different MIDI note, its choke relationships move with it automatically.

### 4.5 Per-Instrument Parameters

For each of the 16 instruments:

- Volume
- Pan
- Mute

These are exposed as **host-automatable parameters** (JUCE makes this essentially free).

---

## 5. Instrument Catalogue

The 16 instruments break down as:

| Group         | Count | Notes |
|---------------|-------|-------|
| Factory       | 5     | The five canonical SDS-8 voices (kick, snare, toms, etc.) |
| Custom        | 5     | Custom-tuned variations |
| FX            | 6     | Hi-hat, cymbal, etc. |

Final instrument identities, names and ordering are determined in GUI iteration 1 and then frozen for iterations 2 and 3.

---

## 6. Output Routing

The plugin supports two output modes:

1. **Single stereo bus** (default on insertion). All 16 instruments mix to one stereo pair.
2. **Multi-out: 16 stereo outputs.** One stereo pair per instrument, allowing the user to apply per-instrument effects (reverb, EQ, compression) inside the host DAW.

Stereo (rather than mono) outputs are used in multi-out mode so that the per-instrument **pan** parameter behaves identically in both modes.

---

## 7. GUI — Three Iterations

### 7.1 Iteration 1 — Configuration Build (separate build target)

This iteration exists **only to harvest configuration data** that gets compiled into iterations 2 and 3. It will be a **separate CMake build target** (e.g. `HexaCube_Config`) so that no iteration-1 code or UI ever ships in the release plugin.

**Per instrument (16 panels):**

- Trigger button (clicking sends a fixed velocity of 100)
- MIDI note assignment dropdown
- Choke note dropdown (self-choke is implicit, so this is for cross-choke discovery)
- Free-text instrument name field
- Inc/dec input for the instrument's display order in iterations 2 and 3

**Output of iteration 1** (to be captured manually and hardcoded into iterations 2/3):

- Cross-choke pairings (instrument → instrument, expressed in instrument indices, not MIDI notes)
- Final instrument names
- Final instrument display order

### 7.2 Iteration 2 — Functional Release

Hardcoded from iteration 1's findings:

- Cross-choke pairings (follow the instrument, not the MIDI note)
- Instrument display order
- Instrument names

Removed from the UI:

- Choke note dropdown
- Instrument name input field
- Instrument ordering inc/dec

Retained:

- Trigger button per instrument (still fixed velocity 100)
- MIDI note assignment dropdown per instrument
- Per-instrument volume / pan / mute controls

A sensible **General-MIDI-based default note map** is provided so that most users will not need to touch the assignments. (Concrete defaults TBD — see §10.)

### 7.3 Iteration 3 — Visual Polish

- Functionally identical to iteration 2.
- Custom artwork: HexaCube logo (no Simmons IP — see §9), hexagonal motifs, approximation of the original SDS-8 colour scheme.

---

## 8. State Persistence

### 8.1 Iteration 1 (Config build)

Persisted to disk only for the developer's benefit during the config phase — not relevant to end users. The data harvested here gets transcribed into source code and compiled into iterations 2/3.

### 8.2 Iterations 2 & 3 (Release builds)

Saved with the host project so the user reopens the plugin in the same state they left it:

- Per-instrument MIDI note assignment
- Per-instrument volume, pan, mute
- Output routing mode (stereo bus vs. 16-stereo-outs)

Frozen at compile time, **not** persisted:

- Instrument names
- Instrument display order
- Cross-choke pairings

---

## 9. Plugin Metadata & Branding

| Field | Value |
|-------|-------|
| Plugin display name | HexaCube |
| Vendor / manufacturer | voXager |
| Manufacturer code (4 chars) | vXgr |
| Plugin format | VST3 |

**IP note.** The Simmons brand and the SDS-8 logo are trademarked. HexaCube does **not** use the Simmons name, the SDS-8 logo, or any other Simmons-owned visual assets. The plugin's own visual identity is built around:

- The name **HexaCube**
- Original hexagonal shape motifs
- A colour scheme that *approximates* the SDS-8 palette without copying any specific protected design

---

## 10. Open Questions / TODO

Items that need to be resolved during implementation but don't block starting work:

- [ ] Concrete General-MIDI-based default note map for the 16 instruments (decided once instrument identities are confirmed in iteration 1)
- [ ] Final instrument names, ordering, and cross-choke pairings (output of iteration 1)
- [ ] HexaCube logo design and final colour palette for iteration 3
- [ ] CMake project skeleton with both `HexaCube` and `HexaCube_Config` targets sharing the sample-loading and voice-engine code
