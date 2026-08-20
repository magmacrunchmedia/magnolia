#include <gccore.h>
#include <ogc/lwp_watchdog.h>
#include "clock.h"

static int   frame = 0;
static u64   last_ticks = 0;
static float dt = 0.0f;
static float elapsed = 0.0f;

void clock_reset(void) {
    frame = 0;
    last_ticks = gettime();
    dt = 0.0f;
    elapsed = 0.0f;
}

void clock_tick(void) {
    u64 now = gettime();

    if (last_ticks == 0) {
        /* First tick has no previous frame to measure against. Assume 60Hz
           rather than reporting a zero or a garbage delta, either of which makes
           the first frame of every animation jump. */
        dt = 1.0f / 60.0f;
    } else {
        dt = (float)ticks_to_microsecs(now - last_ticks) / 1000000.0f;
        /* A frame that took longer than a fifth of a second means the game was
           stalled -- loading, or a slow card. Advancing animations by the real
           gap teleports everything; better to lose the time. */
        if (dt > 0.2f) dt = 0.2f;
    }

    last_ticks = now;
    elapsed += dt;
    frame++;
}

int   clock_frame(void)   { return frame; }
float clock_dt(void)      { return dt; }
float clock_elapsed(void) { return elapsed; }

static float clamp01(float t) {
    if (t < 0.0f) return 0.0f;
    if (t > 1.0f) return 1.0f;
    return t;
}

float ease_out_quad(float t) {
    t = clamp01(t);
    return 1.0f - (1.0f - t) * (1.0f - t);
}

float ease_in_out_quad(float t) {
    t = clamp01(t);
    if (t < 0.5f) return 2.0f * t * t;
    float u = -2.0f * t + 2.0f;
    return 1.0f - (u * u) / 2.0f;
}
