/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED Glide 3.x display (Win32)
 *
 */

#include <glide/glide.h>
#include <algorithm>
#include <vector>
#include <deque>

/*
 * TODOS:
 *
 * - handle alt+tab.
 *
 */

#include "../../../../dendrons/display/win32/display_win32_common.h"
#include "../../../../core/palette.h"
#include "../../../../core/display.h"
#include "../../../../core/render.h"
#include "../../../../core/common.h"
#include "../../../../core/palat.h"
#include "../../../../core/types.h"
#include "../../../../core/main.h"
#include "../../../../core/ui.h"

// How many times per second the display is being updated.
static uint FPS = 0;

static HINSTANCE INSTANCE_HANDLE = 0;
static HDC WINDOW_DC = 0;

static GrContext_t GLIDE_RENDER_CONTEXT = 0;

// Target the given FPS by waiting around until the equivalent number of milliseconds
// has passed. Call this function once per frame to achieve the desired result.
//
// NOTE: This functionality is disabled in this Glide version. It uses the SwapBuffers()
//       method instead.
//
void kd_target_fps(const uint targetFPS)
{
    static DWORD timer = timeGetTime();

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
	/// Not used.

	(void)hWnd;
	(void)message;
	(void)wParam;
	(void)lParam;

	return 0;
}

void kd_release_display()
{
	grSstWinClose(GLIDE_RENDER_CONTEXT);

    return;
}

void kd_update_display(void)
{
    MSG m;

    InvalidateRect(kdc_window_handle(), NULL, FALSE);

    while (PeekMessage(&m, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&m);
        DispatchMessage(&m);
    }

    // Ask Glide to draw its back buffer onto the screen.
	grBufferSwap(1);

	if (kdc_is_window_active())
	{
		kdc_update_keypress_info();
	}

    return;
}

void kd_acquire_display(void)
{
	const resolution_s r = kuil_ideal_display_resolution(true);
    kdc_create_window(r);

    ShowWindow(kdc_window_handle(), SW_SHOW);
    SetForegroundWindow(kdc_window_handle());
    SetFocus(kdc_window_handle());

    UpdateWindow(kdc_window_handle());

	grGlideInit();
	grSstSelect(0);
	GLIDE_RENDER_CONTEXT = grSstWinOpen((FxU32)kdc_window_handle(), GR_RESOLUTION_640x480, GR_REFRESH_60Hz, GR_COLORFORMAT_ARGB, GR_ORIGIN_UPPER_LEFT, 2, 1);

	k_assert(GLIDE_RENDER_CONTEXT != 0, "Failed to initialize Glide.");

    return;
}

