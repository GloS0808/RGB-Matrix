// Chinese Dragon Scene - Lunar New Year
// Compilation: g++ -o dragon dragon.cpp -lrgbmatrix -std=c++11

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

struct DragonSegment {
    float x;
    float y;
    float angle;
};

struct Firework {
    float x;
    float y;
    int life;
    int max_life;
    int color_type;
};

struct Lantern {
    int x;
    int y;
    float sway_phase;
    int color_type;
};

struct Cloud {
    float x;
    float y;
    float speed;
    int size;
};

struct Sparkle {
    int x;
    int y;
    int life;
    int max_life;
    int brightness;
};

class ChineseDragonScene {
private:
    RGBMatrix *matrix;
    FrameCanvas *canvas;
    int width, height;
    std::vector<DragonSegment> dragon;
    std::vector<Firework> fireworks;
    std::vector<Lantern> lanterns;
    std::vector<Cloud> clouds;
    std::vector<Sparkle> sparkles;
    float time_counter;
    int dragon_length;
    float dragon_speed;
    
    // Traditional Chinese colors
    Color dragon_red = Color(200, 20, 20);
    Color dragon_gold = Color(220, 180, 40);
    Color dragon_yellow = Color(240, 220, 60);
    Color dragon_orange = Color(220, 100, 20);
    
    Color sky_blue = Color(40, 80, 140);
    Color sky_light = Color(60, 100, 160);
    
    Color cloud_white = Color(200, 200, 220);
    Color cloud_gray = Color(160, 160, 180);
    
    Color lantern_red = Color(220, 30, 30);
    Color lantern_gold = Color(240, 200, 60);
    
    Color firework_red = Color(250, 50, 50);
    Color firework_gold = Color(250, 220, 80);
    Color firework_green = Color(80, 250, 80);
    Color firework_blue = Color(80, 150, 250);
    
    Color pearl_white = Color(240, 240, 250);
    Color pearl_blue = Color(180, 200, 250);
    
public:
    ChineseDragonScene(RGBMatrix *m) : matrix(m), time_counter(0), dragon_length(20), dragon_speed(0.15) {
        canvas = matrix->CreateFrameCanvas();
        width = canvas->width();
        height = canvas->height();
        
        srand(time(NULL));
        
        // Initialize dragon segments
        for (int i = 0; i < dragon_length; i++) {
            DragonSegment seg;
            seg.x = width / 2 - i * 2;
            seg.y = height / 2;
            seg.angle = 0;
            dragon.push_back(seg);
        }
        
        // Create lanterns
        for (int i = 0; i < 3; i++) {
            Lantern l;
            l.x = 5 + i * (width / 3);
            l.y = 3;
            l.sway_phase = (float)rand() / RAND_MAX * 2 * M_PI;
            l.color_type = i % 2;
            lanterns.push_back(l);
        }
        
        // Create clouds
        for (int i = 0; i < 3; i++) {
            Cloud c;
            c.x = rand() % width;
            c.y = 2 + rand() % 6;
            c.speed = 0.05 + (float)rand() / RAND_MAX * 0.1;
            c.size = 3 + rand() % 3;
            clouds.push_back(c);
        }
    }
    
    Color getFireworkColor(int type) {
        switch(type) {
            case 0: return firework_red;
            case 1: return firework_gold;
            case 2: return firework_green;
            default: return firework_blue;
        }
    }
    
    void drawSky() {
        // Gradient sky (day/dusk feel)
        for (int y = 0; y < height; y++) {
            float ratio = (float)y / height;
            int r = sky_blue.r + (sky_light.r - sky_blue.r) * ratio;
            int g = sky_blue.g + (sky_light.g - sky_blue.g) * ratio;
            int b = sky_blue.b + (sky_light.b - sky_blue.b) * ratio;
            
            for (int x = 0; x < width; x++) {
                canvas->SetPixel(x, y, r, g, b);
            }
        }
    }
    
    void drawClouds() {
        for (auto& cloud : clouds) {
            // Traditional Chinese-style clouds
            for (int i = 0; i < cloud.size; i++) {
                int cx = (int)cloud.x + i;
                int cy = (int)cloud.y;
                
                if (cx >= 0 && cx < width && cy >= 0 && cy < height) {
                    canvas->SetPixel(cx, cy, cloud_white.r, cloud_white.g, cloud_white.b);
                    if (cy + 1 < height && i > 0 && i < cloud.size - 1) {
                        canvas->SetPixel(cx, cy + 1, cloud_gray.r, cloud_gray.g, cloud_gray.b);
                    }
                }
            }
            
            cloud.x += cloud.speed;
            if (cloud.x > width) {
                cloud.x = -cloud.size;
                cloud.y = 2 + rand() % 6;
            }
        }
    }
    
