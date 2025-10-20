#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <signal.h>
using namespace rgb_matrix;

// Flag to control the main loop
static volatile bool running = true;

// Signal handler for Ctrl+C
void handle_interrupt(int sig) {
    running = false;
}

struct RainDrop {
    int x;
    float y;
    float speed;
    int length;
    int char_value;
    bool active;
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

    // Colors for Matrix effect
    Color bg_black(0, 0, 0);
    Color matrix_bright(200, 255, 200);    // Bright green (head)
    Color matrix_green(0, 255, 0);         // Standard green
    Color matrix_medium(0, 180, 0);        // Medium green
    Color matrix_dim(0, 100, 0);           // Dim green
    Color matrix_dark(0, 50, 0);           // Very dim green

    // Initialize rain drops (columns)
    const int num_drops = 32;  // One potential drop per column
    RainDrop drops[num_drops];

    for (int i = 0; i < num_drops; ++i) {
        drops[i].x = i;
        drops[i].y = -(rand() % 32);  // Start above screen
        drops[i].speed = 0.3f + (rand() % 10) / 10.0f;  // Variable speeds
        drops[i].length = 8 + rand() % 12;  // Trail length 8-20
        drops[i].char_value = rand() % 256;
        drops[i].active = (rand() % 100 < 40);  // 40% chance to start active
    }

    // Character pattern (simple patterns that look like characters)
    // Using pixel patterns to simulate characters/symbols
    int char_patterns[16][3] = {
        {0b111, 0b101, 0b111}, // 0
        {0b010, 0b110, 0b010}, // 1
        {0b111, 0b011, 0b110}, // 2
        {0b111, 0b011, 0b111}, // 3
        {0b101, 0b111, 0b001}, // 4
        {0b110, 0b011, 0b111}, // 5
        {0b111, 0b110, 0b111}, // 6
        {0b111, 0b001, 0b001}, // 7
        {0b111, 0b111, 0b111}, // 8
        {0b111, 0b011, 0b111}, // 9
        {0b010, 0b111, 0b101}, // A
        {0b110, 0b111, 0b110}, // B
        {0b111, 0b100, 0b111}, // C
        {0b110, 0b101, 0b110}, // D
        {0b111, 0b110, 0b111}, // E
        {0b111, 0b110, 0b100}, // F
    };

    int frame_count = 0;

    // Display until interrupted
    while (running) {
        // Clear with black background
        canvas->Fill(bg_black.r, bg_black.g, bg_black.b);

        // Update and draw rain drops
        for (int i = 0; i < num_drops; ++i) {
            if (drops[i].active) {
                // Move drop down
                drops[i].y += drops[i].speed;

                // Reset if completely off screen
                if (drops[i].y - drops[i].length > 32) {
                    drops[i].y = -(rand() % 10);
                    drops[i].speed = 0.3f + (rand() % 10) / 10.0f;
                    drops[i].length = 8 + rand() % 12;
                    drops[i].char_value = rand() % 256;
                    drops[i].active = (rand() % 100 < 60);  // 60% chance to restart
                }

                // Draw the trail
                int head_y = (int)drops[i].y;

                for (int t = 0; t < drops[i].length; ++t) {
                    int y_pos = head_y - t;

                    if (y_pos >= 0 && y_pos < 32) {
                        Color trail_color;

                        // Color based on distance from head
                        if (t == 0) {
                            trail_color = matrix_bright;  // Bright head
                        } else if (t < 3) {
                            trail_color = matrix_green;
                        } else if (t < 6) {
                            trail_color = matrix_medium;
                        } else if (t < 10) {
                            trail_color = matrix_dim;
                        } else {
                            trail_color = matrix_dark;
                        }

                        // Draw character pattern
                        int pattern_idx = ((drops[i].char_value + t + frame_count / 5) % 16);
                        int pattern[3] = {
                            char_patterns[pattern_idx][0],
                            char_patterns[pattern_idx][1],
                            char_patterns[pattern_idx][2]
                        };

                        // Draw 3-pixel wide character
                        int x = drops[i].x;
                        if (x >= 0 && x < 32) {
                            // Randomly show or hide some pixels for variety
                            int show_pattern = (rand() % 100 < 85);  // 85% show

                            if (show_pattern) {
                                // Single column version (simpler for 32x32)
                                canvas->SetPixel(x, y_pos, trail_color.r, trail_color.g, trail_color.b);

                                // Occasionally add a brighter glitch
                                if (rand() % 100 < 5) {
                                    canvas->SetPixel(x, y_pos, 255, 255, 255);
                                }
                            }
                        }
                    }
                }
            } else {
                // Randomly activate inactive drops
                if (rand() % 100 < 2) {  // 2% chance per frame
                    drops[i].active = true;
                    drops[i].y = 0;
                    drops[i].char_value = rand() % 256;
                }
            }
        }

        // Occasional random flashes (glitches)
        if (rand() % 100 < 3) {
            int flash_x = rand() % 32;
            int flash_y = rand() % 32;
            canvas->SetPixel(flash_x, flash_y, 255, 255, 255);
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
