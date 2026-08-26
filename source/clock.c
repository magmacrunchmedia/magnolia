#include <gccore.h>
#include <ogc/lwp_watchdog.h>
#include "clock.h"
#include "timestep.h"

static int   frame = 0;
static u64   last_ticks = 0;
static float dt = 0.0f;
static float elapsed = 0.0f;

void clock_reset(void) {
    frame = 0;
    last_ticks = gettime();
    dt = 0.0f;
    elapsed = 0.0f;
    timestep_reset();
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

    /* Fed the same clamped delta the rest of the engine sees, so a stalled frame
       is one decision rather than two that could disagree. Costs nothing when no
       game has asked for a fixed step. */
    timestep_advance(dt);
}

int   clock_frame(void)   { return frame; }
float clock_dt(void)      { return dt; }
float clock_elapsed(void) { return elapsed; }

/* Straight through to timestep.c, which is where the arithmetic lives so that it
   can be tested without a console. */
void  clock_set_fixed_hz(int hz) { timestep_set_hz(hz); }
int   clock_fixed_hz(void)       { return timestep_hz(); }
float clock_fixed_dt(void)       { return timestep_dt(); }
int   clock_fixed_steps(void)    { return timestep_steps(); }

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
