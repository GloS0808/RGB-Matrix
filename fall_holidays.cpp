#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
using namespace rgb_matrix;

enum Scene {
    HALLOWEEN,
    THANKSGIVING,
    WINTER
};

struct Particle {
    float x;
    float y;
    float speed;
    int type;
    bool active;
};

struct Ghost {
    float x;
    float y;
    float float_offset;
    int phase;
};

// Halloween scene
void drawHalloweenScene(Canvas *canvas, Particle particles[], int num_particles, Ghost ghosts[], int frame_count) {
    Color bg_night(5, 0, 20);
    Color moon(255, 255, 200);
    Color ground(20, 10, 0);
    Color pumpkin_orange(255, 120, 0);
    Color pumpkin_dark(180, 80, 0);
    Color jack_face(255, 200, 0);
    Color ghost_white(230, 230, 230);
    Color bat_black(50, 50, 50);
    
    canvas->Fill(bg_night.r, bg_night.g, bg_night.b);
    
    // Moon
    for (int y = 2; y <= 8; ++y) {
        for (int x = 24; x <= 28; ++x) {
            int dx = x - 26;
            int dy = y - 5;
            if (dx*dx + dy*dy <= 12) {
                canvas->SetPixel(x, y, moon.r, moon.g, moon.b);
            }
        }
    }
    
    // Ground
    for (int y = 26; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {
            canvas->SetPixel(x, y, ground.r, ground.g, ground.b);
        }
    }
    
    // Jack-o-lanterns
    int pumpkin_positions[][2] = {{7, 19}, {16, 18}, {25, 19}};
    
    for (int p = 0; p < 3; ++p) {
        int px = pumpkin_positions[p][0];
        int py = pumpkin_positions[p][1];
        
        // Pumpkin body
        for (int x = -2; x <= 2; ++x) {
            canvas->SetPixel(px + x, py, pumpkin_orange.r, pumpkin_orange.g, pumpkin_orange.b);
        }
        for (int x = -3; x <= 3; ++x) {
            canvas->SetPixel(px + x, py + 1, pumpkin_orange.r, pumpkin_orange.g, pumpkin_orange.b);
        }
        for (int y = 2; y <= 4; ++y) {
            for (int x = -3; x <= 3; ++x) {
                canvas->SetPixel(px + x, py + y, pumpkin_orange.r, pumpkin_orange.g, pumpkin_orange.b);
            }
        }
        for (int x = -2; x <= 2; ++x) {
            canvas->SetPixel(px + x, py + 5, pumpkin_orange.r, pumpkin_orange.g, pumpkin_orange.b);
        }
        
        // Face (glowing)
        int flicker = (frame_count / 8 + p * 3) % 10;
        Color face_color = (flicker < 7) ? jack_face : Color(jack_face.r / 2, jack_face.g / 2, jack_face.b / 2);
        
        canvas->SetPixel(px - 2, py + 2, face_color.r, face_color.g, face_color.b);
        canvas->SetPixel(px - 1, py + 2, face_color.r, face_color.g, face_color.b);
        canvas->SetPixel(px + 1, py + 2, face_color.r, face_color.g, face_color.b);
        canvas->SetPixel(px + 2, py + 2, face_color.r, face_color.g, face_color.b);
        
        for (int x = -2; x <= 2; ++x) {
            canvas->SetPixel(px + x, py + 4, face_color.r, face_color.g, face_color.b);
        }
    }
    
    // Ghosts
    for (int g = 0; g < 2; ++g) {
        float float_y = sin((frame_count + ghosts[g].phase) * 0.1f) * 1.5f;
        int gy = (int)(ghosts[g].y + float_y);
        int gx = (int)ghosts[g].x;
        
        canvas->SetPixel(gx, gy, ghost_white.r, ghost_white.g, ghost_white.b);
        canvas->SetPixel(gx - 1, gy + 1, ghost_white.r, ghost_white.g, ghost_white.b);
        canvas->SetPixel(gx, gy + 1, ghost_white.r, ghost_white.g, ghost_white.b);
        canvas->SetPixel(gx + 1, gy + 1, ghost_white.r, ghost_white.g, ghost_white.b);
        
        for (int dy = 2; dy < 5; ++dy) {
            canvas->SetPixel(gx - 1, gy + dy, ghost_white.r, ghost_white.g, ghost_white.b);
            canvas->SetPixel(gx, gy + dy, ghost_white.r, ghost_white.g, ghost_white.b);
            canvas->SetPixel(gx + 1, gy + dy, ghost_white.r, ghost_white.g, ghost_white.b);
        }
        
        canvas->SetPixel(gx - 1, gy + 2, 0, 0, 0);
        canvas->SetPixel(gx + 1, gy + 2, 0, 0, 0);
    }
}

