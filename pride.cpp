// Pride Celebration Scene
// Compilation: g++ -o pride pride.cpp -lrgbmatrix -std=c++11

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

struct RainbowWave {
    float phase;
    int y_offset;
};

struct Confetti {
    float x;
    float y;
    float vx;
    float vy;
    int color_index;
    float rotation;
    float rot_speed;
};

struct Heart {
    float x;
    float y;
    float speed;
    int size;
    int color_type;
    float pulse_phase;
};

struct Sparkle {
    int x;
    int y;
    int life;
    int max_life;
    int color_index;
};

struct RainbowFlag {
    int start_y;
    float wave_phase;
};

class PrideScene {
private:
    RGBMatrix *matrix;
    FrameCanvas *canvas;
    int width, height;
    std::vector<RainbowWave> waves;
    std::vector<Confetti> confetti;
    std::vector<Heart> hearts;
    std::vector<Sparkle> sparkles;
    RainbowFlag flag;
    float time_counter;
    
    // Classic Pride rainbow colors (6-stripe version)
    std::vector<Color> pride_colors = {
        Color(228, 3, 3),      // Red
        Color(255, 140, 0),    // Orange
        Color(255, 237, 0),    // Yellow
        Color(0, 128, 38),     // Green
        Color(0, 77, 255),     // Blue
        Color(117, 7, 135)     // Purple/Violet
    };
    
    // Additional pride flag variations
    Color trans_blue = Color(91, 206, 250);
    Color trans_pink = Color(245, 169, 184);
    Color trans_white = Color(255, 255, 255);
    
    Color bi_pink = Color(214, 2, 112);
    Color bi_purple = Color(115, 79, 150);
    Color bi_blue = Color(0, 56, 168);
    
    Color sparkle_white = Color(255, 255, 255);
    Color sparkle_gold = Color(255, 215, 0);
    
public:
    PrideScene(RGBMatrix *m) : matrix(m), time_counter(0) {
        canvas = matrix->CreateFrameCanvas();
        width = canvas->width();
        height = canvas->height();
        
        srand(time(NULL));
        
        // Create rainbow waves
        for (int i = 0; i < 6; i++) {
            RainbowWave w;
            w.phase = (float)i / 6.0 * 2 * M_PI;
            w.y_offset = i;
            waves.push_back(w);
        }
        
        // Create confetti
        for (int i = 0; i < 20; i++) {
            Confetti c;
            c.x = rand() % width;
            c.y = -(rand() % height);
            c.vx = ((float)rand() / RAND_MAX - 0.5) * 0.2;
            c.vy = 0.3 + (float)rand() / RAND_MAX * 0.3;
            c.color_index = rand() % 6;
            c.rotation = (float)rand() / RAND_MAX * 2 * M_PI;
            c.rot_speed = ((float)rand() / RAND_MAX - 0.5) * 0.1;
            confetti.push_back(c);
        }
        
        // Create floating hearts
        for (int i = 0; i < 4; i++) {
            Heart h;
            h.x = rand() % width;
            h.y = height + rand() % 10;
            h.speed = 0.15 + (float)rand() / RAND_MAX * 0.15;
            h.size = 2 + rand() % 2;
            h.color_type = rand() % 6;
            h.pulse_phase = (float)rand() / RAND_MAX * 2 * M_PI;
            hearts.push_back(h);
        }
        
        // Rainbow flag
        flag.start_y = height / 4;
        flag.wave_phase = 0;
    }
    
    void drawRainbowBackground() {
        // Flowing rainbow stripes
        flag.wave_phase += 0.05;
        
        int stripe_height = (height * 0.7) / 6;
        
        for (int i = 0; i < 6; i++) {
            int start_y = flag.start_y + i * stripe_height;
            
            for (int y = 0; y < stripe_height + 2; y++) {
                int actual_y = start_y + y;
                if (actual_y >= 0 && actual_y < height) {
                    for (int x = 0; x < width; x++) {
                        // Add wave effect
                        float wave = sin((x * 0.2 + flag.wave_phase + i * 0.5)) * 0.2 + 1.0;
                        
                        Color stripe_color = pride_colors[i];
                        int r = std::min(255, (int)(stripe_color.r * wave));
                        int g = std::min(255, (int)(stripe_color.g * wave));
                        int b = std::min(255, (int)(stripe_color.b * wave));
                        
                        canvas->SetPixel(x, actual_y, r, g, b);
                    }
                }
            }
        }
    }
    
