#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace rgb_matrix;

struct Zombie {
    float x;
    float y;
    float speed;
    int arm_frame;
    int leg_frame;
    bool active;
    int zombie_type;  // 0, 1, 2 for variety
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
    Color sky_apocalypse(40, 20, 10);   // Orange-red apocalyptic sky
    Color moon_blood(200, 50, 30);      // Blood red moon
    Color ground(30, 25, 20);           // Dark ground
    Color building_dark(50, 50, 50);    // Dark building
    Color building_light(70, 70, 70);   // Light building
    Color window_lit(200, 180, 100);    // Lit windows
    Color window_dark(20, 20, 25);      // Dark windows
    Color zombie_green(100, 140, 80);   // Zombie flesh
    Color zombie_dark(60, 90, 50);      // Dark zombie
    Color zombie_clothes_brown(80, 60, 50);  // Brown clothes
    Color zombie_clothes_blue(50, 60, 80);   // Blue clothes
    Color zombie_clothes_gray(70, 70, 70);   // Gray clothes
    Color blood_red(150, 0, 0);         // Blood stains
    
    // Initialize zombies
    const int num_zombies = 4;
    Zombie zombies[num_zombies];
    for (int i = 0; i < num_zombies; ++i) {
        zombies[i].x = rand() % 32;
        zombies[i].y = 24;  // Ground level
        zombies[i].speed = 0.05f + (rand() % 5) / 100.0f;  // Very slow shamble
        zombies[i].arm_frame = rand() % 20;
        zombies[i].leg_frame = rand() % 30;
        zombies[i].active = true;
        zombies[i].zombie_type = rand() % 3;
    }
    
    int frame_count = 0;
    
