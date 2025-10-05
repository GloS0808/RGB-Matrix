#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace rgb_matrix;

struct Snowflake {
    float x;
    float y;
    float speed;
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
    
    // Colors
    Color sky(10, 10, 40);              // Dark blue night sky
    Color mountain_dark(60, 60, 80);    // Dark mountain
    Color mountain_light(100, 100, 120); // Lighter mountain
    Color snow_cap(240, 240, 255);      // Snow on mountains
    Color tree_green(0, 80, 0);         // Dark green for trees
    Color trunk(80, 50, 20);            // Brown trunk
    Color snow_white(255, 255, 255);    // White snow
    Color light_red(255, 0, 0);         // Red Christmas lights
    Color light_green(0, 255, 0);       // Green Christmas lights
    Color light_yellow(255, 200, 0);    // Yellow Christmas lights
    
    // Initialize snowflakes
    const int num_snowflakes = 25;
    Snowflake snowflakes[num_snowflakes];
    for (int i = 0; i < num_snowflakes; ++i) {
        snowflakes[i].x = rand() % 32;
        snowflakes[i].y = -(rand() % 32);
        snowflakes[i].speed = 0.15f + (rand() % 10) / 25.0f;
        snowflakes[i].active = true;
    }
    
    // Mountain peaks data (x, y pairs defining mountain outline)
    int back_mountain[][2] = {
        {0, 20}, {5, 15}, {10, 18}, {15, 12}, {20, 16}, {25, 14}, {32, 18}
    };
    
    int front_mountain[][2] = {
        {0, 28}, {6, 22}, {12, 25}, {18, 19}, {24, 23}, {32, 26}
    };
    
    // Twinkling state
    int frame_count = 0;
    
