#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
using namespace rgb_matrix;

struct RainDrop {
    int x;
    float y;
    float speed;
    int length;
    int char_value;
    float opacity;
    bool active;
};

int main() {
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
    Color matrix_bright(220, 255, 220);    // Bright green (head)
    Color matrix_green(0, 200, 0);         // Standard green
    Color matrix_medium(0, 150, 0);        // Medium green
    Color matrix_dim(0, 80, 0);            // Dim green
    Color matrix_dark(0, 40, 0);           // Very dim green
    Color bg_noise(10, 15, 10);            // Subtle background noise

    // Initialize rain drops (columns)
    const int num_drops = 32;
    RainDrop drops[num_drops];

    for (int i = 0; i < num_drops; ++i) {
        drops[i].x = i;
        drops[i].y = -(rand() % 32);
        drops[i].speed = 0.4f + (rand() % 8) / 10.0f;
        drops[i].length = 6 + rand() % 10;
        drops[i].char_value = rand() % 256;
        drops[i].opacity = 0.6f + (rand() % 40) / 100.0f; // 0.6 to 1.0
        drops[i].active = (rand() % 100 < 50);
    }

    // Expanded character patterns (5x3 pixel glyphs)
    int char_patterns[20][5] = {
        {0b01110, 0b10001, 0b10001, 0b10001, 0b01110}, // 0
        {0b00100, 0b01100, 0b00100, 0b00100, 0b01110}, // 1
        {0b11110, 0b00001, 0b01110, 0b10000, 0b11111}, // 2
        {0b11110, 0b00001, 0b00110, 0b00001, 0b11110}, // 3
        {0b10001, 0b10001, 0b11111, 0b00001, 0b00001}, // 4
        {0b11111, 0b10000, 0b11110, 0b00001, 0b11110}, // 5
        {0b01110, 0b10000, 0b11110, 0b10001, 0b01110}, // 6
        {0b11111, 0b00001, 0b00010, 0b00100, 0b00100}, // 7
        {0b01110, 0b10001, 0b01110, 0b10001, 0b01110}, // 8
        {0b01110, 0b10001, 0b01111, 0b00001, 0b01110}, // 9
        {0b01110, 0b10001, 0b11111, 0b10001, 0b10001}, // A
        {0b11110, 0b10001, 0b11110, 0b10001, 0b11110}, // B
        {0b01111, 0b10000, 0b10000, 0b10000, 0b01111}, // C
        {0b11110, 0b10001, 0b10001, 0b10001, 0b11110}, // D
        {0b11111, 0b10000, 0b11110, 0b10000, 0b11111}, // E
        {0b11111, 0b10000, 0b11110, 0b10000, 0b10000}, // F
        {0b01111, 0b10000, 0b10111, 0b10001, 0b01111}, // G
        {0b10001, 0b10001, 0b11111, 0b10001, 0b10001}, // H
        {0b01110, 0b00100, 0b00100, 0b00100, 0b01110}, // I
        {0b00001, 0b00001, 0b00001, 0b10001, 0b01110}, // J
    };

    int frame_count = 0;

    while (true) {
        // Subtle background noise
        canvas->Fill(bg_black.r, bg_black.g, bg_black.b);
        if (rand() % 100 < 10) { // 10% chance for background noise
            for (int i = 0; i < 5; ++i) {
                int x = rand() % 32;
                int y = rand() % 32;
                canvas->SetPixel(x, y, bg_noise.r, bg_noise.g, bg_noise.b);
            }
        }

        // Update and draw rain drops
        for (int i = 0; i < num_drops; ++i) {
            if (drops[i].active) {
                // Dynamic speed adjustment based on length
                drops[i].y += drops[i].speed * (1.0f - drops[i].length / 20.0f);

                // Reset if off screen
                if (drops[i].y - drops[i].length > 32) {
                    drops[i].y = -(rand() % 10);
                    drops[i].speed = 0.4f + (rand() % 8) / 10.0f;
                    drops[i].length = 6 + rand() % 10;
                    drops[i].char_value = rand() % 256;
                    drops[i].opacity = 0.6f + (rand() % 40) / 100.0f;
                    drops[i].active = (rand() % 100 < 70);
                }

                // Draw the trail
                int head_y = (int)drops[i].y;

                for (int t = 0; t < drops[i].length; ++t) {
                    int y_pos = head_y - t;

                    if (y_pos >= 0 && y_pos < 32) {
                        // Smooth color gradient
                        float fade = 1.0f - (float)t / drops[i].length;
                        if (fade < 0.2f) fade = 0.2f;
                        Color trail_color;
                        if (t == 0) {
                            trail_color = matrix_bright;
                        } else if (t < 3) {
                            trail_color = matrix_green;
                        } else if (t < 6) {
                            trail_color = matrix_medium;
                        } else if (t < 10) {
                            trail_color = matrix_dim;
                        } else {
                            trail_color = matrix_dark;
                        }

                        // Apply opacity
                        trail_color.r *= drops[i].opacity * fade;
                        trail_color.g *= drops[i].opacity * fade;
                        trail_color.b *= drops[i].opacity * fade;

                        // Draw character pattern (3x5 pixels)
                        int x = drops[i].x;
                        int pattern_idx = ((drops[i].char_value + t + frame_count / 5) % 20);
                        int pattern[5];
                        for (int j = 0; j < 5; ++j) {
                            pattern[j] = char_patterns[pattern_idx][j];
                        }

                        // Draw 3-pixel wide character
                        for (int px = -1; px <= 1; ++px) {
                            int draw_x = x + px;
                            if (draw_x >= 0 && draw_x < 32) {
                                for (int py = 0; py < 5; ++py) {
                                    int draw_y = y_pos + py - 2;
                                    if (draw_y >= 0 && draw_y < 32 && (pattern[py] & (1 << (2 - px)))) {
                                        if (rand() % 100 < 90) { // 90% chance to draw
                                            canvas->SetPixel(draw_x, draw_y, trail_color.r, trail_color.g, trail_color.b);
                                        }
                                    }
                                }
                            }
                        }

                        // Add faint echo effect
                        if (t > 2 && rand() % 100 < 20) {
                            int echo_y = y_pos + 1;
                            if (echo_y >= 0 && echo_y < 32) {
                                Color echo_color = Color(trail_color.r * 0.3, trail_color.g * 0.3, trail_color.b * 0.3);
                                canvas->SetPixel(x, echo_y, echo_color.r, echo_color.g, echo_color.b);
                            }
                        }

                        // Occasional white glitch
                        if (t == 0 && rand() % 100 < 8) {
                            canvas->SetPixel(x, y_pos, 255, 255, 255);
                        }
                    }
                }
            } else {
                // Randomly activate inactive drops
                if (rand() % 100 < 3) {
                    drops[i].active = true;
                    drops[i].y = 0;
                    drops[i].char_value = rand() % 256;
                    drops[i].opacity = 0.6f + (rand() % 40) / 100.0f;
                }
            }
        }

        // Random glitch pixels
        if (rand() % 100 < 5) {
            int flash_x = rand() % 32;
            int flash_y = rand() % 32;
            canvas->SetPixel(flash_x, flash_y, 255, 255, 255);
        }

        frame_count++;
        canvas = matrix->SwapOnVSync(canvas);
        usleep(40000); // ~25 fps for smoother animation
    }

    delete matrix;
    return 0;
}
