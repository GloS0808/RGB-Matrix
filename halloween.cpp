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

struct Bat {
    float x;
    float y;
    float speed_x;
    float speed_y;
    int wing_frame;
    bool active;
};

struct Ghost {
    float x;
    float y;
    float float_offset;
    int phase;
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
    Color bg_night(5, 0, 20);           // Dark purple/black night
    Color moon(255, 255, 200);          // Pale yellow moon
    Color ground(20, 10, 0);            // Dark brown ground
    Color pumpkin_orange(255, 120, 0);  // Orange pumpkin
    Color pumpkin_dark(180, 80, 0);     // Dark orange for shading
    Color jack_face(255, 200, 0);       // Glowing jack-o-lantern face
    Color ghost_white(230, 230, 230);   // White ghost
    Color bat_black(50, 50, 50);        // Dark gray bat
    Color grass(10, 40, 0);             // Dark grass

    // Initialize bats
    const int num_bats = 4;
    Bat bats[num_bats];
    for (int i = 0; i < num_bats; ++i) {
        bats[i].x = rand() % 32;
        bats[i].y = rand() % 15;
        bats[i].speed_x = (rand() % 2 == 0 ? 1 : -1) * (0.3f + (rand() % 5) / 10.0f);
        bats[i].speed_y = (rand() % 3 - 1) * 0.1f;
        bats[i].wing_frame = rand() % 10;
        bats[i].active = true;
    }

    // Initialize ghosts
    Ghost ghosts[2];
    ghosts[0].x = 6;
    ghosts[0].y = 10;
    ghosts[0].float_offset = 0;
    ghosts[0].phase = 0;

    ghosts[1].x = 20;
    ghosts[1].y = 8;
    ghosts[1].float_offset = 50;
    ghosts[1].phase = 50;

    int frame_count = 0;

