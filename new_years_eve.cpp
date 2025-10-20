// New Year's Eve Winter Scene
// Compilation: g++ -o new_years_eve_winter new_years_eve_winter.cpp -I ~/rgbMatrix/rpi-rgb-led-matrix/include -L ~/rgbMatrix/rpi-rgb-led-matrix/lib -lrgbmatrix -lpthread -lm

#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <signal.h>

using namespace rgb_matrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
    interrupt_received = true;
}

struct Snowflake {
    float x;
    float y;
    float speed;
    bool active;
};

int main() {
    srand(time(NULL));

    // Set up signal handler for graceful exit
    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);

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
    Color sky(10, 10, 40);           // Dark blue night sky
    Color ground(240, 240, 255);     // White snow ground
    Color tree_green(0, 100, 0);     // Dark green for tree
    Color trunk(101, 67, 33);        // Brown trunk
    Color snow_white(255, 255, 255); // White snow
    Color light_silver(192, 192, 192); // Silver New Year's lights
    Color light_gold(200, 180, 0);   // Gold New Year's lights
    Color clock_black(0, 0, 0);      // Black for clock
    Color clock_white(255, 255, 255); // White for clock hands/numbers
    Color year_gold(200, 180, 0);    // Gold for "2026" text

    // Initialize snowflakes
    const int num_snowflakes = 20;
    Snowflake snowflakes[num_snowflakes];
    for (int i = 0; i < num_snowflakes; ++i) {
        snowflakes[i].x = rand() % 32;
        snowflakes[i].y = -(rand() % 32);  // Start above screen
        snowflakes[i].speed = 0.1f + (rand() % 10) / 20.0f;  // 0.1 to 0.6
        snowflakes[i].active = true;
    }

    // New Year's light positions on tree (will alternate silver/gold)
    int light_positions[][2] = {
        {16, 8},   // Top
        {14, 11}, {18, 11},  // Second row
        {13, 14}, {16, 14}, {19, 14},  // Third row
        {12, 17}, {15, 17}, {18, 17}, {21, 17},  // Fourth row
        {11, 20}, {14, 20}, {17, 20}, {20, 20}, {23, 20}  // Fifth row
    };
    int num_lights = 15;

    // Twinkling state
    int frame_count = 0;

    // Static clock at midnight (12:00)
    int clock_x = 8;
    int clock_y = 5;
    int clock_radius = 3;

    // Static "2026" sign
    int sign_x = 10;
    int sign_y = 22;

    // Display continuously
    while (!interrupt_received) {
        // Clear canvas with night sky
        canvas->Fill(sky.r, sky.g, sky.b);

        // Draw snow ground (bottom 5 rows)
        for (int y = 27; y < 32; ++y)
            for (int x = 0; x < 32; ++x)
                canvas->SetPixel(x, y, ground.r, ground.g, ground.b);

        // Draw tree trunk
        for (int y = 21; y < 27; ++y) {
            for (int x = 14; x < 18; ++x) {
                canvas->SetPixel(x, y, trunk.r, trunk.g, trunk.b);
            }
        }

        // Draw evergreen tree (triangle shape, layered)
        // Top section
        for (int y = 6; y <= 10; ++y) {
            int width = (y - 6) * 2 + 1;
            int start_x = 16 - width / 2;
            for (int x = 0; x < width; ++x) {
                canvas->SetPixel(start_x + x, y, tree_green.r, tree_green.g, tree_green.b);
            }
        }

        // Middle section
        for (int y = 11; y <= 15; ++y) {
            int width = (y - 9) * 2 + 1;
            int start_x = 16 - width / 2;
            for (int x = 0; x < width; ++x) {
                canvas->SetPixel(start_x + x, y, tree_green.r, tree_green.g, tree_green.b);
            }
        }

        // Bottom section
        for (int y = 16; y <= 21; ++y) {
            int width = (y - 13) * 2 + 1;
            int start_x = 16 - width / 2;
            for (int x = 0; x < width; ++x) {
                canvas->SetPixel(start_x + x, y, tree_green.r, tree_green.g, tree_green.b);
            }
        }

        // Draw twinkling New Year's lights
        for (int i = 0; i < num_lights; ++i) {
            bool is_bright = ((frame_count / 10 + i) % 2) == 0;
            Color light_color = (i % 2 == 0) ? light_silver : light_gold;

            if (is_bright) {
                canvas->SetPixel(light_positions[i][0], light_positions[i][1],
                               light_color.r, light_color.g, light_color.b);
            } else {
                canvas->SetPixel(light_positions[i][0], light_positions[i][1],
                               light_color.r / 3, light_color.g / 3, light_color.b / 3);
            }
        }

        // Update and draw snowflakes
        for (int i = 0; i < num_snowflakes; ++i) {
            if (snowflakes[i].active) {
                snowflakes[i].y += snowflakes[i].speed;

                if (snowflakes[i].y >= 27) {
                    snowflakes[i].y = 0;
                    snowflakes[i].x = rand() % 32;
                    snowflakes[i].speed = 0.1f + (rand() % 10) / 20.0f;
                }

                int snow_x = static_cast<int>(snowflakes[i].x);
                int snow_y = static_cast<int>(snowflakes[i].y);
                if (snow_y >= 0 && snow_y < 27) {
                    canvas->SetPixel(snow_x, snow_y, snow_white.r, snow_white.g, snow_white.b);
                }
            }
        }

        // Draw static clock at midnight
        for (int x = clock_x - clock_radius; x <= clock_x + clock_radius; x++) {
            for (int y = clock_y - clock_radius; y <= clock_y + clock_radius; y++) {
                if ((x - clock_x) * (x - clock_x) + (y - clock_y) * (y - clock_y) <= clock_radius * clock_radius) {
                    if (x >= 0 && x < 32 && y >= 0 && y < 32) {
                        canvas->SetPixel(x, y, clock_black.r, clock_black.g, clock_black.b);
                    }
                }
            }
        }
        canvas->SetPixel(clock_x, clock_y - clock_radius + 1, clock_white.r, clock_white.g, clock_white.b); // Hour hand
        canvas->SetPixel(clock_x, clock_y - clock_radius + 1, clock_white.r, clock_white.g, clock_white.b); // Minute hand

        // Draw static "2026" sign
        // 2
        canvas->SetPixel(sign_x, sign_y, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 1, sign_y, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 2, sign_y, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x, sign_y + 1, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 2, sign_y + 1, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x, sign_y + 2, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 1, sign_y + 2, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 2, sign_y + 2, year_gold.r, year_gold.g, year_gold.b);

        // 0
        canvas->SetPixel(sign_x + 4, sign_y, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 5, sign_y, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 6, sign_y, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 4, sign_y + 1, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 6, sign_y + 1, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 4, sign_y + 2, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 5, sign_y + 2, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 6, sign_y + 2, year_gold.r, year_gold.g, year_gold.b);

        // 2
        canvas->SetPixel(sign_x + 8, sign_y, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 9, sign_y, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 10, sign_y, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 8, sign_y + 1, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 10, sign_y + 1, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 8, sign_y + 2, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 9, sign_y + 2, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 10, sign_y + 2, year_gold.r, year_gold.g, year_gold.b);

        // 6
        canvas->SetPixel(sign_x + 12, sign_y, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 13, sign_y, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 14, sign_y, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 12, sign_y + 1, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 14, sign_y + 1, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 12, sign_y + 2, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 13, sign_y + 2, year_gold.r, year_gold.g, year_gold.b);
        canvas->SetPixel(sign_x + 14, sign_y + 2, year_gold.r, year_gold.g, year_gold.b);

        frame_count++;
        canvas = matrix->SwapOnVSync(canvas);
        usleep(50000);  // ~20 fps
    }

    // Clean up on exit - clear the display
    canvas->Clear();
    canvas = matrix->SwapOnVSync(canvas);
    delete matrix;

    std::cout << "\nDisplay cleared. Exiting gracefully.\n";
    return 0;
}
