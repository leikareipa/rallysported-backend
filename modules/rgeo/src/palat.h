#ifndef PALAT_H_
#define PALAT_H_

#include "types.h"

/* A PALA texture's resolution.*/
#define KP_PALA_WIDTH 16
#define KP_PALA_HEIGHT 16

void kp_load_palat_data(const file_handle projFileHandle);
void kp_release_palat_data(void);

/* Getters.*/
const u8* kp_pala(const uint palaIdx);

#endif
