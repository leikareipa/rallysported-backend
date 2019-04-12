/*
 * 2019 Tarpeeksi Hyvae Soft
 * 
 */

#include <stdio.h>
#include "renderer.h"
#include "palette.h"
#include "project.h"
#include "common.h"
#include "palat.h"
#include "file.h"

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
    /*printf("Testing file.c... %s\n", kf_test()? "Passes." : "FAILS!");
    printf("Testing project.c... %s\n", kproj_test()? "Passes." : "FAILS!");*/

    initialize_program();

    /* Test rendering.*/
    {
        kproj_load_data_of_project("HELLO");

        kr_enter_video_mode_13h();
        kpal_apply_palette(0);

        kr_draw_pala(3);
        getchar();
    }

    release_program();
    DEBUG(("All done. Bye."));
    return 0;
}
