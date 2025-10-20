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

struct Enemy {
    float x;
    float y;
    float vx;
    float vy;
    bool active;
};

int main() {
    srand(time(NULL));
    
    // Set up signal handler
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
    Color bg_black(0, 0, 0);
    Color star_white(255, 255, 255);
    Color balloon_green(100, 255, 100);
    Color balloon_shadow(50, 150, 50);
    Color player_red(255, 80, 80);
    Color player_blue(80, 120, 255);
    Color player_skin(255, 200, 150);
    Color enemy_white(255, 255, 255);
    Color enemy_orange(255, 150, 80);
    Color water_blue(80, 160, 255);
    Color water_light(120, 200, 255);
    
    // Player position
    float player_x = 16.0f;
    float player_y = 20.0f;
    float player_vy = 0.0f;
    bool flapping = false;
    int flap_timer = 0;
    
    // Enemies
    const int num_enemies = 4;
    Enemy enemies[num_enemies];
    for (int i = 0; i < num_enemies; ++i) {
        enemies[i].x = rand() % 26 + 3;
        enemies[i].y = rand() % 12 + 4;
        enemies[i].vx = (rand() % 2 == 0 ? 1 : -1) * 0.15f;
        enemies[i].vy = 0.05f;
        enemies[i].active = true;
    }
    
    // Stars (background)
    int star_x[20];
    int star_y[20];
    for (int i = 0; i < 20; ++i) {
        star_x[i] = rand() % 32;
        star_y[i] = rand() % 26;
    }
    
    int frame_count = 0;
    
    // Display continuously
    while (!interrupt_received) {
        // Clear with black background
        canvas->Fill(bg_black.r, bg_black.g, bg_black.b);
        
        // Draw twinkling stars
        for (int i = 0; i < 20; ++i) {
            int twinkle = (frame_count + i * 7) % 20;
            if (twinkle < 2) {
                canvas->SetPixel(star_x[i], star_y[i], star_white.r, star_white.g, star_white.b);
            } else if (twinkle < 4) {
                canvas->SetPixel(star_x[i], star_y[i], 150, 150, 150);
            }
        }
        
        // Draw water at bottom
        for (int y = 28; y < 32; ++y) {
            for (int x = 0; x < 32; ++x) {
                if ((x + frame_count / 3) % 4 < 2) {
                    canvas->SetPixel(x, y, water_blue.r, water_blue.g, water_blue.b);
                } else {
                    canvas->SetPixel(x, y, water_light.r, water_light.g, water_light.b);
                }
            }
        }
        
        // Water waves
        for (int x = 0; x < 32; x += 2) {
            int wave_offset = (frame_count / 2 + x) % 4;
            canvas->SetPixel(x + wave_offset, 27, water_light.r, water_light.g, water_light.b);
        }
        
        // Update enemies
        for (int i = 0; i < num_enemies; ++i) {
            if (enemies[i].active) {
                enemies[i].x += enemies[i].vx;
                enemies[i].y += enemies[i].vy;
                
                // Bounce off edges
                if (enemies[i].x < 2 || enemies[i].x > 29) {
                    enemies[i].vx *= -1;
                }
                
                // Simple floating motion
                if (enemies[i].y < 4 || enemies[i].y > 20) {
                    enemies[i].vy *= -1;
                }
                
                // Draw enemy
                int ex = (int)enemies[i].x;
                int ey = (int)enemies[i].y;
                
                // Balloons (white/orange)
                canvas->SetPixel(ex - 1, ey - 3, enemy_white.r, enemy_white.g, enemy_white.b);
                canvas->SetPixel(ex - 1, ey - 2, enemy_white.r, enemy_white.g, enemy_white.b);
                canvas->SetPixel(ex + 1, ey - 3, enemy_white.r, enemy_white.g, enemy_white.b);
                canvas->SetPixel(ex + 1, ey - 2, enemy_white.r, enemy_white.g, enemy_white.b);
                
                // Body
                canvas->SetPixel(ex, ey, enemy_orange.r, enemy_orange.g, enemy_orange.b);
                canvas->SetPixel(ex, ey + 1, enemy_orange.r, enemy_orange.g, enemy_orange.b);
                
                // Arms flapping
                if ((frame_count / 5) % 2 == 0) {
                    canvas->SetPixel(ex - 1, ey, enemy_orange.r, enemy_orange.g, enemy_orange.b);
                    canvas->SetPixel(ex + 1, ey, enemy_orange.r, enemy_orange.g, enemy_orange.b);
                }
            }
        }
        
        // Player input simulation (bouncing up and down)
        flap_timer++;
        if (flap_timer > 25) {
            flapping = true;
            player_vy = -0.3f;
            flap_timer = 0;
        } else {
            flapping = false;
        }
        
        // Apply gravity
        player_vy += 0.02f;
        player_y += player_vy;
        
        // Keep player on screen
        if (player_y < 6) player_y = 6;
        if (player_y > 24) {
            player_y = 24;
            player_vy = 0;
        }
        
        // Draw player
        int px = (int)player_x;
        int py = (int)player_y;
        
        // Balloons (green)
        canvas->SetPixel(px - 1, py - 3, balloon_green.r, balloon_green.g, balloon_green.b);
        canvas->SetPixel(px - 1, py - 4, balloon_green.r, balloon_green.g, balloon_green.b);
        canvas->SetPixel(px - 1, py - 2, balloon_shadow.r, balloon_shadow.g, balloon_shadow.b);
        
        canvas->SetPixel(px + 1, py - 3, balloon_green.r, balloon_green.g, balloon_green.b);
        canvas->SetPixel(px + 1, py - 4, balloon_green.r, balloon_green.g, balloon_green.b);
        canvas->SetPixel(px + 1, py - 2, balloon_shadow.r, balloon_shadow.g, balloon_shadow.b);
        
        // Balloon strings
        canvas->SetPixel(px - 1, py - 1, star_white.r, star_white.g, star_white.b);
        canvas->SetPixel(px + 1, py - 1, star_white.r, star_white.g, star_white.b);
        
        // Head
        canvas->SetPixel(px, py, player_skin.r, player_skin.g, player_skin.b);
        
        // Body (red shirt)
        canvas->SetPixel(px, py + 1, player_red.r, player_red.g, player_red.b);
        canvas->SetPixel(px, py + 2, player_red.r, player_red.g, player_red.b);
        
        // Legs (blue)
        canvas->SetPixel(px - 1, py + 3, player_blue.r, player_blue.g, player_blue.b);
        canvas->SetPixel(px + 1, py + 3, player_blue.r, player_blue.g, player_blue.b);
        
        // Arms (flapping when moving up)
        if (flapping || player_vy < 0) {
            canvas->SetPixel(px - 1, py + 1, player_skin.r, player_skin.g, player_skin.b);
            canvas->SetPixel(px + 1, py + 1, player_skin.r, player_skin.g, player_skin.b);
            canvas->SetPixel(px - 2, py, player_skin.r, player_skin.g, player_skin.b);
            canvas->SetPixel(px + 2, py, player_skin.r, player_skin.g, player_skin.b);
        } else {
            canvas->SetPixel(px - 1, py + 1, player_skin.r, player_skin.g, player_skin.b);
            canvas->SetPixel(px + 1, py + 1, player_skin.r, player_skin.g, player_skin.b);
        }
        
        frame_count++;
        canvas = matrix->SwapOnVSync(canvas);
        usleep(50000);  // ~20 fps
    }
    
    // Clean up on exit
    canvas->Clear();
    canvas = matrix->SwapOnVSync(canvas);
    delete matrix;
    return 0;
}
