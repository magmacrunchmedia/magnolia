#ifndef SCORING_H
#define SCORING_H

#include "core.h"

typedef struct {
    char initials[4];
    int score;
} ScoreEntry;

/* path: where the table persists (an sd:/ path). max_entries is clamped to
   MAGNOLIA_MAX_SCORES. Loads any existing table. Normally called for you by
   magnolia_init(). */
void scoring_init(const char *path, int max_entries);
void scoring_reset(void);
int scoring_get(void);
void scoring_increment(void);
void scoring_save(void);
int scoring_load(void);
int scoring_is_high_score(int score);
int scoring_get_rank(int score);
int scoring_add_entry(const char *initials, int score);
int scoring_get_count(void);
const ScoreEntry *scoring_get_entry(int index);

#endif
