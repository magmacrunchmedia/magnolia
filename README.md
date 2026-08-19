# magnolia

A lightweight Wii homebrew game engine by [MagmaCrunch](https://magmacrunch.com). Provides rendering, input, scoring, and UI utilities for games built with devkitPPC and GRRLIB.

Named after the MagmaCrunch song "Magnolia."

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
    renderer_init();      // Sets up GRRLIB, loads Press Start 2P font
    input_init();         // Initializes Wiimote
    scoring_init();       // Loads high scores from SD card
    stars_init();         // Generates starfield
    characters_init();    // Initializes character selector

    while (1) {
        input_scan();
        stars_update();

        renderer_draw_background();
        renderer_draw_stars();
        renderer_draw_player(&player, thrust_active);
        renderer_draw_score(scoring_get());
        renderer_finish();  // Flip buffers
    }
}
```

## Engine Modules

| Module | Header | Description |
|--------|--------|-------------|
| **renderer** | `renderer.h` | GRRLIB init, sprite loading, TTF font rendering, frame flush |
| **input** | `input.h` | Wiimote input wrapper (buttons, D-pad) |
| **scoring** | `scoring.h` | High score manager with SD card JSON persistence |
| **stars** | `stars.h` | Twinkling starfield background (60 stars, blink/pulse) |
| **theme** | `theme.h` | HSL→RGB color palette generator (3-color harmonious themes) |
| **player** | `player.h` | Generic physics entity (thrust, gravity, velocity, boundary) |
| **characters** | `characters.h` | Character registry with selection, hitbox, and physics |
| **ui_utils** | `ui_utils.h` | Text shadow, centered text, screen border drawing |

## API Reference

### renderer

```c
void renderer_init(void);                              // Init GRRLIB + load font
void renderer_load_sprites(const CharacterData *ch);   // Load character textures
void renderer_draw_background(void);                   // Clear to black
void renderer_draw_stars(void);                        // Draw starfield
void renderer_draw_player(const Player *p, int thrust); // Draw sprite or rectangle
void renderer_draw_score(int score);                   // Draw score with TTF
void renderer_draw_character_name(const char *name);   // Draw name text
void renderer_finish(void);                            // Flip framebuffers
```

### input

```c
void input_init(void);
int  input_scan(void);           // Call each frame
int  input_thrust_pressed(void); // A button held
int  input_start_pressed(void);  // A button pressed
int  input_home_pressed(void);   // HOME button
int  input_left_pressed(void);   // D-pad left
int  input_right_pressed(void);  // D-pad right
```

### scoring

```c
void scoring_init(void);                          // Load from SD card
void scoring_reset(void);                         // Reset session score
int  scoring_get(void);                           // Current score
void scoring_increment(void);                     // Score +1
int  scoring_add_entry(const char *initials, int score); // Add high score
int  scoring_get_count(void);                     // Number of entries
const ScoreEntry *scoring_get_entry(int index);   // Get by index
int  scoring_is_high_score(int score);            // Does it qualify?
int  scoring_get_rank(int score);                 // Placement rank
```

### player

```c
void player_init(Player *p);
void player_set_physics(Player *p, float thrust, float gravity, float maxVel);
void player_set_hitbox(Player *p, int w, int h, int ox, int oy);
int  player_update(Player *p, int thrust_active, int canvas_height); // Returns 1 on boundary hit
```

### characters

```c
void characters_init(void);
const CharacterData *characters_get_current(void);
void characters_set_current(int index);
int  characters_get_current_index(void);
int  characters_get_count(void);
const CharacterData *characters_get_by_index(int index);
```

### ui_utils

```c
void ui_draw_text_shadow(int x, int y, const char *text, unsigned int size, u32 color);
void ui_draw_centered_text(int y, const char *text, unsigned int size, u32 color);
void ui_draw_border(void);  // Cyan border around screen
```

### stars

```c
void stars_init(void);
void stars_update(void);
void stars_get(int index, Star **out);
int  stars_get_count(void);
```

### theme

```c
ThemeColor theme_hsl_to_rgb(float h, float s, float l);
void theme_generate(Theme *t);  // Random 3-color palette
```

## Dependencies

| Package | Source |
|---------|--------|
| devkitPPC | [devkitPro](https://devkitpro.org) |
| GRRLIB 4.6 | [github.com/GRRLIB/GRRLIB](https://github.com/GRRLIB/GRRLIB) |
| libogc | Bundled with devkitPro |
| ppc-libpng, ppc-freetype, ppc-libjpeg-turbo | `dkp-pacman` |
| libfat-ogc | `dkp-pacman` |

## Font

Includes **Press Start 2P** by [CodeMan38](https://fonts.google.com/specimen/Press+Start+2P), embedded as a C array via `raw2c`. Licensed under OFL 1.1.

## License

LGPL-2.1 — same as GRRLIB.
