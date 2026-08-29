#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prefs.h"

typedef struct {
    char key[MAGNOLIA_PREF_KEY_MAX];
    int  value;
} Pref;

static Pref prefs[MAGNOLIA_MAX_PREFS];
static int  pref_count = 0;
static char prefs_path[192] = "";
static int  persisted = 0;

static Pref *find(const char *key) {
    for (int i = 0; i < pref_count; i++) {
        if (strcmp(prefs[i].key, key) == 0) return &prefs[i];
    }
    return NULL;
}

void prefs_init(const char *path) {
    if (path) {
        strncpy(prefs_path, path, sizeof(prefs_path) - 1);
        prefs_path[sizeof(prefs_path) - 1] = '\0';
    }
    pref_count = 0;
    persisted = 0;
    prefs_load();

    /* Probe the card with a real write rather than waiting for the player to
       change something. Before this, prefs_persisted() reported 0 on a perfectly
       healthy card that simply had no settings file yet -- a game surfacing it
       would have told every new player their settings were not being kept. It
       also puts a missing sd:/apps/<app>/ directory on screen at boot, which is
       exactly the failure that hid the longest. */
    prefs_save();
}

int prefs_get_int(const char *key, int fallback) {
    if (!key || !*key) return fallback;
    Pref *p = find(key);
    return p ? p->value : fallback;
}

void prefs_set_int(const char *key, int value) {
    if (!key || !*key) return;
    if (strlen(key) >= MAGNOLIA_PREF_KEY_MAX) return;

    Pref *p = find(key);
    if (!p) {
        if (pref_count >= MAGNOLIA_MAX_PREFS) return;
        p = &prefs[pref_count++];
        snprintf(p->key, sizeof(p->key), "%s", key);
    }
    p->value = value;
    prefs_save();
}

void prefs_save(void) {
    FILE *f = fopen(prefs_path, "w");
    if (!f) {
        persisted = 0;
        return;
    }

    fprintf(f, "{");
    for (int i = 0; i < pref_count; i++) {
        if (i > 0) fprintf(f, ",");
        fprintf(f, "\"%s\":%d", prefs[i].key, prefs[i].value);
    }
    fprintf(f, "}");
    fclose(f);
    persisted = 1;
}

int prefs_load(void) {
    pref_count = 0;

    FILE *f = fopen(prefs_path, "r");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0) {
        fclose(f);
        return 0;
    }

    char *buf = (char *)malloc(size + 1);
    if (!buf) {
        fclose(f);
        return 0;
    }
    size_t got = fread(buf, 1, (size_t)size, f);
    fclose(f);
    /* A short read means the file is not the one it described itself as.
       The bytes past `got` are whatever malloc last left in that block, and
       the parser below would walk straight into them -- the terminator goes
       at the length the file claimed, not the length that arrived. audio.c
       has always checked this; these two never did. Falling back to nothing
       is what the comment below already says a half-read file should do:
       defaults, not values assembled out of whatever happened to parse. */
    if (got != (size_t)size) {
        free(buf);
        pref_count = 0;
        return 0;
    }
    buf[size] = '\0';

    /* Walks "key":value pairs. Anything malformed ends the parse rather than
       being guessed at -- a half-read preferences file should fall back to
       defaults, not to values assembled out of whatever parsed. */
    char *p = buf;
    while (pref_count < MAGNOLIA_MAX_PREFS) {
        char *q = strchr(p, '"');
        if (!q) break;
        q++;
        char *close = strchr(q, '"');
        if (!close) break;

        size_t len = (size_t)(close - q);
        if (len == 0 || len >= MAGNOLIA_PREF_KEY_MAX) break;

        char *colon = strchr(close, ':');
        if (!colon) break;

        memcpy(prefs[pref_count].key, q, len);
        prefs[pref_count].key[len] = '\0';
        prefs[pref_count].value = atoi(colon + 1);
        pref_count++;

        p = colon + 1;
    }

    free(buf);
    /* Deliberately does not touch `persisted`: reading the file proves the card
       can be read, which is a different question from whether it can be
       written, and conflating the two is what made the flag unreliable. */
    return pref_count;
}

int prefs_persisted(void) { return persisted; }
