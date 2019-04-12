/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 */

#include <string.h>
#include <stdio.h>
#include "exe_info.h"
#include "renderer.h"
#include "palette.h"
#include "project.h"
#include "common.h"
#include "palat.h"
#include "file.h"

int test_file_c(void);
int test_project_c(void);

void initialize_program(void)
{
    /* TODO.*/

    return;
}

/* Asks each relevant unit to release its allocated memory, etc.*/
void release_program(void)
{
    kr_leave_video_mode_13h();
    kp_release_palat_data();

    return;
}

int main(int argc, char **argv)
{
    initialize_program();
    
    /*printf("Testing file.c... %s\n", test_file_c()? "Passes." : "FAILS!");
    printf("Testing project.c... %s\n", test_project_c()? "Passes." : "FAILS!");*/

    /* Test rendering.*/
    {
        kproj_load_data_of_project("HELLO");

        kr_enter_video_mode_13();
        kpal_apply_palette(0);

        kr_draw_pala(3);
        getchar();
    }

    release_program();
    DEBUG(("All done. Bye."));
    return 0;
}

#include "project.h"
int test_project_c(void)
{
    if (!kproj_is_valid_project_name("VALIDNAM")) return 0; /* A valid name.*/
    if (kproj_is_valid_project_name("V LIDNAM")) return 0; /* Spaces aren't allowed.*/
    if (kproj_is_valid_project_name("V4L1DN4M")) return 0; /* Numbers aren't allowed.*/
    if (kproj_is_valid_project_name("NOTAVALIDNAME")) return 0; /* Too long.*/

    if (kproj_create_project_for_track(KEXE_NUM_TRACKS+1, "YYYYYYYY")) return 0; /* Track index out of bounds.*/
    if (kproj_create_project_for_track(KEXE_NUM_TRACKS, "YYYYYYYYY")) return 0; /* Invalid track name (too long).*/

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
