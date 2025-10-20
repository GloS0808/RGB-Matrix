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

struct Petal {
    float x;
    float y;
    float speed;
    float sway;
    int sway_offset;
    bool active;
};

void DrawHeart(Canvas *canvas, int cx, int cy, int size, Color color) {
    // Draw a heart shape using pixel patterns
    // Heart is made of two rounded tops and a pointed bottom

    if (size == 3) {
        // Small heart (5x5 pattern)
        int pattern[5][5] = {
            {0, 1, 0, 1, 0},
            {1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1},
            {0, 1, 1, 1, 0},
            {0, 0, 1, 0, 0}
        };
        for (int y = 0; y < 5; ++y) {
            for (int x = 0; x < 5; ++x) {
                if (pattern[y][x]) {
                    int px = cx + x - 2;
                    int py = cy + y - 2;
                    if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                        canvas->SetPixel(px, py, color.r, color.g, color.b);
                    }
                }
            }
        }
    } else if (size == 4) {
        // Medium heart (7x6 pattern)
        int pattern[6][7] = {
            {0, 1, 1, 0, 1, 1, 0},
            {1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1},
            {0, 1, 1, 1, 1, 1, 0},
            {0, 0, 1, 1, 1, 0, 0},
            {0, 0, 0, 1, 0, 0, 0}
        };
        for (int y = 0; y < 6; ++y) {
            for (int x = 0; x < 7; ++x) {
                if (pattern[y][x]) {
                    int px = cx + x - 3;
                    int py = cy + y - 2;
                    if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                        canvas->SetPixel(px, py, color.r, color.g, color.b);
                    }
                }
            }
        }
    } else {
        // Large heart (9x8 pattern)
        int pattern[8][9] = {
            {0, 1, 1, 0, 0, 0, 1, 1, 0},
            {1, 1, 1, 1, 0, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1},
            {0, 1, 1, 1, 1, 1, 1, 1, 0},
            {0, 0, 1, 1, 1, 1, 1, 0, 0},
            {0, 0, 0, 1, 1, 1, 0, 0, 0},
            {0, 0, 0, 0, 1, 0, 0, 0, 0}
        };
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 9; ++x) {
                if (pattern[y][x]) {
                    int px = cx + x - 4;
                    int py = cy + y - 3;
                    if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                        canvas->SetPixel(px, py, color.r, color.g, color.b);
                    }
                }
            }
        }
    }
}

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
    Color bg_pink(40, 10, 20);           // Dark pink/purple background
    Color heart_red(255, 0, 50);         // Bright red heart
    Color heart_pink(255, 100, 150);     // Pink heart
    Color heart_white(255, 200, 220);    // White/light pink heart
    Color petal_pink(255, 150, 180);     // Rose petal pink
    Color petal_light(255, 200, 220);    // Light petal
    Color sparkle(255, 255, 255);        // White sparkle

    // Initialize falling petals
    const int num_petals = 15;
    Petal petals[num_petals];
    for (int i = 0; i < num_petals; ++i) {
        petals[i].x = rand() % 32;
        petals[i].y = -(rand() % 32);
        petals[i].speed = 0.1f + (rand() % 8) / 20.0f;
        petals[i].sway = 0.3f;
        petals[i].sway_offset = rand() % 360;
        petals[i].active = true;
    }

    // Sparkle positions (will twinkle around hearts)
    int sparkle_positions[][2] = {
        {8, 8}, {24, 8}, {16, 20}, {10, 18}, {22, 18},
        {6, 12}, {26, 12}, {14, 24}, {18, 24}
    };
    int num_sparkles = 9;

    int frame_count = 0;

    // Display until interrupted
    while (running) {
        // Clear canvas with pink background
        canvas->Fill(bg_pink.r, bg_pink.g, bg_pink.b);

        // Draw three hearts in a pattern
        // Top left heart (pulsing)
        int pulse1 = (frame_count / 10) % 20;
        int size1 = (pulse1 < 10 ? 3 : 4);
        DrawHeart(canvas, 8, 10, size1, heart_pink);

        // Top right heart (pulsing with offset)
        int pulse2 = ((frame_count + 10) / 10) % 20;
        int size2 = (pulse2 < 10 ? 3 : 4);
        DrawHeart(canvas, 24, 10, size2, heart_white);

        // Center bottom heart (large, pulsing)
        int pulse3 = ((frame_count + 5) / 10) % 20;
        int size3 = (pulse3 < 10 ? 5 : 6);
        DrawHeart(canvas, 16, 22, size3, heart_red);

        // Draw twinkling sparkles around hearts
        for (int i = 0; i < num_sparkles; ++i) {
            int twinkle_phase = (frame_count + i * 7) % 30;
            if (twinkle_phase < 5) {  // Brief twinkle
                int brightness = (twinkle_phase < 3) ? 255 : 128;
                canvas->SetPixel(sparkle_positions[i][0], sparkle_positions[i][1],
                               brightness, brightness, brightness);
            }
        }

        // Update and draw falling rose petals
        for (int i = 0; i < num_petals; ++i) {
            if (petals[i].active) {
                // Move petal down
                petals[i].y += petals[i].speed;

                // Add swaying motion
                float sway_x = sin((frame_count + petals[i].sway_offset) * 0.1f) * petals[i].sway;

                // Reset if it goes off screen
                if (petals[i].y >= 32) {
                    petals[i].y = 0;
                    petals[i].x = rand() % 32;
                    petals[i].speed = 0.1f + (rand() % 8) / 20.0f;
                    petals[i].sway_offset = rand() % 360;
                }

                // Draw petal (small 2x2 or 1x2 shape)
                int petal_x = (int)(petals[i].x + sway_x);
                int petal_y = (int)petals[i].y;

                if (petal_y >= 0 && petal_y < 32 && petal_x >= 0 && petal_x < 32) {
                    // Alternate between pink shades
                    Color petal_color = (i % 2 == 0) ? petal_pink : petal_light;
                    canvas->SetPixel(petal_x, petal_y, petal_color.r, petal_color.g, petal_color.b);
                    if (petal_x + 1 < 32) {
                        canvas->SetPixel(petal_x + 1, petal_y, petal_color.r, petal_color.g, petal_color.b);
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
