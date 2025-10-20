#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
using namespace rgb_matrix;

enum Scene {
    CABIN_WINDOW,
    TRANSITION_1,
    EVERGREEN_SNOW,
    TRANSITION_2,
    SNOWFLAKE_CLOSEUP
};

struct Snowflake {
    float x;
    float y;
    float speed;
    int size;
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
    Color cabin_wood(101, 67, 33);
    Color cabin_dark(70, 45, 20);
    Color window_frame(80, 60, 40);
    Color glass_tint(180, 200, 220);
    Color sky_winter(200, 210, 230);
    Color ground_snow(240, 245, 255);
    Color tree_green(40, 100, 60);
    Color tree_dark(20, 60, 30);
    Color snow_white(255, 255, 255);
    Color frost(220, 235, 255);
    
    // Snowflakes
    const int num_snowflakes = 25;
    Snowflake snowflakes[num_snowflakes];
    for (int i = 0; i < num_snowflakes; ++i) {
        snowflakes[i].x = rand() % 32;
        snowflakes[i].y = -(rand() % 32);
        snowflakes[i].speed = 0.1f + (rand() % 10) / 30.0f;
        snowflakes[i].size = 1;
        snowflakes[i].active = true;
    }
    
    Scene current_scene = CABIN_WINDOW;
    int frame_count = 0;
    int scene_timer = 0;
    float transition_progress = 0.0f;
    
