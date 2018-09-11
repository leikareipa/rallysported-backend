/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED common routines for Win32 displays
 *
 */

#include "../../../dendrons/display/win32/display_win32_common.h"
#include "../../../core/display.h"
#include "../../../core/common.h"
#include "../../../core/render.h"
#include "../../../core/main.h"
#include "../../../core/ui.h"

static char WINDOW_CLASS_NAME[] = "RSED_GEO";
static char WINDOW_TITLE[] = "\"RallySportED\" by Tarpeeksi Hyvae Soft";

static HWND WINDOW_HANDLE = 0;

static uint WINDOW_WIDTH = 640;
static uint WINDOW_HEIGHT = 400;

// For each of the keyboard's keys, whether the particular key is currently held down.
static bool KEY_DOWN[256] = {false};

static bool WINDOW_ACTIVE = false;

LRESULT CALLBACK kdc_win32_common_message_handler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static const HCURSOR normalCursor = LoadCursor(GetModuleHandle(NULL), IDC_ARROW);

	if (!kwin32_specific_message_handler(hWnd, message, wParam, lParam))
	{
		switch (message)
		{
			// Mouse button interaction.
			case WM_LBUTTONDOWN: { kui_set_mouse_button_left_pressed(true); break; }
			case WM_RBUTTONDOWN: { kui_set_mouse_button_right_pressed(true); break; }
			case WM_MBUTTONDOWN: { kui_set_mouse_button_middle_pressed(true); break; }
			case WM_LBUTTONUP: { kui_set_mouse_button_left_pressed(false); break; }
			case WM_RBUTTONUP: { kui_set_mouse_button_right_pressed(false); break; }
			case WM_MBUTTONUP: { kui_set_mouse_button_middle_pressed(false); break; }

			// Hide the cursor inside the window area.
			case WM_SETCURSOR:
			{
				if (LOWORD(lParam) == HTCLIENT)
				{
					SetCursor(NULL);
				}
				else
				{
					SetCursor(normalCursor);
				}

				break;
			}

		    case WM_ACTIVATE:
		    {
		        if (!HIWORD(wParam))
		        {
		            WINDOW_ACTIVE = true;
		        }
		        else
		        {
		            WINDOW_ACTIVE = false;
		        }

		        break;
		    }

			case WM_MOUSEMOVE:
			{
				const uint mX = GET_X_LPARAM(lParam) / (real)WINDOW_WIDTH * 100;
				const uint mY = GET_Y_LPARAM(lParam) / (real)WINDOW_HEIGHT * 100;

				kui_set_cursor_pos(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam),
									  mX, mY);

				break;
			}

		    case WM_DESTROY:
		    {
		        PostQuitMessage(0);

		        kmain_request_program_exit(EXIT_SUCCESS);

		        break;
		    }

			// Keep track of which keys are being pressed.
	        case WM_KEYDOWN:
		    {
		        KEY_DOWN[wParam] = true;

		        break;
		    }
		    /*case WM_KEYUP:
		    {
		        KEY_DOWN[wParam] = false;

		        break;
		    }*/

		    default:
		    {
		         return DefWindowProc(hWnd, message, wParam, lParam);
		    }
		}
	}

   return 0;
}

bool kdc_is_window_active(void)
{
	return WINDOW_ACTIVE;
}

HWND kdc_window_handle(void)
{
	return WINDOW_HANDLE;
}

