// 4/20 Scene - Chill Vibes
// Compilation: g++ -o fourtwenty fourtwenty.cpp -lrgbmatrix -std=c++11

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
    float speed;
    float amplitude;
};

struct FloatingLeaf {
    float x;
    float y;
    float vx;
    float vy;
    float rotation;
    float rot_speed;
    int color_phase;
};

struct PsychedelicCircle {
    int x;
    int y;
    float radius;
    float growth_speed;
    int hue_offset;
};

struct SmokeWisp {
    float x;
    float y;
    float vx;
    float vy;
    int age;
    int max_age;
    float swirl;
};

struct ColorPulse {
    float phase;
    float speed;
};

class FourTwentyScene {
private:
    RGBMatrix *matrix;
    FrameCanvas *canvas;
    int width, height;
    std::vector<RainbowWave> waves;
    std::vector<FloatingLeaf> leaves;
    std::vector<PsychedelicCircle> circles;
    std::vector<SmokeWisp> smoke;
    ColorPulse pulse;
    float time_counter;
    
    // Chill color palette
    Color leaf_green = Color(60, 160, 60);
    Color leaf_dark = Color(40, 100, 40);
    Color smoke_gray = Color(120, 120, 140);
    Color smoke_light = Color(140, 140, 160);
    
public:
    FourTwentyScene(RGBMatrix *m) : matrix(m), time_counter(0) {
        canvas = matrix->CreateFrameCanvas();
        width = canvas->width();
        height = canvas->height();
        
        srand(time(NULL));
        
        // Rainbow waves
        for (int i = 0; i < 3; i++) {
            RainbowWave w;
            w.phase = (float)rand() / RAND_MAX * 2 * M_PI;
            w.speed = 0.3 + (float)rand() / RAND_MAX * 0.3;
            w.amplitude = 3 + rand() % 3;
            waves.push_back(w);
        }
        
        // Floating leaves
        for (int i = 0; i < 5; i++) {
            FloatingLeaf l;
            l.x = rand() % width;
            l.y = rand() % height;
            l.vx = ((float)rand() / RAND_MAX - 0.5) * 0.1;
            l.vy = ((float)rand() / RAND_MAX - 0.5) * 0.1;
            l.rotation = (float)rand() / RAND_MAX * 2 * M_PI;
            l.rot_speed = ((float)rand() / RAND_MAX - 0.5) * 0.05;
            l.color_phase = rand() % 360;
            leaves.push_back(l);
        }
        
        // Psychedelic circles
        for (int i = 0; i < 2; i++) {
            PsychedelicCircle c;
            c.x = rand() % width;
            c.y = rand() % height;
            c.radius = 0;
            c.growth_speed = 0.1 + (float)rand() / RAND_MAX * 0.1;
            c.hue_offset = rand() % 360;
            circles.push_back(c);
        }
        
        // Color pulse
        pulse.phase = 0;
        pulse.speed = 0.5;
        
        // Smoke wisps
        for (int i = 0; i < 8; i++) {
            SmokeWisp s;
            s.x = width / 2 + ((float)rand() / RAND_MAX - 0.5) * 4;
            s.y = height - 3;
            s.vx = ((float)rand() / RAND_MAX - 0.5) * 0.1;
            s.vy = -0.1 - (float)rand() / RAND_MAX * 0.1;
            s.age = rand() % 50;
            s.max_age = 80 + rand() % 40;
            s.swirl = (float)rand() / RAND_MAX * 2 * M_PI;
            smoke.push_back(s);
        }
    }
    
