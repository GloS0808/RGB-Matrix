// Sea Turtle Scene - Graceful Ocean Giants
// Compilation: g++ -o seaturtle seaturtle.cpp -lrgbmatrix -std=c++11

#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <signal.h>
#include <vector>

using namespace rgb_matrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
    interrupt_received = true;
}

struct SeaTurtle {
    float x;
    float y;
    float speed;
    int direction; // 1 = right, -1 = left
    float swim_phase;
    int flipper_frame;
    int size; // 1=baby, 2=adult, 3=elder
    bool diving; // Swimming up or down
    float target_y;
};

struct Bubble {
    float x;
    float y;
    float speed;
    float drift;
    int brightness;
};

struct SmallFish {
    float x;
    float y;
    float speed;
    int direction;
    int color;
    float school_offset;
};

struct Seaweed {
    int x;
    int y;
    int height;
    float sway_phase;
    float sway_speed;
};

struct SunRay {
    int x;
    float angle;
    int length;
    int brightness;
};

struct Jellyfish {
    float x;
    float y;
    float pulse_phase;
    int size;
};

class SeaTurtleScene {
private:
    RGBMatrix *matrix;
    FrameCanvas *canvas;
    int width, height;
    std::vector<SeaTurtle> turtles;
    std::vector<Bubble> bubbles;
    std::vector<SmallFish> school;
    std::vector<Seaweed> seaweed;
    std::vector<SunRay> sun_rays;
    std::vector<Jellyfish> jellyfish;
    float time_counter;
    
    // Ocean colors
    Color ocean_deep = Color(0, 50, 90);
    Color ocean_mid = Color(10, 70, 110);
    Color ocean_light = Color(30, 90, 130);
    Color ocean_surface = Color(60, 120, 160);
    
    // Turtle colors
    Color shell_dark_green = Color(40, 80, 40);
    Color shell_light_green = Color(60, 100, 60);
    Color shell_brown = Color(80, 60, 40);
    Color turtle_skin = Color(80, 120, 80);
    Color turtle_dark = Color(40, 60, 50);
    
    // Scene colors
    Color seaweed_green = Color(30, 100, 50);
    Color seaweed_dark = Color(20, 70, 35);
    Color sand_light = Color(160, 140, 100);
    Color sand_dark = Color(120, 100, 70);
    Color bubble_color = Color(120, 160, 180);
    Color sun_ray = Color(100, 140, 180);
    Color fish_silver = Color(120, 130, 140);
    Color jellyfish_purple = Color(120, 80, 140);
    
public:
    SeaTurtleScene(RGBMatrix *m) : matrix(m), time_counter(0) {
        canvas = matrix->CreateFrameCanvas();
        width = canvas->width();
        height = canvas->height();
        
        srand(time(NULL));
        
        // Create sea turtles
        for (int i = 0; i < 2; i++) {
            SeaTurtle t;
            t.x = rand() % width;
            t.y = 8 + rand() % (int)(height * 0.4);
            t.speed = 0.15 + (float)rand() / RAND_MAX * 0.1;
            t.direction = (rand() % 2) * 2 - 1;
            t.swim_phase = (float)rand() / RAND_MAX * 2 * M_PI;
            t.flipper_frame = 0;
            t.size = 2 + (rand() % 2); // Adult or elder
            t.diving = false;
            t.target_y = t.y;
            turtles.push_back(t);
        }
        
        // Add a baby turtle
        SeaTurtle baby;
        baby.x = width / 2;
        baby.y = height * 0.3;
        baby.speed = 0.2;
        baby.direction = 1;
        baby.swim_phase = 0;
        baby.flipper_frame = 0;
        baby.size = 1; // Baby
        baby.diving = false;
        baby.target_y = baby.y;
        turtles.push_back(baby);
        
        // Create bubbles
        for (int i = 0; i < 20; i++) {
            Bubble b;
            b.x = rand() % width;
            b.y = height * 0.5 + rand() % (int)(height * 0.5);
            b.speed = 0.1 + (float)rand() / RAND_MAX * 0.15;
            b.drift = ((float)rand() / RAND_MAX - 0.5) * 0.08;
            b.brightness = 100 + rand() % 80;
            bubbles.push_back(b);
        }
        
        // Create school of small fish
        for (int i = 0; i < 8; i++) {
            SmallFish f;
            f.x = width / 3;
            f.y = height * 0.4 + i * 2;
            f.speed = 0.3;
            f.direction = 1;
            f.color = rand() % 3;
            f.school_offset = i * 0.5;
            school.push_back(f);
        }
        
        // Create seaweed
        for (int i = 0; i < width / 8; i++) {
            Seaweed s;
            s.x = 3 + i * 8 + rand() % 4;
            s.y = height - 2;
            s.height = 5 + rand() % 5;
            s.sway_phase = (float)rand() / RAND_MAX * 2 * M_PI;
            s.sway_speed = 0.3 + (float)rand() / RAND_MAX * 0.3;
            seaweed.push_back(s);
        }
        
        // Create sun rays from surface
        for (int i = 0; i < 5; i++) {
            SunRay r;
            r.x = i * (width / 5) + rand() % 5;
            r.angle = 0.1 + (float)rand() / RAND_MAX * 0.2;
            r.length = 10 + rand() % 10;
            r.brightness = 40 + rand() % 30;
            sun_rays.push_back(r);
        }
        
        // Add jellyfish
        for (int i = 0; i < 2; i++) {
            Jellyfish j;
            j.x = rand() % width;
            j.y = 5 + rand() % 8;
            j.pulse_phase = (float)rand() / RAND_MAX * 2 * M_PI;
            j.size = 2 + rand() % 2;
            jellyfish.push_back(j);
        }
    }
    
