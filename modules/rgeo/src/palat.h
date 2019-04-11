#ifndef PALAT_H_
#define PALAT_H_

#include "types.h"

/* A PALA texture's resolution.
 *
 * Note: Rally-Sport only supports 16 x 16 PALA textures, so these values are
 * not expected to be altered for the editor, either.*/
#define KP_PALA_WIDTH 16
#define KP_PALA_HEIGHT 16
#define KP_NUM_PIXELS_IN_PALA 256ul

void kp_load_palat_data(const file_handle projFileHandle);
void kp_release_palat_data(void);

/* Getters.*/
const u8* kp_pala(const uint palaIdx);

#endif
