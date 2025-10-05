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
    Color sky(10, 10, 40);           // Dark blue night sky
    Color ground(240, 240, 255);     // White snow ground
    Color tree_green(0, 100, 0);     // Dark green for tree
    Color trunk(101, 67, 33);        // Brown trunk
    Color snow_white(255, 255, 255); // White snow
    Color light_red(255, 0, 0);      // Red Christmas lights
    Color light_green(0, 255, 0);    // Green Christmas lights
    
    // Initialize snowflakes
    const int num_snowflakes = 20;
    Snowflake snowflakes[num_snowflakes];
    for (int i = 0; i < num_snowflakes; ++i) {
        snowflakes[i].x = rand() % 32;
        snowflakes[i].y = -(rand() % 32);  // Start above screen
        snowflakes[i].speed = 0.1f + (rand() % 10) / 20.0f;  // 0.1 to 0.6
        snowflakes[i].active = true;
    }
    
    // Christmas light positions on tree (will alternate red/green)
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
    
    // Display continuously
    while (true) {
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
        
        // Draw twinkling Christmas lights
        for (int i = 0; i < num_lights; ++i) {
            // Alternate colors and add twinkling effect
            bool is_bright = ((frame_count / 10 + i) % 2) == 0;
            Color light_color = (i % 2 == 0) ? light_red : light_green;
            
            if (is_bright) {
                canvas->SetPixel(light_positions[i][0], light_positions[i][1], 
                               light_color.r, light_color.g, light_color.b);
            } else {
                // Dimmed version
                canvas->SetPixel(light_positions[i][0], light_positions[i][1], 
                               light_color.r / 3, light_color.g / 3, light_color.b / 3);
            }
        }
        
        // Update and draw snowflakes
        for (int i = 0; i < num_snowflakes; ++i) {
            if (snowflakes[i].active) {
                // Move snowflake down
                snowflakes[i].y += snowflakes[i].speed;
                
                // Reset if it goes off screen
                if (snowflakes[i].y >= 27) {  // Hit the ground
                    snowflakes[i].y = 0;
                    snowflakes[i].x = rand() % 32;
                    snowflakes[i].speed = 0.1f + (rand() % 10) / 20.0f;
                }
                
                // Draw snowflake
                int snow_x = (int)snowflakes[i].x;
                int snow_y = (int)snowflakes[i].y;
                if (snow_y >= 0 && snow_y < 27) {
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