    void drawRainbowWaves() {
        // Flowing rainbow waves across the top
        for (int x = 0; x < width; x++) {
            for (int i = 0; i < 6; i++) {
                float wave_y = 3 + sin((x * 0.3 + time_counter * 2 + waves[i].phase)) * 2;
                int y = (int)wave_y + i;
                
                if (y >= 0 && y < height) {
                    Color wave_color = pride_colors[i];
                    canvas->SetPixel(x, y, wave_color.r, wave_color.g, wave_color.b);
                }
            }
        }
    }
    
    void drawHeart(int cx, int cy, int size, Color color, float pulse) {
        float scale = sin(pulse) * 0.2 + 0.9;
        int s = size * scale;
        
        // Heart shape using pixels
        if (s >= 2) {
            // Top bumps
            if (cx - 1 >= 0 && cy - 1 >= 0) {
                canvas->SetPixel(cx - 1, cy - 1, color.r, color.g, color.b);
            }
            if (cx + 1 < width && cy - 1 >= 0) {
                canvas->SetPixel(cx + 1, cy - 1, color.r, color.g, color.b);
            }
            
            // Middle row (wider)
            if (cy >= 0) {
                for (int x = -s; x <= s; x++) {
                    if (cx + x >= 0 && cx + x < width) {
                        canvas->SetPixel(cx + x, cy, color.r, color.g, color.b);
                    }
                }
            }
            
            // Bottom point
            for (int i = 1; i <= s; i++) {
                int row_width = s - i;
                if (cy + i < height) {
                    for (int x = -row_width; x <= row_width; x++) {
                        if (cx + x >= 0 && cx + x < width) {
                            float fade = 1.0 - (float)i / (s + 1) * 0.3;
                            canvas->SetPixel(cx + x, cy + i, color.r * fade, color.g * fade, color.b * fade);
                        }
                    }
                }
            }
        } else {
            // Small heart
            if (cx >= 0 && cx < width && cy >= 0 && cy < height) {
                canvas->SetPixel(cx, cy, color.r, color.g, color.b);
            }
        }
    }
    
    void updateHearts() {
        for (auto& heart : hearts) {
            heart.pulse_phase += 0.1;
            
            Color heart_color = pride_colors[heart.color_type];
            drawHeart((int)heart.x, (int)heart.y, heart.size, heart_color, heart.pulse_phase);
            
            // Float upward
            heart.y -= heart.speed;
            
            // Gentle drift
            heart.x += sin(time_counter + heart.pulse_phase) * 0.1;
            
            // Reset when off screen
            if (heart.y < -5) {
                heart.y = height + 5;
                heart.x = rand() % width;
                heart.color_type = rand() % 6;
            }
        }
    }
    
    void drawConfetti() {
        for (auto& conf : confetti) {
            int cx = (int)conf.x;
            int cy = (int)conf.y;
            
            if (cx >= 0 && cx < width && cy >= 0 && cy < height) {
                Color confetti_color = pride_colors[conf.color_index];
                canvas->SetPixel(cx, cy, confetti_color.r, confetti_color.g, confetti_color.b);
                
                // Make confetti bigger with rotation effect
                int offset_x = cos(conf.rotation);
                int offset_y = sin(conf.rotation) * 0.5;
                
                if (cx + offset_x >= 0 && cx + offset_x < width && 
                    cy + offset_y >= 0 && cy + offset_y < height) {
                    canvas->SetPixel(cx + offset_x, cy + offset_y, 
                                   confetti_color.r * 0.7, 
                                   confetti_color.g * 0.7, 
                                   confetti_color.b * 0.7);
                }
            }
            
            // Fall and drift
            conf.y += conf.vy;
            conf.x += conf.vx;
            conf.rotation += conf.rot_speed;
            
            // Wrap horizontally
            if (conf.x < 0) conf.x = width - 1;
            if (conf.x >= width) conf.x = 0;
            
            // Reset at bottom
            if (conf.y > height) {
                conf.y = -2;
                conf.x = rand() % width;
                conf.color_index = rand() % 6;
            }
        }
    }
    
