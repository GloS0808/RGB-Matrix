// Green Dragon Scene - Fantasy Theme
// Compilation: g++ -o greendragon greendragon.cpp -lrgbmatrix -std=c++11

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

struct FireBreath {
    float x;
    float y;
    float vx;
    float vy;
    int life;
    int max_life;
    float size;
};

struct MagicParticle {
    float x;
    float y;
    float vx;
    float vy;
    int life;
    int max_life;
    int hue;
};

struct Treasure {
    int x;
    int y;
    float glow_phase;
    int type;
};

struct Wing {
    float flap_phase;
    int flap_direction;
};

class GreenDragonScene {
private:
    RGBMatrix *matrix;
    FrameCanvas *canvas;
    int width, height;
    std::vector<DragonSegment> dragon;
    std::vector<FireBreath> flames;
    std::vector<MagicParticle> particles;
    std::vector<Treasure> treasures;
    Wing left_wing;
    Wing right_wing;
    float time_counter;
    int dragon_length;
    bool breathing_fire;
    int fire_cooldown;
    
    // Green dragon colors
    Color dragon_green = Color(60, 180, 60);
    Color dragon_dark_green = Color(30, 120, 40);
    Color dragon_emerald = Color(80, 220, 100);
    Color dragon_forest = Color(40, 140, 50);
    
    // Fire colors (green dragon fire)
    Color fire_green = Color(100, 250, 100);
    Color fire_yellow = Color(200, 250, 100);
    Color fire_emerald = Color(120, 255, 140);
    
    // Scene colors
    Color cave_dark = Color(20, 15, 25);
    Color cave_brown = Color(60, 45, 35);
    Color cave_gray = Color(80, 75, 70);
    
    Color eye_red = Color(220, 50, 50);
    Color eye_yellow = Color(250, 220, 60);
    
    Color gold = Color(220, 180, 40);
    Color silver = Color(180, 180, 200);
    Color ruby = Color(200, 30, 30);
    Color sapphire = Color(40, 100, 220);
    
    Color magic_green = Color(100, 250, 150);
    Color magic_blue = Color(80, 200, 250);
    
public:
    GreenDragonScene(RGBMatrix *m) : matrix(m), time_counter(0), dragon_length(18), 
                                      breathing_fire(false), fire_cooldown(0) {
        canvas = matrix->CreateFrameCanvas();
        width = canvas->width();
        height = canvas->height();
        
        srand(time(NULL));
        
        // Initialize dragon segments
        for (int i = 0; i < dragon_length; i++) {
            DragonSegment seg;
            seg.x = width / 3 - i * 2;
            seg.y = height / 2;
            seg.angle = 0;
            dragon.push_back(seg);
        }
        
        // Initialize wings
        left_wing.flap_phase = 0;
        left_wing.flap_direction = 1;
        right_wing.flap_phase = M_PI;
        right_wing.flap_direction = 1;
        
        // Create treasure hoard
        for (int i = 0; i < 8; i++) {
            Treasure t;
            t.x = 5 + rand() % (width / 3);
            t.y = height - 5 + rand() % 3;
            t.glow_phase = (float)rand() / RAND_MAX * 2 * M_PI;
            t.type = rand() % 4; // gold, silver, ruby, sapphire
            treasures.push_back(t);
        }
    }
    
    void HSVtoRGB(float h, float s, float v, int &r, int &g, int &b) {
        float c = v * s;
        float x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
        float m = v - c;
        
        float r1, g1, b1;
        if (h < 60) { r1 = c; g1 = x; b1 = 0; }
        else if (h < 120) { r1 = x; g1 = c; b1 = 0; }
        else if (h < 180) { r1 = 0; g1 = c; b1 = x; }
        else if (h < 240) { r1 = 0; g1 = x; b1 = c; }
        else if (h < 300) { r1 = x; g1 = 0; b1 = c; }
        else { r1 = c; g1 = 0; b1 = x; }
        
        r = (r1 + m) * 255;
        g = (g1 + m) * 255;
        b = (b1 + m) * 255;
    }
    