    void HSVtoRGB(float h, float s, float v, int &r, int &g, int &b) {
        // h: 0-360, s: 0-1, v: 0-1
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
    
    void drawPsychedelicBackground() {
        pulse.phase += pulse.speed * 0.05;
        
        // Trippy rainbow gradient background
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // Create flowing rainbow pattern
                float hue = fmod((x * 10 + y * 10 + time_counter * 30), 360);
                float wave = sin((x * 0.2 + y * 0.2 + time_counter * 2) * M_PI / 10) * 0.3 + 0.7;
                
                int r, g, b;
                HSVtoRGB(hue, 0.6, wave * 0.3, r, g, b);
                canvas->SetPixel(x, y, r, g, b);
            }
        }
    }
    
    void drawRainbowWaves() {
        for (auto& wave : waves) {
            wave.phase += wave.speed * 0.05;
            
            for (int x = 0; x < width; x++) {
                int y = height / 2 + sin(x * 0.3 + wave.phase) * wave.amplitude;
                
                if (y >= 0 && y < height) {
                    float hue = fmod((x * 15 + time_counter * 50), 360);
                    int r, g, b;
                    HSVtoRGB(hue, 1.0, 0.8, r, g, b);
                    canvas->SetPixel(x, y, r, g, b);
                    
                    // Make wave thicker
                    if (y + 1 < height) {
                        HSVtoRGB(hue, 1.0, 0.5, r, g, b);
                        canvas->SetPixel(x, y + 1, r, g, b);
                    }
                }
            }
        }
    }
    
    void drawLeaf(int cx, int cy, float rotation, int color_hue) {
        // Cannabis leaf shape (simplified, stylized 7-pointed leaf)
        
        // Center stem
        if (cx >= 0 && cx < width && cy >= 0 && cy < height) {
            int r, g, b;
            HSVtoRGB(color_hue, 0.7, 0.6, r, g, b);
            canvas->SetPixel(cx, cy, r, g, b);
        }
        
        // Draw 7 stylized points
        for (int point = 0; point < 7; point++) {
            float angle = rotation + (point - 3) * M_PI / 8;
            int length = (point == 3) ? 3 : (abs(point - 3) == 1 ? 2 : 1);
            
            for (int l = 1; l <= length; l++) {
                int px = cx + cos(angle) * l;
                int py = cy + sin(angle) * l;
                
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    int r, g, b;
                    float fade = 1.0 - (float)(l - 1) / length;
                    HSVtoRGB(color_hue, 0.7, 0.6 * fade, r, g, b);
                    canvas->SetPixel(px, py, r, g, b);
                }
            }
        }
    }
    
    void updateLeaves() {
        for (auto& leaf : leaves) {
            // Change color slowly through rainbow
            leaf.color_phase = (int)(leaf.color_phase + 1) % 360;
            int hue = (120 + leaf.color_phase / 3) % 360; // Keep in green-ish spectrum mostly
            
            drawLeaf((int)leaf.x, (int)leaf.y, leaf.rotation, hue);
            
            // Float and drift
            leaf.x += leaf.vx;
            leaf.y += leaf.vy;
            leaf.rotation += leaf.rot_speed;
            
            // Bounce off edges
            if (leaf.x < 0 || leaf.x >= width) leaf.vx *= -1;
            if (leaf.y < 0 || leaf.y >= height) leaf.vy *= -1;
            
            // Keep in bounds
            leaf.x = std::max(0.0f, std::min((float)width - 1, leaf.x));
            leaf.y = std::max(0.0f, std::min((float)height - 1, leaf.y));
        }
    }
    
    void drawPsychedelicCircles() {
        for (auto& circle : circles) {
            circle.radius += circle.growth_speed;
            
            // Draw expanding rainbow circle
            for (float angle = 0; angle < 2 * M_PI; angle += 0.3) {
                int px = circle.x + cos(angle) * circle.radius;
                int py = circle.y + sin(angle) * circle.radius;
                
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    float hue = fmod((angle * 180 / M_PI + circle.hue_offset + time_counter * 50), 360);
                    int r, g, b;
                    HSVtoRGB(hue, 1.0, 0.7, r, g, b);
                    canvas->SetPixel(px, py, r, g, b);
                }
            }
            
            // Reset when too big
            if (circle.radius > width / 2 + 5) {
                circle.radius = 0;
                circle.x = rand() % width;
                circle.y = rand() % height;
                circle.hue_offset = rand() % 360;
            }
        }
    }
    
    void updateSmoke() {
        for (auto it = smoke.begin(); it != smoke.end();) {
            it->age++;
            
            if (it->age < it->max_age) {
                // Draw wispy smoke
                float alpha = 1.0 - (float)it->age / it->max_age;
                int brightness = 100 + alpha * 100;
                
                // Add swirl to smoke
                it->swirl += 0.05;
                it->vx += sin(it->swirl) * 0.02;
                
                int sx = (int)it->x;
                int sy = (int)it->y;
                
                if (sx >= 0 && sx < width && sy >= 0 && sy < height) {
                    // Colorful smoke
                    float hue = fmod((time_counter * 20 + it->age * 3), 360);
                    int r, g, b;
                    HSVtoRGB(hue, 0.3, alpha * 0.6, r, g, b);
                    canvas->SetPixel(sx, sy, r, g, b);
                    
                    // Make smoke puff bigger
                    if (sx + 1 < width) {
                        HSVtoRGB(hue, 0.3, alpha * 0.4, r, g, b);
                        canvas->SetPixel(sx + 1, sy, r, g, b);
                    }
                }
                
                // Move smoke
                it->x += it->vx;
                it->y += it->vy;
                
                ++it;
            } else {
                // Remove old smoke and create new
                SmokeWisp new_smoke;
                new_smoke.x = width / 2 + ((float)rand() / RAND_MAX - 0.5) * 4;
                new_smoke.y = height - 3;
                new_smoke.vx = ((float)rand() / RAND_MAX - 0.5) * 0.1;
                new_smoke.vy = -0.1 - (float)rand() / RAND_MAX * 0.1;
                new_smoke.age = 0;
                new_smoke.max_age = 80 + rand() % 40;
                new_smoke.swirl = (float)rand() / RAND_MAX * 2 * M_PI;
                
                it = smoke.erase(it);
                smoke.push_back(new_smoke);
            }
        }
    }
    
    void draw420Text() {
        // Draw "420" in the middle with rainbow colors
        int text_y = height / 2 - 2;
        int text_x = width / 2 - 6;
        
        if (text_x >= 0 && width >= 12) {
            // Simplified pixel "420"
            float hue = fmod((time_counter * 100), 360);
            int r, g, b;
            HSVtoRGB(hue, 1.0, 0.9, r, g, b);
            
            // "4"
            int x_offset = 0;
            canvas->SetPixel(text_x + x_offset, text_y, r, g, b);
            canvas->SetPixel(text_x + x_offset, text_y + 1, r, g, b);
            canvas->SetPixel(text_x + x_offset, text_y + 2, r, g, b);
            canvas->SetPixel(text_x + x_offset + 1, text_y + 2, r, g, b);
            canvas->SetPixel(text_x + x_offset + 2, text_y + 2, r, g, b);
            canvas->SetPixel(text_x + x_offset + 2, text_y + 3, r, g, b);
            canvas->SetPixel(text_x + x_offset + 2, text_y + 4, r, g, b);
            
            // "2"
            x_offset = 4;
            HSVtoRGB(hue + 120, 1.0, 0.9, r, g, b);
            canvas->SetPixel(text_x + x_offset, text_y, r, g, b);
            canvas->SetPixel(text_x + x_offset + 1, text_y, r, g, b);
            canvas->SetPixel(text_x + x_offset + 2, text_y, r, g, b);
            canvas->SetPixel(text_x + x_offset + 2, text_y + 1, r, g, b);
            canvas->SetPixel(text_x + x_offset + 1, text_y + 2, r, g, b);
            canvas->SetPixel(text_x + x_offset, text_y + 3, r, g, b);
            canvas->SetPixel(text_x + x_offset, text_y + 4, r, g, b);
            canvas->SetPixel(text_x + x_offset + 1, text_y + 4, r, g, b);
            canvas->SetPixel(text_x + x_offset + 2, text_y + 4, r, g, b);
            
            // "0"
            x_offset = 8;
            HSVtoRGB(hue + 240, 1.0, 0.9, r, g, b);
            canvas->SetPixel(text_x + x_offset, text_y, r, g, b);
            canvas->SetPixel(text_x + x_offset + 1, text_y, r, g, b);
            canvas->SetPixel(text_x + x_offset + 2, text_y, r, g, b);
            canvas->SetPixel(text_x + x_offset, text_y + 1, r, g, b);
            canvas->SetPixel(text_x + x_offset + 2, text_y + 1, r, g, b);
            canvas->SetPixel(text_x + x_offset, text_y + 2, r, g, b);
            canvas->SetPixel(text_x + x_offset + 2, text_y + 2, r, g, b);
            canvas->SetPixel(text_x + x_offset, text_y + 3, r, g, b);
            canvas->SetPixel(text_x + x_offset + 2, text_y + 3, r, g, b);
            canvas->SetPixel(text_x + x_offset, text_y + 4, r, g, b);
            canvas->SetPixel(text_x + x_offset + 1, text_y + 4, r, g, b);
            canvas->SetPixel(text_x + x_offset + 2, text_y + 4, r, g, b);
        }
    }
    
    void update() {
        canvas->Clear();
        
        drawPsychedelicBackground();
        drawPsychedelicCircles();
        drawRainbowWaves();
        updateSmoke();
        updateLeaves();
        draw420Text();
        
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
    
    FourTwentyScene scene(matrix);
    
    while (!interrupt_received) {
        scene.update();
        usleep(50000); // 50ms = ~20 FPS
    }
    
    matrix->Clear();
    delete matrix;
    
    return 0;
}
