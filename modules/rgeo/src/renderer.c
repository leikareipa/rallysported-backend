#include "renderer.h"
#include "types.h"

#if __DMC__
    /* Digital Mars C/C++.*/
    #include <bios.h>
    #include <dos.h>
#elif __GNUC__
    /* Assume this is a modern version of GCC that should just ignore the DOS stuff.*/
    union REGS{struct{int ah; int al;} h;};
    #define int86(x,y,z) (void)(x);(void)(y);(void)(z);
#else
    #include <bios.h>
    #include <dos.h>
#endif

/* The video mode we were in prior to entering mode 13h.*/
static u8 ORIGINAL_VIDEO_MODE = 3;

int current_video_mode(void)
{
    union REGS regs;
    regs.h.ah = 0xF;
    int86(0x10, &regs, &regs);

    return regs.h.al;
}

/* Switch to VGA mode 13h.*/
void krend_enter_video_mode_13(void)
{
    if (current_video_mode() == 0x13) return;

    {
        union REGS regs;
        regs.h.ah = 0;
        regs.h.al = 0x13;
        int86(0x10, &regs, &regs);
    }

    return;
}

/* Revert back to whatever video mode we were in before entring mode 13h.*/
void krend_leave_video_mode_13h(void)
{
    union REGS regs;
    regs.h.ah = 0;
    regs.h.al = ORIGINAL_VIDEO_MODE;
    int86(0x10, &regs, &regs);
    
    return;
}
