// Birthday Celebration Scene - 32x32 Optimized
// Compilation: g++ -o birthday32 birthday32.cpp -lrgbmatrix -std=c++11

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

struct Balloon {
    float x;
    float y;
    float speed;
    float bob_phase;
    int color_type;
    bool has_string;
};

struct Confetti {
    float x;
    float y;
    float vx;
    float vy;
    int color_r, color_g, color_b;
    float rotation;
};

struct Candle {
    int x;
    int y;
    float flicker_phase;
    int flame_brightness;
};

struct Present {
    int x;
    int y;
    int width;
    int height;
    int box_color;
    int ribbon_color;
};

struct Sparkle {
    int x;
    int y;
    int life;
    int max_life;
    int brightness;
};

class BirthdayScene {
private:
    RGBMatrix *matrix;
    FrameCanvas *canvas;
    int width, height;
    std::vector<Balloon> balloons;
    std::vector<Confetti> confetti;
    std::vector<Candle> candles;
    std::vector<Present> presents;
    std::vector<Sparkle> sparkles;
    float time_counter;
    
    // Birthday colors
    Color balloon_red = Color(200, 20, 20);
    Color balloon_blue = Color(20, 100, 200);
    Color balloon_yellow = Color(220, 200, 20);
    Color balloon_green = Color(20, 180, 20);
    Color balloon_pink = Color(220, 100, 150);
    Color balloon_purple = Color(150, 50, 200);
    
    Color cake_base = Color(160, 120, 80);
    Color cake_frosting = Color(220, 180, 200);
    Color cake_layer = Color(140, 100, 60);
    
    Color candle_wax = Color(200, 180, 150);
    Color flame_yellow = Color(240, 200, 60);
    Color flame_orange = Color(240, 140, 20);
    
    Color present_red = Color(180, 20, 20);
    Color present_blue = Color(20, 80, 180);
    Color present_green = Color(20, 140, 20);
    Color present_yellow = Color(200, 180, 20);
    Color ribbon_gold = Color(220, 180, 40);
    
    Color party_bg = Color(40, 30, 60);
    
public:
    BirthdayScene(RGBMatrix *m) : matrix(m), time_counter(0) {
        canvas = matrix->CreateFrameCanvas();
        width = canvas->width();
        height = canvas->height();
        
        srand(time(NULL));
        
        // Create balloons
        for (int i = 0; i < 6; i++) {
            Balloon b;
            b.x = 3 + i * 5;
            b.y = 3 + (rand() % 5);
            b.speed = 0.05 + (float)rand() / RAND_MAX * 0.05;
            b.bob_phase = (float)rand() / RAND_MAX * 2 * M_PI;
            b.color_type = rand() % 6;
            b.has_string = true;
            balloons.push_back(b);
        }
        
        // Create confetti
        for (int i = 0; i < 25; i++) {
            Confetti c;
            c.x = rand() % width;
            c.y = -(rand() % 20);
            c.vx = ((float)rand() / RAND_MAX - 0.5) * 0.3;
            c.vy = 0.2 + (float)rand() / RAND_MAX * 0.3;
            c.color_r = 100 + rand() % 155;
            c.color_g = 100 + rand() % 155;
            c.color_b = 100 + rand() % 155;
            c.rotation = (float)rand() / RAND_MAX * 2 * M_PI;
            confetti.push_back(c);
        }
        
        // Create birthday cake candles
        int cake_center = width / 2;
        for (int i = 0; i < 3; i++) {
            Candle can;
            can.x = cake_center - 2 + i * 2;
            can.y = height - 10;
            can.flicker_phase = (float)rand() / RAND_MAX * 2 * M_PI;
            can.flame_brightness = 200 + rand() % 55;
            candles.push_back(can);
        }
        
        // Create presents
        Present p1;
        p1.x = 3;
        p1.y = height - 6;
        p1.width = 4;
        p1.height = 4;
        p1.box_color = 0; // red
        p1.ribbon_color = 3; // gold
        presents.push_back(p1);
        
        Present p2;
        p2.x = width - 7;
        p2.y = height - 5;
        p2.width = 4;
        p2.height = 3;
        p2.box_color = 1; // blue
        p2.ribbon_color = 3; // gold
        presents.push_back(p2);
    }
    
    Color getBalloonColor(int type) {
        switch(type) {
            case 0: return balloon_red;
            case 1: return balloon_blue;
            case 2: return balloon_yellow;
            case 3: return balloon_green;
            case 4: return balloon_pink;
            default: return balloon_purple;
        }
    }
    
    Color getPresentColor(int type) {
        switch(type) {
            case 0: return present_red;
            case 1: return present_blue;
            case 2: return present_green;
            default: return present_yellow;
        }
    }
    
