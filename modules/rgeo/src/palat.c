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

const uint NUM_PIXELS_IN_PALA = (KP_PALA_WIDTH * KP_PALA_HEIGHT);

void kp_release_palat_data(void)
{
    free(PALAT_DATA);
    PALAT_DATA = NULL;

    return;
}

/* Returns a pointer to the master PALAT data such that the subsequent n bytes
 * are the pixels of the given PALA texture (0-indexed).*/
const u8* kp_pala(const uint palaIdx)
{
    k_assert((palaIdx < MAX_NUM_PALAS), "Attempting to access a PALA texture out of bounds.");

    return &PALAT_DATA[palaIdx * NUM_PIXELS_IN_PALA];
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

    k_assert((PALAT_DATA == NULL), "Was asked to re-load PALAT data.");

    /* Get the size of the PALAT data.*/
    {
        const u32 maxDataLen = (MAX_NUM_PALAS * NUM_PIXELS_IN_PALA);
        
        kf_read_bytes((u8*)&dataLen, 4, projFileHandle);

        /* Sanity check. We expect there to never be more than 256 PALA textures
         * in Rally-Sport's PALAT files.*/
        k_assert((dataLen <= (256ul * NUM_PIXELS_IN_PALA)), "Detected an invalid PALAT data length in the project file.");

        if (dataLen > maxDataLen)
        {
            skipNumBytes = (dataLen - maxDataLen);
            dataLen = maxDataLen;
        }
    }

    /* Read the PALAT data from disk to memory.*/
    {
        k_assert(((dataLen % NUM_PIXELS_IN_PALA) == 0), "Detected incomplete PALAT data in the project file.");

        PALAT_DATA = (u8*)malloc(dataLen);
        kf_read_bytes(PALAT_DATA, dataLen, projFileHandle);

        DEBUG(("Read %u bytes of PALAT data into null.", dataLen));
    }

    /* Guarantee that the file cursor will be positioned at the byte immediately
     * following the end of the PALAT data.*/
    if (skipNumBytes) kf_jump(skipNumBytes, projFileHandle);

    return;
}
