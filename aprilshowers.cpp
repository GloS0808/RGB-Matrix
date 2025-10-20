// April Showers Scene
// Compilation: g++ -o aprilshowers aprilshowers.cpp -lrgbmatrix -std=c++11

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

struct Raindrop {
    float x;
    float y;
    float speed;
    int length;
    int brightness;
};

struct Cloud {
    float x;
    float y;
    int size;
    float speed;
    int darkness;
};

struct Puddle {
    int x;
    int y;
    int size;
    int ripple_phase;
};

struct Lightning {
    int x;
    int segments;
    int life;
    int brightness;
};

struct Umbrella {
    int x;
    int y;
    int color_type;
    float bob_phase;
};

class AprilShowersScene {
private:
    RGBMatrix *matrix;
    FrameCanvas *canvas;
    int width, height;
    std::vector<Raindrop> raindrops;
    std::vector<Cloud> clouds;
    std::vector<Puddle> puddles;
    std::vector<Lightning> lightning;
    std::vector<Umbrella> umbrellas;
    float time_counter;
    int thunder_cooldown;
    
    // Colors
    Color sky_dark = Color(40, 50, 70);
    Color sky_storm = Color(30, 35, 50);
    Color cloud_gray = Color(70, 70, 80);
    Color cloud_dark = Color(50, 50, 60);
    Color cloud_light = Color(90, 90, 100);
    Color rain_blue = Color(100, 120, 160);
    Color rain_light = Color(140, 160, 200);
    Color ground_brown = Color(60, 50, 40);
    Color ground_dark = Color(40, 35, 30);
    Color puddle_blue = Color(80, 90, 120);
    Color puddle_light = Color(100, 110, 140);
    Color lightning_white = Color(220, 220, 240);
    Color lightning_yellow = Color(200, 200, 150);
    
    // Umbrella colors
    Color umbrella_red = Color(160, 20, 20);
    Color umbrella_yellow = Color(180, 160, 20);
    Color umbrella_blue = Color(40, 80, 160);
    Color umbrella_green = Color(40, 120, 40);
    
public:
    AprilShowersScene(RGBMatrix *m) : matrix(m), time_counter(0), thunder_cooldown(0) {
        canvas = matrix->CreateFrameCanvas();
        width = canvas->width();
        height = canvas->height();
        
        srand(time(NULL));
        
        // Create rain
        for (int i = 0; i < 40; i++) {
            Raindrop r;
            r.x = rand() % width;
            r.y = rand() % height;
            r.speed = 1.0 + (float)rand() / RAND_MAX * 1.5;
            r.length = 1 + rand() % 3;
            r.brightness = 80 + rand() % 60;
            raindrops.push_back(r);
        }
        
        // Create clouds
        for (int i = 0; i < 4; i++) {
            Cloud c;
            c.x = rand() % width;
            c.y = 2 + rand() % 6;
            c.size = 4 + rand() % 4;
            c.speed = 0.05 + (float)rand() / RAND_MAX * 0.1;
            c.darkness = 50 + rand() % 30;
            clouds.push_back(c);
        }
        
        // Create puddles on ground
        int ground_y = height - 4;
        for (int i = 0; i < 3; i++) {
            Puddle p;
            p.x = 5 + (i * width / 3) + (rand() % 5);
            p.y = ground_y;
            p.size = 3 + rand() % 3;
            p.ripple_phase = rand() % 20;
            puddles.push_back(p);
        }
        
        // Create umbrellas
        if (width >= 32) {
            for (int i = 0; i < 2; i++) {
                Umbrella u;
                u.x = 8 + i * (width - 16);
                u.y = height - 8;
                u.color_type = rand() % 4;
                u.bob_phase = (float)rand() / RAND_MAX * 2 * M_PI;
                umbrellas.push_back(u);
            }
        }
    }
    
    void drawSky() {
        // Stormy sky gradient
        for (int y = 0; y < height * 0.7; y++) {
            float storm_intensity = sin(time_counter * 0.5) * 0.2 + 0.8;
            for (int x = 0; x < width; x++) {
                int r = sky_dark.r * storm_intensity;
                int g = sky_dark.g * storm_intensity;
                int b = sky_dark.b * storm_intensity;
                canvas->SetPixel(x, y, r, g, b);
            }
        }
    }
    