void kdc_update_keypress_info(void)
{
	if (KEY_DOWN[VK_SHIFT])
	{
		kui_set_key_pressed(INPUT_KEY_WIREFRAME);
	}
	else if (KEY_DOWN[VK_F1])
	{
		kui_set_key_pressed(INPUT_KEY_PAINT);
	}
	else if (KEY_DOWN[VK_F5])
	{
		kui_set_key_pressed(INPUT_KEY_REFRESH);
	}
	else if (KEY_DOWN[VK_F2])
	{
		kui_set_key_pressed(INPUT_KEY_PALA);
	}
	else if (KEY_DOWN[VK_TAB])
	{
		kui_set_key_pressed(INPUT_KEY_PANE);
	}
	else if (KEY_DOWN[0x30] ||
			 KEY_DOWN[VK_NUMPAD0])
	{
		kui_set_key_pressed(INPUT_KEY_EXIT);
	}
	else if (KEY_DOWN[VK_ESCAPE])
	{
		kui_set_key_pressed(INPUT_KEY_ESC);
	}

	else if (KEY_DOWN[0x31])
	{
		kui_set_key_pressed(INPUT_KEY_1);
		//kui_set_brush_size(0);
	}
	else if (KEY_DOWN[0x32])
	{
		kui_set_key_pressed(INPUT_KEY_2);
		//kui_set_brush_size(1);
	}
	else if (KEY_DOWN[0x33])
	{
		kui_set_key_pressed(INPUT_KEY_3);
		//kui_set_brush_size(2);
	}
	else if (KEY_DOWN[0x34])
	{
		kui_set_key_pressed(INPUT_KEY_4);
		//kui_set_brush_size(3);
	}
	else if (KEY_DOWN[0x35])
	{
		kui_set_key_pressed(INPUT_KEY_5);
	}
	else if (KEY_DOWN[VK_SPACE])
	{
		kui_set_key_pressed(INPUT_KEY_SPACE);
	}
	else if (KEY_DOWN[0x4B])
	{
		kui_set_key_pressed(INPUT_KEY_SAVE);
	}
	else if (KEY_DOWN[0x57])
	{
		kui_set_key_pressed(INPUT_KEY_WATER);
	}
	else if (KEY_DOWN[0x43])
	{
		kui_set_key_pressed(INPUT_KEY_CAMERA);
	}
	else if (KEY_DOWN[0x46])
	{
		kui_set_key_pressed(INPUT_KEY_FPS);
	}
	else if (KEY_DOWN[0x43])
	{
		kui_set_key_pressed(INPUT_KEY_CAMERA);
	}

	// We've processed the new keypresses (or one of them anyway), so reset everything.
	memset(KEY_DOWN, 0, NUM_ELEMENTS(KEY_DOWN));

	return;
}

void kdc_create_window(const resolution_s &r)
{
	const DWORD dwStyle = WS_CAPTION | WS_BORDER | WS_SYSMENU;
    const HINSTANCE hInstance = GetModuleHandle(NULL);

	WNDCLASSA wc;
	RECT rc;

    WINDOW_WIDTH = r.w;
    WINDOW_HEIGHT = r.h;

    // Register the window class.
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.lpfnWndProc = kdc_win32_common_message_handler;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = NULL;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClassA(&wc);

    rc.left = 0;
    rc.top = 0;
    rc.right = WINDOW_WIDTH;
    rc.bottom = WINDOW_HEIGHT;
    AdjustWindowRect(&rc, dwStyle, FALSE);

    // Create the window.
    WINDOW_HANDLE = CreateWindowA(WINDOW_CLASS_NAME,
                                  WINDOW_TITLE,
                                  dwStyle,
                                  CW_USEDEFAULT,
                                  CW_USEDEFAULT,
                                  rc.right - rc.left,
                                  rc.bottom - rc.top,
                                  NULL,
                                  NULL,
                                  hInstance,
                                  NULL);

	k_assert((WINDOW_HANDLE != NULL),
			 "Failed to create a window for the program. Can't continue.");

	return;
}

resolution_s kd_display_resolution(void)
{
    const resolution_s r = {WINDOW_WIDTH, WINDOW_HEIGHT, 32};

    return r;
}

void kd_show_headless_info_message(const char *const msg)
{
	MessageBox(NULL, msg, "A message from RallySportED", MB_OK | MB_ICONINFORMATION);

    return;
}

void kd_show_headless_warning_message(const char *const msg)
{
	MessageBox(NULL, msg, "A warning from RallySportED", MB_OK | MB_ICONWARNING);

    return;
}

void kd_show_headless_assert_error_message(const char *const msg)
{
	char fullMsg[1024];
	snprintf(fullMsg, NUM_ELEMENTS(fullMsg), "RallySportED has come across an unexpected condition in its code that it has "
               						 "not yet been programmed to deal with. As a precaution, the program will shut itself "
               						 "down now.\n\nThe following additional information was provided:\n\""
               						 "%s\"\n\nIf you ran the program from a console window, further "
               						 "diagnostics should appear there after termination.", msg);
	MessageBox(NULL, fullMsg, "RallySportED Assertion Error", MB_OK | MB_ICONERROR);

    return;
}

void kd_set_display_palette(const color_rgb_s *const p)
{
    return;
}

vector2<real> kd_cursor_position(void)
{
    vector2<real> c = {100, 100};
	POINT p;
	if (!GetCursorPos(&p) ||
		!ScreenToClient(WINDOW_HANDLE, &p))
	{
		ERRORI(("Failed to obtain the current cursor position in the window."));
		return c;
	}

	c.x = p.x;
	c.y = p.y;

    return c;
}

