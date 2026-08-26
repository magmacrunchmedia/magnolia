/* Host-side tests for the two storage modules: the preference store and the
 * high-score tables. Both are about files, and files are exactly what an
 * emulator's SD card emulation can quietly refuse to give you -- these ran green
 * on a Mac while Dolphin was silently rejecting every write, which is the whole
 * reason they are worth having off-console.
 *
 *   make test-storage
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "harness.h"
#include "prefs.h"
#include "scoring.h"

static const char *PREFS  = "/tmp/magnolia_test_prefs.json";
static const char *SCORES = "/tmp/magnolia_test_scores.json";

static void cleanup(void) {
    unlink(PREFS);
    unlink(SCORES);
    unlink("/tmp/magnolia_test_scores-nibble.json");
    unlink("/tmp/magnolia_test_scores-byte.json");
    unlink("/tmp/magnolia_test_scores-gauntlet.json");
}

static void test_prefs(void) {
    printf("prefs\n");
    cleanup();

    prefs_init(PREFS);
    check_int(prefs_get_int("missing", 7), 7, "absent key returns the fallback");

    prefs_set_int("character", 12);
    prefs_set_int("muted", 1);
    check_int(prefs_get_int("character", 0), 12, "value reads back");
    check_int(prefs_get_int("muted", 0), 1, "second value reads back");
    check(prefs_persisted(), "write reached the file");

    /* A fresh init is what a power cycle looks like. */
    prefs_init(PREFS);
    check_int(prefs_get_int("character", 0), 12, "value survives a reload");
    check_int(prefs_get_int("muted", 0), 1, "second value survives a reload");
    check_int(prefs_get_int("never_set", -3), -3, "fallback still works after load");

    prefs_set_int("character", 5);
    prefs_init(PREFS);
    check_int(prefs_get_int("character", 0), 5, "overwrite persists, not appends");

    prefs_set_int("negative", -42);
    prefs_init(PREFS);
    check_int(prefs_get_int("negative", 0), -42, "negative values round-trip");

    /* An unreadable path must not lose the fallback contract. */
    prefs_init("/nonexistent-dir/prefs.json");
    check_int(prefs_get_int("character", 99), 99, "unwritable path still answers");
    prefs_set_int("character", 1);
    check(!prefs_persisted(), "failed write is reported, not silently swallowed");
}

/* Whether a write reached the card, for both stores.
 *
 * This is the question commit 25670ea was really about. Every score and
 * preference write was failing because nothing had created sd:/apps/<app>/, and
 * it hid for as long as it did because nothing reported it: prefs_persisted()
 * was the only signal, and scoring had no equivalent at all.
 *
 * An unwritable path here is a directory that is not there -- the same shape as
 * the real failure, and no permissions games needed to produce it.
 */
static const char *NO_DIR_PREFS  = "/nonexistent-dir/settings.json";
static const char *NO_DIR_SCORES = "/nonexistent-dir/scores.json";

static void test_persistence_reporting(void) {
    printf("storage: whether the last write reached the card\n");
    cleanup();

    /* A healthy card with no settings file yet is what every fresh install
       looks like, and it used to report failure -- prefs_init() zeroed the flag
       and prefs_load() returned early without ever touching it. A game
       surfacing that would have told every new player their settings were not
       being kept. prefs_init() now probes with a real write. */
    prefs_init(PREFS);
    check(prefs_persisted(), "a fresh card reports saving, with no set() call");
    check_int(prefs_get_int("anything", 4), 4, "and the fallback still answers");

    prefs_set_int("music", 1);
    check(prefs_persisted(), "and still reports saving after a real write");

    prefs_init(NO_DIR_PREFS);
    check(!prefs_persisted(), "a missing directory reports not saving");

    /* The flag must recover rather than latch: a game that reports a save
       failure forever after one bad card is its own kind of wrong. */
    prefs_init(PREFS);
    check(prefs_persisted(), "a working path clears the earlier failure");

    /* scoring can now answer the same question. Before this, a failed fopen in
       scoring_save() was dropped on the floor, so a leaderboard that reset on
       every power cycle looked exactly like one nobody had qualified for. */
    cleanup();
    scoring_init(SCORES, 10);
    check(scoring_persisted(), "nothing has failed before the first save");

    scoring_add_entry("ABC", 100, 0, 0);
    check(scoring_persisted(), "a save to a good path reports success");

    scoring_init(NO_DIR_SCORES, 10);
    check(scoring_persisted(), "a fresh init starts clean");
    scoring_add_entry("ABC", 100, 0, 0);
    check(!scoring_persisted(), "a save to a missing directory reports failure");

    scoring_init(SCORES, 10);
    check(scoring_persisted(), "a fresh init clears the previous failure");
    scoring_add_entry("DEF", 200, 0, 0);
    check(scoring_persisted(), "and a good save reports success again");

    cleanup();
}

