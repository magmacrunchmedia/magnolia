#ifndef CLOCK_H
#define CLOCK_H

/* Frame counter and elapsed time, ticked once per presented frame.
 *
 * renderer_finish() calls clock_tick(), so any game that presents through the
 * engine gets this without doing anything. Games previously each kept their own
 * frame_count; animations that need to run at the same speed regardless of what
 * the video mode is doing need the seconds, not the frames.
 */

void  clock_tick(void);
int   clock_frame(void);        /* frames since start */
float clock_dt(void);           /* seconds since the previous frame */
float clock_elapsed(void);      /* seconds since start */
void  clock_reset(void);

/* --- Fixed timestep --------------------------------------------------------
 *
 * clock_dt() reports however long the last frame really took, which is right for
 * anything continuous -- a fade, a slide, a starfield. It is wrong for anything
 * whose rules are written in frames. A fighting game says a move has three
 * frames of startup and twelve of recovery; a physics step says a jump arc is
 * the same arc every time. Run those against a delta that varies with SD reads
 * and the numbers stop meaning anything, and a combo that worked on a clean
 * frame drops on a busy one.
 *
 * So the engine offers a fixed step and does not impose one: a game calls
 * clock_set_fixed_hz() once, then each frame runs its logic clock_fixed_steps()
 * times, integrating clock_fixed_dt() rather than clock_dt(). A game that never
 * calls it sees no change at all.
 *
 *     clock_set_fixed_hz(60);
 *     ...
 *     for (int i = 0; i < clock_fixed_steps(); i++) world_step(clock_fixed_dt());
 *     world_draw();                    // once, however many steps ran
 *
 * Drawing stays outside the loop -- stepping is how often the rules run, not how
 * often the screen is painted.
 */
void  clock_set_fixed_hz(int hz);   /* 0, the default, turns it off */
int   clock_fixed_hz(void);
float clock_fixed_dt(void);         /* seconds per step; 0 when off */
int   clock_fixed_steps(void);      /* steps this frame owes; 0 when off */

/* Eased 0..1 progress, for slides, pops and fades. t outside 0..1 is clamped, so
   an animation that overruns settles at its end state rather than flying past
   it. */
float ease_out_quad(float t);
float ease_in_out_quad(float t);

#endif
