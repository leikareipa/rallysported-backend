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

/* A dummy function for now, just to advance the file cursor.*/
void kp_load_palat_data(const file_handle projFileHandle)
{
    u32 dataLen = 0; /* Type must be 4 bytes.*/
    u32 dataLenExtra = 0;
    u8 *data = NULL;

    kf_read_bytes((u8*)&dataLen, 4, projFileHandle);
    if (dataLen > 65024)
    {
        dataLenExtra = (dataLen - 65024);
        dataLen = 65024;
    }

    /* For now, read the data in to advance the file cursor, then discard it.*/
    data = (u8*)malloc(dataLen);
    kf_read_bytes(data, dataLen, projFileHandle);
    DEBUG(("Read %u bytes of PALAT data into null.", dataLen));
    free(data);

    return;
}
