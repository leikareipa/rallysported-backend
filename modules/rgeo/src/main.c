/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 */

#include <stdio.h>

int test_file_c(void);
int test_palette_c(void);

int main(int argc, char **argv)
{
    printf("Testing file.c... %s\n", test_file_c()? "Passes." : "FAILS!");
    printf("Testing palette.c... %s\n", test_palette_c()? "Passes." : "FAILS!");

    return 0;
}

#include "palette.h"
#include <string.h>
int test_palette_c(void)
{
    /* The first two colors (RGB) of Rally-Sport's palette #1.*/
    const u8 referenceBytes[6] = {0, 0, 0, 0x2, 0x10, 0x4};

    /* Load Rally-Sport's palette #1.*/
    kpal_initialize_palette(0);

    if (memcmp(kpal_palette(), referenceBytes, 6) != 0)
    {
        return 0;
    }

    return 1;
}

#include "exe_info.h"
#include "file.h"
int test_file_c(void)
{
    const file_handle fh = kf_open_file("RALLYE.EXE", "rb+");
    
    /* Test whether a file of the correct size was opened.*/
    if (!kf_is_active_handle(fh) || kf_file_size(fh) != kexe_rallye_executable_file_size())
    {
        return 0;
    }

    /* Test whether we can properly read data from the file.*/
    {
        u8 referenceBytes[4] = {0x4d, 0x5a, 0x4c, 0x1};
        u8 bytes[4];

        kf_read_bytes(bytes, 4, fh);
        kf_close_file(fh);

        if (memcmp(bytes, referenceBytes, 4) != 0)
        {
            return 0;
        }
    }

    return 1;
}
