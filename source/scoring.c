#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scoring.h"

typedef struct {
    char        id[24];                          /* "" for the default table */
    char        path[192];
    ScoreEntry  entries[MAGNOLIA_MAX_SCORES];
    int         count;
} ScoreTable;

static int current_score = 0;
static ScoreTable tables[MAGNOLIA_MAX_TABLES];
static int table_count = 0;
static int current = 0;
static int max_scores = MAGNOLIA_MAX_SCORES;
static char base_path[160] = "";

static ScoreTable *tbl(void) {
    return &tables[current];
}

/* scores.json -> scores-<id>.json, so a named table sits beside the default one
   instead of replacing it. Falls back to appending when the base has no
   extension, which keeps a caller-supplied odd path working rather than
   silently writing every table to the same file. */
static void derive_path(const char *id, char *out, size_t n) {
    /* Built in a local and copied out: the caller's `out` and `id` both live
       inside the tables array, and snprintf's arguments are restrict-qualified,
       so writing straight through would be undefined behaviour even though the
       two members do not actually overlap. */
    char tmp[192];
    const char *dot = strrchr(base_path, '.');
    const char *slash = strrchr(base_path, '/');

    if (dot && (!slash || dot > slash)) {
        int stem = (int)(dot - base_path);
        snprintf(tmp, sizeof(tmp), "%.*s-%s%s", stem, base_path, id, dot);
    } else {
        snprintf(tmp, sizeof(tmp), "%s-%s", base_path, id);
    }

    snprintf(out, n, "%s", tmp);
}

static void load_table(int index);

void scoring_init(const char *path, int max_entries) {
    if (path) {
        strncpy(base_path, path, sizeof(base_path) - 1);
        base_path[sizeof(base_path) - 1] = '\0';
    }
    max_scores = (max_entries > 0 && max_entries < MAGNOLIA_MAX_SCORES)
               ? max_entries : MAGNOLIA_MAX_SCORES;

    memset(tables, 0, sizeof(tables));
    table_count = 1;
    current = 0;
    tables[0].id[0] = '\0';
    snprintf(tables[0].path, sizeof(tables[0].path), "%s", base_path);
    load_table(0);
}

int scoring_add_table(const char *id) {
    if (!id || !*id) return -1;

    for (int i = 0; i < table_count; i++) {
        if (strcmp(tables[i].id, id) == 0) return i;
    }
    if (table_count >= MAGNOLIA_MAX_TABLES) return -1;

    int index = table_count++;
    ScoreTable *t = &tables[index];
    memset(t, 0, sizeof(*t));
    snprintf(t->id, sizeof(t->id), "%s", id);
    derive_path(t->id, t->path, sizeof(t->path));
    load_table(index);
    return index;
}

void scoring_select_table(int table) {
    if (table >= 0 && table < table_count) current = table;
}

int scoring_current_table(void) { return current; }
int scoring_table_count(void)   { return table_count; }

void scoring_reset(void)     { current_score = 0; }
int  scoring_get(void)       { return current_score; }
void scoring_add(int points) { current_score += points; }
void scoring_increment(void) { scoring_add(1); }

int scoring_add_entry(const char *initials, int score) {
    ScoreTable *t = tbl();
    if (score <= 0) return -1;

    int rank = t->count;
    for (int i = 0; i < t->count; i++) {
        if (score > t->entries[i].score) {
            rank = i;
            break;
        }
    }

    if (rank >= max_scores) return -1;

    for (int i = t->count; i > rank; i--) {
        if (i < max_scores) {
            t->entries[i] = t->entries[i - 1];
        }
    }

    strncpy(t->entries[rank].initials, initials, 3);
    t->entries[rank].initials[3] = '\0';
    t->entries[rank].score = score;

    if (t->count < max_scores) t->count++;

    scoring_save();
    return rank;
}

int scoring_get_count(void) {
    return tbl()->count;
}

const ScoreEntry *scoring_get_entry(int index) {
    ScoreTable *t = tbl();
    if (index >= 0 && index < t->count) {
        return &t->entries[index];
    }
    return NULL;
}

void scoring_save(void) {
    ScoreTable *t = tbl();
    FILE *f = fopen(t->path, "w");
    if (!f) return;

    fprintf(f, "{\"scores\":[");
    int count = t->count < max_scores ? t->count : max_scores;
    for (int i = 0; i < count; i++) {
        if (i > 0) fprintf(f, ",");
        fprintf(f, "{\"initials\":\"%s\",\"score\":%d}",
                t->entries[i].initials, t->entries[i].score);
    }
    fprintf(f, "]}");
    fclose(f);
}

static void load_table(int index) {
    int saved = current;
    current = index;
    scoring_load();
    current = saved;
}

int scoring_load(void) {
    ScoreTable *t = tbl();

    FILE *f = fopen(t->path, "r");
    if (!f) {
        t->count = 0;
        return 0;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0) {
        fclose(f);
        t->count = 0;
        return 0;
    }

    char *buf = (char *)malloc(size + 1);
    if (!buf) {
        fclose(f);
        t->count = 0;
        return 0;
    }
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);

    t->count = 0;
    char *p = buf;
    char *end = buf + size;
    while ((p = strstr(p, "\"initials\":")) != NULL && t->count < max_scores) {
        if (p + 11 >= end) break;
        p += 11;
        while (*p == '"' && p < end) p++;
        int len = 0;
        while (p + len < end && p[len] != '"' && len < 3) len++;
        if (len > 0 && len <= 3) {
            memcpy(t->entries[t->count].initials, p, len);
            t->entries[t->count].initials[len] = '\0';
        } else {
            strcpy(t->entries[t->count].initials, "???");
        }
        p += len;

        p = strstr(p, "\"score\":");
        if (p && p + 8 < end) {
            p += 8;
            t->entries[t->count].score = atoi(p);
            t->count++;
        } else {
            break;
        }
    }

    free(buf);
    return t->count;
}

int scoring_is_high_score(int score) {
    ScoreTable *t = tbl();
    if (score <= 0) return 0;
    if (t->count < max_scores) return 1;
    return score > t->entries[t->count - 1].score;
}

int scoring_get_rank(int score) {
    ScoreTable *t = tbl();
    for (int i = 0; i < t->count; i++) {
        if (score > t->entries[i].score) return i + 1;
    }
    return t->count + 1;
}
