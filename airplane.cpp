#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <cmath>
using namespace rgb_matrix;

enum FlightPhase {
    TAXIING,
    TAKEOFF,
    CLIMBING,
    CRUISING,
    DESCENDING,
    LANDING,
    ARRIVED
};

struct Cloud {
    float x;
    float y;
    int size;
};

int main() {
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
    Color sky_blue(100, 180, 255);
    Color sky_horizon(180, 220, 255);
    Color runway_gray(80, 80, 80);
    Color runway_lines(220, 220, 220);
    Color plane_white(240, 240, 240);
    Color plane_dark(150, 150, 150);
    Color plane_red(220, 50, 50);
    Color plane_blue(50, 100, 200);
    Color cloud_white(255, 255, 255);
    Color sun_yellow(255, 220, 100);
    Color grass_green(80, 150, 60);
    
    // Plane state
    float plane_x = 5.0f;
    float plane_y = 24.0f;
    float plane_angle = 0.0f;  // 0 = level, positive = nose up
    FlightPhase phase = TAXIING;
    
    // Clouds
    Cloud clouds[4];
    clouds[0] = {10, 8, 3};
    clouds[1] = {24, 12, 2};
    clouds[2] = {5, 15, 3};
    clouds[3] = {20, 6, 2};
    
    int frame_count = 0;
    int phase_timer = 0;
    
    // Display continuously
    while (true) {
        // Draw gradient sky
        for (int y = 0; y < 32; ++y) {
            float t = y / 32.0f;
            int r = (int)(sky_blue.r + (sky_horizon.r - sky_blue.r) * t);
            int g = (int)(sky_blue.g + (sky_horizon.g - sky_blue.g) * t);
            int b = (int)(sky_blue.b + (sky_horizon.b - sky_blue.b) * t);
            
            for (int x = 0; x < 32; ++x) {
                canvas->SetPixel(x, y, r, g, b);
            }
        }
        
        // Draw sun
        int sun_x = 26;
        int sun_y = 5;
        for (int dy = -2; dy <= 2; ++dy) {
            for (int dx = -2; dx <= 2; ++dx) {
                if (dx*dx + dy*dy <= 4) {
                    int px = sun_x + dx;
                    int py = sun_y + dy;
                    if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                        canvas->SetPixel(px, py, sun_yellow.r, sun_yellow.g, sun_yellow.b);
                    }
                }
            }
        }
        
        // Update plane based on phase
        phase_timer++;
        
        switch(phase) {
            case TAXIING:
                plane_x += 0.1f;
                plane_y = 24.0f;
                plane_angle = 0.0f;
                if (phase_timer > 60) {
                    phase = TAKEOFF;
                    phase_timer = 0;
                }
                break;
                
            case TAKEOFF:
                plane_x += 0.15f;
                plane_y -= 0.08f;
                plane_angle = 15.0f;
                if (phase_timer > 50) {
                    phase = CLIMBING;
                    phase_timer = 0;
                }
                break;
                
            case CLIMBING:
                plane_x += 0.2f;
                plane_y -= 0.15f;
                plane_angle = 20.0f;
                if (plane_y < 10.0f || phase_timer > 50) {
                    phase = CRUISING;
                    phase_timer = 0;
                }
                break;
                
            case CRUISING:
                plane_x += 0.25f;
                plane_y = 8.0f + sin(frame_count * 0.05f) * 1.5f;
                plane_angle = sin(frame_count * 0.05f) * 3.0f;
                if (phase_timer > 80) {
                    phase = DESCENDING;
                    phase_timer = 0;
                }
                break;
                
            case DESCENDING:
                plane_x += 0.2f;
                plane_y += 0.1f;
                plane_angle = -10.0f;
                if (plane_y > 20.0f || phase_timer > 60) {
                    phase = LANDING;
                    phase_timer = 0;
                }
                break;
                
            case LANDING:
                plane_x += 0.12f;
                plane_y += 0.05f;
                plane_angle = -5.0f;
                if (plane_y >= 24.0f) {
                    plane_y = 24.0f;
                    plane_angle = 0.0f;
                    phase = ARRIVED;
                    phase_timer = 0;
                }
                break;
                
            case ARRIVED:
                plane_x += 0.05f;
                plane_y = 24.0f;
                plane_angle = 0.0f;
                if (phase_timer > 40) {
                    // Reset for loop
                    plane_x = 5.0f;
                    plane_y = 24.0f;
                    phase = TAXIING;
                    phase_timer = 0;
                }
                break;
        }
        
        // Wrap plane if off screen
        if (plane_x > 35.0f) {
            plane_x = -5.0f;
        }
        
        // Draw runway when plane is low
        if (plane_y > 20.0f) {
            // Grass
            for (int y = 26; y < 32; ++y) {
                for (int x = 0; x < 32; ++x) {
                    canvas->SetPixel(x, y, grass_green.r, grass_green.g, grass_green.b);
                }
            }
            
            // Runway
            for (int y = 24; y < 26; ++y) {
                for (int x = 0; x < 32; ++x) {
                    canvas->SetPixel(x, y, runway_gray.r, runway_gray.g, runway_gray.b);
                }
            }
            
            // Center line
            for (int x = 0; x < 32; x += 4) {
                canvas->SetPixel(x, 24, runway_lines.r, runway_lines.g, runway_lines.b);
                canvas->SetPixel(x + 1, 24, runway_lines.r, runway_lines.g, runway_lines.b);
            }
        }
        
        // Draw clouds (move slowly)
        for (int i = 0; i < 4; ++i) {
            clouds[i].x -= 0.05f;
            if (clouds[i].x < -5) {
                clouds[i].x = 35;
            }
            
            int cx = (int)clouds[i].x;
            int cy = (int)clouds[i].y;
            int size = clouds[i].size;
            
            // Draw puffy cloud
            for (int dy = -size; dy <= size; ++dy) {
                for (int dx = -size; dx <= size; ++dx) {
                    if (dx*dx + dy*dy <= size*size) {
                        int px = cx + dx;
                        int py = cy + dy;
                        if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                            canvas->SetPixel(px, py, cloud_white.r, cloud_white.g, cloud_white.b);
                        }
                    }
                }
            }
            // Extra puffs
            canvas->SetPixel(cx - size, cy, cloud_white.r, cloud_white.g, cloud_white.b);
            canvas->SetPixel(cx + size, cy, cloud_white.r, cloud_white.g, cloud_white.b);
        }
        
        // Draw airplane
        int px = (int)plane_x;
        int py = (int)plane_y;
        
        // Adjust drawing based on angle
        bool nose_up = (plane_angle > 5.0f);
        bool nose_down = (plane_angle < -5.0f);
        
        if (px >= -5 && px < 35 && py >= 0 && py < 32) {
            // Fuselage (body) - longer and more defined
            for (int i = 0; i < 6; ++i) {
                int draw_x = px + i;
                int draw_y = py;
                
                if (nose_up && i > 3) draw_y -= 1;
                if (nose_down && i > 3) draw_y += 1;
                
                if (draw_x >= 0 && draw_x < 32 && draw_y >= 0 && draw_y < 32) {
                    // Main body
                    canvas->SetPixel(draw_x, draw_y, plane_white.r, plane_white.g, plane_white.b);
                    
                    // Bottom shadow/detail
                    if (draw_y + 1 < 32 && i > 0 && i < 5) {
                        canvas->SetPixel(draw_x, draw_y + 1, plane_dark.r, plane_dark.g, plane_dark.b);
                    }
                }
            }
            
            // Wings (left wing has RED light, right wing has GREEN light)
            int wing_y = py;
            int wing_x = px + 2;
            
            if (wing_x >= 0 && wing_x < 32) {
                // Left wing (top on screen) - RED navigation light
                if (wing_y - 2 >= 0) {
                    canvas->SetPixel(wing_x, wing_y - 2, plane_red.r, plane_red.g, plane_red.b);
                    canvas->SetPixel(wing_x - 1, wing_y - 2, plane_red.r / 3, plane_red.g / 3, plane_red.b / 3);
                }
                if (wing_y - 1 >= 0) {
                    canvas->SetPixel(wing_x, wing_y - 1, plane_white.r, plane_white.g, plane_white.b);
                    canvas->SetPixel(wing_x + 1, wing_y - 1, plane_white.r, plane_white.g, plane_white.b);
                }
                
                // Right wing (bottom on screen) - GREEN navigation light
                if (wing_y + 1 < 32) {
                    canvas->SetPixel(wing_x, wing_y + 1, plane_white.r, plane_white.g, plane_white.b);
                    canvas->SetPixel(wing_x + 1, wing_y + 1, plane_white.r, plane_white.g, plane_white.b);
                }
                if (wing_y + 2 < 32) {
                    Color green_light(50, 255, 50);
                    canvas->SetPixel(wing_x, wing_y + 2, green_light.r, green_light.g, green_light.b);
                    canvas->SetPixel(wing_x - 1, wing_y + 2, green_light.r / 3, green_light.g / 3, green_light.b / 3);
                }
            }
            
            // Tail section
            int tail_x = px;
            int tail_y = py;
            if (nose_up) tail_y += 1;
            if (nose_down) tail_y -= 1;
            
            // Vertical stabilizer (tail fin)
            if (tail_x >= 0 && tail_x < 32) {
                if (tail_y - 1 >= 0) canvas->SetPixel(tail_x, tail_y - 1, plane_white.r, plane_white.g, plane_white.b);
                if (tail_y - 2 >= 0) canvas->SetPixel(tail_x, tail_y - 2, plane_white.r, plane_white.g, plane_white.b);
            }
            
            // Horizontal stabilizer (smaller wings at back)
            if (tail_x >= 0 && tail_x < 32 && tail_y >= 0 && tail_y < 32) {
                if (tail_y - 1 >= 0 && tail_x - 1 >= 0) {
                    canvas->SetPixel(tail_x - 1, tail_y - 1, plane_white.r, plane_white.g, plane_white.b);
                }
                if (tail_y + 1 < 32 && tail_x - 1 >= 0) {
                    canvas->SetPixel(tail_x - 1, tail_y + 1, plane_white.r, plane_white.g, plane_white.b);
                }
            }
            
            // Nose cone (pointed front)
            int nose_x = px + 5;
            int nose_y = py;
            if (nose_up) nose_y -= 1;
            if (nose_down) nose_y += 1;
            
            if (nose_x >= 0 && nose_x < 32 && nose_y >= 0 && nose_y < 32) {
                canvas->SetPixel(nose_x, nose_y, plane_dark.r, plane_dark.g, plane_dark.b);
            }
            
            // Cockpit windows (blue)
            if (px + 4 >= 0 && px + 4 < 32) {
                int window_y = py;
                if (nose_up) window_y -= 1;
                if (nose_down) window_y += 1;
                if (window_y >= 0 && window_y < 32) {
                    canvas->SetPixel(px + 4, window_y, plane_blue.r, plane_blue.g, plane_blue.b);
                }
            }
            
            // Passenger windows
            for (int w = 1; w <= 3; w++) {
                if (px + w >= 0 && px + w < 32 && py >= 0 && py < 32) {
                    canvas->SetPixel(px + w, py, plane_blue.r, plane_blue.g, plane_blue.b);
                }
            }
            
            // Tail beacon (white flashing light)
            if ((frame_count / 10) % 2 == 0) {
                if (tail_x >= 0 && tail_x < 32 && tail_y - 2 >= 0) {
                    canvas->SetPixel(tail_x, tail_y - 2, 255, 255, 255);
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