    void drawGround() {
        int ground_start = height * 0.7;
        
        // Muddy ground
        for (int y = ground_start; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if ((x + y) % 3 == 0) {
                    canvas->SetPixel(x, y, ground_brown.r, ground_brown.g, ground_brown.b);
                } else {
                    canvas->SetPixel(x, y, ground_dark.r, ground_dark.g, ground_dark.b);
                }
            }
        }
    }
    
    void drawClouds() {
        for (auto& cloud : clouds) {
            // Draw fluffy storm cloud
            for (int i = 0; i < cloud.size; i++) {
                int cx = (int)cloud.x + i;
                int cy = (int)cloud.y;
                
                if (cx >= 0 && cx < width && cy >= 0 && cy < height) {
                    // Cloud top
                    canvas->SetPixel(cx, cy, cloud_light.r, cloud_light.g, cloud_light.b);
                    
                    // Cloud middle
                    if (cy + 1 < height && i > 0 && i < cloud.size - 1) {
                        canvas->SetPixel(cx, cy + 1, cloud_gray.r, cloud_gray.g, cloud_gray.b);
                    }
                    
                    // Cloud bottom (darker, rain-heavy)
                    if (cy + 2 < height && i > 0 && i < cloud.size - 1) {
                        canvas->SetPixel(cx, cy + 2, cloud_dark.r, cloud_dark.g, cloud_dark.b);
                    }
                }
            }
            
            // Move cloud slowly
            cloud.x += cloud.speed;
            if (cloud.x > width + cloud.size) {
                cloud.x = -cloud.size;
                cloud.y = 2 + rand() % 6;
            }
        }
    }
    
    void drawRain() {
        for (auto& drop : raindrops) {
            // Draw raindrop streak
            for (int i = 0; i < drop.length; i++) {
                int rx = (int)drop.x;
                int ry = (int)drop.y + i;
                
                if (rx >= 0 && rx < width && ry >= 0 && ry < height) {
                    float fade = 1.0 - (float)i / drop.length;
                    int r = rain_light.r * fade * drop.brightness / 100;
                    int g = rain_light.g * fade * drop.brightness / 100;
                    int b = rain_light.b * fade * drop.brightness / 100;
                    canvas->SetPixel(rx, ry, r, g, b);
                }
            }
            
            // Move rain down
            drop.y += drop.speed;
            
            // Check if hit ground
            if (drop.y > height * 0.7) {
                // Create splash/ripple in nearby puddle
                for (auto& puddle : puddles) {
                    if (abs((int)drop.x - puddle.x) < puddle.size) {
                        puddle.ripple_phase = 0;
                    }
                }
                
                // Reset raindrop
                drop.y = -drop.length;
                drop.x = rand() % width;
            }
        }
    }
    
    void drawPuddles() {
        for (auto& puddle : puddles) {
            // Draw puddle
            for (int i = 0; i < puddle.size; i++) {
                int px = puddle.x + i - puddle.size / 2;
                int py = puddle.y;
                
                if (px >= 0 && px < width && py < height) {
                    // Ripple effect
                    if (puddle.ripple_phase < 10) {
                        int ripple_dist = abs(i - puddle.size / 2);
                        if (ripple_dist == puddle.ripple_phase / 3) {
                            canvas->SetPixel(px, py, puddle_light.r, puddle_light.g, puddle_light.b);
                        } else {
                            canvas->SetPixel(px, py, puddle_blue.r, puddle_blue.g, puddle_blue.b);
                        }
                    } else {
                        // Still water reflection
                        float shimmer = sin(time_counter * 2 + i) * 0.2 + 0.8;
                        canvas->SetPixel(px, py, puddle_blue.r * shimmer, 
                                       puddle_blue.g * shimmer, 
                                       puddle_blue.b * shimmer);
                    }
                }
            }
            
            puddle.ripple_phase++;
            if (puddle.ripple_phase > 50) {
                puddle.ripple_phase = 20; // Keep shimmering
            }
        }
    }
    
    void drawUmbrella(int x, int y, int color_type, float bob) {
        Color umbrella_color;
        switch(color_type) {
            case 0: umbrella_color = umbrella_red; break;
            case 1: umbrella_color = umbrella_yellow; break;
            case 2: umbrella_color = umbrella_blue; break;
            default: umbrella_color = umbrella_green; break;
        }
        
        int bob_offset = sin(bob) * 1;
        int uy = y + bob_offset;
        
        // Umbrella canopy (dome shape)
        if (x >= 2 && x < width - 2 && uy >= 2 && uy < height) {
            // Top center
            canvas->SetPixel(x, uy - 2, umbrella_color.r, umbrella_color.g, umbrella_color.b);
            
            // Middle row
            canvas->SetPixel(x - 1, uy - 1, umbrella_color.r, umbrella_color.g, umbrella_color.b);
            canvas->SetPixel(x, uy - 1, umbrella_color.r * 0.8, umbrella_color.g * 0.8, umbrella_color.b * 0.8);
            canvas->SetPixel(x + 1, uy - 1, umbrella_color.r, umbrella_color.g, umbrella_color.b);
            
            // Bottom edge
            canvas->SetPixel(x - 2, uy, umbrella_color.r * 0.6, umbrella_color.g * 0.6, umbrella_color.b * 0.6);
            canvas->SetPixel(x - 1, uy, umbrella_color.r * 0.7, umbrella_color.g * 0.7, umbrella_color.b * 0.7);
            canvas->SetPixel(x + 1, uy, umbrella_color.r * 0.7, umbrella_color.g * 0.7, umbrella_color.b * 0.7);
            canvas->SetPixel(x + 2, uy, umbrella_color.r * 0.6, umbrella_color.g * 0.6, umbrella_color.b * 0.6);
            
            // Handle
            for (int i = 1; i < 4; i++) {
                if (uy + i < height) {
                    canvas->SetPixel(x, uy + i, 80, 60, 40);
                }
            }
            
            // Person (simple stick figure)
            if (uy + 4 < height) {
                canvas->SetPixel(x, uy + 4, 100, 80, 60); // Head
            }
            if (uy + 5 < height) {
                canvas->SetPixel(x, uy + 5, 60, 80, 100); // Body
            }
        }
    }
    
    void drawUmbrellas() {
        for (auto& umbrella : umbrellas) {
            umbrella.bob_phase += 0.05;
            drawUmbrella(umbrella.x, umbrella.y, umbrella.color_type, umbrella.bob_phase);
        }
    }
    
    void updateLightning() {
        // Random lightning
        thunder_cooldown--;
        if (thunder_cooldown <= 0 && rand() % 200 == 0) {
            Lightning l;
            l.x = 5 + rand() % (width - 10);
            l.segments = 3 + rand() % 5;
            l.life = 3;
            l.brightness = 200 + rand() % 55;
            lightning.push_back(l);
            thunder_cooldown = 100 + rand() % 200;
        }
        
        // Draw and update lightning
        for (auto it = lightning.begin(); it != lightning.end();) {
            if (it->life > 0) {
                int lx = it->x;
                int ly = 8;
                
                // Draw jagged lightning bolt
                for (int seg = 0; seg < it->segments; seg++) {
                    int offset = (rand() % 3) - 1;
                    lx += offset;
                    ly += 3;
                    
                    if (lx >= 0 && lx < width && ly < height * 0.7) {
                        // Bright core
                        canvas->SetPixel(lx, ly, lightning_white.r * it->brightness / 255, 
                                       lightning_white.g * it->brightness / 255, 
                                       lightning_white.b * it->brightness / 255);
                        
                        // Glow
                        if (lx > 0) {
                            canvas->SetPixel(lx - 1, ly, lightning_yellow.r * it->brightness / 255 / 2, 
                                           lightning_yellow.g * it->brightness / 255 / 2, 
                                           lightning_yellow.b * it->brightness / 255 / 2);
                        }
                        if (lx < width - 1) {
                            canvas->SetPixel(lx + 1, ly, lightning_yellow.r * it->brightness / 255 / 2, 
                                           lightning_yellow.g * it->brightness / 255 / 2, 
                                           lightning_yellow.b * it->brightness / 255 / 2);
                        }
                    }
                }
                
                it->life--;
                it->brightness *= 0.7; // Fade out
                ++it;
            } else {
                it = lightning.erase(it);
            }
        }
    }
    
    void drawSpringBuds() {
        // Small spring plants/flowers trying to grow despite the rain
        int ground_y = height * 0.7;
        for (int x = 3; x < width; x += 8) {
            int bud_x = x + (rand() % 3);
            int bud_y = ground_y;
            
            if (bud_x < width && bud_y < height) {
                // Small stem
                canvas->SetPixel(bud_x, bud_y, 40, 100, 40);
                if (bud_y - 1 >= ground_y - 2) {
                    canvas->SetPixel(bud_x, bud_y - 1, 40, 100, 40);
                }
                
                // Small bud
                if (bud_y - 2 >= ground_y - 3) {
                    canvas->SetPixel(bud_x, bud_y - 2, 120, 60, 100);
                }
            }
        }
    }
    
    void update() {
        canvas->Clear();
        
        drawSky();
        drawGround();
        drawClouds();
        drawSpringBuds();
        drawPuddles();
        drawUmbrellas();
        drawRain();
        updateLightning();
        
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
    
    AprilShowersScene scene(matrix);
    
    while (!interrupt_received) {
        scene.update();
        usleep(50000); // 50ms = ~20 FPS
    }
    
    matrix->Clear();
    delete matrix;
    
    return 0;
}