    // Display continuously
    while (true) {
        // Clear canvas with night sky
        canvas->Fill(sky.r, sky.g, sky.b);
        
        // Draw back mountain range (fill from peaks down)
        for (int x = 0; x < 32; ++x) {
            // Interpolate mountain height
            int y_peak = 18; // default
            if (x < 5) y_peak = 20 - (20-15) * x / 5;
            else if (x < 10) y_peak = 15 + (18-15) * (x-5) / 5;
            else if (x < 15) y_peak = 18 - (18-12) * (x-10) / 5;
            else if (x < 20) y_peak = 12 + (16-12) * (x-15) / 5;
            else if (x < 25) y_peak = 16 - (16-14) * (x-20) / 5;
            else y_peak = 14 + (18-14) * (x-25) / 7;
            
            // Fill mountain
            for (int y = y_peak; y < 32; ++y) {
                // Snow cap on top few pixels
                if (y <= y_peak + 2) {
                    canvas->SetPixel(x, y, snow_cap.r, snow_cap.g, snow_cap.b);
                } else {
                    canvas->SetPixel(x, y, mountain_light.r, mountain_light.g, mountain_light.b);
                }
            }
        }
        
        // Draw front mountain range
        for (int x = 0; x < 32; ++x) {
            int y_peak = 26;
            if (x < 6) y_peak = 28 - (28-22) * x / 6;
            else if (x < 12) y_peak = 22 + (25-22) * (x-6) / 6;
            else if (x < 18) y_peak = 25 - (25-19) * (x-12) / 6;
            else if (x < 24) y_peak = 19 + (23-19) * (x-18) / 6;
            else y_peak = 23 + (26-23) * (x-24) / 8;
            
            for (int y = y_peak; y < 32; ++y) {
                if (y <= y_peak + 1) {
                    canvas->SetPixel(x, y, snow_cap.r, snow_cap.g, snow_cap.b);
                } else {
                    canvas->SetPixel(x, y, mountain_dark.r, mountain_dark.g, mountain_dark.b);
                }
            }
        }
        
        // Draw trees on mountain slopes
        // Left tree
        int tree1_x = 8;
        int tree1_y = 23;
        // Trunk
        canvas->SetPixel(tree1_x, tree1_y, trunk.r, trunk.g, trunk.b);
        canvas->SetPixel(tree1_x, tree1_y + 1, trunk.r, trunk.g, trunk.b);
        // Tree layers
        for (int dy = -3; dy <= -1; ++dy) {
            int width = (-dy) * 2 + 1;
            for (int dx = -(width/2); dx <= width/2; ++dx) {
                canvas->SetPixel(tree1_x + dx, tree1_y + dy, tree_green.r, tree_green.g, tree_green.b);
            }
        }
        
        // Middle tree (with lights)
        int tree2_x = 16;
        int tree2_y = 21;
        // Trunk
        canvas->SetPixel(tree2_x, tree2_y, trunk.r, trunk.g, trunk.b);
        canvas->SetPixel(tree2_x, tree2_y + 1, trunk.r, trunk.g, trunk.b);
        // Tree layers
        for (int dy = -4; dy <= -1; ++dy) {
            int width = (-dy) * 2 + 1;
            for (int dx = -(width/2); dx <= width/2; ++dx) {
                canvas->SetPixel(tree2_x + dx, tree2_y + dy, tree_green.r, tree_green.g, tree_green.b);
            }
        }
        
        // Christmas lights on middle tree
        int lights[][2] = {
            {tree2_x, tree2_y - 4},      // Top
            {tree2_x - 1, tree2_y - 3}, {tree2_x + 1, tree2_y - 3},  // Second row
            {tree2_x - 2, tree2_y - 2}, {tree2_x, tree2_y - 2}, {tree2_x + 2, tree2_y - 2},  // Third
            {tree2_x - 3, tree2_y - 1}, {tree2_x - 1, tree2_y - 1}, {tree2_x + 1, tree2_y - 1}, {tree2_x + 3, tree2_y - 1}  // Bottom
        };
        Color light_colors[] = {light_red, light_green, light_yellow};
        
        for (int i = 0; i < 10; ++i) {
            bool is_bright = ((frame_count / 8 + i) % 2) == 0;
            Color light_color = light_colors[i % 3];
            
            if (is_bright) {
                canvas->SetPixel(lights[i][0], lights[i][1], 
                               light_color.r, light_color.g, light_color.b);
            } else {
                canvas->SetPixel(lights[i][0], lights[i][1], 
                               light_color.r / 4, light_color.g / 4, light_color.b / 4);
            }
        }
        
        // Right tree
        int tree3_x = 24;
        int tree3_y = 24;
        // Trunk
        canvas->SetPixel(tree3_x, tree3_y, trunk.r, trunk.g, trunk.b);
        canvas->SetPixel(tree3_x, tree3_y + 1, trunk.r, trunk.g, trunk.b);
        // Tree layers (smaller)
        for (int dy = -2; dy <= -1; ++dy) {
            int width = (-dy) * 2 + 1;
            for (int dx = -(width/2); dx <= width/2; ++dx) {
                canvas->SetPixel(tree3_x + dx, tree3_y + dy, tree_green.r, tree_green.g, tree_green.b);
            }
        }
        
        // Update and draw snowflakes
        for (int i = 0; i < num_snowflakes; ++i) {
            if (snowflakes[i].active) {
                snowflakes[i].y += snowflakes[i].speed;
                
                if (snowflakes[i].y >= 32) {
                    snowflakes[i].y = 0;
                    snowflakes[i].x = rand() % 32;
                    snowflakes[i].speed = 0.15f + (rand() % 10) / 25.0f;
                }
                
                int snow_x = (int)snowflakes[i].x;
                int snow_y = (int)snowflakes[i].y;
                if (snow_y >= 0 && snow_y < 32) {
                    canvas->SetPixel(snow_x, snow_y, snow_white.r, snow_white.g, snow_white.b);
                }
            }
        }
        
        frame_count++;
        canvas = matrix->SwapOnVSync(canvas);
        usleep(50000);  // ~20 fps
    }
    
    delete matrix;
    return 0;
}
