#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <signal.h>
using namespace rgb_matrix;
using namespace std::chrono;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
    interrupt_received = true;
}

int main() {
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
    Color sky(0, 0, 255);
    Color ground(0, 128, 0);
    Color cloud(255, 255, 255);
    Color balloon(255, 0, 0);
    Color string_color(255, 255, 255);
    
    // Initial balloon positions
    int balloon_start_x[] = {5, 20, 27};
    int balloon_start_y[] = {18, 16, 20};
    float balloon_y[] = {18.0f, 16.0f, 20.0f};
    steady_clock::time_point last_disappeared[3];
    for (int i = 0; i < 3; ++i) {
        last_disappeared[i] = steady_clock::now();
    }
    bool balloon_visible[] = {true, true, true};
    
    float balloon_speed = 0.05f;  // pixels per frame (adjust for faster/slower)
    
    // Display continuously
    while (!interrupt_received) {
        // Clear canvas and redraw background
        canvas->Fill(sky.r, sky.g, sky.b);
        
        // Draw ground (bottom 8 rows)
        for (int y = 24; y < 32; ++y)
            for (int x = 0; x < 32; ++x)
                canvas->SetPixel(x, y, ground.r, ground.g, ground.b);
        
        // Draw cloud
        DrawCircle(canvas, 10, 6, 3, cloud);
        DrawCircle(canvas, 12, 7, 2, cloud);
        DrawCircle(canvas, 8, 7, 2, cloud);
        
        // Update and draw balloons
        for (int i = 0; i < 3; ++i) {
            auto now = steady_clock::now();
            auto time_since_disappear = duration_cast<seconds>(now - last_disappeared[i]).count();
            
            // Check if balloon should reappear
            if (!balloon_visible[i] && time_since_disappear >= 10) {
                balloon_visible[i] = true;
                balloon_y[i] = balloon_start_y[i];
            }
            
            if (balloon_visible[i]) {
                // Move balloon up
                balloon_y[i] -= balloon_speed;
                
                // Check if balloon has gone off screen (above top with string)
                if (balloon_y[i] + 8 < 0) {  // balloon radius + string length
                    balloon_visible[i] = false;
                    last_disappeared[i] = steady_clock::now();
                }
                
                // Draw balloon and string if still on screen
                int y_pos = (int)balloon_y[i];
                if (y_pos + 2 >= 0) {  // If any part of balloon is visible
                    DrawCircle(canvas, balloon_start_x[i], y_pos, 2, balloon);
                }
                
                // Draw string
                for (int s = 0; s < 6; ++s) {
                    int string_y = y_pos + 2 + s;
                    if (string_y >= 0 && string_y < 32)
                        canvas->SetPixel(balloon_start_x[i], string_y,
                                       string_color.r, string_color.g, string_color.b);
                }
            }
        }
        
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
