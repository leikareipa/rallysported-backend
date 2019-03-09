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

    printf("\n<Press any key to exit>");
    getchar();
    return 0;
}

#include "file.h"
#include <string.h>
int test_file_c(void)
{
    const file_handle fh = kf_open_file("VARIMAA.001", "rb+");
    
    /* Test whether a file of the correct size was opened.*/
    if (!kf_is_active_handle(fh) || kf_file_size(fh) != 4096)
    {
        return 0;
    }

    /* Test whether we can properly read data from the file.*/
    {
        u8 referenceBytes[4] = {0x3, 0x3, 0x3, 0x2};
        u8 bytes[4];

        kf_read_bytes(bytes, 4, fh);
        if (memcmp(bytes, referenceBytes, 4))
        {
            return 0;
        }
    }

    return 1;
}