    // Display continuously
    while (true) {
        // Clear canvas with apocalyptic sky
        canvas->Fill(sky_apocalypse.r, sky_apocalypse.g, sky_apocalypse.b);
        
        // Draw blood moon
        for (int y = 3; y <= 7; ++y) {
            for (int x = 24; x <= 28; ++x) {
                int dx = x - 26;
                int dy = y - 5;
                if (dx*dx + dy*dy <= 6) {
                    canvas->SetPixel(x, y, moon_blood.r, moon_blood.g, moon_blood.b);
                }
            }
        }
        
        // Draw city skyline silhouette (buildings in background)
        // Building 1 (left)
        for (int y = 8; y < 20; ++y) {
            for (int x = 2; x < 8; ++x) {
                canvas->SetPixel(x, y, building_dark.r, building_dark.g, building_dark.b);
            }
        }
        // Windows
        for (int wy = 10; wy < 19; wy += 3) {
            for (int wx = 3; wx < 7; wx += 2) {
                int flicker = (frame_count / 10 + wx + wy) % 5;
                Color win_color = (flicker < 3) ? window_lit : window_dark;
                canvas->SetPixel(wx, wy, win_color.r, win_color.g, win_color.b);
            }
        }
        
        // Building 2 (center, taller)
        for (int y = 4; y < 20; ++y) {
            for (int x = 10; x < 16; ++x) {
                canvas->SetPixel(x, y, building_light.r, building_light.g, building_light.b);
            }
        }
        // Windows
        for (int wy = 6; wy < 19; wy += 3) {
            for (int wx = 11; wx < 15; wx += 2) {
                int flicker = (frame_count / 10 + wx + wy) % 5;
                Color win_color = (flicker < 3) ? window_lit : window_dark;
                canvas->SetPixel(wx, wy, win_color.r, win_color.g, win_color.b);
            }
        }
        
        // Building 3 (right)
        for (int y = 10; y < 20; ++y) {
            for (int x = 20; x < 26; ++x) {
                canvas->SetPixel(x, y, building_dark.r, building_dark.g, building_dark.b);
            }
        }
        // Windows
        for (int wy = 12; wy < 19; wy += 3) {
            for (int wx = 21; wx < 25; wx += 2) {
                int flicker = (frame_count / 10 + wx + wy) % 5;
                Color win_color = (flicker < 3) ? window_lit : window_dark;
                canvas->SetPixel(wx, wy, win_color.r, win_color.g, win_color.b);
            }
        }
        
        // Draw ground/street
        for (int y = 28; y < 32; ++y) {
            for (int x = 0; x < 32; ++x) {
                canvas->SetPixel(x, y, ground.r, ground.g, ground.b);
            }
        }
        
        // Road lines
        for (int x = 0; x < 32; x += 4) {
            canvas->SetPixel(x, 29, 80, 80, 80);
            canvas->SetPixel(x + 1, 29, 80, 80, 80);
        }
        
        // Update and draw zombies
        for (int i = 0; i < num_zombies; ++i) {
            if (zombies[i].active) {
                // Shamble forward
                zombies[i].x += zombies[i].speed;
                zombies[i].arm_frame = (zombies[i].arm_frame + 1) % 20;
                zombies[i].leg_frame = (zombies[i].leg_frame + 1) % 30;
                
                // Wrap around
                if (zombies[i].x > 32) {
                    zombies[i].x = -3;
                    zombies[i].zombie_type = rand() % 3;
                }
                
                int zx = (int)zombies[i].x;
                int zy = (int)zombies[i].y;
                
                // Choose clothes color based on type
                Color clothes;
                if (zombies[i].zombie_type == 0) clothes = zombie_clothes_brown;
                else if (zombies[i].zombie_type == 1) clothes = zombie_clothes_blue;
                else clothes = zombie_clothes_gray;
                
                if (zx >= -2 && zx < 32 && zy >= 0 && zy < 32) {
                    // Zombie head (green)
                    if (zy - 4 >= 0) {
                        canvas->SetPixel(zx, zy - 4, zombie_green.r, zombie_green.g, zombie_green.b);
                        canvas->SetPixel(zx + 1, zy - 4, zombie_green.r, zombie_green.g, zombie_green.b);
                    }
                    
                    // Eyes (dark/red)
                    if (zy - 4 >= 0 && (frame_count / 15) % 2 == 0) {
                        canvas->SetPixel(zx, zy - 4, blood_red.r / 2, 0, 0);
                        canvas->SetPixel(zx + 1, zy - 4, blood_red.r / 2, 0, 0);
                    }
                    
                    // Body (clothes)
                    if (zy - 3 >= 0) {
                        canvas->SetPixel(zx, zy - 3, clothes.r, clothes.g, clothes.b);
                        canvas->SetPixel(zx + 1, zy - 3, clothes.r, clothes.g, clothes.b);
                    }
                    if (zy - 2 >= 0) {
                        canvas->SetPixel(zx, zy - 2, clothes.r, clothes.g, clothes.b);
                        canvas->SetPixel(zx + 1, zy - 2, clothes.r, clothes.g, clothes.b);
                    }
                    
                    // Arms (reaching out, animated)
                    bool arms_up = zombies[i].arm_frame < 10;
                    if (arms_up) {
                        // Arms raised
                        if (zy - 3 >= 0 && zx - 1 >= 0) {
                            canvas->SetPixel(zx - 1, zy - 3, zombie_green.r, zombie_green.g, zombie_green.b);
                        }
                        if (zy - 3 >= 0 && zx + 2 < 32) {
                            canvas->SetPixel(zx + 2, zy - 3, zombie_green.r, zombie_green.g, zombie_green.b);
                        }
                    } else {
                        // Arms forward
                        if (zy - 2 >= 0 && zx - 1 >= 0) {
                            canvas->SetPixel(zx - 1, zy - 2, zombie_green.r, zombie_green.g, zombie_green.b);
                        }
                        if (zy - 2 >= 0 && zx + 2 < 32) {
                            canvas->SetPixel(zx + 2, zy - 2, zombie_green.r, zombie_green.g, zombie_green.b);
                        }
                    }
                    
                    // Legs (shambling, animated)
                    bool left_leg_forward = zombies[i].leg_frame < 15;
                    if (left_leg_forward) {
                        if (zy - 1 >= 0) canvas->SetPixel(zx, zy - 1, zombie_dark.r, zombie_dark.g, zombie_dark.b);
                        if (zy >= 0) canvas->SetPixel(zx, zy, zombie_green.r, zombie_green.g, zombie_green.b);
                        
                        if (zy - 1 >= 0) canvas->SetPixel(zx + 1, zy - 1, zombie_dark.r, zombie_dark.g, zombie_dark.b);
                    } else {
                        if (zy - 1 >= 0) canvas->SetPixel(zx, zy - 1, zombie_dark.r, zombie_dark.g, zombie_dark.b);
                        
                        if (zy - 1 >= 0) canvas->SetPixel(zx + 1, zy - 1, zombie_dark.r, zombie_dark.g, zombie_dark.b);
                        if (zy >= 0) canvas->SetPixel(zx + 1, zy, zombie_green.r, zombie_green.g, zombie_green.b);
                    }
                    
                    // Blood stains (random on some zombies)
                    if (zombies[i].zombie_type == 1 && zy - 2 >= 0) {
                        canvas->SetPixel(zx, zy - 2, blood_red.r, blood_red.g, blood_red.b);
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