    void drawOcean() {
        // Ocean gradient with depth
        for (int y = 0; y < height; y++) {
            float depth = (float)y / height;
            int r = ocean_surface.r - (ocean_surface.r - ocean_deep.r) * depth;
            int g = ocean_surface.g - (ocean_surface.g - ocean_deep.g) * depth;
            int b = ocean_surface.b - (ocean_surface.b - ocean_deep.b) * depth;
            
            // Add subtle wave shimmer
            float shimmer = sin(time_counter * 1.5 + y * 0.2) * 5;
            r = std::min(255, std::max(0, r + (int)shimmer));
            g = std::min(255, std::max(0, g + (int)shimmer));
            b = std::min(255, std::max(0, b + (int)shimmer));
            
            for (int x = 0; x < width; x++) {
                canvas->SetPixel(x, y, r, g, b);
            }
        }
    }
    
    void drawSunRays() {
        for (auto& ray : sun_rays) {
            // Draw rays coming down from surface
            for (int i = 0; i < ray.length; i++) {
                int rx = ray.x + sin(ray.angle + time_counter * 0.2) * i * 0.3;
                int ry = i;
                
                if (rx >= 0 && rx < width && ry < height) {
                    float fade = 1.0 - (float)i / ray.length;
                    int brightness = ray.brightness * fade;
                    canvas->SetPixel(rx, ry, 
                                   ocean_surface.r + brightness,
                                   ocean_surface.g + brightness,
                                   ocean_surface.b + brightness);
                }
            }
        }
    }
    
