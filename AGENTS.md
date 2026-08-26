# AGENTS.md — magnolia

C game engine for Wii homebrew, built on devkitPPC + GRRLIB 4.6 + libogc. Version
0.3.0 (the `VERSION` file is the source of truth). Games do not link a library —
they compile magnolia's sources directly; the games live in sibling repos
(moonlight-drift, george-boole, texas-holdem-lava-dome). Each of those holds
every version of its game, with the magnolia port under `wii/`, so the engine
resolves at `../../magnolia` from there rather than `../magnolia`. Apache-2.0.

## AI Attribution

**No AI attribution.** Do not append `Co-Authored-By: Claude …`, "Generated with …",
or any similar trailer to commit messages, PR bodies, or release notes. If your
tooling adds such a line by default, remove it before committing.

## Layout

```
magnolia.h              umbrella header games include
source/                 the 12 engine modules, one .c/.h pair each:
                        core (boot order, SD paths), renderer (video, TTF font),
                        sprite, input, audio, scoring, prefs, gamestate, menu,
                        clock, theme, ui_utils
font/                   Press Start 2P embedded as a C array (raw2c), OFL 1.1
tests/                  host-side tests + harness.h + fake_input.c (input.h stand-in)
template/               new-game skeleton: Makefile, meta.xml, source/
tools/new-game.sh       stamps template/ out as ../<name> beside the engine
Makefile                standalone libmagnolia.a build + host test targets
build/, libmagnolia.a   standalone-build outputs (generated)
.github/workflows/ci.yml  runs the host tests on every push
```

## Commands

```bash
make          # build libmagnolia.a standalone; needs DEVKITPPC + DEVKITPRO set
make test     # all host tests; needs only a host C compiler (HOSTCC ?= cc)
make test-storage | test-menu | test-gamestate | test-theme   # one at a time
make clean
tools/new-game.sh my-game     # create ../my-game from template/
```

`make` exists only to prove the engine compiles with no game on the include path —
if engine code reaches into a game header, this build fails immediately. Run it
before committing engine changes. It is *not* how games consume magnolia; a game's
Makefile adds the sources directly:

```makefile
SOURCES  := source ../magnolia/source ../magnolia/font
INCLUDES := ../magnolia ../magnolia/source ../magnolia/font source
```

Deploy targets live in each game's Makefile (from `template/`), not here:
`make deploy` (stage `sdcard/apps/<game>/`), `make dolphin` (emulator SD folder),
`make card SD=/mnt/e` (real card — merges, only boot.dol/meta.xml overwritten so
saves survive), `make wii WIILOAD=tcp:<wii-ip>` (run over network, card untouched).

## Conventions

- C99, `-Wall -Wextra`, warning-clean. If the compiler complains, fix it.
- **No wildcards in the Makefile's test source lists.** The per-binary list is the
  record of which modules are host-clean. (The standalone build does wildcard.)
- Host tests cover every libogc-free module: prefs, scoring, menu, gamestate,
  theme. gamestate reaches libogc only through `input.h`, so its test links
  `tests/fake_input.c` in place of `source/input.c`. theme compiles on the host
  because its libogc use is guarded by `#ifdef GEKKO`.
- Adding a host-clean module: add it to `HOST_TESTS`, write
  `tests/test_<module>.c` on the existing `harness.h`, verify `make test-<module>`.
- **Design rule:** magnolia holds what more than one game needs. Sprite loading and
  the origin concept are engine; a character roster is not. One consumer means it
  stays in the game until a second appears.
- `magnolia_init()` return codes: `0` all OK; `-1` SD not mounted
  (`magnolia_sd_mounted()`); `-2` video failed — return immediately; `-3` font
  missing (`magnolia_fonts_loaded()`). Negative is degraded, not dead — query and
  tell the player, don't render placeholders silently.
- Never hardcode 640×480: use `renderer_screen_width()/_height()` (PAL is
  640×528). Author UI in `UI_DESIGN_WIDTH`×`UI_DESIGN_HEIGHT` design space and
  draw through the `ui_map_*` helpers (TV-safe projection).
- Audio assets are raw s16le PCM, held decoded in RAM:
  `ffmpeg -i in.ogg -f s16le -acodec pcm_s16le -ar 48000 -ac 2 out.pcm`.
  48 kHz stereo is ~11.5 MB/min against 24 MB total — keep SFX at 48 kHz stereo,
  pass mono/lower rates for music via `audio_play_music_fmt()`. Trim, don't stream.
- Header comments are design documents — explain *why*, not just *what*.
- Commits: imperative subject < 72 chars, body wrapped at 80 explaining why.
  Keep PRs to one logical change. `make test` must pass before pushing; engine
  changes must not break `make` or any dependent game.

## Sprite sheets (shared contract — do not change unilaterally)

Uniform grid PNG: frames are frameWidth x frameHeight cells, counted left-to-right
then top-to-bottom. The origin/anchor is stored with the sheet at load time, not
re-derived at call sites. This format is read by all three engines — adenosine (TS),
magnolia (C/Wii), texastoast (Python) — so a sheet exported from SPRITE//FORGE
(adenosine/tools/sprites.html) feeds all of them. Canonical spec:
adenosine/packages/rpg/API.md. Changing the format is a three-repo change.

Here the reader is `source/sprite.c`/`sprite.h`: texture load/free, draw-at-origin,
uniform and per-axis scaled draw — the origin is taken at load time.

## Debugging on Dolphin

- Games can run controller-free via `AUTOSTART_GAMEPLAY` and
  `DEBUG_HEARTBEAT_FRAMES` in the generated `config.h`.
- For `printf` to reach Dolphin's log, its `Logger.ini` needs `OSREPORT = True`
  and `WriteToFile = True`; both default to False, which makes a working trace
  look like a dead one.
- An emulated SD card can report mounted while refusing every write —
  `prefs_persisted()` / `scoring_persisted()` tell a broken save from a broken
  card; `magnolia_init()` probes with a real write at boot.

## Git

Commit and push as magmacrunchmedia. No AI attribution trailers, ever.

<!-- Update this file in the same commit as any change to build, test, deploy, or layout. -->
