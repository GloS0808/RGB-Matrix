#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <cstdlib>
#include <ctime>
#include <cmath> // Added for sin and M_PI
using namespace rgb_matrix;

// Flag to control the main loop
static volatile bool running = true;

// Signal handler for Ctrl+C and SIGTERM
static void InterruptHandler(int signo) {
    running = false;
}

int main() {
    // Seed random number generator
    srand(time(NULL));

    // Set up signal handlers
    signal(SIGINT, InterruptHandler);
    signal(SIGTERM, InterruptHandler);

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
    Color bg_dark(15, 10, 20);           // Dark lab background
    Color bg_storm(10, 10, 30);          // Stormy night background
    Color rain_blue(50, 50, 100);        // Rain drops
    Color cloud_gray(80, 80, 100);       // Clouds
    Color skin_green(140, 160, 120);     // Greenish pale skin
    Color skin_shadow(100, 120, 90);     // Shadow areas
    Color skin_highlight(160, 180, 140); // Lighter skin for highlights
    Color hair_black(20, 20, 25);        // Black hair
    Color hair_gray(50, 50, 60);         // Gray for hair texture
    Color bolt_metal(180, 180, 190);     // Metal bolts
    Color bolt_dark(120, 120, 130);      // Bolt shadows
    Color bolt_highlight(220, 220, 230); // Bolt highlights
    Color scar_red(160, 80, 80);         // Reddish scars
    Color scar_bright(180, 100, 100);    // Brighter scar for pulsing
    Color teeth_yellow(230, 220, 180);   // Yellowish teeth
    Color pupil_black(10, 10, 15);       // Eye pupils
    Color eye_white(240, 240, 230);      // Eye whites
    Color lightning_flash(220, 220, 255); // Lightning effect
    Color coat_dark(40, 30, 50);         // Dark coat for back view

    int frame_count = 0;
    int scene = 0; // 0: wide back shot, 1: back of head, 2: detailed face, 3: lightning/sparkle
    const int scene_duration = 200; // ~10 seconds per scene at 20 fps

    struct RainDrop {
        int x;
        float y;
        float speed;
    };
    const int num_rain = 20;
    RainDrop rain[num_rain];
    for (int i = 0; i < num_rain; ++i) {
        rain[i].x = rand() % 32;
        rain[i].y = -(rand() % 32);
        rain[i].speed = 0.5f + (rand() % 5) / 10.0f;
    }

    // Display until interrupted
    while (running) {
        // Clear canvas
        canvas->Fill(bg_dark.r, bg_dark.g, bg_dark.b);

        // Scene switching
        if (frame_count % scene_duration == 0 && frame_count > 0) {
            scene = (scene + 1) % 4;
        }

        // Common lightning flash effect (occasional, more intense in scene 3)
        bool lightning = (frame_count % 120) < 3;
        if (lightning) {
            int num_flashes = (scene == 3) ? 30 : 15;
            for (int i = 0; i < num_flashes; ++i) {
                int lx = rand() % 32;
                int ly = rand() % 32; // Full screen in scene 3
                if (scene != 3) ly = rand() % 10; // Top only in other scenes
                canvas->SetPixel(lx, ly, lightning_flash.r, lightning_flash.g, lightning_flash.b);
            }
        }

        int cx = 16;  // center x

        if (scene == 0) {
            // Scene 1: Wide shot from back on stormy night
            canvas->Fill(bg_storm.r, bg_storm.g, bg_storm.b);

            // Clouds
            for (int y = 0; y < 12; ++y) {
                for (int x = 0; x < 32; ++x) {
                    if ((x + y * 2) % 5 == 0) {
                        canvas->SetPixel(x, y, cloud_gray.r, cloud_gray.g, cloud_gray.b);
                    }
                }
            }

            // Rain
            for (int i = 0; i < num_rain; ++i) {
                rain[i].y += rain[i].speed;
                if (rain[i].y >= 32) {
                    rain[i].y = 0;
                    rain[i].x = rand() % 32;
                }
                int rx = (int)rain[i].x;
                int ry = (int)rain[i].y;
                if (ry >= 0 && ry < 32) {
                    canvas->SetPixel(rx, ry, rain_blue.r, rain_blue.g, rain_blue.b);
                }
            }

            // Frankenstein from back (silhouette, smaller scale)
            int back_cx = 16;
            int back_cy = 20; // Lower position for wide shot

            // Coat/body
            for (int y = 15; y < 28; ++y) {
                int width = (y < 20) ? 8 : 12; // Wider at bottom
                int start_x = back_cx - width / 2;
                for (int x = start_x; x < start_x + width; ++x) {
                    if (x >= 0 && x < 32) {
                        canvas->SetPixel(x, y, coat_dark.r, coat_dark.g, coat_dark.b);
                    }
                }
            }

            // Head from back
            for (int y = 10; y < 15; ++y) {
                for (int x = 13; x < 19; ++x) {
                    canvas->SetPixel(x, y, hair_black.r, hair_black.g, hair_black.b);
                }
            }

            // Bolts visible from side
            canvas->SetPixel(12, 12, bolt_metal.r, bolt_metal.g, bolt_metal.b);
            canvas->SetPixel(19, 12, bolt_metal.r, bolt_metal.g, bolt_metal.b);

        } else if (scene == 1) {
            // Scene 2: Close on back of head
            canvas->Fill(bg_dark.r, bg_dark.g, bg_dark.b);

            // Back of head (larger)
            int head_cx = 16;
            int head_cy = 16;

            // Hair from back
            for (int y = 6; y < 16; ++y) {
                for (int x = 8; x < 24; ++x) {
                    canvas->SetPixel(x, y, hair_black.r, hair_black.g, hair_black.b);
                }
            }
            // Hair texture
            for (int i = 0; i < 10; ++i) {
                int hx = 8 + rand() % 16;
                int hy = 6 + rand() % 10;
                canvas->SetPixel(hx, hy, hair_gray.r, hair_gray.g, hair_gray.b);
            }

            // Bolts from side view (more visible)
            // Left bolt
            for (int x = 6; x < 9; ++x) {
                canvas->SetPixel(x, 12, bolt_metal.r, bolt_metal.g, bolt_metal.b);
                canvas->SetPixel(x, 13, bolt_metal.r, bolt_metal.g, bolt_metal.b);
            }
            canvas->SetPixel(6, 11, bolt_highlight.r, bolt_highlight.g, bolt_highlight.b);
            canvas->SetPixel(8, 14, bolt_dark.r, bolt_dark.g, bolt_dark.b);

            // Right bolt
            for (int x = 23; x < 26; ++x) {
                canvas->SetPixel(x, 12, bolt_metal.r, bolt_metal.g, bolt_metal.b);
                canvas->SetPixel(x, 13, bolt_metal.r, bolt_metal.g, bolt_metal.b);
            }
            canvas->SetPixel(25, 11, bolt_highlight.r, bolt_highlight.g, bolt_highlight.b);
            canvas->SetPixel(23, 14, bolt_dark.r, bolt_dark.g, bolt_dark.b);

            // Neck visible below
            for (int y = 16; y < 32; ++y) {
                for (int x = 10; x < 22; ++x) {
                    canvas->SetPixel(x, y, skin_green.r, skin_green.g, skin_green.b);
                }
            }

        } else if (scene == 2) {
            // Scene 3: Detailed face
            // FOREHEAD
            for (int y = 8; y < 15; ++y) {
                for (int x = 8; x < 24; ++x) {
                    canvas->SetPixel(x, y, skin_green.r, skin_green.g, skin_green.b);
                }
            }
            // Forehead shading
            for (int x = 9; x < 23; ++x) {
                canvas->SetPixel(x, 8, skin_highlight.r, skin_highlight.g, skin_highlight.b);
                canvas->SetPixel(x, 14, skin_shadow.r, skin_shadow.g, skin_shadow.b);
            }

            // EYES
            bool blink = (frame_count % 200) < 5;
            // Left eye
            for (int y = 15; y < 18; ++y) {
                for (int x = 10; x < 14; ++x) {
                    canvas->SetPixel(x, y, skin_shadow.r, skin_shadow.g, skin_shadow.b);
                }
            }
            canvas->SetPixel(11, 16, eye_white.r, eye_white.g, eye_white.b);
            canvas->SetPixel(12, 16, eye_white.r, eye_white.g, eye_white.b);
            canvas->SetPixel(13, 16, eye_white.r, eye_white.g, eye_white.b);
            if (!blink) {
                canvas->SetPixel(12, 16, pupil_black.r, pupil_black.g, pupil_black.b);
            }

            // Right eye
            for (int y = 15; y < 18; ++y) {
                for (int x = 18; x < 22; ++x) {
                    canvas->SetPixel(x, y, skin_shadow.r, skin_shadow.g, skin_shadow.b);
                }
            }
            canvas->SetPixel(18, 16, eye_white.r, eye_white.g, eye_white.b);
            canvas->SetPixel(19, 16, eye_white.r, eye_white.g, eye_white.b);
            canvas->SetPixel(20, 16, eye_white.r, eye_white.g, eye_white.b);
            if (!blink) {
                canvas->SetPixel(19, 16, pupil_black.r, pupil_black.g, pupil_black.b);
            }

            // Nose
            for (int y = 17; y < 22; ++y) {
                canvas->SetPixel(cx - 1, y, skin_green.r, skin_green.g, skin_green.b);
                canvas->SetPixel(cx, y, skin_shadow.r, skin_shadow.g, skin_shadow.b);
                canvas->SetPixel(cx + 1, y, skin_green.r, skin_green.g, skin_green.b);
            }
            canvas->SetPixel(cx - 2, 19, skin_shadow.r, skin_shadow.g, skin_shadow.b);
            canvas->SetPixel(cx + 2, 19, skin_shadow.r, skin_shadow.g, skin_shadow.b);

            // Mouth
            for (int x = 12; x < 20; ++x) {
                canvas->SetPixel(x, 23, pupil_black.r, pupil_black.g, pupil_black.b);
            }
            // Teeth
            for (int x = 13; x < 19; ++x) {
                if (x % 2 == 0) {
                    canvas->SetPixel(x, 24, teeth_yellow.r, teeth_yellow.g, teeth_yellow.b);
                }
            }

            // Scars (pulsing)
            bool scar_pulse = (frame_count % 60) < 30;
            Color scar_color = scar_pulse ? scar_bright : scar_red;
            // Forehead scar
            for (int x = 9; x < 23; ++x) {
                canvas->SetPixel(x, 12, scar_color.r, scar_color.g, scar_color.b);
            }
            // Stitches
            for (int x = 10; x < 22; x += 3) {
                canvas->SetPixel(x, 11, scar_color.r, scar_color.g, scar_color.b);
                canvas->SetPixel(x, 13, scar_color.r, scar_color.g, scar_color.b);
            }
            // Cheek scar
            for (int y = 18; y < 22; ++y) {
                canvas->SetPixel(10, y, scar_color.r, scar_color.g, scar_color.b);
            }

            // Hair
            for (int y = 4; y < 8; ++y) {
                for (int x = 8; x < 24; ++x) {
                    canvas->SetPixel(x, y, hair_black.r, hair_black.g, hair_black.b);
                }
            }
            // Side hair
            for (int y = 8; y < 15; ++y) {
                canvas->SetPixel(7, y, hair_black.r, hair_black.g, hair_black.b);
                canvas->SetPixel(24, y, hair_black.r, hair_black.g, hair_black.b);
            }
            // Texture
            for (int i = 0; i < 8; ++i) {
                int hx = 8 + rand() % 16;
                int hy = 4 + rand() % 4;
                canvas->SetPixel(hx, hy, hair_gray.r, hair_gray.g, hair_gray.b);
            }

            // Bolts
            canvas->SetPixel(7, 9, bolt_metal.r, bolt_metal.g, bolt_metal.b);
            canvas->SetPixel(7, 10, bolt_metal.r, bolt_metal.g, bolt_metal.b);
            canvas->SetPixel(24, 9, bolt_metal.r, bolt_metal.g, bolt_metal.b);
            canvas->SetPixel(24, 10, bolt_metal.r, bolt_metal.g, bolt_metal.b);
            canvas->SetPixel(6, 9, bolt_highlight.r, bolt_highlight.g, bolt_highlight.b);
            canvas->SetPixel(25, 9, bolt_highlight.r, bolt_highlight.g, bolt_highlight.b);

        } else if (scene == 3) {
            // Scene 4: Lightning and sparkle between head
            // Draw detailed face from scene 2
            // FOREHEAD
            for (int y = 8; y < 15; ++y) {
                for (int x = 8; x < 24; ++x) {
                    canvas->SetPixel(x, y, skin_green.r, skin_green.g, skin_green.b);
                }
            }
            // Forehead shading
            for (int x = 9; x < 23; ++x) {
                canvas->SetPixel(x, 8, skin_highlight.r, skin_highlight.g, skin_highlight.b);
                canvas->SetPixel(x, 14, skin_shadow.r, skin_shadow.g, skin_shadow.b);
            }

            // EYES
            bool blink = (frame_count % 200) < 5;
            // Left eye
            for (int y = 15; y < 18; ++y) {
                for (int x = 10; x < 14; ++x) {
                    canvas->SetPixel(x, y, skin_shadow.r, skin_shadow.g, skin_shadow.b);
                }
            }
            canvas->SetPixel(11, 16, eye_white.r, eye_white.g, eye_white.b);
            canvas->SetPixel(12, 16, eye_white.r, eye_white.g, eye_white.b);
            canvas->SetPixel(13, 16, eye_white.r, eye_white.g, eye_white.b);
            if (!blink) {
                canvas->SetPixel(12, 16, pupil_black.r, pupil_black.g, pupil_black.b);
            }

            // Right eye
            for (int y = 15; y < 18; ++y) {
                for (int x = 18; x < 22; ++x) {
                    canvas->SetPixel(x, y, skin_shadow.r, skin_shadow.g, skin_shadow.b);
                }
            }
            canvas->SetPixel(18, 16, eye_white.r, eye_white.g, eye_white.b);
            canvas->SetPixel(19, 16, eye_white.r, eye_white.g, eye_white.b);
            canvas->SetPixel(20, 16, eye_white.r, eye_white.g, eye_white.b);
            if (!blink) {
                canvas->SetPixel(19, 16, pupil_black.r, pupil_black.g, pupil_black.b);
            }

            // Nose
            for (int y = 17; y < 22; ++y) {
                canvas->SetPixel(cx - 1, y, skin_green.r, skin_green.g, skin_green.b);
                canvas->SetPixel(cx, y, skin_shadow.r, skin_shadow.g, skin_shadow.b);
                canvas->SetPixel(cx + 1, y, skin_green.r, skin_green.g, skin_green.b);
            }
            canvas->SetPixel(cx - 2, 19, skin_shadow.r, skin_shadow.g, skin_shadow.b);
            canvas->SetPixel(cx + 2, 19, skin_shadow.r, skin_shadow.g, skin_shadow.b);

            // Mouth
            for (int x = 12; x < 20; ++x) {
                canvas->SetPixel(x, 23, pupil_black.r, pupil_black.g, pupil_black.b);
            }
            // Teeth
            for (int x = 13; x < 19; ++x) {
                if (x % 2 == 0) {
                    canvas->SetPixel(x, 24, teeth_yellow.r, teeth_yellow.g, teeth_yellow.b);
                }
            }

            // Scars
            bool scar_pulse = (frame_count % 60) < 30;
            Color scar_color = scar_pulse ? scar_bright : scar_red;
            for (int x = 9; x < 23; ++x) {
                canvas->SetPixel(x, 12, scar_color.r, scar_color.g, scar_color.b);
            }
            for (int x = 10; x < 22; x += 3) {
                canvas->SetPixel(x, 11, scar_color.r, scar_color.g, scar_color.b);
                canvas->SetPixel(x, 13, scar_color.r, scar_color.g, scar_color.b);
            }
            for (int y = 18; y < 22; ++y) {
                canvas->SetPixel(10, y, scar_color.r, scar_color.g, scar_color.b);
            }

            // Hair
            for (int y = 4; y < 8; ++y) {
                for (int x = 8; x < 24; ++x) {
                    canvas->SetPixel(x, y, hair_black.r, hair_black.g, hair_black.b);
                }
            }
            for (int i = 0; i < 8; ++i) {
                int hx = 8 + rand() % 16;
                int hy = 4 + rand() % 4;
                canvas->SetPixel(hx, hy, hair_gray.r, hair_gray.g, hair_gray.b);
            }
            for (int y = 8; y < 15; ++y) {
                canvas->SetPixel(7, y, hair_black.r, hair_black.g, hair_black.b);
                canvas->SetPixel(24, y, hair_black.r, hair_black.g, hair_black.b);
            }

            // Bolts
            canvas->SetPixel(7, 9, bolt_metal.r, bolt_metal.g, bolt_metal.b);
            canvas->SetPixel(7, 10, bolt_metal.r, bolt_metal.g, bolt_metal.b);
            canvas->SetPixel(24, 9, bolt_metal.r, bolt_metal.g, bolt_metal.b);
            canvas->SetPixel(24, 10, bolt_metal.r, bolt_metal.g, bolt_metal.b);
            canvas->SetPixel(6, 9, bolt_highlight.r, bolt_highlight.g, bolt_highlight.b);
            canvas->SetPixel(25, 9, bolt_highlight.r, bolt_highlight.g, bolt_highlight.b);

            // Sparkle (arc) between head bolts
            if (rand() % 100 < 10) { // 10% chance for arc spark
                int arc_steps = 5;
                for (int step = 0; step < arc_steps; ++step) {
                    float t = step / (float)(arc_steps - 1);
                    int ax = 7 + (int)(17.0f * t); // From left to right bolt
                    int ay = 9 - (int)(sin(t * M_PI) * 4.0f); // Arc shape
                    int noise = rand() % 3 - 1; // Jaggedness
                    ay += noise;
                    if (ax >= 0 && ax < 32 && ay >= 0 && ay < 32) {
                        canvas->SetPixel(ax, ay, lightning_flash.r, lightning_flash.g, lightning_flash.b);
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
