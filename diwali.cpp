
#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
using namespace rgb_matrix;

struct Diya {
    int x;
    int y;
    int flicker_phase;
};

struct Sparkle {
    float x;
    float y;
    float vx;
    float vy;
    int lifetime;
    int max_lifetime;
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
    
    // Colors for Diwali
    Color bg_night(5, 5, 20);              // Deep night blue
    Color diya_clay(180, 100, 60);         // Clay lamp color
    Color flame_yellow(255, 220, 100);     // Yellow flame
    Color flame_orange(255, 150, 50);      // Orange flame
    Color flame_red(255, 80, 40);          // Red flame base
    Color flame_white(255, 255, 230);      // White flame tip
    Color rangoli_red(255, 50, 80);        // Rangoli colors
    Color rangoli_orange(255, 150, 50);
    Color rangoli_yellow(255, 220, 100);
    Color rangoli_green(100, 255, 150);
    Color rangoli_blue(100, 150, 255);
    Color rangoli_pink(255, 150, 200);
    Color rangoli_purple(200, 100, 255);
    
    // Diya lamps (oil lamps) positions
    Diya diyas[5];
    diyas[0] = {6, 26, 0};
    diyas[1] = {13, 28, 10};
    diyas[2] = {16, 25, 5};
    diyas[3] = {20, 28, 15};
    diyas[4] = {26, 26, 8};
    
    // Sparkles/fireworks particles
    const int num_sparkles = 30;
    Sparkle sparkles[num_sparkles];
    for (int i = 0; i < num_sparkles; ++i) {
        sparkles[i].active = false;
    }
    
    int frame_count = 0;
    int firework_timer = 0;
    
    // Display continuously
    while (true) {
        // Clear with night sky
        canvas->Fill(bg_night.r, bg_night.g, bg_night.b);
        
        // Draw decorative rangoli pattern in background (centered)
        int cx = 16;
        int cy = 12;
        
        // Outer ring - alternating colors
        Color ring_colors[] = {rangoli_red, rangoli_orange, rangoli_yellow, 
                              rangoli_green, rangoli_blue, rangoli_purple, rangoli_pink};
        
        for (int angle = 0; angle < 360; angle += 30) {
            float rad = angle * M_PI / 180.0f;
            for (int r = 6; r <= 8; ++r) {
                int px = cx + (int)(cos(rad) * r);
                int py = cy + (int)(sin(rad) * r);
                if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                    Color col = ring_colors[(angle / 30) % 7];
                    canvas->SetPixel(px, py, col.r / 3, col.g / 3, col.b / 3);
                }
            }
        }
        
