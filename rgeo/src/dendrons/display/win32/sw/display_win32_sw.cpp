/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED software renderer display (Win32)
 *
 */

#include <numeric>
#include <vector>
#include <deque>
#include <cmath>
#include "../../../../dendrons/display/win32/display_win32_common.h"
#include "../../../../core/palette.h"
#include "../../../../core/display.h"
#include "../../../../core/memory.h"
#include "../../../../core/common.h"
#include "../../../../core/render.h"
#include "../../../../core/palat.h"
#include "../../../../core/types.h"
#include "../../../../core/ui.h"

static HINSTANCE INSTANCE_HANDLE;
static HDC OFFSCREEN_DC;

// How many times per second the display is being updated.
static uint FPS = 0;

// A buffer we draw the rendered (paletted) image, for drawing it to screen then.
//static color_bgra_s *SCREEN_BUFFER = NULL;
//static u16 *SCREEN_BUFFER_16 = NULL;
static uint WINDOW_BPP = 0;

static const frame_buffer_s *CUR_FRAME_BUFFER = NULL;

// Target the given FPS by waiting around until the equivalent number of milliseconds
// has passed. Call this function once per frame to achieve the desired result.
//
void kd_target_fps(const uint targetFPS)
{
    static DWORD timer = timeGetTime();

    while ((targetFPS != 0) &&
           ((timeGetTime() - timer) < (1000 / targetFPS)))
    {
        Sleep(1);
    }

	static std::deque<uint> fpsList;
    const DWORD timePassed = timeGetTime() - timer;
	const uint curFPS = (timePassed == 0)? 1000 : round(1000.0 / (timePassed));

	fpsList.push_back(curFPS);
	if (fpsList.size() > 30)
	{
		fpsList.pop_front();
	}

	FPS = std::accumulate(fpsList.begin(), fpsList.end(), 0) / fpsList.size();

    timer = timeGetTime();

    return;
}

uint kd_current_fps(void)
{
    return FPS;
}

bool kwin32_specific_message_handler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
		// Draw the latest rendered frame to screen.
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            // Blit the rendered image onto the screen.
            if (CUR_FRAME_BUFFER != NULL)
            {
				k_assert(CUR_FRAME_BUFFER->r.bpp == 32, "Was asked to draw a non-32-bit frame buffer to screen. Don't want to.");

				const HBITMAP frameBitmap = CreateBitmap(CUR_FRAME_BUFFER->r.w, CUR_FRAME_BUFFER->r.h, 1, CUR_FRAME_BUFFER->r.bpp, (void*)CUR_FRAME_BUFFER->canvas);

                if (frameBitmap != NULL)
                {
					HGDIOBJ prevObj;

                    prevObj = SelectObject(OFFSCREEN_DC, frameBitmap);

                    // Resize the image to fit the window, then blit it onto it.
                    StretchBlt(hdc,
                               0,
                               0,
                               kd_display_resolution().w,
                               kd_display_resolution().h,
                               OFFSCREEN_DC,
                               0,
                               0,
                               CUR_FRAME_BUFFER->r.w,
                               CUR_FRAME_BUFFER->r.h,
                               SRCCOPY);

					SelectObject(OFFSCREEN_DC, prevObj);

					DeleteObject(frameBitmap);
                }
            }

            EndPaint(hWnd, &ps);

            return 1;
        }

		default:
		{
			break;
		}
   }

   return 0;
}

void kd_release_display()
{
    DeleteDC(OFFSCREEN_DC);

    return;
}

void kd_update_display(void)
{
    //d_convert_paletted_frame_buffer_to_screen_buffer(fb);

	CUR_FRAME_BUFFER = kr_framebuffer_ptr();

    InvalidateRect(kdc_window_handle(), NULL, FALSE);

	MSG m;
    while (PeekMessage(&m, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&m);
        DispatchMessage(&m);
    }

	if (kdc_is_window_active())
	{
		kdc_update_keypress_info();
	}

    return;
}

void kd_acquire_display(void)
{
	int displayBPP = 0;

	const resolution_s r = kuil_ideal_display_resolution(false);
    kdc_create_window(r);

    ShowWindow(kdc_window_handle(), SW_SHOW);
    UpdateWindow(kdc_window_handle());
    ShowCursor(1);

    OFFSCREEN_DC = CreateCompatibleDC(GetDC(kdc_window_handle()));
	WINDOW_BPP = GetDeviceCaps(OFFSCREEN_DC, BITSPIXEL);
	k_assert(WINDOW_BPP == 32, "Display color depth must be 32 bits.");
	DEBUG(("Obtained a display of %d x %d (%d-bit).", kd_display_resolution().w, kd_display_resolution().h, WINDOW_BPP));

    return;
}

