#ifndef RENDERER_H_
#define RENDERER_H_

#include "types.h"

int kr_enter_video_mode_13(void);
int kr_leave_video_mode_13h(void);

void kr_draw_pala(const uint palaIdx);

#endif
