/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED OpenGL display (Win32)
 *
 * Based loosely on Jeff 'NeHe' Molofee's Win32 window-creation routines.
 *
 */

#include <gl/glu.h>
#include <gl/gl.h>
#include <algorithm>
#include <numeric>
#include <vector>
#include <deque>
#include <cmath>
#include "../../../../dendrons/display/win32/display_win32_common.h"
#include "../../../../core/palette.h"
#include "../../../../core/display.h"
#include "../../../../core/common.h"
#include "../../../../core/render.h"
#include "../../../../core/palat.h"
#include "../../../../core/types.h"
#include "../../../../core/main.h"
#include "../../../../core/ui.h"

// How many times per second the display is being updated.
static uint FPS = 0;

static HINSTANCE INSTANCE_HANDLE = 0;
static HDC WINDOW_DC = 0;
static HGLRC RENDER_CONTEXT = 0;

// Target the given FPS by waiting around until the equivalent number of milliseconds
// has passed. Call this function once per frame to achieve the desired result.
//
// NOTE: This functionality is disabled in this OpenGL version. It uses the SwapBuffers()
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

static void resize_gl(GLsizei width, GLsizei height)
{
    glViewport(0, 0, width, height);

    glLoadIdentity();
    glTranslatef(0, 0, -1);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, kd_display_resolution().w, 0, kd_display_resolution().h, -1, 8000);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0, kd_display_resolution().h, 0);

    return;
}

// Set starting parameters for OpenGL.
//
static void initialize_gl_state(void)
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glAlphaFunc(GL_GREATER, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);

    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_DENSITY, 0.01);
    glHint(GL_FOG_HINT, GL_DONT_CARE);
    glFogf(GL_FOG_START, 3700.0f);
    glFogf(GL_FOG_END, 4500.0f);

    return;
}

static void set_gl_vsync(const bool vsyncOn)
{
	typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);
	const char *const extensions = (char*)glGetString(GL_EXTENSIONS);

	if (strstr(extensions,"WGL_EXT_swap_control"))
	{
		PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
		wglSwapIntervalEXT(vsyncOn);
		DEBUG(("VSync status: %d.", vsyncOn));
	}
	else
	{
		DEBUG(("The video card doesn't seem to support vsync control. Never mind, then."));
	}

	return;
}

static void shutdown_gl(void)
{
    if (RENDER_CONTEXT != NULL)
    {
        if (!wglMakeCurrent(NULL, NULL))
        {
            ERRORI(("Detected an error while releasing the OpenGL render context."));
        }

        if (!wglDeleteContext(RENDER_CONTEXT))
        {
            ERRORI(("Detected an error while releasing the OpenGL render context."));
        }

        //RENDER_CONTEXT = NULL;
    }

    if (WINDOW_DC != NULL &&
        !ReleaseDC(kdc_window_handle(), WINDOW_DC))
    {
        ERRORI(("Detected an error while releasing the OpenGL window."));
        //WINDOW_DC = NULL;
    }

    if (kdc_window_handle() != NULL &&
        !DestroyWindow(kdc_window_handle()))
    {
        ERRORI(("Detected an error while releasing the OpenGL window."));
        //kdc_window_handle() = NULL;
    }

    return;
}

bool kwin32_specific_message_handler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   return 0;
}

void kd_release_display()
{
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

    // Ask OpenGL to draw its back buffer onto the screen.
    SwapBuffers(WINDOW_DC);

	if (kdc_is_window_active())
	{
		kdc_update_keypress_info();
	}

    return;
}

void kd_acquire_display(void)
{
    GLuint pixelFormat;

	const resolution_s r = kuil_ideal_display_resolution(false);
    kdc_create_window(r);

    static PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        16,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        16,
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    WINDOW_DC = GetDC(kdc_window_handle());
    pixelFormat = ChoosePixelFormat(WINDOW_DC, &pfd);

    if (!SetPixelFormat(WINDOW_DC, pixelFormat, &pfd))
    {
        k_assert(0, "Failed to obtain an OpenGL-compatible display.");
        return;
    }
    RENDER_CONTEXT = wglCreateContext(WINDOW_DC);
    if (!wglMakeCurrent(WINDOW_DC, RENDER_CONTEXT))
    {
        k_assert(0, "Failed to obtain an OpenGL-compatible display.");
        return;
    }

    ShowWindow(kdc_window_handle(), SW_SHOW);
    SetForegroundWindow(kdc_window_handle());
    SetFocus(kdc_window_handle());

    resize_gl(kd_display_resolution().w, kd_display_resolution().h);
    initialize_gl_state();
	set_gl_vsync(true);

    UpdateWindow(kdc_window_handle());

    return;
}