    void addSparkles() {
        // Randomly add sparkles
        if (rand() % 10 < 4) {
            Sparkle s;
            s.x = rand() % width;
            s.y = rand() % height;
            s.life = 0;
            s.max_life = 10 + rand() % 15;
            s.color_index = rand() % 6;
            sparkles.push_back(s);
        }
        
        // Update and draw sparkles
        for (auto it = sparkles.begin(); it != sparkles.end();) {
            it->life++;
            
            if (it->life < it->max_life) {
                float brightness = 1.0 - (float)it->life / it->max_life;
                brightness = sin(brightness * M_PI); // Smooth fade in/out
                
                Color spark_color = pride_colors[it->color_index];
                int r = spark_color.r * brightness;
                int g = spark_color.g * brightness;
                int b = spark_color.b * brightness;
                
                canvas->SetPixel(it->x, it->y, r, g, b);
                
                // Sparkle rays
                if (it->life < it->max_life / 2) {
                    if (it->x > 0) canvas->SetPixel(it->x - 1, it->y, r * 0.6, g * 0.6, b * 0.6);
                    if (it->x < width - 1) canvas->SetPixel(it->x + 1, it->y, r * 0.6, g * 0.6, b * 0.6);
                    if (it->y > 0) canvas->SetPixel(it->x, it->y - 1, r * 0.6, g * 0.6, b * 0.6);
                    if (it->y < height - 1) canvas->SetPixel(it->x, it->y + 1, r * 0.6, g * 0.6, b * 0.6);
                }
                
                ++it;
            } else {
                it = sparkles.erase(it);
            }
        }
    }
    
    void drawPrideText() {
        // Draw "PRIDE" or rainbow elements
        int center_y = height / 2 - 3;
        
        // Draw rainbow circles/dots pattern
        for (int i = 0; i < 6; i++) {
            int x = width / 2 - 8 + i * 3;
            int y = center_y + sin(time_counter * 2 + i * 0.5) * 2;
            
            if (x >= 0 && x < width && y >= 0 && y < height) {
                Color dot_color = pride_colors[i];
                canvas->SetPixel(x, y, dot_color.r, dot_color.g, dot_color.b);
                
                // Make dots bigger
                if (y + 1 < height) {
                    canvas->SetPixel(x, y + 1, dot_color.r * 0.7, dot_color.g * 0.7, dot_color.b * 0.7);
                }
            }
        }
    }
    
    void drawCelebrationBurst() {
        // Periodic celebration burst from center
        if ((int)time_counter % 10 == 0 && ((int)(time_counter * 10) % 10) < 3) {
            int burst_x = width / 2;
            int burst_y = height / 2;
            
            for (int angle = 0; angle < 360; angle += 30) {
                float rad = angle * M_PI / 180.0;
                int length = 5 + sin(time_counter * 10) * 3;
                
                for (int r = 3; r < length; r++) {
                    int px = burst_x + cos(rad) * r;
                    int py = burst_y + sin(rad) * r;
                    
                    if (px >= 0 && px < width && py >= 0 && py < height) {
                        Color burst_color = pride_colors[angle / 60];
                        float fade = 1.0 - (float)r / length;
                        canvas->SetPixel(px, py, burst_color.r * fade, burst_color.g * fade, burst_color.b * fade);
                    }
                }
            }
        }
    }
    
    void update() {
        canvas->Clear();
        
        drawRainbowBackground();
        drawCelebrationBurst();
        updateHearts();
        drawConfetti();
        addSparkles();
        drawPrideText();
        
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
    
    PrideScene scene(matrix);
    
    while (!interrupt_received) {
        scene.update();
        usleep(50000); // 50ms = ~20 FPS
    }
    
    matrix->Clear();
    delete matrix;
    
    return 0;
}
