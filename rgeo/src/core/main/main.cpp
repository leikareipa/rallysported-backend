/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED main entry
 *
 */

#include <sstream>
#include <cstdlib>
#include "../../core/frame_timer.h"
#include "../../core/manifesto.h"
#include "../../core/cmd_line.h"
#include "../../core/game_exe.h"
#include "../../core/geometry.h"
#include "../../core/display.h"
#include "../../core/palette.h"
#include "../../core/common.h"
#include "../../core/camera.h"
#include "../../core/memory.h"
#include "../../core/maasto.h"
#include "../../core/render.h"
#include "../../core/memory.h"
#include "../../core/props.h"
#include "../../core/palat.h"
#include "../../core/text.h"
#include "../../core/main.h"
#include "../../core/file.h"
#include "../../core/ui.h"

// Will be set to !0 when we want to ask the program to exit (there is a specific
// function in this unit that you should call, rather than changing this value
// directly; at the moment, that would be kmain_request_program_exit()).
static int PROGRAM_EXIT_REQUESTED = 0;

static void initialize_all(void)
{
    // Returns a string containing the project's name, appended with the local directory
    // structure and given file extension (don't include the '.' when passing the latter).
    // E.g. "DTA" returns "TRACK/TRACK.DTA".
    auto project_file_name = [](std::string extension)
                             {
                                  std::ostringstream ss;
                                  ss << kcmdl_track_name() << DIR_SEPARATOR << kcmdl_track_name() << "." << extension;
                                  return ss.str();
                             };

    // Hardcoded data.
    kge_acquire_access_to_game_executables("RALLYE.EXE", "VALIKKO.EXE");
    kmanif_apply_manifesto(project_file_name("$FT").c_str(), kge_file_handle_rallye_exe());
    kpal_initialize_palettes();

    // Display and rendering.
    kd_acquire_display();
    kr_acquire_renderer();
    kg_initialize_geometry();

    // Misc.
    kftimer_initialize_frame_timer();
    kui_initialize_ui_logic();
    ktext_initialize_text();

    // Track assets.
    const file_handle_t projFile = kfile_open_file(project_file_name("DTA").c_str(), "rb+");
    kmaasto_initialize_maasto(projFile, kmanif_track_idx());
    kpalat_initialize_palat(projFile);
    kprop_initialize_props();

    kuil_initialize_ui_layout(); // Should be called only after loading all assets, since may depend on some of them.

    kr_report_total_texture_size();

    return;
}

static void cleanup_all(void)
{
    kd_release_display();
    kr_release_renderer();
    kuil_release_ui_layout();
    kg_release_geometry();
    ktext_release_text();
    kui_release_ui_logic();
    kprop_release_props();
    kpalat_release_palat();
    kmaasto_release_maasto();
    kpal_release_palette();
    kftimer_release_frame_timer();

    kmem_deallocate_memory_cache(); // Should be called last, since will free the memory cache that other units may depend on.

    return;
}

// Call to ask the program to terminate, possibly with the given  exit code. That
// the program honors this request at a particular time isn't guaranteed.
//
void kmain_request_program_exit(const int exitCode)
{
    DEBUG(("Main has been asked to terminate the program with exit code %d.", exitCode));

    if (exitCode == EXIT_FAILURE)
    {
        exit(EXIT_FAILURE);
    }

    PROGRAM_EXIT_REQUESTED = true; /// For now, ignore the given exit code and just set to signal a generic, successful exit.

    return;
}

// If the exit condition is !false, the program is about to shut down or may even
// already be in the process of doing so.
//
bool kmain_program_is_exiting(void)
{
    return bool(PROGRAM_EXIT_REQUESTED);
}

int main(int argc, char *argv[])
{
    INFO(("Launching RallySportED/RGEO v.%d_%d.", INTERNAL_VERSION_MAJOR, INTERNAL_VERSION_MINOR));

    DEBUG(("Parsing the command line."));
    if (!kcmdl_parse_command_line(argc, argv))
    {
        return EXIT_FAILURE;
    }

    DEBUG(("Initializing the program."));
    initialize_all();

    // Triangle containers for rendering. With view to performance on fairly old
    // systems, we pre-reserve a reasonable number of elements.
    /// TODO. Get custom containers.
    std::vector<triangle_s> scene;              // 3d view.
    std::vector<triangle_s> transfScene;        // Transformed to screen-space.
    std::vector<triangle_s> uiScene;            // User interface.
    scene.reserve(TRIANGLE_CACHE_SIZE);
    transfScene.reserve(TRIANGLE_CACHE_SIZE);
    uiScene.reserve(UI_TRIANGLE_CACHE_SIZE);

    DEBUG(("Entering the main loop."));
    while (!kmain_program_is_exiting())
    {
        // Prepare states for the new frame.
        kftimer_update();
        kmaasto_clear_highlights();
        kui_process_user_input(transfScene);
        scene.clear();
        transfScene.clear();
        uiScene.clear();

        // Construct the scene's 3d mesh.
        if (!kui_paint_view_is_open() &&
            !kui_texedit_view_is_open())
        {
            kmaasto_add_maasto_tri_mesh(&scene);
        }

        // Once the scene is complete (don't want to add any more triangles),
        // transform it for rendering.
        kg_transform_mesh_to_screen_space(scene, &transfScene);

        // Render the scene.
        kr_clear_frame();
        kr_depth_sort_mesh(&transfScene);
        kr_set_depth_testing_enabled(true);
        kr_rasterize_mesh(transfScene, kui_is_wireframe_on());

        // Create and render the user interface.
        kuil_add_user_interface_tri_mesh(&uiScene);
        kr_set_depth_testing_enabled(false);
        kr_rasterize_mesh(uiScene, false);

        /// Temp hack. Combine the scene with the UI so we get coverage of them both
        /// for mouse-picking.
        transfScene.insert(transfScene.end(), uiScene.begin(), uiScene.end());

        // Update the display with the newly-rendered frame.
        kd_update_display();
        kd_target_fps(60);  // Kludge for when we suspect we don't have refresh-synced rendering. Otherwise will be ignored.
    }

    INFO(("Received orders to exit. Initiating cleanup."));
    cleanup_all();

    INFO(("Bye."));
    return 0;
}
