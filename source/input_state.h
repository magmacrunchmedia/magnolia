#ifndef INPUT_STATE_H
#define INPUT_STATE_H

#include "input.h"

/* The half of input.h that is arithmetic rather than hardware.
 *
 * source/input.c reads WPAD and hands the result to input_state_feed(); every
 * query in input.h is answered from here. The split is the one moonlight-drift
 * made when projection.c came out of playfield.c: edge detection and auto-repeat
 * are exactly the kind of off-by-one-prone counting that wants assertions, and
 * they cannot have them while they live behind a libogc call.
 *
 * Nothing outside the engine and its tests should include this -- games talk to
 * input.h. It is a seam, not a second public API.
 */

/* Clears every player's buttons, edges and hold counters. input_init() calls
   this; a test calls it between cases, since a button left down is the fake
   equivalent of a stuck controller and makes the next case lie. */
void input_state_reset(void);

/* One player's raw held mask for this frame, as a bit per InputButton. Feed
   every player every frame -- a controller that is off is fed (0, 0), not
   skipped, so the frame it goes quiet still produces the release edges for
   whatever its player was holding.

   Edges and repeat counters advance here, once, which is what lets a caller
   poll the same direction twice in a frame and get the same answer both times. */
void input_state_feed(int player, int connected, unsigned short held);

#endif
