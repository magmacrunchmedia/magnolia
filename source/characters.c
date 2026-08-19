#include <stddef.h>
#include "characters.h"

/* hitbox_* mirror the `hitbox` block in the source game's js/characters/<id>.js.
   sprite_origin_* locate the character's local (0,0) inside its 80x80 PNG. */
static const CharacterData all_characters[] = {
    {
        "tardigrade", "Tardigrade",
        "sd:/apps/moonlight-drift/sprites/tardigrade-idle.png",
        "sd:/apps/moonlight-drift/sprites/tardigrade-thrust.png",
        26, 25, 7, 8,
        20, 20,
        -0.7f, 0.35f, 11.0f
    },
    {
        "dag-henderson", "Dag Henderson",
        "sd:/apps/moonlight-drift/sprites/dag-henderson-idle.png",
        "sd:/apps/moonlight-drift/sprites/dag-henderson-thrust.png",
        40, 35, 0, 0,
        20, 23,
        -0.6f, 0.4f, 10.0f
    },
    {
        "cat-synth", "Cat Synth",
        "sd:/apps/moonlight-drift/sprites/cat-synth-idle.png",
        "sd:/apps/moonlight-drift/sprites/cat-synth-thrust.png",
        42, 30, -1, 18,
        20, 7,
        -0.6f, 0.36f, 10.5f
    },
    {
        "vinny-bobarino", "Vinny Bobarino",
        "sd:/apps/moonlight-drift/sprites/vinny-bobarino-idle.png",
        "sd:/apps/moonlight-drift/sprites/vinny-bobarino-thrust.png",
        35, 35, 15, 0,
        7, 23,
        -0.6f, 0.4f, 10.0f
    },
    {
        "carl-spatski", "Carl Spatski",
        "sd:/apps/moonlight-drift/sprites/carl-spatski-idle.png",
        "sd:/apps/moonlight-drift/sprites/carl-spatski-thrust.png",
        42, 42, -1, -2,
        20, 21,
        -0.65f, 0.4f, 10.5f
    },
    {
        "gangsta-beaver", "Gangsta Beaver",
        "sd:/apps/moonlight-drift/sprites/gangsta-beaver-idle.png",
        "sd:/apps/moonlight-drift/sprites/gangsta-beaver-thrust.png",
        40, 35, 0, 0,
        15, 23,
        -0.6f, 0.4f, 10.0f
    }
};

#define CHARACTER_COUNT (sizeof(all_characters) / sizeof(all_characters[0]))

static int current_index = 0;

void characters_init(void) {
    current_index = 0;
}

const CharacterData *characters_get_current(void) {
    return &all_characters[current_index];
}

void characters_set_current(int index) {
    if (index >= 0 && index < (int)CHARACTER_COUNT) {
        current_index = index;
    }
}

int characters_get_current_index(void) {
    return current_index;
}

int characters_get_count(void) {
    return (int)CHARACTER_COUNT;
}

const CharacterData *characters_get_by_index(int index) {
    if (index >= 0 && index < (int)CHARACTER_COUNT) {
        return &all_characters[index];
    }
    return NULL;
}
