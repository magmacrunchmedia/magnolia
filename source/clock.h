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

/* Eased 0..1 progress, for slides, pops and fades. t outside 0..1 is clamped, so
   an animation that overruns settles at its end state rather than flying past
   it. */
float ease_out_quad(float t);
float ease_in_out_quad(float t);

#endif
