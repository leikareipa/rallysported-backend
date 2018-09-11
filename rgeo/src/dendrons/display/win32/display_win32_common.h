#ifndef DISPLAY_WIN32_COMMON_H
#define DISPLAY_WIN32_COMMON_H

#include <windows.h>
#include <windowsx.h>

#include "../../../core/common.h"

bool kdc_is_key_down(const u8 keyId);

bool kdc_is_window_active(void);

HWND kdc_window_handle(void);

void kdc_update_keypress_info(void);

void kdc_create_window(const resolution_s  &r);

bool kwin32_specific_message_handler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK kdc_win32_common_message_handler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

#endif
