#ifndef SCORING_H
#define SCORING_H

#include "core.h"

typedef struct {
    char initials[4];
    int score;
} ScoreEntry;

/* High scores, in one or more independent tables.
 *
 * A game with a single leaderboard can ignore tables entirely: magnolia_init()
 * creates the default table and every accessor below works on it. A game with
 * several -- one per difficulty, per mode, per board size -- registers each with
 * scoring_add_table() and switches between them with scoring_select_table().
 *
 * Each table persists to its own file so that adding a second one cannot corrupt
 * the first, and so a card written by an earlier single-table build still loads:
 * the default table keeps the plain scores.json path it always had, and named
 * tables land beside it as scores-<id>.json.
 */

/* path: where the default table persists (an sd:/ path). max_entries is clamped
   to MAGNOLIA_MAX_SCORES. Loads any existing table. Discards any tables from a
   previous call. Normally called for you by magnolia_init(). */
void scoring_init(const char *path, int max_entries);

/* Registers a named table and loads it from the card, returning its index, or
   the existing index if `id` is already registered. Returns -1 when full or when
   id is unusable. `id` becomes part of a filename, so keep it short and plain --
   letters, digits, dash. */
int scoring_add_table(const char *id);

/* Which table the accessors below act on. Out-of-range selections are ignored,
   so a bad index leaves the previous table selected rather than corrupting a
   different one. The default table is index 0. */
void scoring_select_table(int table);
int scoring_current_table(void);
int scoring_table_count(void);

/* The run in progress. Not per-table: a game plays one run at a time, and which
   table it will be filed under is a question for when it ends. */
void scoring_reset(void);
int  scoring_get(void);
void scoring_add(int points);
void scoring_increment(void);   /* scoring_add(1) */

/* All act on the selected table. */
void scoring_save(void);
int  scoring_load(void);
int  scoring_is_high_score(int score);
int  scoring_get_rank(int score);
int  scoring_add_entry(const char *initials, int score);
int  scoring_get_count(void);
const ScoreEntry *scoring_get_entry(int index);

#endif