// Thanksgiving scene
void drawThanksgivingScene(Canvas *canvas, Particle particles[], int num_particles, int frame_count) {
    Color sky_autumn(100, 140, 180);
    Color ground(90, 70, 40);
    Color grass(120, 100, 50);
    Color turkey_brown(120, 80, 40);
    Color turkey_dark(80, 50, 25);
    Color beak_yellow(255, 200, 0);
    Color wattle_red(200, 50, 50);
    Color feather_red(180, 50, 50);
    Color feather_orange(255, 140, 50);
    Color feather_yellow(255, 200, 80);
    Color feather_brown(140, 90, 50);
    Color pumpkin_orange(255, 140, 50);
    Color leaf_red(200, 50, 50);
    Color leaf_orange(255, 140, 50);
    Color leaf_yellow(255, 200, 80);
    
    canvas->Fill(sky_autumn.r, sky_autumn.g, sky_autumn.b);
    
    // Ground
    for (int y = 24; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {
            canvas->SetPixel(x, y, ground.r, ground.g, ground.b);
        }
    }
    for (int x = 0; x < 32; x += 2) {
        canvas->SetPixel(x, 23, grass.r, grass.g, grass.b);
        canvas->SetPixel(x, 24, grass.r, grass.g, grass.b);
    }
    
    // Turkey (center, bigger and more obvious)
    int tx = 16;
    int ty = 18;
    
    // Large tail feathers (dramatic fan shape)
    // Back row (tallest)
    Color tail_colors[] = {feather_red, feather_orange, feather_yellow, feather_brown, feather_red, feather_orange, feather_yellow};
    int tail_x[] = {-6, -4, -2, 0, 2, 4, 6};
    int tail_heights[] = {5, 6, 7, 8, 7, 6, 5};
    
    for (int f = 0; f < 7; ++f) {
        for (int fy = 0; fy < tail_heights[f]; ++fy) {
            int draw_x = tx + tail_x[f];
            int draw_y = ty - fy;
            if (draw_x >= 0 && draw_x < 32 && draw_y >= 0) {
                canvas->SetPixel(draw_x, draw_y, tail_colors[f].r, tail_colors[f].g, tail_colors[f].b);
            }
        }
        // Feather tips (lighter)
        int tip_y = ty - tail_heights[f];
        int draw_x = tx + tail_x[f];
        if (draw_x >= 0 && draw_x < 32 && tip_y >= 0) {
            canvas->SetPixel(draw_x, tip_y, 255, 255, 200);
        }
    }
    
    // Body (larger, rounder)
    for (int y = 0; y < 5; ++y) {
        int width = 5;
        if (y == 0) width = 3;
        if (y == 4) width = 4;
        
        for (int x = -width/2; x <= width/2; ++x) {
            canvas->SetPixel(tx + x, ty + y, turkey_brown.r, turkey_brown.g, turkey_brown.b);
        }
    }
    
    // Wing details (darker feathers on sides)
    canvas->SetPixel(tx - 2, ty + 1, turkey_dark.r, turkey_dark.g, turkey_dark.b);
    canvas->SetPixel(tx - 2, ty + 2, turkey_dark.r, turkey_dark.g, turkey_dark.b);
    canvas->SetPixel(tx + 2, ty + 1, turkey_dark.r, turkey_dark.g, turkey_dark.b);
    canvas->SetPixel(tx + 2, ty + 2, turkey_dark.r, turkey_dark.g, turkey_dark.b);
    
    // Neck (going up)
    canvas->SetPixel(tx, ty - 1, turkey_brown.r, turkey_brown.g, turkey_brown.b);
    canvas->SetPixel(tx + 1, ty - 1, turkey_brown.r, turkey_brown.g, turkey_brown.b);
    canvas->SetPixel(tx, ty - 2, turkey_brown.r, turkey_brown.g, turkey_brown.b);
    canvas->SetPixel(tx + 1, ty - 2, turkey_brown.r, turkey_brown.g, turkey_brown.b);
    
    // Head (larger, more visible)
    canvas->SetPixel(tx, ty - 3, turkey_brown.r, turkey_brown.g, turkey_brown.b);
    canvas->SetPixel(tx + 1, ty - 3, turkey_brown.r, turkey_brown.g, turkey_brown.b);
    canvas->SetPixel(tx + 2, ty - 3, turkey_brown.r, turkey_brown.g, turkey_brown.b);
    
    // Beak (pointing right)
    canvas->SetPixel(tx + 3, ty - 3, beak_yellow.r, beak_yellow.g, beak_yellow.b);
    canvas->SetPixel(tx + 3, ty - 2, beak_yellow.r, beak_yellow.g, beak_yellow.b);
    
    // Wattle (hanging down from head) - RED and obvious
    canvas->SetPixel(tx + 2, ty - 2, wattle_red.r, wattle_red.g, wattle_red.b);
    canvas->SetPixel(tx + 2, ty - 1, wattle_red.r, wattle_red.g, wattle_red.b);
    canvas->SetPixel(tx + 1, ty - 1, wattle_red.r, wattle_red.g, wattle_red.b);
    
    // Eye (black dot)
    canvas->SetPixel(tx + 2, ty - 3, 0, 0, 0);
    
    // Legs/feet (sticking out bottom)
    canvas->SetPixel(tx - 1, ty + 5, turkey_dark.r, turkey_dark.g, turkey_dark.b);
    canvas->SetPixel(tx + 1, ty + 5, turkey_dark.r, turkey_dark.g, turkey_dark.b);
    canvas->SetPixel(tx - 2, ty + 6, turkey_dark.r, turkey_dark.g, turkey_dark.b);
    canvas->SetPixel(tx - 1, ty + 6, turkey_dark.r, turkey_dark.g, turkey_dark.b);
    canvas->SetPixel(tx + 1, ty + 6, turkey_dark.r, turkey_dark.g, turkey_dark.b);
    canvas->SetPixel(tx + 2, ty + 6, turkey_dark.r, turkey_dark.g, turkey_dark.b);
    
    // Pumpkins on sides (smaller to not compete with turkey)
    int pump_x[] = {5, 27};
    for (int p = 0; p < 2; ++p) {
        int px = pump_x[p];
        for (int y = 0; y < 2; ++y) {
            for (int x = -1; x <= 1; ++x) {
                canvas->SetPixel(px + x, 23 + y, pumpkin_orange.r, pumpkin_orange.g, pumpkin_orange.b);
            }
        }
    }
    
    // Falling leaves
    for (int i = 0; i < num_particles; ++i) {
        if (particles[i].active) {
            int lx = (int)particles[i].x;
            int ly = (int)particles[i].y;
            
            Color leaf_color;
            if (particles[i].type == 0) leaf_color = leaf_red;
            else if (particles[i].type == 1) leaf_color = leaf_orange;
            else leaf_color = leaf_yellow;
            
            if (lx >= 0 && lx < 32 && ly >= 0 && ly < 24) {
                canvas->SetPixel(lx, ly, leaf_color.r, leaf_color.g, leaf_color.b);
                if ((frame_count / 5) % 2 == 0 && lx + 1 < 32) {
                    canvas->SetPixel(lx + 1, ly, leaf_color.r, leaf_color.g, leaf_color.b);
                }
            }
        }
    }
}

