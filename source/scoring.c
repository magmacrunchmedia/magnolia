#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scoring.h"
#include "config.h"

static int current_score = 0;
static ScoreEntry scores[MAX_SCORES];
static int score_count = 0;

void scoring_init(void) {
    scoring_load();
}

void scoring_reset(void) {
    current_score = 0;
}

int scoring_get(void) {
    return current_score;
}

void scoring_increment(void) {
    current_score++;
}

int scoring_add_entry(const char *initials, int score) {
    if (score <= 0) return -1;

    int rank = score_count;
    for (int i = 0; i < score_count; i++) {
        if (score > scores[i].score) {
            rank = i;
            break;
        }
    }

    if (rank >= MAX_SCORES) return -1;

    for (int i = score_count; i > rank; i--) {
        if (i < MAX_SCORES) {
            scores[i] = scores[i - 1];
        }
    }

    strncpy(scores[rank].initials, initials, 3);
    scores[rank].initials[3] = '\0';
    scores[rank].score = score;

    if (score_count < MAX_SCORES) score_count++;

    scoring_save();
    return rank;
}

int scoring_get_count(void) {
    return score_count;
}

const ScoreEntry *scoring_get_entry(int index) {
    if (index >= 0 && index < score_count) {
        return &scores[index];
    }
    return NULL;
}

void scoring_save(void) {
    FILE *f = fopen(SCORES_PATH, "w");
    if (!f) return;

    fprintf(f, "{\"scores\":[");
    int count = score_count < MAX_SCORES ? score_count : MAX_SCORES;
    for (int i = 0; i < count; i++) {
        if (i > 0) fprintf(f, ",");
        fprintf(f, "{\"initials\":\"%s\",\"score\":%d}", scores[i].initials, scores[i].score);
    }
    fprintf(f, "]}");
    fclose(f);
}

int scoring_load(void) {
    FILE *f = fopen(SCORES_PATH, "r");
    if (!f) {
        score_count = 0;
        return 0;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0) {
        fclose(f);
        score_count = 0;
        return 0;
    }

    char *buf = (char *)malloc(size + 1);
    if (!buf) {
        fclose(f);
        score_count = 0;
        return 0;
    }
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);

    score_count = 0;
    char *p = buf;
    char *end = buf + size;
    while ((p = strstr(p, "\"initials\":")) != NULL && score_count < MAX_SCORES) {
        if (p + 11 >= end) break;
        p += 11;
        while (*p == '"' && p < end) p++;
        int len = 0;
        while (p + len < end && p[len] != '"' && len < 3) len++;
        if (len > 0 && len <= 3) {
            memcpy(scores[score_count].initials, p, len);
            scores[score_count].initials[len] = '\0';
        } else {
            strcpy(scores[score_count].initials, "???");
        }
        p += len;

        p = strstr(p, "\"score\":");
        if (p && p + 8 < end) {
            p += 8;
            scores[score_count].score = atoi(p);
            score_count++;
        } else {
            break;
        }
    }

    free(buf);
    return score_count;
}

int scoring_is_high_score(int score) {
    if (score <= 0) return 0;
    if (score_count < MAX_SCORES) return 1;
    return score > scores[score_count - 1].score;
}

int scoring_get_rank(int score) {
    for (int i = 0; i < score_count; i++) {
        if (score > scores[i].score) return i + 1;
    }
    return score_count + 1;
}