    while (true) {
        scene_timer++;
        
        // Scene timing
        if (current_scene == CABIN_WINDOW && scene_timer > 150) {
            current_scene = TRANSITION_1;
            scene_timer = 0;
        } else if (current_scene == TRANSITION_1 && scene_timer > 30) {
            current_scene = EVERGREEN_SNOW;
            scene_timer = 0;
        } else if (current_scene == EVERGREEN_SNOW && scene_timer > 150) {
            current_scene = TRANSITION_2;
            scene_timer = 0;
        } else if (current_scene == TRANSITION_2 && scene_timer > 30) {
            current_scene = SNOWFLAKE_CLOSEUP;
            scene_timer = 0;
        } else if (current_scene == SNOWFLAKE_CLOSEUP && scene_timer > 150) {
            current_scene = CABIN_WINDOW;
            scene_timer = 0;
        }
        
        // Calculate transition progress
        if (current_scene == TRANSITION_1 || current_scene == TRANSITION_2) {
            transition_progress = scene_timer / 30.0f;
        }
        
        // Draw based on current scene
        if (current_scene == CABIN_WINDOW || current_scene == TRANSITION_1) {
            float alpha = (current_scene == TRANSITION_1) ? (1.0f - transition_progress) : 1.0f;
            
            // Draw cabin interior wall (left side)
            for (int y = 0; y < 32; ++y) {
                for (int x = 0; x < 8; ++x) {
                    canvas->SetPixel(x, y, 
                                   (int)(cabin_wood.r * alpha),
                                   (int)(cabin_wood.g * alpha),
                                   (int)(cabin_wood.b * alpha));
                }
            }
            
            // Wood grain detail
            for (int y = 5; y < 32; y += 6) {
                for (int x = 0; x < 8; ++x) {
                    canvas->SetPixel(x, y,
                                   (int)(cabin_dark.r * alpha),
                                   (int)(cabin_dark.g * alpha),
                                   (int)(cabin_dark.b * alpha));
                }
            }
            
            // Window frame
            for (int y = 4; y < 28; ++y) {
                canvas->SetPixel(8, y, (int)(window_frame.r * alpha), (int)(window_frame.g * alpha), (int)(window_frame.b * alpha));
                canvas->SetPixel(27, y, (int)(window_frame.r * alpha), (int)(window_frame.g * alpha), (int)(window_frame.b * alpha));
            }
            for (int x = 8; x < 28; ++x) {
                canvas->SetPixel(x, 4, (int)(window_frame.r * alpha), (int)(window_frame.g * alpha), (int)(window_frame.b * alpha));
                canvas->SetPixel(x, 27, (int)(window_frame.r * alpha), (int)(window_frame.g * alpha), (int)(window_frame.b * alpha));
            }
            // Center mullion
            for (int y = 4; y < 28; ++y) {
                canvas->SetPixel(17, y, (int)(window_frame.r * alpha), (int)(window_frame.g * alpha), (int)(window_frame.b * alpha));
            }
            
            // View through window - snowy landscape
            for (int y = 5; y < 27; ++y) {
                for (int x = 9; x < 27; ++x) {
                    if (x == 17) continue; // Skip mullion
                    
                    // Sky
                    if (y < 18) {
                        canvas->SetPixel(x, y,
                                       (int)(sky_winter.r * alpha),
                                       (int)(sky_winter.g * alpha),
                                       (int)(sky_winter.b * alpha));
                    } else {
                        // Ground
                        canvas->SetPixel(x, y,
                                       (int)(ground_snow.r * alpha),
                                       (int)(ground_snow.g * alpha),
                                       (int)(ground_snow.b * alpha));
                    }
                }
            }
            
            // Tree visible through window (larger, more defined)
            int tree_x = 21;
            int tree_y = 18;
            
            // Trunk
            for (int y = 0; y < 6; ++y) {
                if (tree_y + y < 27) {
                    canvas->SetPixel(tree_x, tree_y + y,
                                   (int)(cabin_dark.r * alpha),
                                   (int)(cabin_dark.g * alpha),
                                   (int)(cabin_dark.b * alpha));
                }
            }
            
            // Layered evergreen shape (3 tiers)
            // Top tier
            for (int dy = -7; dy <= -5; ++dy) {
                int width = (-dy - 5) + 1;
                for (int dx = -width; dx <= width; ++dx) {
                    int px = tree_x + dx;
                    int py = tree_y + dy;
                    if (px > 8 && px < 27 && px != 17 && py >= 5 && py < 27) {
                        canvas->SetPixel(px, py,
                                       (int)(tree_green.r * alpha),
                                       (int)(tree_green.g * alpha),
                                       (int)(tree_green.b * alpha));
                    }
                }
            }
            
            // Middle tier
            for (int dy = -5; dy <= -3; ++dy) {
                int width = (-dy - 3) + 2;
                for (int dx = -width; dx <= width; ++dx) {
                    int px = tree_x + dx;
                    int py = tree_y + dy;
                    if (px > 8 && px < 27 && px != 17 && py >= 5 && py < 27) {
                        canvas->SetPixel(px, py,
                                       (int)(tree_green.r * alpha),
                                       (int)(tree_green.g * alpha),
                                       (int)(tree_green.b * alpha));
                    }
                }
            }
            
            // Bottom tier
            for (int dy = -3; dy <= -1; ++dy) {
                int width = (-dy - 1) + 3;
                for (int dx = -width; dx <= width; ++dx) {
                    int px = tree_x + dx;
                    int py = tree_y + dy;
                    if (px > 8 && px < 27 && px != 17 && py >= 5 && py < 27) {
                        canvas->SetPixel(px, py,
                                       (int)(tree_green.r * alpha),
                                       (int)(tree_green.g * alpha),
                                       (int)(tree_green.b * alpha));
                    }
                }
            }
            
            // Snow on tree tips
            canvas->SetPixel(tree_x, tree_y - 7, (int)(snow_white.r * alpha), (int)(snow_white.g * alpha), (int)(snow_white.b * alpha));
            if (tree_x - 2 > 8 && tree_x - 2 != 17) {
                canvas->SetPixel(tree_x - 2, tree_y - 5, (int)(snow_white.r * alpha), (int)(snow_white.g * alpha), (int)(snow_white.b * alpha));
            }
            if (tree_x + 2 < 27 && tree_x + 2 != 17) {
                canvas->SetPixel(tree_x + 2, tree_y - 5, (int)(snow_white.r * alpha), (int)(snow_white.g * alpha), (int)(snow_white.b * alpha));
            }
            
            // Frost on window edges
            if ((frame_count / 5) % 2 == 0) {
                for (int i = 0; i < 5; ++i) {
                    int fx = 9 + rand() % 2;
                    int fy = 5 + rand() % 22;
                    canvas->SetPixel(fx, fy, (int)(frost.r * alpha), (int)(frost.g * alpha), (int)(frost.b * alpha));
                }
            }
        }
        
        if (current_scene == EVERGREEN_SNOW || 
            current_scene == TRANSITION_1 || 
            current_scene == TRANSITION_2) {
            
            float alpha = 1.0f;
            if (current_scene == TRANSITION_1) alpha = transition_progress;
            if (current_scene == TRANSITION_2) alpha = 1.0f - transition_progress;
            
            // Sky
            for (int y = 0; y < 24; ++y) {
                for (int x = 0; x < 32; ++x) {
                    canvas->SetPixel(x, y,
                                   (int)(sky_winter.r * alpha),
                                   (int)(sky_winter.g * alpha),
                                   (int)(sky_winter.b * alpha));
                }
            }
            
            // Ground
            for (int y = 24; y < 32; ++y) {
                for (int x = 0; x < 32; ++x) {
                    canvas->SetPixel(x, y,
                                   (int)(ground_snow.r * alpha),
                                   (int)(ground_snow.g * alpha),
                                   (int)(ground_snow.b * alpha));
                }
            }
            
            // Large evergreen tree (center) - improved layered design
            int tx = 16;
            int ty = 24;
            
            // Trunk
            for (int y = 0; y < 6; ++y) {
                for (int x = -1; x <= 0; ++x) {
                    canvas->SetPixel(tx + x, ty + y, (int)(cabin_dark.r * alpha), (int)(cabin_dark.g * alpha), (int)(cabin_dark.b * alpha));
                }
            }
            
            // Tree - layered triangular sections (like a real evergreen)
            // Top section (small triangle)
            for (int dy = -20; dy <= -16; ++dy) {
                int width = ((-dy - 16) / 2);
                for (int dx = -width; dx <= width; ++dx) {
                    int py = ty + dy;
                    int px = tx + dx;
                    if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                        canvas->SetPixel(px, py,
                                       (int)(tree_green.r * alpha),
                                       (int)(tree_green.g * alpha),
                                       (int)(tree_green.b * alpha));
                    }
                }
            }
            
            // Second section
            for (int dy = -16; dy <= -11; ++dy) {
                int width = ((-dy - 11) / 2) + 2;
                for (int dx = -width; dx <= width; ++dx) {
                    int py = ty + dy;
                    int px = tx + dx;
                    if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                        canvas->SetPixel(px, py,
                                       (int)(tree_green.r * alpha),
                                       (int)(tree_green.g * alpha),
                                       (int)(tree_green.b * alpha));
                    }
                }
            }
            
