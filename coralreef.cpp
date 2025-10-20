// Coral Reef Scene - Underwater Beauty
// Compilation: g++ -o coralreef coralreef.cpp -lrgbmatrix -std=c++11

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

struct Fish {
    float x;
    float y;
    float speed;
    int direction; // 1 = right, -1 = left
    int color_type;
    int size;
    int tail_frame;
    float swim_wave;
};

struct Bubble {
    float x;
    float y;
    float speed;
    float drift;
    int size;
};

struct Coral {
    int x;
    int y;
    int height;
    int type; // 0=branching, 1=fan, 2=tube
    int color_type;
    float sway_phase;
};

struct Starfish {
    int x;
    int y;
    int color_type;
};

struct Jellyfish {
    float x;
    float y;
    float pulse_phase;
    int tentacle_length;
    int color_type;
};

struct WaterParticle {
    float x;
    float y;
    float vx;
    float vy;
    int brightness;
};

class CoralReefScene {
private:
    RGBMatrix *matrix;
    FrameCanvas *canvas;
    int width, height;
    std::vector<Fish> fish;
    std::vector<Bubble> bubbles;
    std::vector<Coral> corals;
    std::vector<Starfish> starfish;
    std::vector<Jellyfish> jellyfish;
    std::vector<WaterParticle> particles;
    float time_counter;
    
    // Water colors
    Color water_deep = Color(0, 40, 80);
    Color water_mid = Color(10, 60, 100);
    Color water_light = Color(20, 80, 120);
    Color water_surface = Color(40, 100, 140);
    
    // Coral colors
    Color coral_pink = Color(180, 60, 100);
    Color coral_orange = Color(180, 80, 20);
    Color coral_purple = Color(120, 40, 140);
    Color coral_red = Color(160, 20, 40);
    Color coral_yellow = Color(160, 140, 20);
    
    // Fish colors
    Color fish_blue = Color(40, 100, 180);
    Color fish_yellow = Color(200, 180, 20);
    Color fish_orange = Color(200, 100, 20);
    Color fish_purple = Color(140, 60, 160);
    Color fish_green = Color(60, 160, 80);
    
    // Other colors
    Color sand_light = Color(180, 160, 120);
    Color sand_dark = Color(140, 120, 90);
    Color bubble_white = Color(140, 180, 200);
    Color jellyfish_pink = Color(160, 100, 140);
    Color jellyfish_blue = Color(80, 120, 180);
    Color starfish_orange = Color(200, 100, 40);
    Color starfish_red = Color(180, 40, 40);
    
public:
    CoralReefScene(RGBMatrix *m) : matrix(m), time_counter(0) {
        canvas = matrix->CreateFrameCanvas();
        width = canvas->width();
        height = canvas->height();
        
        srand(time(NULL));
        
        // Create fish
        for (int i = 0; i < 6; i++) {
            Fish f;
            f.x = rand() % width;
            f.y = 5 + rand() % (int)(height * 0.5);
            f.speed = 0.2 + (float)rand() / RAND_MAX * 0.3;
            f.direction = (rand() % 2) * 2 - 1;
            f.color_type = rand() % 5;
            f.size = 2 + rand() % 2;
            f.tail_frame = 0;
            f.swim_wave = (float)rand() / RAND_MAX * 2 * M_PI;
            fish.push_back(f);
        }
        
        // Create bubbles
        for (int i = 0; i < 15; i++) {
            Bubble b;
            b.x = rand() % width;
            b.y = height - 10 + rand() % 10;
            b.speed = 0.1 + (float)rand() / RAND_MAX * 0.2;
            b.drift = ((float)rand() / RAND_MAX - 0.5) * 0.1;
            b.size = 1;
            bubbles.push_back(b);
        }
        
        // Create coral formations
        int num_corals = width / 6;
        for (int i = 0; i < num_corals; i++) {
            Coral c;
            c.x = 3 + i * (width / num_corals) + (rand() % 3);
            c.y = height - 3;
            c.height = 4 + rand() % 6;
            c.type = rand() % 3;
            c.color_type = rand() % 5;
            c.sway_phase = (float)rand() / RAND_MAX * 2 * M_PI;
            corals.push_back(c);
        }
        
        // Create starfish
        for (int i = 0; i < 2; i++) {
            Starfish s;
            s.x = 5 + rand() % (width - 10);
            s.y = height - 2;
            s.color_type = rand() % 2;
            starfish.push_back(s);
        }
        
        // Create jellyfish
        for (int i = 0; i < 2; i++) {
            Jellyfish j;
            j.x = rand() % width;
            j.y = 5 + rand() % 10;
            j.pulse_phase = (float)rand() / RAND_MAX * 2 * M_PI;
            j.tentacle_length = 3 + rand() % 3;
            j.color_type = rand() % 2;
            jellyfish.push_back(j);
        }
        
        // Create water particles
        for (int i = 0; i < 20; i++) {
            WaterParticle p;
            p.x = rand() % width;
            p.y = rand() % height;
            p.vx = ((float)rand() / RAND_MAX - 0.5) * 0.05;
            p.vy = ((float)rand() / RAND_MAX - 0.5) * 0.05;
            p.brightness = 100 + rand() % 100;
            particles.push_back(p);
        }
    }
    
