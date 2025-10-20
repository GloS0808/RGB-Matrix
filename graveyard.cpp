#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <signal.h>
using namespace rgb_matrix;

// Flag to control the main loop
static volatile bool running = true;

// Signal handler for Ctrl+C
void handle_interrupt(int sig) {
    running = false;
}

struct Mist {
    float x;
    float y;
    float speed;
    int width;
    bool active;
};

struct RisingGhost {
    float x;
    float y;
    float speed;
    int phase;
    bool active;
    bool rising;
};

int main() {
    // Set up signal handler for Ctrl+C
    signal(SIGINT, handle_interrupt);

    srand(time(NULL));

    // Matrix setup
    RGBMatrix::Options matrix_options;
    matrix_options.rows = 32;
    matrix_options.cols = 32;
    matrix_options.chain_length = 1;
    matrix_options.parallel = 1;
    matrix_options.hardware_mapping = "adafruit-hat";
    RuntimeOptions rt_opts;
    rt_opts.gpio_slowdown = 2;
    RGBMatrix *matrix = CreateMatrixFromOptions(matrix_options, rt_opts);
    if (!matrix) {
        std::cerr << "Could not initialize RGB matrix." << std::endl;
        return 1;
    }
    FrameCanvas *canvas = matrix->CreateFrameCanvas();

    // Colors
    Color sky_dark(10, 5, 25);          // Very dark purple sky
    Color moon(255, 255, 180);          // Pale moon
    Color ground(15, 10, 5);            // Dark brown ground
    Color grass(20, 30, 10);            // Dead grass
    Color tombstone_gray(100, 100, 110); // Gray tombstone
    Color tombstone_dark(60, 60, 70);   // Dark gray
    Color ghost_white(200, 200, 220);   // Ghostly white
    Color ghost_transparent(150, 150, 170); // Transparent ghost
    Color mist_color(80, 80, 100);      // Purple mist
    Color tree_dead(40, 30, 20);        // Dead tree brown

    // Initialize mist particles
    const int num_mist = 8;
    Mist mist[num_mist];
    for (int i = 0; i < num_mist; ++i) {
        mist[i].x = rand() % 32;
        mist[i].y = 20 + rand() % 10;
        mist[i].speed = 0.05f + (rand() % 5) / 50.0f;
        mist[i].width = 2 + rand() % 3;
        mist[i].active = true;
    }

    // Initialize rising ghosts
    const int num_ghosts = 2;
    RisingGhost ghosts[num_ghosts];
    for (int i = 0; i < num_ghosts; ++i) {
        ghosts[i].x = 8 + i * 16;
        ghosts[i].y = 26;  // Start underground
        ghosts[i].speed = 0.02f;
        ghosts[i].phase = rand() % 360;
        ghosts[i].active = false;
        ghosts[i].rising = false;
    }

    int frame_count = 0;
    int ghost_spawn_timer = 0;

    // Display until interrupted
    while (running) {
        // Clear canvas with night sky
        canvas->Fill(sky_dark.r, sky_dark.g, sky_dark.b);

        // Draw crescent moon (top left)
        for (int y = 2; y <= 8; ++y) {
            for (int x = 3; x <= 9; ++x) {
                int dx = x - 6;
                int dy = y - 5;
                if (dx*dx + dy*dy <= 12) {
                    canvas->SetPixel(x, y, moon.r, moon.g, moon.b);
                }
            }
        }
        // Shadow for crescent
        for (int y = 3; y <= 7; ++y) {
            for (int x = 5; x <= 9; ++x) {
                int dx = x - 7;
                int dy = y - 5;
                if (dx*dx + dy*dy <= 9) {
                    canvas->SetPixel(x, y, sky_dark.r, sky_dark.g, sky_dark.b);
                }
            }
        }

        // Draw ground
        for (int y = 22; y < 32; ++y) {
            for (int x = 0; x < 32; ++x) {
                canvas->SetPixel(x, y, ground.r, ground.g, ground.b);
            }
        }

        // Draw dead grass patches
        for (int x = 0; x < 32; x += 2) {
            int offset = (frame_count / 30 + x) % 3;
            canvas->SetPixel(x + offset, 21, grass.r, grass.g, grass.b);
            canvas->SetPixel(x + offset, 22, grass.r, grass.g, grass.b);
        }

        // Draw dead tree (left side)
        // Trunk
        for (int y = 14; y < 22; ++y) {
            canvas->SetPixel(4, y, tree_dead.r, tree_dead.g, tree_dead.b);
            canvas->SetPixel(5, y, tree_dead.r, tree_dead.g, tree_dead.b);
        }
        // Branches
        for (int x = 2; x <= 7; ++x) {
            canvas->SetPixel(x, 16, tree_dead.r, tree_dead.g, tree_dead.b);
        }
        canvas->SetPixel(2, 15, tree_dead.r, tree_dead.g, tree_dead.b);
        canvas->SetPixel(7, 15, tree_dead.r, tree_dead.g, tree_dead.b);

        // Draw tombstones
        // Tombstone 1 (left)
        for (int y = 20; y <= 26; ++y) {
            for (int x = 8; x <= 11; ++x) {
                canvas->SetPixel(x, y, tombstone_gray.r, tombstone_gray.g, tombstone_gray.b);
            }
        }
        // Rounded top
        canvas->SetPixel(9, 19, tombstone_gray.r, tombstone_gray.g, tombstone_gray.b);
        canvas->SetPixel(10, 19, tombstone_gray.r, tombstone_gray.g, tombstone_gray.b);
        canvas->SetPixel(8, 20, tombstone_gray.r, tombstone_gray.g, tombstone_gray.b);
        canvas->SetPixel(11, 20, tombstone_gray.r, tombstone_gray.g, tombstone_gray.b);
        // Shadow
        for (int y = 20; y <= 26; ++y) {
            canvas->SetPixel(11, y, tombstone_dark.r, tombstone_dark.g, tombstone_dark.b);
        }
        // RIP text
        canvas->SetPixel(9, 22, tombstone_dark.r, tombstone_dark.g, tombstone_dark.b);
        canvas->SetPixel(10, 22, tombstone_dark.r, tombstone_dark.g, tombstone_dark.b);

        // Tombstone 2 (center)
        for (int y = 18; y <= 26; ++y) {
            for (int x = 15; x <= 18; ++x) {
                canvas->SetPixel(x, y, tombstone_gray.r, tombstone_gray.g, tombstone_gray.b);
            }
        }
        // Cross on top
        canvas->SetPixel(16, 16, tombstone_gray.r, tombstone_gray.g, tombstone_gray.b);
        canvas->SetPixel(17, 16, tombstone_gray.r, tombstone_gray.g, tombstone_gray.b);
        canvas->SetPixel(16, 17, tombstone_gray.r, tombstone_gray.g, tombstone_gray.b);
        canvas->SetPixel(17, 17, tombstone_gray.r, tombstone_gray.g, tombstone_gray.b);
        canvas->SetPixel(15, 17, tombstone_gray.r, tombstone_gray.g, tombstone_gray.b);
        canvas->SetPixel(18, 17, tombstone_gray.r, tombstone_gray.g, tombstone_gray.b);
        // Shadow
        for (int y = 18; y <= 26; ++y) {
            canvas->SetPixel(18, y, tombstone_dark.r, tombstone_dark.g, tombstone_dark.b);
        }

        // Tombstone 3 (right, smaller)
        for (int y = 21; y <= 26; ++y) {
            for (int x = 23; x <= 25; ++x) {
                canvas->SetPixel(x, y, tombstone_gray.r, tombstone_gray.g, tombstone_gray.b);
            }
        }
        // Rounded top
        canvas->SetPixel(24, 20, tombstone_gray.r, tombstone_gray.g, tombstone_gray.b);
        // Shadow
        for (int y = 21; y <= 26; ++y) {
            canvas->SetPixel(25, y, tombstone_dark.r, tombstone_dark.g, tombstone_dark.b);
        }

        // Draw mist crawling along ground
        for (int i = 0; i < num_mist; ++i) {
            if (mist[i].active) {
                mist[i].x += mist[i].speed;

                // Wrap around
                if (mist[i].x > 32) {
                    mist[i].x = -mist[i].width;
                    mist[i].y = 20 + rand() % 10;
                }

                // Draw mist wisp
                int mx = (int)mist[i].x;
                int my = (int)mist[i].y;
                for (int w = 0; w < mist[i].width; ++w) {
                    if (mx + w >= 0 && mx + w < 32 && my >= 0 && my < 32) {
                        canvas->SetPixel(mx + w, my, mist_color.r, mist_color.g, mist_color.b);
                        if (my + 1 < 32 && w % 2 == 0) {
                            canvas->SetPixel(mx + w, my + 1, mist_color.r / 2, mist_color.g / 2, mist_color.b / 2);
                        }
                    }
                }
            }
        }

        // Spawn ghosts periodically
        ghost_spawn_timer++;
        if (ghost_spawn_timer > 200) {  // Every ~10 seconds
            for (int i = 0; i < num_ghosts; ++i) {
                if (!ghosts[i].active) {
                    ghosts[i].active = true;
                    ghosts[i].rising = true;
                    ghosts[i].y = 26;
                    ghost_spawn_timer = 0;
                    break;
                }
            }
        }

        // Draw rising ghosts
        for (int i = 0; i < num_ghosts; ++i) {
            if (ghosts[i].active) {
                if (ghosts[i].rising) {
                    ghosts[i].y -= ghosts[i].speed;

                    // Stop rising and start floating
                    if (ghosts[i].y < 12) {
                        ghosts[i].rising = false;
                    }
                } else {
                    // Gentle floating motion
                    float float_y = sin((frame_count + ghosts[i].phase) * 0.05f) * 0.5f;
                    ghosts[i].y = 12 + float_y;
                }

                // Disappear after a while
                if (frame_count % 400 == 0 && !ghosts[i].rising) {
                    ghosts[i].active = false;
                }

                int gx = (int)ghosts[i].x;
                int gy = (int)ghosts[i].y;

                // Draw ghost (semi-transparent effect with alternating pixels)
                Color g_color = (ghosts[i].y > 20) ? ghost_transparent : ghost_white;

                // Ghost head
                if (gy >= 0 && gy < 32) {
                    canvas->SetPixel(gx, gy, g_color.r, g_color.g, g_color.b);
                    if (gx - 1 >= 0) canvas->SetPixel(gx - 1, gy + 1, g_color.r, g_color.g, g_color.b);
                    canvas->SetPixel(gx, gy + 1, g_color.r, g_color.g, g_color.b);
                    if (gx + 1 < 32) canvas->SetPixel(gx + 1, gy + 1, g_color.r, g_color.g, g_color.b);

                    // Body
                    for (int dy = 2; dy < 5; ++dy) {
                        if (gy + dy < 32) {
                            if (gx - 1 >= 0) canvas->SetPixel(gx - 1, gy + dy, g_color.r, g_color.g, g_color.b);
                            canvas->SetPixel(gx, gy + dy, g_color.r, g_color.g, g_color.b);
                            if (gx + 1 < 32) canvas->SetPixel(gx + 1, gy + dy, g_color.r, g_color.g, g_color.b);
                        }
                    }

                    // Eyes (only when fully risen)
                    if (!ghosts[i].rising && gy + 2 < 32) {
                        if (gx - 1 >= 0) canvas->SetPixel(gx - 1, gy + 2, 0, 0, 0);
                        if (gx + 1 < 32) canvas->SetPixel(gx + 1, gy + 2, 0, 0, 0);
                    }
                }
            }
        }

        frame_count++;
        canvas = matrix->SwapOnVSync(canvas);
        usleep(50000);  // ~20 fps
    }

    // Cleanup
    delete matrix;
    std::cout << "Program terminated gracefully." << std::endl;
    return 0;
}
