/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 */

#include <stdio.h>
#include "exe_info.h"
#include "project.h"
#include "common.h"
#include "file.h"

int test_file_c(void);
int test_palette_c(void);
int test_project_c(void);

int main(int argc, char **argv)
{
    printf("Testing file.c... %s\n", test_file_c()? "Passes." : "FAILS!");
    printf("Testing palette.c... %s\n", test_palette_c()? "Passes." : "FAILS!");
    printf("Testing project.c... %s\n", test_project_c()? "Passes." : "FAILS!");

    DEBUG(("All done. Bye.\n"));
    return 0;
}

#include "project.h"
int test_project_c(void)
{
    if (!kproj_is_valid_project_name("VALIDNAM")) return 0; /* Up to 8 characters in range A-Z.*/
    if (kproj_is_valid_project_name("V LIDNAM")) return 0; /* Spaces aren't allowed.*/
    if (kproj_is_valid_project_name("V4L1DN4M")) return 0; /* Numbers aren't allowed.*/
    if (kproj_is_valid_project_name("NOTAVALIDNAME")) return 0; /* Too long.*/

    if (kproj_create_project_file_for_track(KEXE_NUM_TRACKS, "HELLO") != KF_INVALID_HANDLE) return 0; /* Track index out of bounds.*/
    if (kproj_create_project_file_for_track(KEXE_NUM_TRACKS-1, "HELLOOOOO") != KF_INVALID_HANDLE) return 0; /* Invalid track name (too long).*/

    return 1;
}

#include "palette.h"
#include <string.h>
int test_palette_c(void)
{
    /* The first two colors (RGB) of Rally-Sport's palette #1.*/
    const u8 referenceBytes[6] = {0, 0, 0, 0x2, 0x10, 0x4};

    /* Load Rally-Sport's palette #1.*/
    kpal_initialize_palette(0);

    if (memcmp(kpal_palette(), referenceBytes, 6) != 0) return 0;

    return 1;
}

int test_file_c(void)
{
    const file_handle fh = kf_open_file("RALLYE.EXE", "rb+");
    
    /* Test whether a file of the correct size was opened.*/
    if (!kf_is_valid_handle(fh) || kf_file_size(fh) != kexe_rallye_executable_file_size())
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