static void test_scoring_single(void) {
    printf("scoring: single table\n");
    cleanup();

    scoring_init(SCORES, 5);
    check_int(scoring_get_count(), 0, "empty to start");
    check_int(scoring_table_count(), 1, "default table exists");

    check(scoring_is_high_score(10), "any score qualifies while there is room");
    check(!scoring_is_high_score(0), "zero never qualifies");
    check(!scoring_is_high_score(-5), "negative never qualifies");

    scoring_add_entry("AAA", 100, 0, 0);
    scoring_add_entry("BBB", 300, 0, 0);
    scoring_add_entry("CCC", 200, 0, 0);
    check_int(scoring_get_count(), 3, "three entries");
    check_int(scoring_get_entry(0)->score, 300, "sorted descending");
    check_int(scoring_get_entry(1)->score, 200, "middle entry placed");
    check_int(scoring_get_entry(2)->score, 100, "lowest last");
    check(strcmp(scoring_get_entry(0)->initials, "BBB") == 0, "initials follow the score");

    check_int(scoring_get_rank(400), 1, "beating the top is rank 1");
    check_int(scoring_get_rank(250), 2, "middling score ranks correctly");
    check_int(scoring_get_rank(50), 4, "worst score ranks last");

    /* Fill past the cap. */
    scoring_add_entry("DDD", 50, 0, 0);
    scoring_add_entry("EEE", 25, 0, 0);
    check_int(scoring_get_count(), 5, "table fills to max_entries");
    check(!scoring_is_high_score(10), "full table rejects a low score");
    check(scoring_is_high_score(150), "full table accepts a qualifying score");

    scoring_add_entry("FFF", 150, 0, 0);
    check_int(scoring_get_count(), 5, "table does not grow past the cap");
    check_int(scoring_get_entry(2)->score, 150, "new entry lands in order");
    check_int(scoring_get_entry(4)->score, 50, "lowest entry was pushed off");

    scoring_init(SCORES, 5);
    check_int(scoring_get_count(), 5, "table survives a reload");
    check_int(scoring_get_entry(0)->score, 300, "order survives a reload");
    check(strcmp(scoring_get_entry(0)->initials, "BBB") == 0, "initials survive a reload");
}

static void test_scoring_tables(void) {
    printf("scoring: multiple tables\n");
    cleanup();

    scoring_init(SCORES, 10);
    int nibble   = scoring_add_table("nibble");
    int byte     = scoring_add_table("byte");
    int gauntlet = scoring_add_table("gauntlet");

    check(nibble > 0 && byte > 0 && gauntlet > 0, "tables registered");
    check(nibble != byte && byte != gauntlet, "tables are distinct");
    check_int(scoring_table_count(), 4, "default plus three named");
    check_int(scoring_add_table("nibble"), nibble, "re-registering returns the same table");

    scoring_select_table(nibble);
    scoring_add_entry("NIB", 15, 0, 0);
    scoring_select_table(byte);
    scoring_add_entry("BYT", 255, 0, 0);
    scoring_add_entry("BY2", 200, 0, 0);

    scoring_select_table(nibble);
    check_int(scoring_get_count(), 1, "nibble table kept its own entries");
    check_int(scoring_get_entry(0)->score, 15, "nibble entry intact");

    scoring_select_table(byte);
    check_int(scoring_get_count(), 2, "byte table kept its own entries");
    check_int(scoring_get_entry(0)->score, 255, "byte entry intact");

    scoring_select_table(gauntlet);
    check_int(scoring_get_count(), 0, "untouched table stays empty");

    scoring_select_table(0);
    check_int(scoring_get_count(), 0, "default table unaffected by named tables");

    /* Out-of-range selection must not silently move to another table. */
    scoring_select_table(byte);
    scoring_select_table(999);
    check_int(scoring_current_table(), byte, "bad index leaves selection alone");
    scoring_select_table(-1);
    check_int(scoring_current_table(), byte, "negative index leaves selection alone");

    /* Reload everything: each table must come back from its own file. */
    scoring_init(SCORES, 10);
    int n2 = scoring_add_table("nibble");
    int b2 = scoring_add_table("byte");
    scoring_select_table(n2);
    check_int(scoring_get_count(), 1, "nibble reloaded from its own file");
    check_int(scoring_get_entry(0)->score, 15, "nibble score reloaded");
    scoring_select_table(b2);
    check_int(scoring_get_count(), 2, "byte reloaded from its own file");
    check(strcmp(scoring_get_entry(0)->initials, "BYT") == 0, "byte initials reloaded");
}