            // Third section
            for (int dy = -11; dy <= -6; ++dy) {
                int width = ((-dy - 6) / 2) + 4;
                for (int dx = -width; dx <= width; ++dx) {
                    int py = ty + dy;
                    int px = tx + dx;
                    if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                        canvas->SetPixel(px, py,
                                       (int)(tree_green.r * alpha),
                                       (int)(tree_green.g * alpha),
                                       (int)(tree_green.b * alpha));
                    }
                }
            }
            
            // Bottom section (widest)
            for (int dy = -6; dy <= -1; ++dy) {
                int width = ((-dy - 1) / 2) + 6;
                for (int dx = -width; dx <= width; ++dx) {
                    int py = ty + dy;
                    int px = tx + dx;
                    if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                        canvas->SetPixel(px, py,
                                       (int)(tree_green.r * alpha),
                                       (int)(tree_green.g * alpha),
                                       (int)(tree_green.b * alpha));
                    }
                }
            }
            
            // Snow on tree (tips of branches)
            // Top snow
            canvas->SetPixel(tx, ty - 20, (int)(snow_white.r * alpha), (int)(snow_white.g * alpha), (int)(snow_white.b * alpha));
            
            // Snow on layer tips
            int snow_layers[] = {-16, -11, -6};
            int snow_widths[] = {2, 4, 6};
            for (int layer = 0; layer < 3; ++layer) {
                int sy = ty + snow_layers[layer];
                int sw = snow_widths[layer];
                
                for (int dx = -sw; dx <= sw; dx += 2) {
                    if (tx + dx >= 0 && tx + dx < 32 && sy >= 0 && sy < 32) {
                        canvas->SetPixel(tx + dx, sy,
                                       (int)(snow_white.r * alpha),
                                       (int)(snow_white.g * alpha),
                                       (int)(snow_white.b * alpha));
                    }
                }
            }
            
            // Falling snow
            for (int i = 0; i < num_snowflakes; ++i) {
                if (snowflakes[i].active) {
                    int sx = (int)snowflakes[i].x;
                    int sy = (int)snowflakes[i].y;
                    if (sx >= 0 && sx < 32 && sy >= 0 && sy < 24) {
                        canvas->SetPixel(sx, sy,
                                       (int)(snow_white.r * alpha),
                                       (int)(snow_white.g * alpha),
                                       (int)(snow_white.b * alpha));
                    }
                }
            }
        }
        
        if (current_scene == SNOWFLAKE_CLOSEUP || current_scene == TRANSITION_2) {
            float alpha = (current_scene == TRANSITION_2) ? transition_progress : 1.0f;
            
            // Soft blue gradient background
            for (int y = 0; y < 32; ++y) {
                for (int x = 0; x < 32; ++x) {
                    int r = (int)(100 + y * 2) * alpha;
                    int g = (int)(150 + y * 2) * alpha;
                    int b = (int)(200 + y) * alpha;
                    canvas->SetPixel(x, y, r, g, b);
                }
            }
            
            // Large sparkling snowflake (center)
            int cx = 16;
            int cy = 16;
            
            // Rotation for sparkle effect
            float rotation = frame_count * 0.05f;
            float sparkle = sin(frame_count * 0.2f) * 0.3f + 0.7f;
            
            // Six main arms
            for (int arm = 0; arm < 6; ++arm) {
                float angle = arm * M_PI / 3.0f + rotation;
                
                // Main arm
                for (int r = 1; r <= 10; ++r) {
                    int px = cx + (int)(cos(angle) * r);
                    int py = cy + (int)(sin(angle) * r);
                    
                    if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                        // Color shifts along arm - blues and touch of green
                        int blue_intensity = (int)(255 * sparkle * alpha);
                        int green_intensity = (int)((50 + r * 5) * sparkle * alpha);
                        int cyan_intensity = (int)((200 - r * 10) * sparkle * alpha);
                        
                        canvas->SetPixel(px, py, cyan_intensity / 2, green_intensity + cyan_intensity, blue_intensity);
                    }
                }
                
                // Branches
                for (int branch_pos = 3; branch_pos <= 8; branch_pos += 3) {
                    int bx = cx + (int)(cos(angle) * branch_pos);
                    int by = cy + (int)(sin(angle) * branch_pos);
                    
                    // Left branch
                    float branch_angle_l = angle - M_PI / 6.0f;
                    for (int br = 1; br <= 3; ++br) {
                        int px = bx + (int)(cos(branch_angle_l) * br);
                        int py = by + (int)(sin(branch_angle_l) * br);
                        if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                            int blue_val = (int)(200 * sparkle * alpha);
                            int cyan_val = (int)(150 * sparkle * alpha);
                            canvas->SetPixel(px, py, cyan_val / 2, cyan_val, blue_val);
                        }
                    }
                    
                    // Right branch
                    float branch_angle_r = angle + M_PI / 6.0f;
                    for (int br = 1; br <= 3; ++br) {
                        int px = bx + (int)(cos(branch_angle_r) * br);
                        int py = by + (int)(sin(branch_angle_r) * br);
                        if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                            int blue_val = (int)(200 * sparkle * alpha);
                            int cyan_val = (int)(150 * sparkle * alpha);
                            canvas->SetPixel(px, py, cyan_val / 2, cyan_val, blue_val);
                        }
                    }
                }
            }
            
            // Bright center
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    int bright = (int)(255 * sparkle * alpha);
                    canvas->SetPixel(cx + dx, cy + dy, bright / 2, bright, bright);
                }
            }
            
            // Sparkle points around snowflake
            for (int s = 0; s < 8; ++s) {
                float spark_angle = s * M_PI / 4.0f + frame_count * 0.1f;
                int spark_dist = 12 + (int)(sin(frame_count * 0.15f + s) * 2);
                int sx = cx + (int)(cos(spark_angle) * spark_dist);
                int sy = cy + (int)(sin(spark_angle) * spark_dist);
                
                if (sx >= 0 && sx < 32 && sy >= 0 && sy < 32) {
                    int spark = (int)(200 * sparkle * alpha);
                    canvas->SetPixel(sx, sy, spark / 3, spark, spark);
                }
            }
        }
        
        // Update snowflakes
        for (int i = 0; i < num_snowflakes; ++i) {
            if (snowflakes[i].active) {
                snowflakes[i].y += snowflakes[i].speed;
                if (snowflakes[i].y > 32) {
                    snowflakes[i].y = 0;
                    snowflakes[i].x = rand() % 32;
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