    void drawSandBottom() {
        int sand_start = height - 3;
        
        for (int y = sand_start; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // Textured sand
                if ((x + y * 2) % 5 == 0) {
                    canvas->SetPixel(x, y, sand_light.r, sand_light.g, sand_light.b);
                } else {
                    canvas->SetPixel(x, y, sand_dark.r, sand_dark.g, sand_dark.b);
                }
            }
        }
    }
    
    void drawSeaweed() {
        for (auto& weed : seaweed) {
            float sway = sin(time_counter * weed.sway_speed + weed.sway_phase) * 2;
            
            for (int i = 0; i < weed.height; i++) {
                int wx = weed.x + sin((float)i / weed.height * M_PI + time_counter * weed.sway_speed) * sway;
                int wy = weed.y - i;
                
                if (wx >= 0 && wx < width && wy >= 0 && wy < height) {
                    Color color = (i % 2 == 0) ? seaweed_green : seaweed_dark;
                    canvas->SetPixel(wx, wy, color.r, color.g, color.b);
                }
            }
        }
    }
    
    void drawTurtle(int cx, int cy, int direction, int size, int flipper_frame) {
        // Scale based on size (1=baby, 2=adult, 3=elder)
        int shell_width = size * 2;
        int shell_height = size;
        
        // Shell (hexagonal pattern)
        for (int sy = -shell_height; sy <= shell_height; sy++) {
            int shell_w = shell_width - abs(sy);
            for (int sx = -shell_w; sx <= shell_w; sx++) {
                int px = cx + sx * direction;
                int py = cy + sy;
                
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    // Shell pattern
                    if ((abs(sx) + abs(sy)) % 2 == 0) {
                        canvas->SetPixel(px, py, shell_dark_green.r, shell_dark_green.g, shell_dark_green.b);
                    } else {
                        canvas->SetPixel(px, py, shell_light_green.r, shell_light_green.g, shell_light_green.b);
                    }
                    
                    // Shell center (darker)
                    if (abs(sx) <= 1 && abs(sy) <= 1) {
                        canvas->SetPixel(px, py, shell_brown.r, shell_brown.g, shell_brown.b);
                    }
                }
            }
        }
        
        // Head
        int head_x = cx + (shell_width + 1) * direction;
        int head_y = cy - shell_height / 2;
        if (head_x >= 0 && head_x < width && head_y >= 0 && head_y < height) {
            canvas->SetPixel(head_x, head_y, turtle_skin.r, turtle_skin.g, turtle_skin.b);
            
            // Eye
            if (head_x + direction >= 0 && head_x + direction < width) {
                canvas->SetPixel(head_x + direction, head_y, 20, 20, 20);
            }
            
            // Neck
            int neck_x = cx + shell_width * direction;
            if (neck_x >= 0 && neck_x < width && head_y + 1 < height) {
                canvas->SetPixel(neck_x, head_y + 1, turtle_dark.r, turtle_dark.g, turtle_dark.b);
            }
        }
        
        // Front flippers (animated)
        int flipper_offset = (flipper_frame % 4 < 2) ? 0 : 1;
        
        // Front top flipper
        int front_flipper_x = cx + shell_width * direction;
        int front_flipper_y = cy - shell_height - flipper_offset;
        if (front_flipper_x >= 0 && front_flipper_x < width && front_flipper_y >= 0 && front_flipper_y < height) {
            canvas->SetPixel(front_flipper_x, front_flipper_y, turtle_skin.r, turtle_skin.g, turtle_skin.b);
            canvas->SetPixel(front_flipper_x, front_flipper_y + 1, turtle_dark.r, turtle_dark.g, turtle_dark.b);
        }
        
        // Front bottom flipper
        int front_bottom_y = cy + shell_height + flipper_offset;
        if (front_flipper_x >= 0 && front_flipper_x < width && front_bottom_y < height) {
            canvas->SetPixel(front_flipper_x, front_bottom_y, turtle_skin.r, turtle_skin.g, turtle_skin.b);
            if (front_bottom_y - 1 >= 0) {
                canvas->SetPixel(front_flipper_x, front_bottom_y - 1, turtle_dark.r, turtle_dark.g, turtle_dark.b);
            }
        }
        
        // Back flippers
        int back_flipper_x = cx - shell_width * direction;
        int back_offset = ((flipper_frame + 2) % 4 < 2) ? 0 : 1;
        
        // Back top flipper
        int back_top_y = cy - shell_height - back_offset;
        if (back_flipper_x >= 0 && back_flipper_x < width && back_top_y >= 0 && back_top_y < height) {
            canvas->SetPixel(back_flipper_x, back_top_y, turtle_dark.r, turtle_dark.g, turtle_dark.b);
        }
        
        // Back bottom flipper
        int back_bottom_y = cy + shell_height + back_offset;
        if (back_flipper_x >= 0 && back_flipper_x < width && back_bottom_y < height) {
            canvas->SetPixel(back_flipper_x, back_bottom_y, turtle_dark.r, turtle_dark.g, turtle_dark.b);
        }
        
        // Tail
        int tail_x = cx - (shell_width + 1) * direction;
        int tail_y = cy;
        if (tail_x >= 0 && tail_x < width && tail_y >= 0 && tail_y < height) {
            canvas->SetPixel(tail_x, tail_y, turtle_dark.r, turtle_dark.g, turtle_dark.b);
        }
    }
    
    void updateTurtles() {
        for (auto& turtle : turtles) {
            drawTurtle((int)turtle.x, (int)turtle.y, turtle.direction, turtle.size, turtle.flipper_frame);
            
            // Swimming motion
            turtle.swim_phase += 0.05;
            float vertical_bob = sin(turtle.swim_phase) * 1.5;
            
            // Move turtle
            turtle.x += turtle.speed * turtle.direction;
            
            // Gentle diving motion
            if (abs(turtle.y - turtle.target_y) < 0.5) {
                // Pick new depth
                turtle.target_y = 8 + rand() % (int)(height * 0.4);
            } else {
                // Move towards target depth
                if (turtle.y < turtle.target_y) {
                    turtle.y += 0.05;
                } else {
                    turtle.y -= 0.05;
                }
            }
            
            // Flipper animation
            if ((int)(time_counter * 10) % 2 == 0) {
                turtle.flipper_frame++;
            }
            
            // Turn around at edges
            if (turtle.x < -turtle.size * 3) {
                turtle.x = width + turtle.size * 3;
            } else if (turtle.x > width + turtle.size * 3) {
                turtle.x = -turtle.size * 3;
            }
        }
    }
    
    void drawBubbles() {
        for (auto& bubble : bubbles) {
            int bx = (int)bubble.x;
            int by = (int)bubble.y;
            
            if (bx >= 0 && bx < width && by >= 0 && by < height) {
                int b = bubble.brightness;
                canvas->SetPixel(bx, by, b, b + 20, b + 40);
            }
            
            // Rise and drift
            bubble.y -= bubble.speed;
            bubble.x += sin(time_counter * 2 + bubble.y) * bubble.drift;
            
            // Wrap horizontally
            if (bubble.x < 0) bubble.x = width - 1;
            if (bubble.x >= width) bubble.x = 0;
            
            // Reset at top
            if (bubble.y < 0) {
                bubble.y = height - 5;
                bubble.x = rand() % width;
            }
        }
    }
    
    void drawSmallFish() {
        for (auto& fish : school) {
            int fx = (int)fish.x;
            int fy = (int)(fish.y + sin(time_counter * 2 + fish.school_offset) * 2);
            
            if (fx >= 0 && fx < width && fy >= 0 && fy < height) {
                canvas->SetPixel(fx, fy, fish_silver.r, fish_silver.g, fish_silver.b);
            }
            
            // School movement
            fish.x += fish.speed * fish.direction;
            
            // Stay in school formation
            float center_x = width / 2;
            if (abs(fish.x - center_x) > width / 3) {
                fish.direction *= -1;
            }
            
            if (fish.x < -2) fish.x = width + 2;
            if (fish.x > width + 2) fish.x = -2;
        }
    }
    
    void drawJellyfish() {
        for (auto& jelly : jellyfish) {
            jelly.pulse_phase += 0.03;
            
            int jx = (int)jelly.x;
            int jy = (int)jelly.y + sin(jelly.pulse_phase) * 1;
            
            // Bell
            if (jx >= 1 && jx < width - 1 && jy >= 0 && jy < height) {
                canvas->SetPixel(jx - 1, jy, jellyfish_purple.r * 0.6, jellyfish_purple.g * 0.6, jellyfish_purple.b * 0.6);
                canvas->SetPixel(jx, jy, jellyfish_purple.r, jellyfish_purple.g, jellyfish_purple.b);
                canvas->SetPixel(jx + 1, jy, jellyfish_purple.r * 0.6, jellyfish_purple.g * 0.6, jellyfish_purple.b * 0.6);
            }
            
            // Tentacles
            for (int t = 0; t < 3; t++) {
                for (int i = 1; i <= jelly.size; i++) {
                    int ty = jy + i;
                    int tx = jx + (t - 1) + sin(time_counter * 2 + t + i * 0.3) * 1;
                    
                    if (tx >= 0 && tx < width && ty < height) {
                        float fade = 1.0 - (float)i / (jelly.size + 1);
                        canvas->SetPixel(tx, ty, jellyfish_purple.r * fade * 0.5, 
                                       jellyfish_purple.g * fade * 0.5, 
                                       jellyfish_purple.b * fade * 0.5);
                    }
                }
            }
            
            // Slow drift
            jelly.x += cos(jelly.pulse_phase * 0.5) * 0.08;
            if (jelly.x < 0) jelly.x = width - 1;
            if (jelly.x >= width) jelly.x = 0;
        }
    }
    
    void update() {
        canvas->Clear();
        
        drawOcean();
        drawSunRays();
        drawSandBottom();
        drawSeaweed();
        drawJellyfish();
        drawSmallFish();
        drawBubbles();
        updateTurtles();
        
        canvas = matrix->SwapOnVSync(canvas);
        
        time_counter += 0.05;
    }
};

int main(int argc, char *argv[]) {
    RGBMatrix::Options matrix_options;
    rgb_matrix::RuntimeOptions runtime_opt;
    
    // Default settings
    matrix_options.rows = 32;
    matrix_options.cols = 32;
    matrix_options.chain_length = 1;
    matrix_options.parallel = 1;
    matrix_options.hardware_mapping = "adafruit-hat";
    
    // Parse flags
    if (!ParseOptionsFromFlags(&argc, &argv, &matrix_options, &runtime_opt)) {
        return 1;
    }
    
    // Create matrix
    RGBMatrix *matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
    if (matrix == NULL) {
        return 1;
    }
    
    // Set up interrupt handler
    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);
    
    SeaTurtleScene scene(matrix);
    
    while (!interrupt_received) {
        scene.update();
        usleep(50000); // 50ms = ~20 FPS
    }
    
    matrix->Clear();
    delete matrix;
    
    return 0;
}