// Winter scene
void drawWinterScene(Canvas *canvas, Particle particles[], int num_particles, int frame_count) {
    Color sky(10, 10, 40);
    Color ground(240, 240, 255);
    Color tree_green(0, 100, 0);
    Color trunk(101, 67, 33);
    Color snow_white(255, 255, 255);
    Color light_red(255, 0, 0);
    Color light_green(0, 255, 0);
    
    canvas->Fill(sky.r, sky.g, sky.b);
    
    // Ground
    for (int y = 27; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {
            canvas->SetPixel(x, y, ground.r, ground.g, ground.b);
        }
    }
    
    // Tree trunk
    for (int y = 21; y < 27; ++y) {
        for (int x = 14; x < 18; ++x) {
            canvas->SetPixel(x, y, trunk.r, trunk.g, trunk.b);
        }
    }
    
    // Tree (triangular)
    for (int y = 6; y <= 10; ++y) {
        int width = (y - 6) * 2 + 1;
        int start_x = 16 - width / 2;
        for (int x = 0; x < width; ++x) {
            canvas->SetPixel(start_x + x, y, tree_green.r, tree_green.g, tree_green.b);
        }
    }
    for (int y = 11; y <= 15; ++y) {
        int width = (y - 9) * 2 + 1;
        int start_x = 16 - width / 2;
        for (int x = 0; x < width; ++x) {
            canvas->SetPixel(start_x + x, y, tree_green.r, tree_green.g, tree_green.b);
        }
    }
    for (int y = 16; y <= 21; ++y) {
        int width = (y - 13) * 2 + 1;
        int start_x = 16 - width / 2;
        for (int x = 0; x < width; ++x) {
            canvas->SetPixel(start_x + x, y, tree_green.r, tree_green.g, tree_green.b);
        }
    }
    
    // Christmas lights
    int lights[][2] = {
        {16, 8}, {14, 11}, {18, 11}, {13, 14}, {16, 14}, {19, 14},
        {12, 17}, {15, 17}, {18, 17}, {21, 17}
    };
    
    for (int i = 0; i < 10; ++i) {
        bool is_bright = ((frame_count / 10 + i) % 2) == 0;
        Color light_color = (i % 2 == 0) ? light_red : light_green;
        
        if (is_bright) {
            canvas->SetPixel(lights[i][0], lights[i][1], light_color.r, light_color.g, light_color.b);
        } else {
            canvas->SetPixel(lights[i][0], lights[i][1], light_color.r / 3, light_color.g / 3, light_color.b / 3);
        }
    }
    
    // Falling snow
    for (int i = 0; i < num_particles; ++i) {
        if (particles[i].active) {
            int sx = (int)particles[i].x;
            int sy = (int)particles[i].y;
            
            if (sx >= 0 && sx < 32 && sy >= 0 && sy < 27) {
                canvas->SetPixel(sx, sy, snow_white.r, snow_white.g, snow_white.b);
            }
        }
    }
}

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
    
    // Initialize particles (used for snow/leaves)
    const int num_particles = 20;
    Particle particles[num_particles];
    for (int i = 0; i < num_particles; ++i) {
        particles[i].x = rand() % 32;
        particles[i].y = -(rand() % 32);
        particles[i].speed = 0.1f + (rand() % 10) / 20.0f;
        particles[i].type = rand() % 3;
        particles[i].active = true;
    }
    
    // Initialize ghosts for Halloween
    Ghost ghosts[2];
    ghosts[0].x = 6;
    ghosts[0].y = 10;
    ghosts[0].float_offset = 0;
    ghosts[0].phase = 0;
    
    ghosts[1].x = 20;
    ghosts[1].y = 8;
    ghosts[1].float_offset = 50;
    ghosts[1].phase = 50;
    
    Scene current_scene = HALLOWEEN;
    int frame_count = 0;
    int scene_duration = 400;  // ~20 seconds per scene at 20fps
    
    while (true) {
        // Update particles
        for (int i = 0; i < num_particles; ++i) {
            if (particles[i].active) {
                particles[i].y += particles[i].speed;
                
                int max_y = (current_scene == THANKSGIVING) ? 24 : 27;
                if (particles[i].y >= max_y) {
                    particles[i].y = 0;
                    particles[i].x = rand() % 32;
                    particles[i].speed = 0.1f + (rand() % 10) / 20.0f;
                    particles[i].type = rand() % 3;
                }
            }
        }
        
        // Draw current scene
        switch (current_scene) {
            case HALLOWEEN:
                drawHalloweenScene(canvas, particles, num_particles, ghosts, frame_count);
                break;
            case THANKSGIVING:
                drawThanksgivingScene(canvas, particles, num_particles, frame_count);
                break;
            case WINTER:
                drawWinterScene(canvas, particles, num_particles, frame_count);
                break;
        }
        
        // Switch scenes
        if (frame_count > 0 && frame_count % scene_duration == 0) {
            if (current_scene == HALLOWEEN) current_scene = THANKSGIVING;
            else if (current_scene == THANKSGIVING) current_scene = WINTER;
            else current_scene = HALLOWEEN;
        }
        
        frame_count++;
        canvas = matrix->SwapOnVSync(canvas);
        usleep(50000);  // ~20 fps
    }
    
    delete matrix;
    return 0;
}
