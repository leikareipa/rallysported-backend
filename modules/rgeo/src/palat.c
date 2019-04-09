/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Deals with Rally-Sport's PALAT data, i.e. track textures.
 * 
 */

#include <stdlib.h>
#include "common.h"
#include "palat.h"
#include "file.h"

/* All of the current project's PALA data as one big array. Each PALA texture
 * is a 16 x 16 slice in this array.*/
static u8* PALAT_DATA = NULL;

/* The maximum number of PALA textures we can load in per project. The PALAT_DATA
 * array will hold no more than this number of PALA textures.*/
#define MAX_NUM_PALAS 253

#define PALA_WIDTH 16
#define PALA_HEIGHT 16
const uint NUM_PIXELS_IN_PALA = (PALA_WIDTH * PALA_HEIGHT);

void kp_release_palat_data(void)
{
    free(PALAT_DATA);

    return;
}

/* Loads the PALAT data from the given RallySportED project file.
 *
 * NOTE: The file's cursor is expected to be positioned at the beginning of the
 * PALAT data when calling this function - that is, the first four bytes read
 * from the file should give the length of the PALAT data in the file, and the
 * subsequent n bytes should be those data.
 * 
 * When this function returns, the file's cursor will be positioned at the byte
 * immediately following the end of the PALAT data.*/
void kp_load_palat_data(const file_handle projFileHandle)
{
    u32 dataLen = 0; /* Type must be 4 bytes.*/
    u32 skipNumBytes = 0;
    const uint maxDataLen = (MAX_NUM_PALAS * NUM_PIXELS_IN_PALA);

    k_assert((PALAT_DATA == NULL), "Was asked to re-load PALAT data.");

    kf_read_bytes((u8*)&dataLen, 4, projFileHandle);
    if (dataLen > maxDataLen)
    {
        skipNumBytes = (dataLen - maxDataLen);
        dataLen = maxDataLen;
    }

    PALAT_DATA = (u8*)malloc(dataLen);
    kf_read_bytes(PALAT_DATA, dataLen, projFileHandle);
    DEBUG(("Read %u bytes of PALAT data into null.", dataLen));

    /* Guarantee that the file's cursor will be positioned one byte past the
     * end of the PALAT data.*/
    if (skipNumBytes) kf_jump(skipNumBytes, projFileHandle);

    return;
}
