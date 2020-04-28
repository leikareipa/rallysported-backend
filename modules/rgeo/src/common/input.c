/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: RallySportED-DOS / RGEO
 * 
 */

#include <stdint.h>
#include "common/input.h"

#if MSDOS
#else
    #include "SDL2/SDL.h"
#endif

// For each keyboard key, a 1 if the key is currently down, and 0 otherwise.
static uint8_t KEYPRESS[NUM_VIRTUAL_KEYS];

void kinput_update_input(void)
{
    #if MSDOS
    #else
        for (SDL_Event event; SDL_PollEvent(&event);)
        {
            switch (event.type)
            {
                case SDL_KEYDOWN:
                {
                    const unsigned keycode = ((SDL_KeyboardEvent*)&event)->keysym.scancode;

                    switch (keycode)
                    {
                        case SDL_SCANCODE_E: KEYPRESS[VIRTUAL_KEY_UP] = 1; break;
                        case SDL_SCANCODE_S: KEYPRESS[VIRTUAL_KEY_LEFT] = 1; break;
                        case SDL_SCANCODE_D: KEYPRESS[VIRTUAL_KEY_DOWN] = 1; break;
                        case SDL_SCANCODE_F: KEYPRESS[VIRTUAL_KEY_RIGHT] = 1; break;
                        case SDL_SCANCODE_ESCAPE: KEYPRESS[VIRTUAL_KEY_EXIT] = 1; break;
                        default: break;
                    }

                    break;
                }
                case SDL_KEYUP:
                {
                    const unsigned keycode = ((SDL_KeyboardEvent*)&event)->keysym.scancode;

                    switch (keycode)
                    {
                        case SDL_SCANCODE_E: KEYPRESS[VIRTUAL_KEY_UP] = 0; break;
                        case SDL_SCANCODE_S: KEYPRESS[VIRTUAL_KEY_LEFT] = 0; break;
                        case SDL_SCANCODE_D: KEYPRESS[VIRTUAL_KEY_DOWN] = 0; break;
                        case SDL_SCANCODE_F: KEYPRESS[VIRTUAL_KEY_RIGHT] = 0; break;
                        case SDL_SCANCODE_ESCAPE: KEYPRESS[VIRTUAL_KEY_EXIT] = 0; break;
                        default: break;
                    }

                    break;
                }
                default: break;
            }
        }
    #endif

    return;
}

int kinput_is_key_down(const unsigned virtualKeyCode)
{
    if (virtualKeyCode >= NUM_VIRTUAL_KEYS)
    {
        return 0;
    }

    return KEYPRESS[virtualKeyCode];
}
