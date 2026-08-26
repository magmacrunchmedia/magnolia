# Changelog

All notable changes to the magnolia engine are documented here.

## v0.3.0 (unreleased)

The second-game release: George Boole and Lava Dome drove the extraction of
shared modules, the host-side test suite, and the deployment targets that every
game now starts with -- and, latterly, the input, sprite-sheet and timestep work
that a game with two players in front of it needs.

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
- **font/OFL.txt** — The SIL Open Font License 1.1 text now ships beside the
  Press Start 2P `.ttf` the engine distributes. The licence was named in the
  README but its text was nowhere in the repository.

### Two-player groundwork

Groundwork for a local two-player game. Three engine gaps stood between magnolia
and any game with two people in front of it; none of them were genre decisions,
so all three are closed here and everything genuinely fighting-game-shaped —
hitboxes, movelists, round flow, a roster — stays in the game.

### Input: more than one player

- **Every query takes a player index.** `input_scan()` samples all four
  controller channels; `input_pressed(1, INPUT_BTN_A)` is player two's A. Before
  this, `input.c` passed a hardcoded `0` to every WPAD call, so a second
  controller was invisible to the engine.
- **Releases and held states for every button.** `input_held()`,
  `input_released()`. Previously only `A` had a held query and nothing could
  report a button coming up, which rules out blocking, charging and hold-to-aim.
- **`input_snapshot()`** returns a player's whole frame as a copyable value —
  what an input buffer is made of. Recognising patterns in those frames stays
  with the game.
- **The zero-argument spellings are unchanged.** `input_a_pressed()` and friends
  are now player-0 wrappers; all three shipped games compile and build untouched.
- **`input_state.c` split out of `input.c`**, leaving `input.c` as the WPAD read
  and putting edges and auto-repeat somewhere they can be tested. Two behaviours
  changed as a result: hold counters are per-player (they were one global, so one
  player holding a direction advanced everyone's repeat), and the repeat edge is
  settled once per frame rather than inside the query — `input_dir_repeat()` used
  to be able to fire twice if a caller asked twice on the delay frame, which the
  header already promised it would not.

### Sprite sheets and mirroring

- **`SpriteSheet`** — uniform grid of frames in one texture, cells counted
  left-to-right then top-to-bottom. This is the cross-engine format AGENTS.md
  already described and named `sprite.c` as the reader of; until now `sprite.c`
  could only draw whole textures.
- **Mirroring** via `sprite_sheet_draw_ex()` and `sprite_draw_ex()`, reflected
  about the frame's own origin so an anchor point does not move when a character
  turns around. Drawn through GRRLIB's `*Quad` calls, which take explicit
  corners — a negative `scaleX` through `GRRLIB_DrawImg()` reflects about the
  draw position instead, landing the art a full width away.
- Unflipped draws still go through `GRRLIB_DrawImg`/`DrawTile` exactly as before,
  so no shipped game's output moves.
- Frame sizes that do not divide the image leave the sheet empty rather than
  drawing the plausible part of a mis-exported asset.

### Optional fixed timestep

- **`clock_set_fixed_hz()` / `clock_fixed_steps()` / `clock_fixed_dt()`**, with
  the accumulator in a host-clean `timestep.c`. `clock_dt()` is unchanged and
  still the right answer for anything continuous; a game that never asks for a
  fixed step sees no difference.
- Rules written in frames — three frames of startup, twelve of recovery — need a
  step that does not vary with SD reads. A frame owing more than
  `TIMESTEP_MAX_STEPS` is treated as a stall and its backlog dropped, rather than
  repaid over the following frames as a burst of speed.

### Tests

- **`make test-input`** (54 checks) and **`make test-timestep`** (26 checks).
  Neither module could be tested before the splits; both now assert the things
  that were being taken on trust, including that a second player has their own
  hold counters and that a second of real time buys the same number of steps
  however the frames it arrived in were shaped.
- **`tests/fake_input.c` no longer reimplements `input.h`.** It feeds the real
  `input_state.c`, so the presses the `gamestate` cases see are computed by the
  same lines the console runs — the copy could previously drift from the original.
- **Repaired `make test`**, which had not compiled since v0.3.0's
  "Add moves and highest_earned to ScoreEntry": that commit widened
  `scoring_add_entry()` to four arguments and updated no test. CI had been red
  since. The call sites now pass the two new fields, and `test_storage` now
  asserts what that commit never did: that `moves` and `highest_earned` survive
  a save/load round trip, that a `scores.json` written without either key still
  loads with both defaulting to 0, and that the fields follow their entry when a
  new score displaces it in the sort.

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
