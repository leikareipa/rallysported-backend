/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 * Defines the notion of the interactible, a link between user input and causation.
 * If for instance the user clicks on a 3d object on the screen, the interactible
 * attached to the 3d mesh determines which effect the click will cause.
 *
 */

#ifndef INTERACTIBLE_H
#define INTERACTIBLE_H

enum interactible_type_e
{
    INTERACTIBLE_GROUND,                // Interpret as raiseble/lowerable/paintable ground.
    INTERACTIBLE_PROP,                  // Interpret as a movable track prop.
    INTERACTIBLE_PALATPANE,
    INTERACTIBLE_NONE,                  // Do nothing.
    INTERACTIBLE_IGNORE,                // Mark the holder as transparent, i.e. the system should behave as if the holder doesn't exist when evaluating user interaction.
};

struct interactible_s
{
    interactible_type_e type;
    int params[6];              // An array for custom params (see below).
};

// Indices to the parameters for the particular interactible type.
const int INTERACT_PARAM_GROUND_X = 0;
const int INTERACT_PARAM_GROUND_Z = 1;
const int INTERACT_PARAM_GROUND_X_GLOBAL = 2;
const int INTERACT_PARAM_GROUND_Z_GLOBAL = 3;
const int INTERACT_PARAM_GROUND_TWIN = 4;       // Index to the current frame's scene mesh (before depth sorting) of the second triangle which together with this forms the ground rectangle.
const int INTERACT_PARAM_GROUND_IS_MAIN = 5;

const int INTERACT_PARAM_PROP_IDX = 0;          // Index to the master prop list for this particular prop.
const int INTERACT_PARAM_PROP_FIRST_TRI = 1;    // Index to the current frame's scene mesh (before depth sorting) of the first triangle that belongs to this prop.
const int INTERACT_PARAM_PROP_LAST_TRI = 2;     // Index to the current frame's scene mesh (before depth sorting) of the last triangle that belongs to this prop. (Assumes consecutive organization, i.e.from first to last.)

#endif
