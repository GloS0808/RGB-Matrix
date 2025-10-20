// Birthday Cake Scene (Animated)
// Compilation: g++ -o birthday_cake birthday_cake.cpp -I ~/rgbMatrix/rpi-rgb-led-matrix/include -L ~/rgbMatrix/rpi-rgb-led-matrix/lib -lrgbmatrix -lpthread -lm

#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <signal.h>
#include <vector>

using namespace rgb_matrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
    interrupt_received = true; // Set flag to exit loop
}

struct Sprinkle {
    float x;
    float y;
    float speed;
    float sway;
    int sway_offset;
    bool active;
};

void DrawCakeTier(Canvas *canvas, int cx, int cy, int width, int height, Color base_color, Color frosting_color) {
    // Draw a rectangular cake tier with frosting on top, ensuring no gaps
    for (int x = cx - width / 2; x < cx + width / 2; x++) {
        for (int y = cy; y < cy + height; y++) {
            if (x >= 0 && x < 32 && y >= 0 && y < 32) {
                canvas->SetPixel(x, y, base_color.r, base_color.g, base_color.b);
            }
        }
    }
    // Frosting layer, corrected to align with tier edge
    for (int x = cx - width / 2; x < cx + width / 2 - 1; x++) {
        if (cy - 1 >= 0 && cy - 1 < 32) {
            canvas->SetPixel(x, cy - 1, frosting_color.r, frosting_color.g, frosting_color.b);
        }
    }
}

void DrawCandle(Canvas *canvas, int cx, int cy, Color flame_color) {
    // Draw candle stick
    for (int y = cy; y < cy + 4; y++) {
        if (cx >= 0 && cx < 32 && y >= 0 && y < 32) {
            canvas->SetPixel(cx, y, 150, 75, 0); // Warm yellow-brown for candle
        }
    }
    // Flickering flame
    if (rand() % 10 < 8) { // 80% chance to show flame
        for (int x = cx - 1; x <= cx + 1; x++) {
            for (int y = cy - 1; y < cy; y++) {
                if ((x - cx) * (x - cx) + (y - (cy - 0.5)) * (y - (cy - 0.5)) <= 1) {
                    if (x >= 0 && x < 32 && y >= 0 && y < 32) {
                        canvas->SetPixel(x, y, flame_color.r, flame_color.g, flame_color.b);
                    }
                }
            }
        }
    }
}