    void drawCaveBackground() {
        // Dark cave background
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // Add some texture
                if ((x + y) % 5 == 0) {
                    canvas->SetPixel(x, y, cave_brown.r, cave_brown.g, cave_brown.b);
                } else if ((x * 2 + y) % 7 == 0) {
                    canvas->SetPixel(x, y, cave_gray.r, cave_gray.g, cave_gray.b);
                } else {
                    canvas->SetPixel(x, y, cave_dark.r, cave_dark.g, cave_dark.b);
                }
            }
        }
    }
    
    void drawTreasures() {
        for (auto& treasure : treasures) {
            treasure.glow_phase += 0.05;
            float glow = sin(treasure.glow_phase) * 0.3 + 0.7;
            
            Color t_color;
            switch(treasure.type) {
                case 0: t_color = gold; break;
                case 1: t_color = silver; break;
                case 2: t_color = ruby; break;
                default: t_color = sapphire; break;
            }
            
            if (treasure.x >= 0 && treasure.x < width && treasure.y >= 0 && treasure.y < height) {
                canvas->SetPixel(treasure.x, treasure.y, 
                               t_color.r * glow, t_color.g * glow, t_color.b * glow);
                
                // Sparkle effect
                if (glow > 0.9 && treasure.x + 1 < width) {
                    canvas->SetPixel(treasure.x + 1, treasure.y, 
                                   t_color.r * 0.5, t_color.g * 0.5, t_color.b * 0.5);
                }
            }
        }
    }
    
    void drawDragonHead(int x, int y, float angle) {
        // Dragon head - more detailed
        
        // Snout/jaw
        int snout_x = x + cos(angle) * 4;
        int snout_y = y + sin(angle) * 4;
        
        // Draw snout
        for (int i = 0; i < 3; i++) {
            int sx = x + cos(angle) * (2 + i);
            int sy = y + sin(angle) * (2 + i);
            if (sx >= 0 && sx < width && sy >= 0 && sy < height) {
                Color snout_color = (i == 2) ? dragon_emerald : dragon_green;
                canvas->SetPixel(sx, sy, snout_color.r, snout_color.g, snout_color.b);
            }
        }
        
        // Head main body
        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -2; dx <= 2; dx++) {
                if (dx*dx + dy*dy <= 4) {
                    int px = x + dx;
                    int py = y + dy;
                    if (px >= 0 && px < width && py >= 0 && py < height) {
                        if (dx*dx + dy*dy <= 2) {
                            canvas->SetPixel(px, py, dragon_emerald.r, dragon_emerald.g, dragon_emerald.b);
                        } else {
                            canvas->SetPixel(px, py, dragon_green.r, dragon_green.g, dragon_green.b);
                        }
                    }
                }
            }
        }
        
        // Eyes - fierce red/yellow
        int eye_x = x + cos(angle + M_PI / 4) * 2;
        int eye_y = y + sin(angle + M_PI / 4) * 2 - 1;
        if (eye_x >= 0 && eye_x < width && eye_y >= 0 && eye_y < height) {
            canvas->SetPixel(eye_x, eye_y, eye_red.r, eye_red.g, eye_red.b);
        }
        
        // Horns
        int horn1_x = x + cos(angle + M_PI / 2) * 3;
        int horn1_y = y + sin(angle + M_PI / 2) * 3;
        if (horn1_x >= 0 && horn1_x < width && horn1_y >= 0 && horn1_y < height) {
            canvas->SetPixel(horn1_x, horn1_y, dragon_dark_green.r, dragon_dark_green.g, dragon_dark_green.b);
        }
        
        int horn2_x = x + cos(angle - M_PI / 2) * 3;
        int horn2_y = y + sin(angle - M_PI / 2) * 3;
        if (horn2_x >= 0 && horn2_x < width && horn2_y >= 0 && horn2_y < height) {
            canvas->SetPixel(horn2_x, horn2_y, dragon_dark_green.r, dragon_dark_green.g, dragon_dark_green.b);
        }
        
        // Spikes along neck
        for (int s = 1; s <= 2; s++) {
            int spike_x = x - cos(angle) * s * 2 + cos(angle + M_PI / 2) * 2;
            int spike_y = y - sin(angle) * s * 2 + sin(angle + M_PI / 2) * 2;
            if (spike_x >= 0 && spike_x < width && spike_y >= 0 && spike_y < height) {
                canvas->SetPixel(spike_x, spike_y, dragon_forest.r, dragon_forest.g, dragon_forest.b);
            }
        }
    }
    
    void drawDragonBody(int x, int y, int segment_index) {
        // Body with scales
        Color body_color;
        if (segment_index % 4 == 0) {
            body_color = dragon_emerald;
        } else if (segment_index % 4 == 1) {
            body_color = dragon_green;
        } else if (segment_index % 4 == 2) {
            body_color = dragon_forest;
        } else {
            body_color = dragon_dark_green;
        }
        
        // Thicker body segments
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int px = x + dx;
                int py = y + dy;
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    if (dx == 0 && dy == 0) {
                        canvas->SetPixel(px, py, body_color.r * 1.2, body_color.g * 1.2, body_color.b * 1.2);
                    } else {
                        canvas->SetPixel(px, py, body_color.r, body_color.g, body_color.b);
                    }
                }
            }
        }
        
        // Spines/ridges on back
        if (segment_index % 3 == 0) {
            if (y - 2 >= 0 && x >= 0 && x < width) {
                canvas->SetPixel(x, y - 2, dragon_forest.r, dragon_forest.g, dragon_forest.b);
            }
        }
    }
    
    void drawWing(int base_x, int base_y, Wing& wing, bool is_left) {
        wing.flap_phase += 0.08;
        
        float flap = sin(wing.flap_phase) * 0.5 + 0.5; // 0 to 1
        int wing_spread = 3 + flap * 4;
        
        int direction = is_left ? -1 : 1;
        
        // Wing membrane
        for (int i = 1; i <= wing_spread; i++) {
            for (int j = 0; j <= i; j++) {
                int wx = base_x + direction * i;
                int wy = base_y - j + 2;
                
                if (wx >= 0 && wx < width && wy >= 0 && wy < height) {
                    float fade = 1.0 - (float)i / wing_spread * 0.5;
                    canvas->SetPixel(wx, wy, 
                                   dragon_forest.r * fade,
                                   dragon_forest.g * fade,
                                   dragon_forest.b * fade);
                }
            }
        }
        
        // Wing bones/structure
        for (int i = 1; i <= wing_spread; i++) {
            int wx = base_x + direction * i;
            int wy = base_y - i + 3;
            if (wx >= 0 && wx < width && wy >= 0 && wy < height) {
                canvas->SetPixel(wx, wy, dragon_dark_green.r, dragon_dark_green.g, dragon_dark_green.b);
            }
        }
    }
    
    void updateDragon() {
        // Slow, majestic movement
        float head_x = width / 2 + sin(time_counter * 0.3) * (width / 5);
        float head_y = height / 3 + cos(time_counter * 0.2) * 3;
        float head_angle = sin(time_counter * 0.3) * 0.3;
        
        // Update head
        dragon[0].x = head_x;
        dragon[0].y = head_y;
        dragon[0].angle = head_angle;
        
        // Body follows head
        for (size_t i = 1; i < dragon.size(); i++) {
            float dx = dragon[i-1].x - dragon[i].x;
            float dy = dragon[i-1].y - dragon[i].y;
            float dist = sqrt(dx*dx + dy*dy);
            
            if (dist > 0) {
                float segment_dist = 2.5;
                dragon[i].x += (dx / dist) * (dist - segment_dist) * 0.25;
                dragon[i].y += (dy / dist) * (dist - segment_dist) * 0.25;
                dragon[i].angle = atan2(dy, dx);
            }
        }
        
        // Draw wings (behind body)
        if (dragon.size() > 3) {
            drawWing((int)dragon[2].x, (int)dragon[2].y, left_wing, true);
            drawWing((int)dragon[2].x, (int)dragon[2].y, right_wing, false);
        }
        
        // Draw body
        for (int i = dragon.size() - 1; i >= 1; i--) {
            drawDragonBody((int)dragon[i].x, (int)dragon[i].y, i);
        }
        
        // Draw head last
        drawDragonHead((int)dragon[0].x, (int)dragon[0].y, dragon[0].angle);
    }
    
    void updateFireBreath() {
        fire_cooldown--;
        
        // Start breathing fire periodically
        if (fire_cooldown <= 0 && rand() % 100 < 5) {
            breathing_fire = true;
            fire_cooldown = 60;
        }
        
        // Create fire particles
        if (breathing_fire && flames.size() < 30) {
            for (int i = 0; i < 3; i++) {
                FireBreath fb;
                float angle = dragon[0].angle + ((float)rand() / RAND_MAX - 0.5) * 0.5;
                fb.x = dragon[0].x + cos(dragon[0].angle) * 5;
                fb.y = dragon[0].y + sin(dragon[0].angle) * 5;
                fb.vx = cos(angle) * (0.8 + (float)rand() / RAND_MAX * 0.4);
                fb.vy = sin(angle) * (0.8 + (float)rand() / RAND_MAX * 0.4);
                fb.life = 0;
                fb.max_life = 15 + rand() % 10;
                fb.size = 1.0 + (float)rand() / RAND_MAX;
                flames.push_back(fb);
            }
        } else if (flames.size() >= 30 || fire_cooldown > 50) {
            breathing_fire = false;
        }
        
        // Update flames
        for (auto it = flames.begin(); it != flames.end();) {
            it->life++;
            
            if (it->life < it->max_life) {
                float fade = 1.0 - (float)it->life / it->max_life;
                
                int fx = (int)it->x;
                int fy = (int)it->y;
                
                if (fx >= 0 && fx < width && fy >= 0 && fy < height) {
                    // Green fire
                    int r = fire_yellow.r * fade * 0.5;
                    int g = fire_green.g * fade;
                    int b = fire_emerald.b * fade * 0.7;
                    canvas->SetPixel(fx, fy, r, g, b);
                    
                    // Larger flames
                    if (it->size > 1.5 && fx + 1 < width) {
                        canvas->SetPixel(fx + 1, fy, r * 0.7, g * 0.7, b * 0.7);
                    }
                }
                
                it->x += it->vx;
                it->y += it->vy;
                it->vy += 0.02; // Slight gravity
                
                ++it;
            } else {
                it = flames.erase(it);
            }
        }
    }
    
    void addMagicParticles() {
        // Magical aura around dragon
        if (rand() % 3 == 0) {
            for (size_t i = 0; i < dragon.size(); i += 2) {
                MagicParticle mp;
                mp.x = dragon[i].x + (rand() % 5 - 2);
                mp.y = dragon[i].y + (rand() % 5 - 2);
                mp.vx = ((float)rand() / RAND_MAX - 0.5) * 0.1;
                mp.vy = -0.1 - (float)rand() / RAND_MAX * 0.1;
                mp.life = 0;
                mp.max_life = 20 + rand() % 15;
                mp.hue = 100 + rand() % 80; // Green-cyan spectrum
                particles.push_back(mp);
            }
        }
        
        // Update particles
        for (auto it = particles.begin(); it != particles.end();) {
            it->life++;
            
            if (it->life < it->max_life) {
                float fade = 1.0 - (float)it->life / it->max_life;
                
                int px = (int)it->x;
                int py = (int)it->y;
                
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    int r, g, b;
                    HSVtoRGB(it->hue, 0.8, fade * 0.7, r, g, b);
                    canvas->SetPixel(px, py, r, g, b);
                }
                
                it->x += it->vx;
                it->y += it->vy;
                
                ++it;
            } else {
                it = particles.erase(it);
            }
        }
    }
    
    void update() {
        canvas->Clear();
        
        drawCaveBackground();
        drawTreasures();
        addMagicParticles();
        updateFireBreath();
        updateDragon();
        
        canvas = matrix->SwapOnVSync(canvas);
        
        time_counter += 0.05;
    }
};

int main(int argc, char *argv[]) {
    RGBMatrix::Options matrix_options;
    rgb_matrix::RuntimeOptions runtime_opt;
    
    // Default settings
    matrix_options.rows = 32;
    matrix_options.cols = 64;
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
    
    GreenDragonScene scene(matrix);
    
    while (!interrupt_received) {
        scene.update();
        usleep(50000); // 50ms = ~20 FPS
    }
    
    matrix->Clear();
    delete matrix;
    
    return 0;
}