    void drawBackground() {
        // Party background with subtle pattern
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int pattern = (x + y) % 4;
                int brightness = (pattern == 0) ? 1.1 : 1.0;
                canvas->SetPixel(x, y, 
                               party_bg.r * brightness, 
                               party_bg.g * brightness, 
                               party_bg.b * brightness);
            }
        }
    }
    
    void drawBalloon(int cx, int cy, int color_type, float bob) {
        Color balloon_color = getBalloonColor(color_type);
        
        int bob_offset = sin(bob) * 1;
        int by = cy + bob_offset;
        
        // Balloon body (oval shape)
        // Top
        if (by - 2 >= 0 && cx >= 0 && cx < width) {
            canvas->SetPixel(cx, by - 2, balloon_color.r * 0.8, balloon_color.g * 0.8, balloon_color.b * 0.8);
        }
        
        // Middle rows (wider)
        for (int dy = -1; dy <= 1; dy++) {
            int y = by + dy;
            if (y >= 0 && y < height) {
                if (cx - 1 >= 0) {
                    canvas->SetPixel(cx - 1, y, balloon_color.r, balloon_color.g, balloon_color.b);
                }
                if (cx >= 0 && cx < width) {
                    // Highlight
                    canvas->SetPixel(cx, y, 
                                   std::min(255, balloon_color.r + 40), 
                                   std::min(255, balloon_color.g + 40), 
                                   std::min(255, balloon_color.b + 40));
                }
                if (cx + 1 < width) {
                    canvas->SetPixel(cx + 1, y, balloon_color.r, balloon_color.g, balloon_color.b);
                }
            }
        }
        
        // Bottom (narrower)
        if (by + 2 < height && cx >= 0 && cx < width) {
            canvas->SetPixel(cx, by + 2, balloon_color.r * 0.7, balloon_color.g * 0.7, balloon_color.b * 0.7);
        }
        
        // Knot
        if (by + 3 < height && cx >= 0 && cx < width) {
            canvas->SetPixel(cx, by + 3, balloon_color.r * 0.5, balloon_color.g * 0.5, balloon_color.b * 0.5);
        }
        
        // String
        for (int s = 4; s < 8; s++) {
            int sy = by + s;
            int sx = cx + sin((bob + s) * 0.5) * 1;
            if (sy < height && sx >= 0 && sx < width) {
                canvas->SetPixel(sx, sy, 100, 100, 100);
            }
        }
    }
    
    void updateBalloons() {
        for (auto& balloon : balloons) {
            balloon.bob_phase += 0.05;
            
            drawBalloon((int)balloon.x, (int)balloon.y, balloon.color_type, balloon.bob_phase);
            
            // Slow upward drift
            balloon.y -= balloon.speed;
            
            // Reset at top
            if (balloon.y < -5) {
                balloon.y = height + 2;
                balloon.x = rand() % width;
                balloon.color_type = rand() % 6;
            }
        }
    }
    
    void drawCake() {
        int cake_x = width / 2 - 4;
        int cake_y = height - 8;
        int cake_width = 8;
        
        // Cake layers
        // Bottom layer
        for (int y = 0; y < 3; y++) {
            for (int x = 0; x < cake_width; x++) {
                int px = cake_x + x;
                int py = cake_y + y;
                if (px >= 0 && px < width && py < height) {
                    if (y == 0) {
                        canvas->SetPixel(px, py, cake_frosting.r, cake_frosting.g, cake_frosting.b);
                    } else {
                        canvas->SetPixel(px, py, cake_base.r, cake_base.g, cake_base.b);
                    }
                }
            }
        }
        
        // Top layer (smaller)
        int top_offset = 1;
        for (int y = 0; y < 2; y++) {
            for (int x = top_offset; x < cake_width - top_offset; x++) {
                int px = cake_x + x;
                int py = cake_y - 2 + y;
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    if (y == 0) {
                        canvas->SetPixel(px, py, cake_frosting.r, cake_frosting.g, cake_frosting.b);
                    } else {
                        canvas->SetPixel(px, py, cake_layer.r, cake_layer.g, cake_layer.b);
                    }
                }
            }
        }
    }
    
    void drawCandles() {
        for (auto& candle : candles) {
            candle.flicker_phase += 0.15;
            
            // Candle wax
            if (candle.y >= 0 && candle.y < height && candle.x >= 0 && candle.x < width) {
                canvas->SetPixel(candle.x, candle.y, candle_wax.r, candle_wax.g, candle_wax.b);
                if (candle.y + 1 < height) {
                    canvas->SetPixel(candle.x, candle.y + 1, candle_wax.r * 0.9, candle_wax.g * 0.9, candle_wax.b * 0.9);
                }
            }
            
            // Flame (flickering)
            float flicker = sin(candle.flicker_phase) * 0.3 + 0.7;
            int flame_y = candle.y - 1 - (rand() % 2);
            
            if (flame_y >= 0 && flame_y < height && candle.x >= 0 && candle.x < width) {
                canvas->SetPixel(candle.x, flame_y, 
                               flame_yellow.r * flicker, 
                               flame_yellow.g * flicker, 
                               flame_yellow.b * flicker);
            }
            
            // Flame glow
            if (flame_y - 1 >= 0 && candle.x >= 0 && candle.x < width) {
                canvas->SetPixel(candle.x, flame_y - 1, 
                               flame_orange.r * flicker * 0.7, 
                               flame_orange.g * flicker * 0.7, 
                               flame_orange.b * flicker * 0.7);
            }
        }
    }
    
    void drawPresent(Present& present) {
        Color box_color = getPresentColor(present.box_color);
        
        // Box
        for (int y = 0; y < present.height; y++) {
            for (int x = 0; x < present.width; x++) {
                int px = present.x + x;
                int py = present.y + y;
                if (px >= 0 && px < width && py < height) {
                    canvas->SetPixel(px, py, box_color.r, box_color.g, box_color.b);
                }
            }
        }
        
        // Ribbon (vertical)
        int ribbon_x = present.x + present.width / 2;
        for (int y = 0; y < present.height; y++) {
            int py = present.y + y;
            if (ribbon_x >= 0 && ribbon_x < width && py < height) {
                canvas->SetPixel(ribbon_x, py, ribbon_gold.r, ribbon_gold.g, ribbon_gold.b);
            }
        }
        
        // Ribbon (horizontal)
        int ribbon_y = present.y + present.height / 2;
        for (int x = 0; x < present.width; x++) {
            int px = present.x + x;
            if (px >= 0 && px < width && ribbon_y < height) {
                canvas->SetPixel(px, ribbon_y, ribbon_gold.r, ribbon_gold.g, ribbon_gold.b);
            }
        }
        
        // Bow on top
        int bow_x = present.x + present.width / 2;
        int bow_y = present.y - 1;
        if (bow_y >= 0 && bow_x - 1 >= 0 && bow_x + 1 < width) {
            canvas->SetPixel(bow_x, bow_y, ribbon_gold.r * 1.2, ribbon_gold.g * 1.2, ribbon_gold.b * 1.2);
            canvas->SetPixel(bow_x - 1, bow_y, ribbon_gold.r, ribbon_gold.g, ribbon_gold.b);
            canvas->SetPixel(bow_x + 1, bow_y, ribbon_gold.r, ribbon_gold.g, ribbon_gold.b);
        }
    }
    
    void drawPresents() {
        for (auto& present : presents) {
            drawPresent(present);
        }
    }
    
    void updateConfetti() {
        for (auto& conf : confetti) {
            int cx = (int)conf.x;
            int cy = (int)conf.y;
            
            if (cx >= 0 && cx < width && cy >= 0 && cy < height) {
                canvas->SetPixel(cx, cy, conf.color_r, conf.color_g, conf.color_b);
            }
            
            // Fall and spin
            conf.y += conf.vy;
            conf.x += conf.vx;
            conf.rotation += 0.1;
            
            // Wrap horizontally
            if (conf.x < 0) conf.x = width - 1;
            if (conf.x >= width) conf.x = 0;
            
            // Reset at bottom
            if (conf.y > height) {
                conf.y = -2;
                conf.x = rand() % width;
                conf.color_r = 100 + rand() % 155;
                conf.color_g = 100 + rand() % 155;
                conf.color_b = 100 + rand() % 155;
            }
        }
    }
    
    void addSparkles() {
        // Add sparkles around candles
        if (rand() % 5 < 2) {
            for (auto& candle : candles) {
                Sparkle s;
                s.x = candle.x + (rand() % 5 - 2);
                s.y = candle.y - 2 - rand() % 3;
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
        
        drawBackground();
        updateConfetti();
        drawPresents();
        drawCake();
        drawCandles();
        updateBalloons();
        addSparkles();
        
        canvas = matrix->SwapOnVSync(canvas);
        
        time_counter += 0.05;
    }
};

int main(int argc, char *argv[]) {
    RGBMatrix::Options matrix_options;
    rgb_matrix::RuntimeOptions runtime_opt;
    
    // Optimized for 32x32
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
    
    BirthdayScene scene(matrix);
    
    while (!interrupt_received) {
        scene.update();
        usleep(50000); // 50ms = ~20 FPS
    }
    
    matrix->Clear();
    delete matrix;
    
    return 0;
}
