# HexaCube — Claude Context

## What this project is

A Windows-first VST3 drum sampler plugin built with JUCE/CMake (C++17). Reproduces a 16-voice vintage analog drum machine library. Full spec is in DEVELOPER.md — read it before starting any implementation work.

---

## Sample library

All 384 samples are in `Samples/`. Naming convention: `snd<aa>-vel<bb>-rr<cc>.wav` (instrument 01–16, velocity layer 01–06, round-robin 01–04). They are embedded into the plugin binary at compile time via JUCE's `juce_add_binary_data` — they are never read from disk at runtime.

---

## Hard constraints

**Cross-platform from day one.** The first release target is Windows, but macOS and Linux follow. No Windows-only APIs, file path separators, or `#ifdef _WIN32` blocks that would block later builds. Write portable C++ throughout.

**`HexaCube_Config` is dev-only.** It is a separate CMake target used only to empirically determine choke pairings, instrument names, and display order. None of its UI code or config-phase logic belongs in the release targets (`HexaCube` / `HexaCube_Standalone`).

**No runtime disk I/O.** Samples are loaded into `AudioBuffer<float>` at instantiation. The disk is never touched after that.

**No Simmons IP.** Do not use the Simmons brand name, the SDS-8 logo, or any other Simmons-owned visual assets anywhere in the codebase, UI, or metadata.

---

## Iteration boundaries

There are three GUI iterations. Do not add features or UI elements that belong to a later iteration, and do not carry iteration-1 UI into the release builds.

| | Iteration 1 (Config) | Iteration 2 (Release) | Iteration 3 (Polish) |
|---|---|---|---|
| Build target | `HexaCube_Config` | `HexaCube` | `HexaCube` |
| Choke dropdowns | Yes — for discovery | Removed | Removed |
| Instrument name input | Yes | Removed | Removed |
| Display order inc/dec | Yes | Removed | Removed |
| Trigger button | Yes (vel 100) | Yes (vel 100) | Yes (vel 100) |
| MIDI note dropdown | Yes | Yes | Yes |
| Volume / pan / mute | Yes | Yes | Yes |
| Custom artwork | No | No | Yes |

**What gets hardcoded into iterations 2 & 3 from iteration 1's output:**
- Cross-choke pairings (instrument → instrument index, not MIDI note)
- Final instrument names
- Final instrument display order

These are compile-time constants in the release build — not persisted state, not runtime config.

---

## State persistence (release builds only)

Saved with the host project:
- Per-instrument MIDI note assignment
- Per-instrument volume, pan, mute
- Output routing mode (stereo bus vs. 16 stereo outs)

Never persisted (frozen at compile time):
- Instrument names
- Instrument display order
- Cross-choke pairings

---

## Open questions — do not invent answers

These are unresolved and must be confirmed before implementing the relevant code:

- **Default MIDI note map** — TBD once instrument identities are confirmed in iteration 1. Do not pick arbitrary defaults.
- **Final instrument names, display order, cross-choke pairings** — output of iteration 1. Do not hardcode placeholders that might ship.
- **HexaCube logo and colour palette** — needed for iteration 3 only.

---

## Plugin metadata

| Field | Value |
|---|---|
| Display name | HexaCube |
| Vendor | voXager |
| Manufacturer code | vXgr |
| Format | VST3 |
