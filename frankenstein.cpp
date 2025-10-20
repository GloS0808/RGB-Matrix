#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <cstdlib>
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

    int frame_count = 0;

    // Display until interrupted
    while (running) {
        // Dark background
        canvas->Fill(bg_dark.r, bg_dark.g, bg_dark.b);

        // Lightning flash effect (occasional)
        bool lightning = (frame_count % 120) < 3;
        if (lightning) {
            for (int i = 0; i < 15; ++i) {
                int lx = rand() % 32;
                int ly = rand() % 10;
                canvas->SetPixel(lx, ly, lightning_flash.r, lightning_flash.g, lightning_flash.b);
            }
        }

        // Electric sparks from head bolts
        int bolt_positions[][2] = {{8, 10}, {8, 11}, {23, 10}, {23, 11}};
        if (rand() % 100 < 5) { // 5% chance per frame for sparks
            for (int i = 0; i < 4; ++i) {
                int bx = bolt_positions[i][0];
                int by = bolt_positions[i][1];
                int spark_length = 1 + rand() % 3; // 1-3 pixels
                int direction = rand() % 4; // 0: up, 1: down, 2: left, 3: right

                for (int j = 1; j <= spark_length; ++j) {
                    int sx, sy;
                    if (direction == 0) { // Up
                        sx = bx;
                        sy = by - j;
                    } else if (direction == 1) { // Down
                        sx = bx;
                        sy = by + j;
                    } else if (direction == 2) { // Left
                        sx = bx - j;
                        sy = by;
                    } else { // Right
                        sx = bx + j;
                        sy = by;
                    }

                    if (sx >= 0 && sx < 32 && sy >= 0 && sy < 32) {
                        canvas->SetPixel(sx, sy, lightning_flash.r, lightning_flash.g, lightning_flash.b);
                    }
                }
            }
        }

        // Frankenstein's face (centered)
        int cx = 16;  // center x

        // NECK (bottom)
        for (int y = 28; y < 32; ++y) {
            for (int x = 12; x < 20; ++x) {
                canvas->SetPixel(x, y, skin_shadow.r, skin_shadow.g, skin_shadow.b);
            }
        }
        // Neck highlight for depth
        for (int x = 13; x < 19; ++x) {
            canvas->SetPixel(x, 28, skin_green.r, skin_green.g, skin_green.b);
        }

        // Neck bolts
        // Left bolt
        canvas->SetPixel(11, 28, bolt_metal.r, bolt_metal.g, bolt_metal.b);
        canvas->SetPixel(11, 29, bolt_metal.r, bolt_metal.g, bolt_metal.b);
        canvas->SetPixel(12, 28, bolt_dark.r, bolt_dark.g, bolt_dark.b);
        canvas->SetPixel(12, 29, bolt_dark.r, bolt_dark.g, bolt_dark.b);
        canvas->SetPixel(11, 27, bolt_highlight.r, bolt_highlight.g, bolt_highlight.b); // Highlight

        // Right bolt
        canvas->SetPixel(19, 28, bolt_metal.r, bolt_metal.g, bolt_metal.b);
        canvas->SetPixel(19, 29, bolt_metal.r, bolt_metal.g, bolt_metal.b);
        canvas->SetPixel(20, 28, bolt_dark.r, bolt_dark.g, bolt_dark.b);
        canvas->SetPixel(20, 29, bolt_dark.r, bolt_dark.g, bolt_dark.b);
        canvas->SetPixel(19, 27, bolt_highlight.r, bolt_highlight.g, bolt_highlight.b); // Highlight

        // JAW/LOWER FACE
        for (int y = 23; y < 28; ++y) {
            for (int x = 10; x < 22; ++x) {
                canvas->SetPixel(x, y, skin_green.r, skin_green.g, skin_green.b);
            }
        }
        // Jaw shading
        for (int x = 11; x < 21; ++x) {
            canvas->SetPixel(x, 27, skin_shadow.r, skin_shadow.g, skin_shadow.b);
        }
        // Jaw highlight
        for (int x = 12; x < 20; ++x) {
            canvas->SetPixel(x, 23, skin_highlight.r, skin_highlight.g, skin_highlight.b);
        }

        // Mouth (grim expression, more detailed)
        for (int x = 13; x < 19; ++x) {
            canvas->SetPixel(x, 24, pupil_black.r, pupil_black.g, pupil_black.b);
        }
        // Lip shadow
        canvas->SetPixel(13, 25, skin_shadow.r, skin_shadow.g, skin_shadow.b);
        canvas->SetPixel(18, 25, skin_shadow.r, skin_shadow.g, skin_shadow.b);

        // Teeth showing slightly
        canvas->SetPixel(14, 25, teeth_yellow.r, teeth_yellow.g, teeth_yellow.b);
        canvas->SetPixel(15, 25, teeth_yellow.r, teeth_yellow.g, teeth_yellow.b);
        canvas->SetPixel(16, 25, teeth_yellow.r, teeth_yellow.g, teeth_yellow.b);
        canvas->SetPixel(17, 25, teeth_yellow.r, teeth_yellow.g, teeth_yellow.b);

        // MID FACE / CHEEKS
        for (int y = 17; y < 23; ++y) {
            for (int x = 8; x < 24; ++x) {
                canvas->SetPixel(x, y, skin_green.r, skin_green.g, skin_green.b);
            }
        }
        // Cheek shading
        for (int y = 18; y < 22; ++y) {
            canvas->SetPixel(8, y, skin_shadow.r, skin_shadow.g, skin_shadow.b);
            canvas->SetPixel(23, y, skin_shadow.r, skin_shadow.g, skin_shadow.b);
        }
        // Cheek highlight
        for (int x = 10; x < 22; ++x) {
            canvas->SetPixel(x, 17, skin_highlight.r, skin_highlight.g, skin_highlight.b);
        }

        // Nose (more defined)
        canvas->SetPixel(cx, 19, skin_green.r, skin_green.g, skin_green.b);
        canvas->SetPixel(cx, 20, skin_shadow.r, skin_shadow.g, skin_shadow.b);
        canvas->SetPixel(cx, 21, skin_shadow.r, skin_shadow.g, skin_shadow.b);
        canvas->SetPixel(cx - 1, 20, skin_green.r, skin_green.g, skin_green.b);
        canvas->SetPixel(cx + 1, 20, skin_green.r, skin_green.g, skin_green.b);
        canvas->SetPixel(cx - 1, 19, skin_highlight.r, skin_highlight.g, skin_highlight.b);
        canvas->SetPixel(cx + 1, 19, skin_highlight.r, skin_highlight.g, skin_highlight.b);

        // FOREHEAD
        for (int y = 10; y < 17; ++y) {
            for (int x = 8; x < 24; ++x) {
                canvas->SetPixel(x, y, skin_green.r, skin_green.g, skin_green.b);
            }
        }
        // Forehead shading
        for (int x = 9; x < 23; ++x) {
            canvas->SetPixel(x, 10, skin_highlight.r, skin_highlight.g, skin_highlight.b);
            canvas->SetPixel(x, 16, skin_shadow.r, skin_shadow.g, skin_shadow.b);
        }
        // Forehead sides
        canvas->SetPixel(8, 11, skin_shadow.r, skin_shadow.g, skin_shadow.b);
        canvas->SetPixel(8, 12, skin_shadow.r, skin_shadow.g, skin_shadow.b);
        canvas->SetPixel(23, 11, skin_shadow.r, skin_shadow.g, skin_shadow.b);
        canvas->SetPixel(23, 12, skin_shadow.r, skin_shadow.g, skin_shadow.b);

        // EYES (deep set, intense, with blinking)
        bool blink = (frame_count % 200) < 5; // Blink every ~10 seconds
        // Left eye
        // Eye socket (dark)
        for (int y = 17; y < 20; ++y) {
            for (int x = 10; x < 14; ++x) {
                canvas->SetPixel(x, y, skin_shadow.r, skin_shadow.g, skin_shadow.b);
            }
        }
        // Eye white
        canvas->SetPixel(11, 18, eye_white.r, eye_white.g, eye_white.b);
        canvas->SetPixel(12, 18, eye_white.r, eye_white.g, eye_white.b);
        // Pupil (disappears during blink)
        if (!blink) {
            canvas->SetPixel(11, 18, pupil_black.r, pupil_black.g, pupil_black.b);
        }

        // Right eye
        // Eye socket
        for (int y = 17; y < 20; ++y) {
            for (int x = 18; x < 22; ++x) {
                canvas->SetPixel(x, y, skin_shadow.r, skin_shadow.g, skin_shadow.b);
            }
        }
        // Eye white
        canvas->SetPixel(19, 18, eye_white.r, eye_white.g, eye_white.b);
        canvas->SetPixel(20, 18, eye_white.r, eye_white.g, eye_white.b);
        // Pupil (disappears during blink)
        if (!blink) {
            canvas->SetPixel(20, 18, pupil_black.r, pupil_black.g, pupil_black.b);
        }

        // Heavy brow ridge (prominent forehead)
        for (int x = 10; x < 22; ++x) {
            canvas->SetPixel(x, 16, skin_shadow.r, skin_shadow.g, skin_shadow.b);
        }

        // HAIR (flat top, textured)
        for (int y = 6; y < 10; ++y) {
            for (int x = 8; x < 24; ++x) {
                canvas->SetPixel(x, y, hair_black.r, hair_black.g, hair_black.b);
            }
        }
        // Hair texture (random gray patches)
        for (int i = 0; i < 6; ++i) {
            int hx = 8 + rand() % 16;
            int hy = 6 + rand() % 4;
            canvas->SetPixel(hx, hy, hair_gray.r, hair_gray.g, hair_gray.b);
        }
        // Hair extends down sides
        for (int y = 10; y < 18; ++y) {
            canvas->SetPixel(7, y, hair_black.r, hair_black.g, hair_black.b);
            canvas->SetPixel(24, y, hair_black.r, hair_black.g, hair_black.b);
        }
        // Widow's peak
        canvas->SetPixel(cx, 9, hair_black.r, hair_black.g, hair_black.b);
        canvas->SetPixel(cx - 1, 10, hair_black.r, hair_black.g, hair_black.b);
        canvas->SetPixel(cx + 1, 10, hair_black.r, hair_black.g, hair_black.b);

        // SCARS (stitches across forehead, pulsing)
        bool scar_pulse = (frame_count % 60) < 30; // Pulse every ~3 seconds
        Color scar_color = scar_pulse ? scar_bright : scar_red;
        // Horizontal scar across forehead
        for (int x = 9; x < 23; ++x) {
            canvas->SetPixel(x, 14, scar_color.r, scar_color.g, scar_color.b);
        }
        // Stitch marks (vertical lines)
        for (int x = 10; x < 22; x += 2) {
            canvas->SetPixel(x, 13, scar_color.r, scar_color.g, scar_color.b);
            canvas->SetPixel(x, 15, scar_color.r, scar_color.g, scar_color.b);
        }
        // Vertical scar on left cheek
        canvas->SetPixel(10, 20, scar_color.r, scar_color.g, scar_color.b);
        canvas->SetPixel(10, 21, scar_color.r, scar_color.g, scar_color.b);
        canvas->SetPixel(10, 22, scar_color.r, scar_color.g, scar_color.b);

        // Electrode bolt tops (on head)
        canvas->SetPixel(8, 10, bolt_metal.r, bolt_metal.g, bolt_metal.b);
        canvas->SetPixel(8, 11, bolt_metal.r, bolt_metal.g, bolt_metal.b);
        canvas->SetPixel(23, 10, bolt_metal.r, bolt_metal.g, bolt_metal.b);
        canvas->SetPixel(23, 11, bolt_metal.r, bolt_metal.g, bolt_metal.b);
        canvas->SetPixel(8, 9, bolt_highlight.r, bolt_highlight.g, bolt_highlight.b);  // Highlight
        canvas->SetPixel(23, 9, bolt_highlight.r, bolt_highlight.g, bolt_highlight.b); // Highlight

        frame_count++;
        canvas = matrix->SwapOnVSync(canvas);
        usleep(50000);  // ~20 fps
    }

    // Cleanup
    delete matrix;
    std::cout << "Program terminated gracefully." << std::endl;
    return 0;
}
