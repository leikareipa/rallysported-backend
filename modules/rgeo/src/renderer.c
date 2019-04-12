/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 */

#include <string.h>
#include "renderer.h"
#include "common.h"
#include "palat.h"

#if __DMC__
    /* Digital Mars C/C++.*/
    #include <bios.h>
    #include <dos.h>
#elif __GNUC__
    /* Assume this is a modern version of GCC that should just ignore the DOS stuff.*/
    union REGS{struct{int ah; int al;} h;};
    #define int86(x,y,z) (void)(x); (void)(y); (void)(z);
#else
    #include <bios.h>
    #include <dos.h>
#endif

/* Pointer to the beginning of video memory in VGA mode 13h.*/
u8 *const MODE_13H_VRAM = (u8*)0xA0000000L;

/* Index into pixel position x,y in 1d video memory in VGA mode 13h, i.e. a
 * pixel buffer of 320 x 200 in size.*/
#define MODE_13H_XY_OFFS(x, y) (uint)(((uint)(x)) + (((uint)(y)) << 6) + (((uint)(y)) << 8))

/* The video mode we were in prior to entering mode 13h.*/
static int ORIGINAL_VIDEO_MODE = 3;

/* Set to true if we're in VGA mode 13h; otherwise false.*/
static int IN_MODE_13H = 0;

int kr_current_video_mode(void)
{
    union REGS regs;
    regs.h.ah = 0xf;
    regs.h.al = 0;

    int86(0x10, &regs, &regs);

    return regs.h.al;
}

/* A temporary function to test-drive rendering to video memory.*/
void kr_draw_pala(const uint palaIdx)
{
    k_assert(IN_MODE_13H, "Expected to be in VGA mode 13h for rendering.");

    memset(MODE_13H_VRAM, 0, 320*200);

    {
        uint i;
        const u8* pala = kp_pala(palaIdx);

        for (i = 0; i < KP_NUM_PIXELS_IN_PALA; (i++, pala++))
        { 
            MODE_13H_VRAM[MODE_13H_XY_OFFS(i&0xf, i>>4)] = *pala;
        }
    }

    return;
}

/* Switch to VGA mode 13h. Returns true if succeeds, false otherwise.*/
int kr_enter_video_mode_13(void)
{
    const int videoModeNow = kr_current_video_mode();
    k_assert((videoModeNow != 0x13), "Trying to enter mode 13h from mode 13h.");

    ORIGINAL_VIDEO_MODE = videoModeNow;

    {
        union REGS regs;
        regs.h.ah = 0;
        regs.h.al = 0x13;

        int86(0x10, &regs, &regs);

        IN_MODE_13H = (kr_current_video_mode() == 0x13);
    }

    return IN_MODE_13H;
}

/* Revert back to whatever video mode we were in before entring mode 13h.
 * Returns true if succeeds, false otherwise.*/
int kr_leave_video_mode_13h(void)
{
    union REGS regs;
    regs.h.ah = 0;
    regs.h.al = ORIGINAL_VIDEO_MODE;

    int86(0x10, &regs, &regs);

    IN_MODE_13H = (kr_current_video_mode() == 0x13);
    
    return (kr_current_video_mode() == ORIGINAL_VIDEO_MODE);
}
