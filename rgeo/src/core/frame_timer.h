/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef FRAME_TIMER_H
#define FRAME_TIMER_H

void kftimer_initialize_frame_timer(void);

void kftimer_release_frame_timer(void);

void kftimer_update(void);

long kftimer_elapsed(void);

#endif
