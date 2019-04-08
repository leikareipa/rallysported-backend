/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Deals with Rally-Sport's MAASTO data, i.e. track heightmaps.
 * 
 */

#include <stdlib.h>
#include "common.h"
#include "maasto.h"
#include "file.h"

/* A dummy function for now, just to advance the file cursor.*/
void km_load_maasto_data(const file_handle projFileHandle)
{
    u32 dataLen = 0; /* Type must be 4 bytes.*/
    u8 *data = NULL;

    kf_read_bytes((u8*)&dataLen, 4, projFileHandle);

    /* For now, read the data in to advance the file cursor, then discard it.*/
    data = (u8*)malloc(dataLen);
    kf_read_bytes(data, dataLen, projFileHandle);
    DEBUG(("Read %u bytes of MAASTO data into null.", dataLen));
    free(data);

    return;
}
