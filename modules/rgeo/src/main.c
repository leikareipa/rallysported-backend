/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Software: RallySportED-DOS / RGEO
 * 
 */

#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "common/genstack.h"
#include "assets/mesh.h"
#include "assets/ground.h"
#include "renderer/renderer.h"
#include "renderer/polygon.h"
#include "common/globals.h"
#include "common/input.h"

int main(void)
{
    ktexture_initialize_textures();
    kmesh_initialize_meshes();
    kground_initialize_ground(3);
    krender_initialize();
    krender_use_palette(0);

    time_t startTime = time(NULL);
    unsigned numFrames = 0;

#if MSDOS
    while ((time(NULL) - startTime) < 6)
#else
    while (1)
#endif
    {
        kinput_update_input();

        if (kinput_is_key_down(VIRTUAL_KEY_EXIT)) break;
        
        // Move the camera, for testing purposes.
        {
            static float px = 1;
            static float pz = 1;
            const float movementSpeedX = 0.4;
            const float movementSpeedZ = 0.5;

            if (kinput_is_key_down(VIRTUAL_KEY_RIGHT)) px += movementSpeedX;
            if (kinput_is_key_down(VIRTUAL_KEY_LEFT))  px -= movementSpeedX;
            if (kinput_is_key_down(VIRTUAL_KEY_UP))    pz -= movementSpeedZ;
            if (kinput_is_key_down(VIRTUAL_KEY_DOWN))  pz += movementSpeedZ;

            kground_set_ground_view_offset(px, pz);
            kground_regenerate_ground_view();
        }

        krender_clear_surface();

        // Render the ground.
        {
            const struct kelpo_generic_stack_s *const groundMeshes = kground_ground_view();

            for (unsigned i = 0; i < groundMeshes->count; i++)
            {
                krender_draw_mesh(kelpo_generic_stack__at(groundMeshes, i), 1);
            }
        }

        krender_flip_surface();

        numFrames++;
    }

    DEBUG(("~%d FPS\n", (int)round(numFrames / (float)(time(NULL) - startTime))));

    printf("Press Enter to exit");
    getchar();

    kmesh_release_meshes();
    ktexture_release_textures();
    kground_release_ground();
    krender_release();

    return 0;
}