    // Display until interrupted
    while (running) {
        // Clear canvas with night sky
        canvas->Fill(bg_night.r, bg_night.g, bg_night.b);

        // Draw moon (top right)
        for (int y = 3; y <= 7; ++y) {
            for (int x = 24; x <= 28; ++x) {
                int dx = x - 26;
                int dy = y - 5;
                if (dx*dx + dy*dy <= 6) {
                    canvas->SetPixel(x, y, moon.r, moon.g, moon.b);
                }
            }
        }

        // Draw ground with grass
        for (int y = 26; y < 32; ++y) {
            for (int x = 0; x < 32; ++x) {
                canvas->SetPixel(x, y, ground.r, ground.g, ground.b);
            }
        }
        // Grass tufts
        for (int x = 0; x < 32; x += 3) {
            int offset = (frame_count / 20 + x) % 3;
            canvas->SetPixel(x + offset, 25, grass.r, grass.g, grass.b);
        }

        // Draw jack-o-lanterns (3 pumpkins) - BIGGER!
        int pumpkin_positions[][2] = {{7, 19}, {16, 18}, {25, 19}};

        for (int p = 0; p < 3; ++p) {
            int px = pumpkin_positions[p][0];
            int py = pumpkin_positions[p][1];

            // Pumpkin body (larger 7x6 shape)
            // Top row
            for (int x = -2; x <= 2; ++x) {
                canvas->SetPixel(px + x, py, pumpkin_orange.r, pumpkin_orange.g, pumpkin_orange.b);
            }
            // Second row
            for (int x = -3; x <= 3; ++x) {
                canvas->SetPixel(px + x, py + 1, pumpkin_orange.r, pumpkin_orange.g, pumpkin_orange.b);
            }
            // Middle rows (widest)
            for (int y = 2; y <= 4; ++y) {
                for (int x = -3; x <= 3; ++x) {
                    canvas->SetPixel(px + x, py + y, pumpkin_orange.r, pumpkin_orange.g, pumpkin_orange.b);
                }
            }
            // Bottom row
            for (int x = -2; x <= 2; ++x) {
                canvas->SetPixel(px + x, py + 5, pumpkin_orange.r, pumpkin_orange.g, pumpkin_orange.b);
            }

            // Pumpkin shading (vertical lines for segments)
            for (int y = 1; y <= 4; ++y) {
                canvas->SetPixel(px - 2, py + y, pumpkin_dark.r, pumpkin_dark.g, pumpkin_dark.b);
                canvas->SetPixel(px, py + y, pumpkin_dark.r, pumpkin_dark.g, pumpkin_dark.b);
                canvas->SetPixel(px + 2, py + y, pumpkin_dark.r, pumpkin_dark.g, pumpkin_dark.b);
            }

            // Stem (green/brown)
            canvas->SetPixel(px, py - 1, 80, 50, 20);

            // Glowing face (flickers)
            int flicker = (frame_count / 8 + p * 3) % 10;
            Color face_color = (flicker < 7) ? jack_face : Color(jack_face.r / 2, jack_face.g / 2, jack_face.b / 2);

            // Triangle eyes (larger)
            canvas->SetPixel(px - 2, py + 2, face_color.r, face_color.g, face_color.b);
            canvas->SetPixel(px - 1, py + 2, face_color.r, face_color.g, face_color.b);
            canvas->SetPixel(px - 2, py + 3, face_color.r, face_color.g, face_color.b);

            canvas->SetPixel(px + 1, py + 2, face_color.r, face_color.g, face_color.b);
            canvas->SetPixel(px + 2, py + 2, face_color.r, face_color.g, face_color.b);
            canvas->SetPixel(px + 2, py + 3, face_color.r, face_color.g, face_color.b);

            // Jagged mouth (bigger grin)
            canvas->SetPixel(px - 2, py + 4, face_color.r, face_color.g, face_color.b);
            canvas->SetPixel(px - 1, py + 4, face_color.r, face_color.g, face_color.b);
            canvas->SetPixel(px, py + 4, face_color.r, face_color.g, face_color.b);
            canvas->SetPixel(px + 1, py + 4, face_color.r, face_color.g, face_color.b);
            canvas->SetPixel(px + 2, py + 4, face_color.r, face_color.g, face_color.b);
            // Teeth gaps
            canvas->SetPixel(px - 1, py + 5, face_color.r, face_color.g, face_color.b);
            canvas->SetPixel(px + 1, py + 5, face_color.r, face_color.g, face_color.b);
        }

        // Draw floating ghosts
        for (int g = 0; g < 2; ++g) {
            float float_y = sin((frame_count + ghosts[g].phase) * 0.1f) * 1.5f;
            int gy = (int)(ghosts[g].y + float_y);
            int gx = (int)ghosts[g].x;

            // Ghost body (rounded top, wavy bottom)
            // Top of head
            canvas->SetPixel(gx, gy, ghost_white.r, ghost_white.g, ghost_white.b);
            canvas->SetPixel(gx - 1, gy + 1, ghost_white.r, ghost_white.g, ghost_white.b);
            canvas->SetPixel(gx, gy + 1, ghost_white.r, ghost_white.g, ghost_white.b);
            canvas->SetPixel(gx + 1, gy + 1, ghost_white.r, ghost_white.g, ghost_white.b);

            // Middle body
            for (int dy = 2; dy < 5; ++dy) {
                canvas->SetPixel(gx - 1, gy + dy, ghost_white.r, ghost_white.g, ghost_white.b);
                canvas->SetPixel(gx, gy + dy, ghost_white.r, ghost_white.g, ghost_white.b);
                canvas->SetPixel(gx + 1, gy + dy, ghost_white.r, ghost_white.g, ghost_white.b);
            }

            // Wavy bottom
            int wave = (frame_count / 5 + g * 3) % 3;
            if (wave == 0) {
                canvas->SetPixel(gx - 1, gy + 5, ghost_white.r, ghost_white.g, ghost_white.b);
                canvas->SetPixel(gx + 1, gy + 5, ghost_white.r, ghost_white.g, ghost_white.b);
            } else if (wave == 1) {
                canvas->SetPixel(gx, gy + 5, ghost_white.r, ghost_white.g, ghost_white.b);
            } else {
                canvas->SetPixel(gx - 1, gy + 5, ghost_white.r, ghost_white.g, ghost_white.b);
                canvas->SetPixel(gx, gy + 5, ghost_white.r, ghost_white.g, ghost_white.b);
                canvas->SetPixel(gx + 1, gy + 5, ghost_white.r, ghost_white.g, ghost_white.b);
            }

            // Eyes (black)
            canvas->SetPixel(gx - 1, gy + 2, 0, 0, 0);
            canvas->SetPixel(gx + 1, gy + 2, 0, 0, 0);

            // Mouth (black)
            canvas->SetPixel(gx, gy + 3, 0, 0, 0);
        }

        // Update and draw bats
        for (int i = 0; i < num_bats; ++i) {
            if (bats[i].active) {
                // Move bat
                bats[i].x += bats[i].speed_x;
                bats[i].y += bats[i].speed_y;
                bats[i].wing_frame = (bats[i].wing_frame + 1) % 10;

                // Wrap around screen
                if (bats[i].x < -2) bats[i].x = 33;
                if (bats[i].x > 33) bats[i].x = -2;
                if (bats[i].y < 0) bats[i].y = 0;
                if (bats[i].y > 20) bats[i].y = 20;

                int bx = (int)bats[i].x;
                int by = (int)bats[i].y;

                // Draw bat (simple shape with flapping wings)
                if (bx >= 0 && bx < 32 && by >= 0 && by < 32) {
                    // Body
                    canvas->SetPixel(bx, by, bat_black.r, bat_black.g, bat_black.b);

                    // Wings (flap)
                    bool wings_up = bats[i].wing_frame < 5;
                    if (wings_up) {
                        if (bx - 1 >= 0) canvas->SetPixel(bx - 1, by - 1, bat_black.r, bat_black.g, bat_black.b);
                        if (bx + 1 < 32) canvas->SetPixel(bx + 1, by - 1, bat_black.r, bat_black.g, bat_black.b);
                    } else {
                        if (bx - 1 >= 0) canvas->SetPixel(bx - 1, by, bat_black.r, bat_black.g, bat_black.b);
                        if (bx + 1 < 32) canvas->SetPixel(bx + 1, by, bat_black.r, bat_black.g, bat_black.b);
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
