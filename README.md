# magnolia

A lightweight Wii homebrew game engine by [nagmacrunch media](https://magmacrunch.com). Provides rendering, input, scoring, and UI utilities for games built with devkitPPC and GRRLIB.

Named after the song "Magnolia" published by magmacrunch music.

## Quick Start

### 1. Clone or copy magnolia into your project

```
my-game/
├── Makefile
├── source/
│   └── main.c
└── ../magnolia/        ← this repo
    ├── magnolia.h
    ├── source/
    └── font/
```

### 2. Update your Makefile

```makefile
SOURCES     := source ../magnolia/source ../magnolia/font
INCLUDES    := ../magnolia ../magnolia/source ../magnolia/font source
LIBS        := -lgrrlib -lpngu -lfreetype -lpng -ljpeg -lz -lbrotlidec -lbrotlicommon -lbz2 -lfat -lwiiuse -lbte -logc -lm
```

### 3. Include and use

```c
#include "magnolia.h"

int main(void) {
    const MagnoliaConfig cfg = {
        .app_name     = "my-game",   /* -> sd:/apps/my-game/ */
        .max_scores   = 10,
        .overscan_pct = 6
    };

    /* Brings up SD, video, font, UI metrics and scoring in the right order.
       A non-zero return means the engine is usable but degraded -- check
       magnolia_sd_mounted() / magnolia_fonts_loaded() and tell the player,
       rather than silently rendering placeholder art. */
    int status = magnolia_init(&cfg);
    input_init();

    GameStateMachine gs;
    gamestate_init(&gs);

    while (1) {
        input_scan();
        if (input_home_pressed()) break;

        if (gamestate_current(&gs) == GS_PLAYING) {
            /* your game */
        } else if (gamestate_update(&gs, score)) {
            /* just entered GS_PLAYING -- reset your world */
        }

        renderer_draw_background();
        renderer_finish();
    }

    magnolia_shutdown();
    return 0;
}
```

### 4. Verify the engine still stands alone

```bash
cd magnolia && make      # builds libmagnolia.a
```

This compiles the engine with **no game directory on the include path**. It is
not how games consume magnolia -- they build the sources directly -- but if
engine code ever reaches back into a game header, this build fails immediately
instead of the coupling surfacing in the next game months later. Run it before
committing engine changes.

## Design rule

magnolia holds what more than one game needs. Anything shaped by a single game
belongs in that game. Concretely: sprite loading and the origin concept are
engine; a particular character roster is not. A high-score table is engine; a
jetman's gravity is not. When something looks reusable but has exactly one
consumer, leave it in the game until a second one appears -- an abstraction
designed against one example usually fits only that example.

## Engine Modules

| Module | Header | Description |
|--------|--------|-------------|
| **core** | `core.h` | `magnolia_init()` bring-up, init status, SD asset paths |
| **renderer** | `renderer.h` | GRRLIB init, TTF font, real video-mode geometry, frame flush |
| **sprite** | `sprite.h` | Texture load/free, draw-at-origin, scaled draw |
| **input** | `input.h` | Wiimote button/D-pad wrapper |
| **audio** | `audio.h` | PCM16 music loop + SFX over ASND |
| **scoring** | `scoring.h` | High-score table with SD JSON persistence |
| **gamestate** | `gamestate.h` | Score-attack shell and initials editor |
| **theme** | `theme.h` | HSL→RGB palette generator, complementary colours |
| **ui_utils** | `ui_utils.h` | Design-space → TV-safe layout, text, border |

### Not in the engine, by design

`player`, `stars` and `characters` used to live here and now belong to
Moonlight Drift. They encoded one game's decisions — jetman physics, a specific
starfield, a fixed character roster — and forced the engine to depend on that
game's `config.h`. If a second game needs something similar, generalise it then,
against two real examples.

### Screen geometry

Never hardcode 640×480. `renderer_screen_width()`/`_height()` report the running
video mode (NTSC 640×480, PAL 640×528). UI should be authored in the fixed
`UI_DESIGN_WIDTH`×`UI_DESIGN_HEIGHT` design space and drawn through the
`ui_map_*` helpers, which project it into the TV-safe area — consumer sets clip
roughly 5–10% of every edge.

### Audio format

Assets are raw signed 16-bit little-endian stereo at 48kHz:

```bash
ffmpeg -i in.ogg -f s16le -acodec pcm_s16le -ar 48000 -ac 2 out.pcm
```

Clips are held in main RAM — budget ~192KB per second of audio against the
Wii's 24MB, and trim long tracks rather than streaming them.

## Dependencies

| Package | Source |
|---------|--------|
| devkitPPC | [devkitPro](https://devkitpro.org) |
| GRRLIB 4.6 | [github.com/GRRLIB/GRRLIB](https://github.com/GRRLIB/GRRLIB) |
| libogc | Bundled with devkitPro |
| ppc-libpng, ppc-freetype, ppc-libjpeg-turbo | `dkp-pacman` |
| libfat-ogc | `dkp-pacman` |
| libasnd (audio) | Bundled with devkitPro |

## Font

Includes **Press Start 2P** by [CodeMan38](https://fonts.google.com/specimen/Press+Start+2P), embedded as a C array via `raw2c`. Licensed under OFL 1.1.

## License

Apache 2.0 — same as adenosine.
