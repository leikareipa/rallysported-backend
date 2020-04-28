/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: RallySportED-DOS / RGEO
 * 
 */

#ifndef INPUT_H
#define INPUT_H

enum
{
    VIRTUAL_KEY_UP = 0,
    VIRTUAL_KEY_DOWN,
    VIRTUAL_KEY_LEFT,
    VIRTUAL_KEY_RIGHT,

    VIRTUAL_KEY_EXIT,

    // Must be the final item in the list.
    NUM_VIRTUAL_KEYS,
};

// Poll for the current status of user input.
void kinput_update_input(void);

// Returns true if the given key is currently pressed down; flase otherwise.
int kinput_is_key_down(const unsigned virtualKeyCode);

#endif
