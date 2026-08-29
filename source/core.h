#ifndef MAGNOLIA_CORE_H
#define MAGNOLIA_CORE_H

/* Engine-owned ceiling on the high-score table; the table is a static array, so
   this is a compile-time cap and MagnoliaConfig.max_scores a runtime limit. */
#define MAGNOLIA_MAX_SCORES 16

/* Ceiling on independent high-score tables. One game wants a single leaderboard;
   another wants one per mode. Both are static arrays, so this is the compile-time
   cap on how many a game may ask for. */
#define MAGNOLIA_MAX_TABLES 12

typedef struct {
    /* Homebrew Channel app directory name. The engine derives SD paths from it,
       e.g. sd:/apps/<app_name>/scores.json -- games do not hardcode paths. */
    const char *app_name;
    /* Entries kept in the high-score table. <= 0 selects MAGNOLIA_MAX_SCORES. */
    int max_scores;
    /* Percent of each screen edge assumed lost to TV overscan.
       < 0 keeps the engine default. */
    int overscan_pct;
} MagnoliaConfig;

/* Brings up SD, video, font, UI metrics and scoring in the right order.
   Returns 0 when everything came up. A negative return means the engine is
   usable but degraded -- query the accessors below and tell the player, rather
   than silently rendering placeholder art. */
int  magnolia_init(const MagnoliaConfig *cfg);

/* Unmounts the card and shuts down video, in the reverse of the order
   magnolia_init() brought them up. Call it last: the SD card is gone
   afterwards, so anything still to be saved must be saved before this.
   Audio is not touched -- a game that called audio_init() owns the matching
   audio_shutdown(), since the engine never started it. */
void magnolia_shutdown(void);

int  magnolia_sd_mounted(void);
int  magnolia_fonts_loaded(void);

/* sd:/apps/<app_name>/<leaf>, valid until the next call. Convenience for games
   loading their own assets off the card. */
const char *magnolia_asset_path(const char *leaf);

#endif
