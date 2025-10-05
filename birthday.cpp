#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace rgb_matrix;

struct Confetti {
    float x;
    float y;
    float speed;
    float rotation;
    Color color;
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
    Color bg_purple(30, 10, 50);         // Purple party background
    Color cake_pink(255, 150, 200);      // Pink cake layer
    Color cake_blue(100, 200, 255);      // Blue cake layer
    Color cake_yellow(255, 220, 100);    // Yellow cake layer
    Color frosting(255, 255, 255);       // White frosting
    Color candle_red(255, 0, 0);         // Red candle
    Color candle_yellow(255, 200, 0);    // Yellow candle
    Color candle_blue(0, 150, 255);      // Blue candle
    Color flame_yellow(255, 255, 0);     // Yellow flame
    Color flame_orange(255, 150, 0);     // Orange flame
    Color table(139, 90, 43);            // Brown table
    
    // Confetti colors
    Color confetti_colors[] = {
        Color(255, 0, 0),      // Red
        Color(255, 150, 0),    // Orange
        Color(255, 255, 0),    // Yellow
        Color(0, 255, 0),      // Green
        Color(0, 150, 255),    // Blue
        Color(200, 0, 255),    // Purple
        Color(255, 0, 200)     // Pink
    };
    
    // Initialize confetti
    const int num_confetti = 30;
    Confetti confetti[num_confetti];
    for (int i = 0; i < num_confetti; ++i) {
        confetti[i].x = rand() % 32;
        confetti[i].y = -(rand() % 32);
        confetti[i].speed = 0.2f + (rand() % 10) / 20.0f;
        confetti[i].rotation = rand() % 360;
        confetti[i].color = confetti_colors[rand() % 7];
        confetti[i].active = true;
    }
    
    int frame_count = 0;
    
    // Display continuously
    while (true) {
        // Clear canvas with party background
        canvas->Fill(bg_purple.r, bg_purple.g, bg_purple.b);
        
        // Draw table (bottom section)
        for (int y = 26; y < 32; ++y) {
            for (int x = 0; x < 32; ++x) {
                canvas->SetPixel(x, y, table.r, table.g, table.b);
            }
        }
        
        // Draw birthday cake (centered, 3 tiers)
        int cake_center_x = 16;
        
        // Bottom tier (yellow)
        for (int y = 20; y < 26; ++y) {
            for (int x = 8; x < 24; ++x) {
                canvas->SetPixel(x, y, cake_yellow.r, cake_yellow.g, cake_yellow.b);
            }
        }
        // Bottom frosting line
        for (int x = 8; x < 24; ++x) {
            canvas->SetPixel(x, 20, frosting.r, frosting.g, frosting.b);
        }
        
        // Middle tier (blue)
        for (int y = 15; y < 20; ++y) {
            for (int x = 10; x < 22; ++x) {
                canvas->SetPixel(x, y, cake_blue.r, cake_blue.g, cake_blue.b);
            }
        }
        // Middle frosting line
        for (int x = 10; x < 22; ++x) {
            canvas->SetPixel(x, 15, frosting.r, frosting.g, frosting.b);
        }
        
        // Top tier (pink)
        for (int y = 11; y < 15; ++y) {
            for (int x = 12; x < 20; ++x) {
                canvas->SetPixel(x, y, cake_pink.r, cake_pink.g, cake_pink.b);
            }
        }
        // Top frosting line
        for (int x = 12; x < 20; ++x) {
            canvas->SetPixel(x, 11, frosting.r, frosting.g, frosting.b);
        }
        
        // Draw candles (3 candles on top tier)
        int candle_positions[] = {13, 16, 19};
        Color candle_colors[] = {candle_red, candle_yellow, candle_blue};
        
        for (int i = 0; i < 3; ++i) {
            int cx = candle_positions[i];
            // Candle stick
            canvas->SetPixel(cx, 9, candle_colors[i].r, candle_colors[i].g, candle_colors[i].b);
            canvas->SetPixel(cx, 10, candle_colors[i].r, candle_colors[i].g, candle_colors[i].b);
            
            // Flickering flame
            int flicker = (frame_count + i * 5) % 10;
            if (flicker < 5) {
                canvas->SetPixel(cx, 8, flame_yellow.r, flame_yellow.g, flame_yellow.b);
                if (flicker < 3) {
                    canvas->SetPixel(cx, 7, flame_orange.r, flame_orange.g, flame_orange.b);
                }
            } else {
                canvas->SetPixel(cx, 8, flame_orange.r, flame_orange.g, flame_orange.b);
            }
        }
        
        // Draw decorative dots on cake
        for (int tier = 0; tier < 3; ++tier) {
            int y_pos = 22 - (tier * 5);
            int x_start = 9 + tier * 2;
            int x_end = 23 - tier * 2;
            
            for (int x = x_start; x < x_end; x += 3) {
                int dot_frame = (frame_count / 5 + x) % 2;
                if (dot_frame == 0) {
                    canvas->SetPixel(x, y_pos, frosting.r, frosting.g, frosting.b);
                }
            }
        }
        
        // Update and draw confetti
        for (int i = 0; i < num_confetti; ++i) {
            if (confetti[i].active) {
                // Move confetti down
                confetti[i].y += confetti[i].speed;
                confetti[i].rotation += 5;
                
                // Reset if it goes off screen
                if (confetti[i].y >= 26) {  // Stop at table
                    confetti[i].y = 0;
                    confetti[i].x = rand() % 32;
                    confetti[i].speed = 0.2f + (rand() % 10) / 20.0f;
                    confetti[i].color = confetti_colors[rand() % 7];
                }
                
                // Draw confetti piece (1-2 pixels depending on rotation)
                int conf_x = (int)confetti[i].x;
                int conf_y = (int)confetti[i].y;
                
                if (conf_y >= 0 && conf_y < 26 && conf_x >= 0 && conf_x < 32) {
                    canvas->SetPixel(conf_x, conf_y, 
                                   confetti[i].color.r, 
                                   confetti[i].color.g, 
                                   confetti[i].color.b);
                    
                    // Make some confetti pieces 2 pixels for variety
                    if (((int)(confetti[i].rotation / 45)) % 2 == 0 && conf_x + 1 < 32) {
                        canvas->SetPixel(conf_x + 1, conf_y, 
                                       confetti[i].color.r, 
                                       confetti[i].color.g, 
                                       confetti[i].color.b);
                    }
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
