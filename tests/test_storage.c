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

static void test_scoring_single(void) {
    printf("scoring: single table\n");
    cleanup();

    scoring_init(SCORES, 5);
    check_int(scoring_get_count(), 0, "empty to start");
    check_int(scoring_table_count(), 1, "default table exists");

    check(scoring_is_high_score(10), "any score qualifies while there is room");
    check(!scoring_is_high_score(0), "zero never qualifies");
    check(!scoring_is_high_score(-5), "negative never qualifies");

    scoring_add_entry("AAA", 100);
    scoring_add_entry("BBB", 300);
    scoring_add_entry("CCC", 200);
    check_int(scoring_get_count(), 3, "three entries");
    check_int(scoring_get_entry(0)->score, 300, "sorted descending");
    check_int(scoring_get_entry(1)->score, 200, "middle entry placed");
    check_int(scoring_get_entry(2)->score, 100, "lowest last");
    check(strcmp(scoring_get_entry(0)->initials, "BBB") == 0, "initials follow the score");

    check_int(scoring_get_rank(400), 1, "beating the top is rank 1");
    check_int(scoring_get_rank(250), 2, "middling score ranks correctly");
    check_int(scoring_get_rank(50), 4, "worst score ranks last");

    /* Fill past the cap. */
    scoring_add_entry("DDD", 50);
    scoring_add_entry("EEE", 25);
    check_int(scoring_get_count(), 5, "table fills to max_entries");
    check(!scoring_is_high_score(10), "full table rejects a low score");
    check(scoring_is_high_score(150), "full table accepts a qualifying score");

    scoring_add_entry("FFF", 150);
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
    scoring_add_entry("NIB", 15);
    scoring_select_table(byte);
    scoring_add_entry("BYT", 255);
    scoring_add_entry("BY2", 200);

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
    test_scoring_single();
    test_scoring_tables();
    test_scoring_compat();
    test_running_score();
    cleanup();

    return report();
}
