/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 */

#include <stdio.h>

int test_file_c(void);

int main(int argc, char **argv)
{
    printf("Hello there.\n");
    printf("Testing file.c... %s\n", test_file_c()? "Passes." : "Fails!");

    return 0;
}

#include "file.h"
#include <string.h>
int test_file_c(void)
{
    const file_handle fh = kf_open_file("RALLYE.EXE", "rb+");
    
    /* Test whether a file of the correct size was opened.*/
    if (!kf_is_active_handle(fh) || kf_file_size(fh) != 133452)
    {
        return 0;
    }

    /* Test whether we can properly read data from the file.*/
    {
        u8 referenceBytes[4] = {0x4d, 0x5a, 0x4c, 0x1};
        u8 bytes[4];

        kf_read_bytes(bytes, 4, fh);
        if (memcmp(bytes, referenceBytes, 4) != 0)
        {
            return 0;
        }
    }

    return 1;
}
