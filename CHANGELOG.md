# Changelog

All notable changes to the magnolia engine are documented here.

## v0.3.0 (unreleased)

16 commits after v0.2.0. The second-game release: George Boole and Lava Dome
drove the extraction of shared modules, the host-side test suite, and the
deployment targets that every game now starts with.

### Engine core

- **magnolia_init() reorders boot** — video comes up before `fatInitDefault()`, so a
  slow or wedged SD mount shows a splash screen instead of a blank framebuffer.
  Adds `renderer_splash()` for status frames during bring-up.
- **App directory auto-created** — `magnolia_init()` creates `sd:/apps/<app_name>/`
  before anything tries to persist into it. Without this, `fopen(..., "w")` fails
  when the parent is missing, and libfat does not create one implicitly.
- **Save diagnostics** — `prefs_persisted()` and `scoring_persisted()` report whether
  the last save reached the card. `magnolia_init()` probes with a real write, so
  the answer is valid from boot.

### New modules

- **prefs** — Persisted int key/value store for player preferences. Replaces
  hand-rolled settings readers in individual games.
- **scoring grows tables** — Named tables persist to `scores-<id>.json` beside the
  default `scores.json`. `scoring_increment()` supports per-table overflow bonuses.
  Backwards-compatible: old single-table saves still load.
- **menu** — Grid/list cursor with wrapping and a scrolling window. A one-column
  grid is a vertical list. Exhaustively tested: every shape, every start position,
  every move.
- **gamestate** — Grows `GS_MENU` and `GS_PAUSED`. The pre-run character select that
  was a loose flag is now a state.
- **ui_utils** — Design-space text measurement, centring, filled panels with
  rounded corners, a modal scrim, and word wrap.
- **clock** — Frame counter, delta time, and easing functions for tile animations.
- **theme** — HSL-to-RGB palette generator with complementary colours. Guarded on
  `#ifdef GEKKO` so it compiles on the host for testing.

### Sprites and audio

- **Memory-backed loaders** — `sprite_load_mem()`, `audio_load_sfx_mem()`,
  `audio_play_music_mem()`. Assets linked into the binary cannot go missing or
  fall out of step with the code.
- **Per-axis scaled sprite drawing** — `sprite_draw_scaled_xy()` takes separate
  `scaleX` and `scaleY` for non-square framebuffer pixels (16:9, PAL).

### Input

- **Button 1 and 2** added to the Wiimote wrapper.

### Testing

- **Host-side test suite** — `prefs`, `scoring`, `menu`, `gamestate`, and `theme` run on
  any machine with a C compiler. No devkitPPC, no console, no emulator.
- **CI on every push** — GitHub Actions runs the host tests on a GitHub-hosted
  runner. The standalone engine build is reserved for the self-hosted runner on MC1.
- **`make test-theme`** — HSL primaries, secondaries, complementaries, grey, black,
  white, wrap-around, and `theme_generate()` range checks over 2000 palettes.

### Tooling

- **`tools/new-game.sh`** — Scaffolds a game from `template/` with bin2s rules,
  deploy targets, and a host-test target.
- **`make card SD=/mnt/e`** — Installs onto a mounted SD card (merges, preserves
  saves).
- **`make wii WIILOAD=tcp:<ip>`** — Sends a build to a running console over the
  network (temporary, nothing written to card).
- **`make dolphin`** — Clears the app directory on every deploy (dev loop).

### Documentation

- **AGENTS.md** — No-AI-attribution rule for commits, PRs, and release notes.
- **README.md** — Rewritten for the real API, module table updated, design rule
  documented.

## v0.2.0

Breaking release. Under 0.x semver a breaking change bumps the minor.

### Breaking changes

- **`renderer_init()` replaced by `magnolia_init(MagnoliaConfig)`** — sequences
  SD, video, font, UI metrics and scoring. Returns a status so games can report a
  degraded start.
- **`scoring_init()` takes a path and capacity** — no more hardcoded `SCORES_PATH`.
- **player, stars, characters moved out** — they encoded one game's decisions and
  forced the engine to depend on that game's `config.h`.

### Added

- **Audio module** — PCM16 music loop and fire-and-forget SFX over ASND. Voice 0
  reserved for music. Format is `VOICE_STEREO_16BIT_LE` (little-endian on a
  big-endian console).
- **Score-attack gamestate** — title → ready → playing → game over → initials →
  high scores, plus the initials editor.
- **Standalone build** — `make` builds `libmagnolia.a` with no game on the include
  path, so any reach into a game header is an immediate build failure.
- **Sprite origin concept** — `sprite_load()` takes an origin point so exported art
  lands exactly where you draw it.

### Fixed

- Typo in README.md.
- Project name origin revised in README.

## v0.1.0

Initial release.

- Engine init, GRRLIB renderer, TTF font, frame flush.
- Wiimote input with D-pad, hold state, and auto-repeat.
- Sprite loading and drawing.
- Apache 2.0 license.
