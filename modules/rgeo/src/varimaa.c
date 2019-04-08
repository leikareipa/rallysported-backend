/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 * Deals with Rally-Sport's VARIMAA data, i.e. track tilemaps.
 * 
 */

#include <stdlib.h>
#include "common.h"
#include "varimaa.h"
#include "file.h"

/* A dummy function for now, just to advance the file cursor.*/
void kv_load_varimaa_data(const file_handle projFileHandle)
{
    u32 dataLen = 0; /* Type must be 4 bytes.*/
    u8 *data = NULL;

    kf_read_bytes((u8*)&dataLen, 4, projFileHandle);

    /* For now, read the data in to advance the file cursor, then discard it.*/
    data = (u8*)malloc(dataLen);
    kf_read_bytes(data, dataLen, projFileHandle);
    DEBUG(("Read %u bytes of VARIMAA data into null.", dataLen));
    free(data);

    return;
}