static void test_scoring_compat(void) {
    printf("scoring: single-table save files still load\n");
    cleanup();

    /* Exactly what a build before multi-table support wrote. */
    FILE *f = fopen(SCORES, "w");
    fprintf(f, "{\"scores\":[{\"initials\":\"OLD\",\"score\":420},"
               "{\"initials\":\"TWO\",\"score\":99}]}");
    fclose(f);

    scoring_init(SCORES, 10);
    check_int(scoring_get_count(), 2, "old file loads");
    check_int(scoring_get_entry(0)->score, 420, "old score intact");
    check(strcmp(scoring_get_entry(0)->initials, "OLD") == 0, "old initials intact");

    /* Adding a named table must not disturb the default one's file. */
    scoring_add_table("extra");
    scoring_select_table(0);
    check_int(scoring_get_count(), 2, "default table untouched by a new table");
}

/* moves and highest_earned, the two fields a ScoreEntry gained after the
 * initials-and-score pair it started as. They are written by scoring_save() and
 * read back by scoring_load(), so they are exactly the sort of thing that
 * round-trips fine in memory and quietly comes back as zero off the card.
 */
static void test_scoring_extra_fields(void) {
    printf("scoring: moves and highest_earned round-trip\n");
    cleanup();

    scoring_init(SCORES, 5);
    scoring_add_entry("RUN", 400, 137, 2048);
    scoring_add_entry("LOW", 100, 42, 256);

    scoring_save();
    check_int(scoring_load(), 2, "both entries load back");

    check_str(scoring_get_entry(0)->initials, "RUN", "the top entry is the one it was");
    check_int(scoring_get_entry(0)->score, 400, "and kept its score");
    check_int(scoring_get_entry(0)->moves, 137, "moves survive the file");
    check_int(scoring_get_entry(0)->highest_earned, 2048, "highest_earned survives the file");

    check_str(scoring_get_entry(1)->initials, "LOW", "the second entry is the one it was");
    check_int(scoring_get_entry(1)->moves, 42, "the second entry's moves are its own");
    check_int(scoring_get_entry(1)->highest_earned, 256,
              "the second entry's highest_earned is its own");

    /* A fresh init is what a power cycle looks like. */
    scoring_init(SCORES, 5);
    check_int(scoring_get_entry(0)->moves, 137, "moves survive a reload");
    check_int(scoring_get_entry(0)->highest_earned, 2048, "highest_earned survives a reload");
}