        // Inner petals (8 petals)
        for (int petal = 0; petal < 8; ++petal) {
            float angle = petal * M_PI / 4.0f + frame_count * 0.01f;
            for (int d = 3; d <= 5; ++d) {
                int px = cx + (int)(cos(angle) * d);
                int py = cy + (int)(sin(angle) * d);
                if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                    Color col = ring_colors[petal % 7];
                    canvas->SetPixel(px, py, col.r / 2, col.g / 2, col.b / 2);
                }
            }
        }
        
        // Center dot - glowing
        float pulse = sin(frame_count * 0.1f) * 0.3f + 0.7f;
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (abs(dx) + abs(dy) <= 1) {
                    canvas->SetPixel(cx + dx, cy + dy, 
                                   (int)(rangoli_yellow.r * pulse),
                                   (int)(rangoli_yellow.g * pulse),
                                   (int)(rangoli_yellow.b * pulse));
                }
            }
        }
        
        // Draw diyas (oil lamps)
        for (int d = 0; d < 5; ++d) {
            int dx = diyas[d].x;
            int dy = diyas[d].y;
            
            // Diya base (clay bowl)
            canvas->SetPixel(dx - 1, dy + 1, diya_clay.r, diya_clay.g, diya_clay.b);
            canvas->SetPixel(dx, dy + 1, diya_clay.r, diya_clay.g, diya_clay.b);
            canvas->SetPixel(dx + 1, dy + 1, diya_clay.r, diya_clay.g, diya_clay.b);
            canvas->SetPixel(dx - 1, dy + 2, diya_clay.r, diya_clay.g, diya_clay.b);
            canvas->SetPixel(dx, dy + 2, diya_clay.r, diya_clay.g, diya_clay.b);
            canvas->SetPixel(dx + 1, dy + 2, diya_clay.r, diya_clay.g, diya_clay.b);
            
            // Flickering flame
            int flicker = (frame_count + diyas[d].flicker_phase) % 12;
            
            // Flame base (red)
            canvas->SetPixel(dx, dy, flame_red.r, flame_red.g, flame_red.b);
            
            // Flame middle (orange/yellow)
            if (flicker < 6) {
                canvas->SetPixel(dx, dy - 1, flame_orange.r, flame_orange.g, flame_orange.b);
                canvas->SetPixel(dx, dy - 2, flame_yellow.r, flame_yellow.g, flame_yellow.b);
                if (flicker < 3) {
                    canvas->SetPixel(dx, dy - 3, flame_white.r, flame_white.g, flame_white.b);
                }
            } else {
                canvas->SetPixel(dx, dy - 1, flame_yellow.r, flame_yellow.g, flame_yellow.b);
                canvas->SetPixel(dx, dy - 2, flame_orange.r, flame_orange.g, flame_orange.b);
            }
            
            // Glow around flame
            if ((frame_count / 3) % 2 == 0) {
                canvas->SetPixel(dx - 1, dy - 1, flame_orange.r / 2, flame_orange.g / 2, flame_orange.b / 2);
                canvas->SetPixel(dx + 1, dy - 1, flame_orange.r / 2, flame_orange.g / 2, flame_orange.b / 2);
            }
        }
        
        // Launch firework sparkles periodically
        firework_timer++;
        if (firework_timer > 30) {
            for (int i = 0; i < num_sparkles; ++i) {
                if (!sparkles[i].active) {
                    // Launch from random top position
                    sparkles[i].x = 8 + rand() % 16;
                    sparkles[i].y = 3 + rand() % 8;
                    
                    // Explode outward
                    float angle = (rand() % 360) * M_PI / 180.0f;
                    float speed = 0.3f + (rand() % 10) / 20.0f;
                    sparkles[i].vx = cos(angle) * speed;
                    sparkles[i].vy = sin(angle) * speed;
                    
                    sparkles[i].lifetime = 0;
                    sparkles[i].max_lifetime = 20 + rand() % 20;
                    sparkles[i].color = ring_colors[rand() % 7];
                    sparkles[i].active = true;
                    
                    if (i % 3 == 0) break; // Only launch a few at a time
                }
            }
            firework_timer = 0;
        }
        
        // Update and draw sparkles
        for (int i = 0; i < num_sparkles; ++i) {
            if (sparkles[i].active) {
                sparkles[i].x += sparkles[i].vx;
                sparkles[i].y += sparkles[i].vy;
                sparkles[i].vy += 0.05f; // Gravity
                sparkles[i].lifetime++;
                
                if (sparkles[i].lifetime > sparkles[i].max_lifetime ||
                    sparkles[i].y > 32 || sparkles[i].x < 0 || sparkles[i].x > 31) {
                    sparkles[i].active = false;
                }
                
                // Draw sparkle with fading
                int sx = (int)sparkles[i].x;
                int sy = (int)sparkles[i].y;
                
                if (sx >= 0 && sx < 32 && sy >= 0 && sy < 32) {
                    float fade = 1.0f - ((float)sparkles[i].lifetime / sparkles[i].max_lifetime);
                    int r = (int)(sparkles[i].color.r * fade);
                    int g = (int)(sparkles[i].color.g * fade);
                    int b = (int)(sparkles[i].color.b * fade);
                    
                    canvas->SetPixel(sx, sy, r, g, b);
                    
                    // Trail
                    if (sparkles[i].lifetime < 10 && sy + 1 < 32) {
                        canvas->SetPixel(sx, sy + 1, r / 2, g / 2, b / 2);
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