    void drawWater() {
        // Water gradient - darker at bottom, lighter at top
        for (int y = 0; y < height; y++) {
            float depth = 1.0 - (float)y / height;
            int r = water_deep.r + (water_surface.r - water_deep.r) * (1.0 - depth);
            int g = water_deep.g + (water_surface.g - water_deep.g) * (1.0 - depth);
            int b = water_deep.b + (water_surface.b - water_deep.b) * (1.0 - depth);
            
            // Add wave effect
            float wave = sin(time_counter * 2 + y * 0.3) * 5;
            r = std::min(255, std::max(0, r + (int)wave));
            g = std::min(255, std::max(0, g + (int)wave));
            b = std::min(255, std::max(0, b + (int)wave));
            
            for (int x = 0; x < width; x++) {
                canvas->SetPixel(x, y, r, g, b);
            }
        }
    }
    
    void drawSand() {
        int sand_start = height - 3;
        
        // Sandy bottom
        for (int y = sand_start; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if ((x + y) % 3 == 0 || (rand() % 10 < 2)) {
                    canvas->SetPixel(x, y, sand_light.r, sand_light.g, sand_light.b);
                } else {
                    canvas->SetPixel(x, y, sand_dark.r, sand_dark.g, sand_dark.b);
                }
            }
        }
    }
    
    Color getCoralColor(int type) {
        switch(type) {
            case 0: return coral_pink;
            case 1: return coral_orange;
            case 2: return coral_purple;
            case 3: return coral_red;
            default: return coral_yellow;
        }
    }
    
    void drawBranchingCoral(int x, int y, int height, Color color, float sway) {
        int sway_offset = sin(time_counter + sway) * 1;
        
        // Main stem
        for (int i = 0; i < height; i++) {
            int cx = x + (sway_offset * i / height);
            int cy = y - i;
            if (cx >= 0 && cx < width && cy >= 0 && cy < height) {
                canvas->SetPixel(cx, cy, color.r, color.g, color.b);
            }
        }
        
        // Branches
        for (int i = height / 3; i < height; i += 2) {
            int cy = y - i;
            int cx = x + (sway_offset * i / height);
            
            // Left branch
            if (cx - 1 >= 0 && cy >= 0 && cy < height) {
                canvas->SetPixel(cx - 1, cy, color.r * 0.8, color.g * 0.8, color.b * 0.8);
            }
            // Right branch
            if (cx + 1 < width && cy >= 0 && cy < height) {
                canvas->SetPixel(cx + 1, cy, color.r * 0.8, color.g * 0.8, color.b * 0.8);
            }
        }
    }
    
    void drawFanCoral(int x, int y, int height, Color color, float sway) {
        int sway_offset = sin(time_counter + sway) * 1;
        
        // Stem
        for (int i = 0; i < 2; i++) {
            if (y - i >= 0 && y - i < height) {
                canvas->SetPixel(x, y - i, color.r * 0.6, color.g * 0.6, color.b * 0.6);
            }
        }
        
        // Fan shape
        for (int i = 2; i < height; i++) {
            int fan_width = (i - 1);
            int cy = y - i;
            
            for (int fx = -fan_width; fx <= fan_width; fx++) {
                int cx = x + fx + sway_offset;
                if (cx >= 0 && cx < width && cy >= 0 && cy < height) {
                    float fade = 1.0 - abs(fx) / (float)fan_width * 0.3;
                    canvas->SetPixel(cx, cy, color.r * fade, color.g * fade, color.b * fade);
                }
            }
        }
    }
    
    void drawTubeCoral(int x, int y, int height, Color color) {
        // Tube shape
        for (int i = 0; i < height; i++) {
            int cy = y - i;
            if (cy >= 0 && cy < height) {
                canvas->SetPixel(x, cy, color.r, color.g, color.b);
                
                // Opening at top
                if (i == height - 1) {
                    canvas->SetPixel(x, cy, color.r * 1.2, color.g * 1.2, color.b * 1.2);
                }
            }
        }
    }
    
    void drawCorals() {
        for (auto& coral : corals) {
            Color color = getCoralColor(coral.color_type);
            
            switch(coral.type) {
                case 0:
                    drawBranchingCoral(coral.x, coral.y, coral.height, color, coral.sway_phase);
                    break;
                case 1:
                    drawFanCoral(coral.x, coral.y, coral.height, color, coral.sway_phase);
                    break;
                case 2:
                    drawTubeCoral(coral.x, coral.y, coral.height, color);
                    break;
            }
        }
    }
    
    Color getFishColor(int type) {
        switch(type) {
            case 0: return fish_blue;
            case 1: return fish_yellow;
            case 2: return fish_orange;
            case 3: return fish_purple;
            default: return fish_green;
        }
    }
    
    void drawFish(int x, int y, int direction, int color_type, int size, int tail_frame) {
        Color fish_color = getFishColor(color_type);
        
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        
        // Body
        canvas->SetPixel(x, y, fish_color.r, fish_color.g, fish_color.b);
        
        if (size > 2) {
            canvas->SetPixel(x, y + 1, fish_color.r * 0.8, fish_color.g * 0.8, fish_color.b * 0.8);
        }
        
        // Eye
        int eye_x = x + direction;
        if (eye_x >= 0 && eye_x < width) {
            canvas->SetPixel(eye_x, y, 20, 20, 20);
        }
        
        // Tail (animated)
        int tail_offset = (tail_frame % 4 < 2) ? 0 : 1;
        int tail_x = x - direction;
        if (tail_x >= 0 && tail_x < width) {
            canvas->SetPixel(tail_x, y + tail_offset, fish_color.r * 0.7, fish_color.g * 0.7, fish_color.b * 0.7);
            if (size > 2 && y + 1 < height) {
                canvas->SetPixel(tail_x, y + 1 + tail_offset, fish_color.r * 0.6, fish_color.g * 0.6, fish_color.b * 0.6);
            }
        }
    }
    
    void updateFish() {
        for (auto& f : fish) {
            // Swimming wave motion
            f.swim_wave += 0.1;
            int wave_y = sin(f.swim_wave) * 2;
            
            drawFish((int)f.x, (int)f.y + wave_y, f.direction, f.color_type, f.size, f.tail_frame);
            
            f.x += f.speed * f.direction;
            f.tail_frame++;
            
            // Turn around at edges
            if (f.x < -5) {
                f.x = width + 5;
            } else if (f.x > width + 5) {
                f.x = -5;
            }
        }
    }
    
    void drawBubble(int x, int y, int size) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            canvas->SetPixel(x, y, bubble_white.r, bubble_white.g, bubble_white.b);
            
            // Highlight
            if (size > 1 && x + 1 < width && y > 0) {
                canvas->SetPixel(x, y - 1, 200, 220, 240);
            }
        }
    }
    
    void updateBubbles() {
        for (auto& bubble : bubbles) {
            drawBubble((int)bubble.x, (int)bubble.y, bubble.size);
            
            bubble.y -= bubble.speed;
            bubble.x += sin(time_counter * 3 + bubble.y) * bubble.drift;
            
            // Reset at top
            if (bubble.y < 0) {
                bubble.y = height - 3;
                bubble.x = rand() % width;
            }
        }
    }
    
    void drawStarfish(int x, int y, int color_type) {
        Color star_color = (color_type == 0) ? starfish_orange : starfish_red;
        
        // Center
        if (x >= 0 && x < width && y >= 0 && y < height) {
            canvas->SetPixel(x, y, star_color.r, star_color.g, star_color.b);
        }
        
        // Five arms
        int arms[][2] = {{0,-1}, {-1,0}, {1,0}, {-1,1}, {1,1}};
        for (int i = 0; i < 5; i++) {
            int ax = x + arms[i][0];
            int ay = y + arms[i][1];
            if (ax >= 0 && ax < width && ay >= 0 && ay < height) {
                canvas->SetPixel(ax, ay, star_color.r * 0.8, star_color.g * 0.8, star_color.b * 0.8);
            }
        }
    }
    
    void drawStarfish() {
        for (auto& star : starfish) {
            drawStarfish(star.x, star.y, star.color_type);
        }
    }
    
    void drawJellyfish(float x, float y, float pulse, int tentacles, int color_type) {
        Color jelly_color = (color_type == 0) ? jellyfish_pink : jellyfish_blue;
        
        int jx = (int)x;
        int jy = (int)y + sin(pulse) * 1;
        
        // Bell/head
        if (jx >= 1 && jx < width - 1 && jy >= 0 && jy < height) {
            canvas->SetPixel(jx - 1, jy, jelly_color.r * 0.7, jelly_color.g * 0.7, jelly_color.b * 0.7);
            canvas->SetPixel(jx, jy, jelly_color.r, jelly_color.g, jelly_color.b);
            canvas->SetPixel(jx + 1, jy, jelly_color.r * 0.7, jelly_color.g * 0.7, jelly_color.b * 0.7);
            
            if (jy + 1 < height) {
                canvas->SetPixel(jx, jy + 1, jelly_color.r * 0.8, jelly_color.g * 0.8, jelly_color.b * 0.8);
            }
        }
        
        // Tentacles
        for (int t = 0; t < tentacles; t++) {
            for (int i = 2; i < tentacles + 2; i++) {
                int ty = jy + i;
                int tx = jx + (t - 1) + sin(time_counter * 2 + t + i * 0.5) * 1;
                
                if (tx >= 0 && tx < width && ty < height) {
                    float fade = 1.0 - (float)i / (tentacles + 2);
                    canvas->SetPixel(tx, ty, jelly_color.r * fade * 0.6, 
                                   jelly_color.g * fade * 0.6, 
                                   jelly_color.b * fade * 0.6);
                }
            }
        }
    }
    
    void updateJellyfish() {
        for (auto& jelly : jellyfish) {
            jelly.pulse_phase += 0.05;
            drawJellyfish(jelly.x, jelly.y, jelly.pulse_phase, jelly.tentacle_length, jelly.color_type);
            
            // Slow drift
            jelly.y += sin(jelly.pulse_phase) * 0.05;
            jelly.x += cos(jelly.pulse_phase * 0.5) * 0.1;
            
            // Keep in bounds
            if (jelly.x < 0) jelly.x = width - 1;
            if (jelly.x >= width) jelly.x = 0;
            if (jelly.y < 3) jelly.y = 3;
            if (jelly.y > height * 0.5) jelly.y = height * 0.5;
        }
    }
    
    void updateWaterParticles() {
        for (auto& p : particles) {
            if (rand() % 100 < 3) {
                canvas->SetPixel((int)p.x, (int)p.y, 
                               water_light.r * p.brightness / 255,
                               water_light.g * p.brightness / 255,
                               water_light.b * p.brightness / 255);
            }
            
            p.x += p.vx;
            p.y += p.vy;
            
            if (p.x < 0) p.x = width - 1;
            if (p.x >= width) p.x = 0;
            if (p.y < 0) p.y = height - 1;
            if (p.y >= height) p.y = 0;
        }
    }
    
    void update() {
        canvas->Clear();
        
        drawWater();
        drawSand();
        updateWaterParticles();
        drawCorals();
        drawStarfish();
        updateBubbles();
        updateJellyfish();
        updateFish();
        
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
    
    CoralReefScene scene(matrix);
    
    while (!interrupt_received) {
        scene.update();
        usleep(50000); // 50ms = ~20 FPS
    }
    
    matrix->Clear();
    delete matrix;
    
    return 0;
}
