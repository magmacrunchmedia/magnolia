#ifndef PREFS_H
#define PREFS_H

/* Small persisted key/value store for player preferences -- chosen character,
 * muted sound, last mode, whatever a game wants remembered across a power cycle.
 *
 * Deliberately int-only and deliberately tiny. Preferences are a handful of
 * toggles and indices; a game that needs structured data wants its own file, not
 * a more clever version of this. The on-card format is the same hand-written JSON
 * object the score tables use, so an SD card stays readable on a PC.
 *
 * Values are written through on set: preferences are changed a few times per
 * session, always at a moment the player would expect a save, and losing them to
 * a power-off is worse than the write.
 */

#define MAGNOLIA_MAX_PREFS    16
#define MAGNOLIA_PREF_KEY_MAX 24

/* path: an sd:/ path. Loads any existing file, then writes it straight back.
   Normally called by magnolia_init(), which points it at
   sd:/apps/<app_name>/settings.json.

   The write-back is deliberate. It costs one tiny file per boot and it buys a
   real answer from prefs_persisted() immediately, instead of after whenever the
   player next happens to change a setting -- which is how a card that refuses
   every write went unnoticed for as long as it did. */
void prefs_init(const char *path);

/* `fallback` is returned when the key was never set, when the card is absent, and
   when the file is unreadable -- a game asking for a preference gets a usable
   answer in every case rather than having to test for one. */
int  prefs_get_int(const char *key, int fallback);

/* Silently ignored once the table is full, or if key is empty or too long. */
void prefs_set_int(const char *key, int value);

void prefs_save(void);
int  prefs_load(void);

/* Whether the last save reached the card. Lets a game tell the player their
   settings are not being kept, rather than appearing to forget them at random.

   Answers for the current card from prefs_init() onwards, because that probes
   with a real write. Reading the file successfully is not evidence that writing
   works, so a successful load does not set this. */
int  prefs_persisted(void);

#endif