    void drawDragonHead(int x, int y, float angle) {
        // Dragon head facing right
        int head_dx = cos(angle) * 3;
        int head_dy = sin(angle) * 3;
        
        // Snout/mouth
        int snout_x = x + head_dx;
        int snout_y = y + head_dy;
        
        if (snout_x >= 0 && snout_x < width && snout_y >= 0 && snout_y < height) {
            canvas->SetPixel(snout_x, snout_y, dragon_red.r, dragon_red.g, dragon_red.b);
        }
        
        // Head body (main)
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int px = x + dx;
                int py = y + dy;
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    if (dx == 0 && dy == 0) {
                        // Center - golden
                        canvas->SetPixel(px, py, dragon_gold.r, dragon_gold.g, dragon_gold.b);
                    } else {
                        // Outer - red
                        canvas->SetPixel(px, py, dragon_red.r, dragon_red.g, dragon_red.b);
                    }
                }
            }
        }
        
        // Eyes (white dots)
        int eye_offset_x = cos(angle + M_PI / 4);
        int eye_offset_y = sin(angle + M_PI / 4);
        int eye_x = x + eye_offset_x;
        int eye_y = y + eye_offset_y - 1;
        if (eye_x >= 0 && eye_x < width && eye_y >= 0 && eye_y < height) {
            canvas->SetPixel(eye_x, eye_y, 240, 240, 250);
        }
        
        // Horns/whiskers
        int horn1_x = x + cos(angle + M_PI / 3) * 2;
        int horn1_y = y + sin(angle + M_PI / 3) * 2;
        if (horn1_x >= 0 && horn1_x < width && horn1_y >= 0 && horn1_y < height) {
            canvas->SetPixel(horn1_x, horn1_y, dragon_yellow.r, dragon_yellow.g, dragon_yellow.b);
        }
        
        int horn2_x = x + cos(angle - M_PI / 3) * 2;
        int horn2_y = y + sin(angle - M_PI / 3) * 2;
        if (horn2_x >= 0 && horn2_x < width && horn2_y >= 0 && horn2_y < height) {
            canvas->SetPixel(horn2_x, horn2_y, dragon_yellow.r, dragon_yellow.g, dragon_yellow.b);
        }
    }
    
    void drawDragonBody(int x, int y, int segment_index) {
        // Body segments alternate red and gold
        Color body_color;
        if (segment_index % 3 == 0) {
            body_color = dragon_gold;
        } else if (segment_index % 3 == 1) {
            body_color = dragon_red;
        } else {
            body_color = dragon_orange;
        }
        
        // Draw body segment (cross shape)
        if (x >= 0 && x < width && y >= 0 && y < height) {
            canvas->SetPixel(x, y, body_color.r, body_color.g, body_color.b);
        }
        if (x - 1 >= 0 && y >= 0 && y < height) {
            canvas->SetPixel(x - 1, y, body_color.r * 0.8, body_color.g * 0.8, body_color.b * 0.8);
        }
        if (x + 1 < width && y >= 0 && y < height) {
            canvas->SetPixel(x + 1, y, body_color.r * 0.8, body_color.g * 0.8, body_color.b * 0.8);
        }
        if (y - 1 >= 0 && x >= 0 && x < width) {
            canvas->SetPixel(x, y - 1, body_color.r * 0.8, body_color.g * 0.8, body_color.b * 0.8);
        }
        if (y + 1 < height && x >= 0 && x < width) {
            canvas->SetPixel(x, y + 1, body_color.r * 0.8, body_color.g * 0.8, body_color.b * 0.8);
        }
    }
    
    void updateDragon() {
        // Move dragon head in a sinusoidal pattern
        float head_x = width / 2 + sin(time_counter * 0.5) * (width / 3);
        float head_y = height / 2 + cos(time_counter * 0.3) * (height / 4);
        float head_angle = time_counter * 0.5;
        
        // Update head
        dragon[0].x = head_x;
        dragon[0].y = head_y;
        dragon[0].angle = head_angle;
        
        // Follow the leader for body segments
        for (size_t i = 1; i < dragon.size(); i++) {
            float dx = dragon[i-1].x - dragon[i].x;
            float dy = dragon[i-1].y - dragon[i].y;
            float dist = sqrt(dx*dx + dy*dy);
            
            if (dist > 0) {
                float segment_dist = 2.0;
                dragon[i].x += (dx / dist) * (dist - segment_dist) * 0.3;
                dragon[i].y += (dy / dist) * (dist - segment_dist) * 0.3;
                dragon[i].angle = atan2(dy, dx);
            }
        }
        
        // Draw dragon body (back to front)
        for (int i = dragon.size() - 1; i >= 1; i--) {
            drawDragonBody((int)dragon[i].x, (int)dragon[i].y, i);
        }
        
        // Draw dragon head last (on top)
        drawDragonHead((int)dragon[0].x, (int)dragon[0].y, dragon[0].angle);
        
        // Draw pearl in front of dragon (chasing the pearl)
        int pearl_x = dragon[0].x + cos(dragon[0].angle) * 4;
        int pearl_y = dragon[0].y + sin(dragon[0].angle) * 4;
        
        if (pearl_x >= 0 && pearl_x < width && pearl_y >= 0 && pearl_y < height) {
            // Pearl with glow
            canvas->SetPixel(pearl_x, pearl_y, pearl_white.r, pearl_white.g, pearl_white.b);
            if (pearl_x + 1 < width) {
                canvas->SetPixel(pearl_x + 1, pearl_y, pearl_blue.r, pearl_blue.g, pearl_blue.b);
            }
            if (pearl_x - 1 >= 0) {
                canvas->SetPixel(pearl_x - 1, pearl_y, pearl_blue.r, pearl_blue.g, pearl_blue.b);
            }
            if (pearl_y + 1 < height) {
                canvas->SetPixel(pearl_x, pearl_y + 1, pearl_blue.r, pearl_blue.g, pearl_blue.b);
            }
            if (pearl_y - 1 >= 0) {
                canvas->SetPixel(pearl_x, pearl_y - 1, pearl_blue.r, pearl_blue.g, pearl_blue.b);
            }
        }
    }
    
    void drawLantern(Lantern& lantern) {
        lantern.sway_phase += 0.05;
        int sway = sin(lantern.sway_phase) * 1;
        
        Color lantern_color = (lantern.color_type == 0) ? lantern_red : lantern_gold;
        
        int lx = lantern.x + sway;
        int ly = lantern.y;
        
        // String
        if (ly - 1 >= 0 && lx >= 0 && lx < width) {
            canvas->SetPixel(lx, ly - 1, 100, 100, 100);
        }
        
        // Lantern body
        for (int y = 0; y < 3; y++) {
            if (ly + y < height && lx >= 0 && lx < width) {
                canvas->SetPixel(lx, ly + y, lantern_color.r, lantern_color.g, lantern_color.b);
                
                // Sides (wider in middle)
                if (y == 1) {
                    if (lx - 1 >= 0) {
                        canvas->SetPixel(lx - 1, ly + y, lantern_color.r * 0.8, lantern_color.g * 0.8, lantern_color.b * 0.8);
                    }
                    if (lx + 1 < width) {
                        canvas->SetPixel(lx + 1, ly + y, lantern_color.r * 0.8, lantern_color.g * 0.8, lantern_color.b * 0.8);
                    }
                }
            }
        }
        
        // Tassel
        if (ly + 3 < height && lx >= 0 && lx < width) {
            canvas->SetPixel(lx, ly + 3, lantern_gold.r * 0.7, lantern_gold.g * 0.7, lantern_gold.b * 0.7);
        }
    }
    
    void drawLanterns() {
        for (auto& lantern : lanterns) {
            drawLantern(lantern);
        }
    }
    
    void updateFireworks() {
        // Randomly create fireworks
        if (rand() % 80 == 0) {
            Firework fw;
            fw.x = 5 + rand() % (width - 10);
            fw.y = 3 + rand() % (int)(height * 0.3);
            fw.life = 0;
            fw.max_life = 15 + rand() % 10;
            fw.color_type = rand() % 4;
            fireworks.push_back(fw);
        }
        
        // Update fireworks
        for (auto it = fireworks.begin(); it != fireworks.end();) {
            it->life++;
            
            if (it->life < it->max_life) {
                float fade = 1.0 - (float)it->life / it->max_life;
                Color fw_color = getFireworkColor(it->color_type);
                
                // Expanding circle
                int radius = 1 + it->life / 3;
                for (int angle = 0; angle < 360; angle += 30) {
                    float rad = angle * M_PI / 180.0;
                    int px = it->x + cos(rad) * radius;
                    int py = it->y + sin(rad) * radius;
                    
                    if (px >= 0 && px < width && py >= 0 && py < height) {
                        canvas->SetPixel(px, py, fw_color.r * fade, fw_color.g * fade, fw_color.b * fade);
                    }
                }
                
                ++it;
            } else {
                it = fireworks.erase(it);
            }
        }
    }
    
    void addSparkles() {
        // Sparkles around dragon
        if (rand() % 5 < 2) {
            for (size_t i = 0; i < dragon.size(); i += 3) {
                Sparkle s;
                s.x = dragon[i].x + (rand() % 5 - 2);
                s.y = dragon[i].y + (rand() % 5 - 2);
                s.life = 0;
                s.max_life = 8 + rand() % 8;
                s.brightness = 180 + rand() % 75;
                sparkles.push_back(s);
            }
        }
        
        // Update sparkles
        for (auto it = sparkles.begin(); it != sparkles.end();) {
            it->life++;
            
            if (it->life < it->max_life) {
                float fade = 1.0 - (float)it->life / it->max_life;
                int brightness = it->brightness * fade;
                
                if (it->x >= 0 && it->x < width && it->y >= 0 && it->y < height) {
                    canvas->SetPixel(it->x, it->y, brightness, brightness * 0.9, brightness * 0.7);
                }
                
                ++it;
            } else {
                it = sparkles.erase(it);
            }
        }
    }
    
    void update() {
        canvas->Clear();
        
        drawSky();
        drawClouds();
        drawLanterns();
        updateFireworks();
        updateDragon();
        addSparkles();
        
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
    
    ChineseDragonScene scene(matrix);
    
    while (!interrupt_received) {
        scene.update();
        usleep(50000); // 50ms = ~20 FPS
    }
    
    matrix->Clear();
    delete matrix;
    
    return 0;
}