static void test_scoring_field_compat(void) {
    printf("scoring: save files without the newer fields still load\n");
    cleanup();

    /* Exactly what a build before moves and highest_earned wrote. scoring.c
       treats both keys as optional for this reason, and a card written by that
       build is still sitting in players' consoles. */
    FILE *f = fopen(SCORES, "w");
    fprintf(f, "{\"scores\":[{\"initials\":\"OLD\",\"score\":420},"
               "{\"initials\":\"TWO\",\"score\":99}]}");
    fclose(f);

    scoring_init(SCORES, 10);
    check_int(scoring_get_count(), 2, "a file with neither key still loads");
    check_int(scoring_get_entry(0)->score, 420, "old score intact");
    check_int(scoring_get_entry(0)->moves, 0, "absent moves defaults to 0");
    check_int(scoring_get_entry(0)->highest_earned, 0, "absent highest_earned defaults to 0");
    check_int(scoring_get_entry(1)->score, 99, "second old score intact");
    check_int(scoring_get_entry(1)->moves, 0, "absent moves defaults to 0 on every entry");
    check_int(scoring_get_entry(1)->highest_earned, 0,
              "absent highest_earned defaults to 0 on every entry");

    /* Once a run is filed against the old table, the whole file is rewritten in
       the current format -- the defaults must not stick to the old entries. */
    scoring_add_entry("NEW", 500, 64, 1024);
    scoring_init(SCORES, 10);
    check_int(scoring_get_entry(0)->moves, 64, "the new entry's moves reached the card");
    check_int(scoring_get_entry(0)->highest_earned, 1024,
              "the new entry's highest_earned reached the card");
    check_int(scoring_get_entry(1)->moves, 0, "the upgraded old entry still reads 0");
}

static void test_scoring_fields_follow_the_sort(void) {
    printf("scoring: moves and highest_earned follow the sort\n");
    cleanup();

    scoring_init(SCORES, 3);
    scoring_add_entry("TOP", 300, 30, 1024);
    scoring_add_entry("BOT", 100, 10, 128);

    /* Entries are held in score order, so an insertion in the middle shifts the
       ones below it. Each field has to travel with the entry it belongs to. */
    scoring_add_entry("MID", 200, 20, 512);

    check_int(scoring_get_entry(0)->score, 300, "top entry stayed put");
    check_int(scoring_get_entry(0)->moves, 30, "top entry kept its moves");
    check_int(scoring_get_entry(0)->highest_earned, 1024, "top entry kept its highest_earned");

    check_int(scoring_get_entry(1)->score, 200, "new entry landed in the middle");
    check_int(scoring_get_entry(1)->moves, 20, "new entry brought its own moves");
    check_int(scoring_get_entry(1)->highest_earned, 512,
              "new entry brought its own highest_earned");

    check_int(scoring_get_entry(2)->score, 100, "displaced entry moved down a rank");
    check_int(scoring_get_entry(2)->moves, 10, "displaced entry kept its moves");
    check_int(scoring_get_entry(2)->highest_earned, 128,
              "displaced entry kept its highest_earned");

    /* A full table, so this one pushes the lowest entry off the end. */
    scoring_add_entry("WIN", 400, 40, 4096);
    check_int(scoring_get_count(), 3, "table does not grow past the cap");
    check_int(scoring_get_entry(0)->moves, 40, "the new leader carries its own moves");
    check_int(scoring_get_entry(0)->highest_earned, 4096,
              "the new leader carries its own highest_earned");
    check_int(scoring_get_entry(2)->score, 200, "the lowest entry was pushed off");
    check_int(scoring_get_entry(2)->moves, 20, "and the survivor shifted down with its moves");
    check_int(scoring_get_entry(2)->highest_earned, 512,
              "and with its highest_earned");

    /* The reordering has to reach the card, not just the array. */
    scoring_init(SCORES, 3);
    check_int(scoring_get_entry(1)->moves, 30, "the sorted order reloads with its moves");
    check_int(scoring_get_entry(2)->highest_earned, 512,
              "and the shifted entry's highest_earned");
}

static void test_running_score(void) {
    printf("scoring: the run in progress\n");
    cleanup();
    scoring_init(SCORES, 10);

    scoring_reset();
    check_int(scoring_get(), 0, "reset clears");
    scoring_increment();
    check_int(scoring_get(), 1, "increment adds one");
    scoring_add(765);
    check_int(scoring_get(), 766, "add takes arbitrary points");
    scoring_add(-6);
    check_int(scoring_get(), 760, "add accepts a penalty");
    scoring_reset();
    check_int(scoring_get(), 0, "reset clears again");
}

int main(void) {
    test_prefs();
    test_persistence_reporting();
    test_scoring_single();
    test_scoring_tables();
    test_scoring_compat();
    test_scoring_extra_fields();
    test_scoring_field_compat();
    test_scoring_fields_follow_the_sort();
    test_running_score();
    cleanup();

    return report();
}