int main() {
    // Set up signal handler for Ctrl+C
    signal(SIGINT, InterruptHandler);

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
        return 1;
    }
    FrameCanvas *canvas = matrix->CreateFrameCanvas();

    // Colors
    Color sky_night = Color(10, 20, 40);       // Deep night sky
    Color cake_base = Color(150, 75, 0);       // Warm brown cake base
    Color tier1_frosting = Color(255, 200, 200); // Light pink frosting
    Color tier2_frosting = Color(255, 150, 200); // Medium pink frosting
    Color tier3_frosting = Color(255, 100, 150); // Dark pink frosting
    Color sprinkle_red = Color(255, 0, 0);     // Red sprinkle
    Color sprinkle_green = Color(0, 255, 0);   // Green sprinkle
    Color sprinkle_blue = Color(0, 0, 255);    // Blue sprinkle
    Color flame_color = Color(255, 165, 0);    // Orange flame
    Color sparkle = Color(255, 255, 255);      // White sparkle

    // Initialize falling sprinkles
    const int num_sprinkles = 15;
    Sprinkle sprinkles[num_sprinkles];
    for (int i = 0; i < num_sprinkles; ++i) {
        sprinkles[i].x = rand() % 32;
        sprinkles[i].y = -(rand() % 32);
        sprinkles[i].speed = 0.5f + (rand() % 10) / 20.0f;
        sprinkles[i].sway = 0.2f;
        sprinkles[i].sway_offset = rand() % 360;
        sprinkles[i].active = true;
    }

    // Sparkle positions (will twinkle around cake)
    int sparkle_positions[][2] = {
        {8, 8}, {24, 8}, {16, 16}, {10, 12}, {22, 12},
        {6, 18}, {26, 18}, {14, 22}, {18, 22}
    };
    int num_sparkles = 9;

    int frame_count = 0;

    // Display until interrupted
    while (!interrupt_received) {
        // Clear canvas with night sky
        canvas->Fill(sky_night.r, sky_night.g, sky_night.b);

        // Draw 3-tier cake with pulsing sizes
        int base_x = 16;
        int base_y = 20;
        int pulse1 = (frame_count / 10) % 20;
        int width1 = (pulse1 < 10 ? 10 : 12);
        int height1 = (pulse1 < 10 ? 4 : 5);
        DrawCakeTier(canvas, base_x, base_y, width1, height1, cake_base, tier1_frosting);

        int mid_y = base_y - height1;
        int pulse2 = ((frame_count + 10) / 10) % 20;
        int width2 = (pulse2 < 10 ? 8 : 9);
        int height2 = (pulse2 < 10 ? 3 : 4);
        DrawCakeTier(canvas, base_x, mid_y, width2, height2, cake_base, tier2_frosting);

        int top_y = mid_y - height2;
        int pulse3 = ((frame_count + 5) / 10) % 20;
        int width3 = (pulse3 < 10 ? 6 : 7);
        int height3 = (pulse3 < 10 ? 2 : 3);
        DrawCakeTier(canvas, base_x, top_y, width3, height3, cake_base, tier3_frosting);

        // Animated icing drips
        int drip_y = base_y - 1;
        for (int x = base_x - width1 / 2 + 1; x < base_x + width1 / 2 - 1; x++) {
            if (frame_count % 20 < 10 && rand() % 5 == 0) {
                if (x >= 0 && x < 32 && drip_y >= 0 && drip_y < 32) {
                    canvas->SetPixel(x, drip_y, tier1_frosting.r, tier1_frosting.g, tier1_frosting.b);
                }
            }
        }
        drip_y = mid_y - 1;
        for (int x = base_x - width2 / 2 + 1; x < base_x + width2 / 2 - 1; x++) {
            if (frame_count % 15 < 7 && rand() % 4 == 0) {
                if (x >= 0 && x < 32 && drip_y >= 0 && drip_y < 32) {
                    canvas->SetPixel(x, drip_y, tier2_frosting.r, tier2_frosting.g, tier2_frosting.b);
                }
            }
        }
        drip_y = top_y - 1;
        for (int x = base_x - width3 / 2 + 1; x < base_x + width3 / 2 - 1; x++) {
            if (frame_count % 10 < 5 && rand() % 3 == 0) {
                if (x >= 0 && x < 32 && drip_y >= 0 && drip_y < 32) {
                    canvas->SetPixel(x, drip_y, tier3_frosting.r, tier3_frosting.g, tier3_frosting.b);
                }
            }
        }

        // Draw three candles on top tier
        int candle_x_center = base_x;
        int candle_y = top_y - height3;
        int spacing = 2; // Space between candles
        DrawCandle(canvas, candle_x_center - spacing, candle_y, flame_color); // Left candle
        DrawCandle(canvas, candle_x_center, candle_y, flame_color);           // Center candle
        DrawCandle(canvas, candle_x_center + spacing, candle_y, flame_color); // Right candle

        // Draw twinkling sparkles around cake
        for (int i = 0; i < num_sparkles; ++i) {
            int twinkle_phase = (frame_count + i * 7) % 30;
            if (twinkle_phase < 5) {  // Brief twinkle
                int brightness = (twinkle_phase < 3) ? 255 : 128;
                canvas->SetPixel(sparkle_positions[i][0], sparkle_positions[i][1],
                               brightness, brightness, brightness);
            }
        }

        // Update and draw falling sprinkles
        for (int i = 0; i < num_sprinkles; ++i) {
            if (sprinkles[i].active) {
                // Move sprinkle down
                sprinkles[i].y += sprinkles[i].speed;

                // Add swaying motion
                float sway_x = sin((frame_count + sprinkles[i].sway_offset) * 0.1f) * sprinkles[i].sway;

                // Reset if it goes off screen
                if (sprinkles[i].y >= 32) {
                    sprinkles[i].y = -2; // Start just above top
                    sprinkles[i].x = rand() % 32;
                    sprinkles[i].speed = 0.5f + (rand() % 10) / 20.0f;
                    sprinkles[i].sway_offset = rand() % 360;
                }

                // Draw sprinkle (1x1 or 1x2 shape)
                int sprinkle_x = static_cast<int>(sprinkles[i].x + sway_x);
                int sprinkle_y = static_cast<int>(sprinkles[i].y);

                if (sprinkle_y >= 0 && sprinkle_y < 32 && sprinkle_x >= 0 && sprinkle_x < 32) {
                    Color* colors[] = {&sprinkle_red, &sprinkle_green, &sprinkle_blue};
                    Color color = *colors[i % 3];
                    canvas->SetPixel(sprinkle_x, sprinkle_y, color.r, color.g, color.b);
                    if (sprinkle_y + 1 < 32 && rand() % 2 == 0) {
                        canvas->SetPixel(sprinkle_x, sprinkle_y + 1, color.r, color.g, color.b);
                    }
                }
            }
        }

        frame_count++;
        canvas = matrix->SwapOnVSync(canvas);
        usleep(50000);  // ~20 fps
    }

    // Cleanup and graceful exit
    matrix->Clear();
    delete matrix;
    return 0;
}
