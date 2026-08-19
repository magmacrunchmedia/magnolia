#ifndef CHARACTERS_H
#define CHARACTERS_H

typedef struct {
    const char *id;
    const char *name;
    const char *sprite_idle;
    const char *sprite_thrust;
    int hitbox_w;
    int hitbox_h;
    int hitbox_offset_x;
    int hitbox_offset_y;
    /* Where the character's local (0,0) drawing origin sits inside its sprite PNG.
       Recovered empirically per character by re-rendering the source draw() call and
       diffing against the exported PNG -- it is not a fixed value. */
    int sprite_origin_x;
    int sprite_origin_y;
    float thrust;
    float gravity;
    float maxVelocity;
} CharacterData;

void characters_init(void);
const CharacterData *characters_get_current(void);
void characters_set_current(int index);
int characters_get_current_index(void);
int characters_get_count(void);
const CharacterData *characters_get_by_index(int index);

#endif
