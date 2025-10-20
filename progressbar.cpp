// Progress Bar Scene
// Compilation: g++ -o progressbar progressbar.cpp -lrgbmatrix -std=c++11

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

struct ProgressBar {
    int x;
    int y;
    int width;
    int height;
    float progress; // 0.0 to 1.0
    int style; // 0=classic, 1=gradient, 2=animated, 3=rainbow
    float animation_phase;
};

struct Particle {
    float x;
    float y;
    float vx;
    float vy;
    int life;
    int max_life;
    int r, g, b;
};

class ProgressBarScene {
private:
    RGBMatrix *matrix;
    FrameCanvas *canvas;
    int width, height;
    std::vector<ProgressBar> progress_bars;
    std::vector<Particle> particles;
    float time_counter;
    
    // Colors
    Color bg_dark = Color(20, 20, 30);
    Color border_gray = Color(100, 100, 120);
    Color bar_green = Color(50, 200, 50);
    Color bar_blue = Color(50, 150, 250);
    Color bar_red = Color(250, 80, 80);
    Color bar_yellow = Color(250, 220, 50);
    Color bar_purple = Color(180, 80, 250);
    Color empty_gray = Color(40, 40, 50);
    
public:
    ProgressBarScene(RGBMatrix *m) : matrix(m), time_counter(0) {
        canvas = matrix->CreateFrameCanvas();
        width = canvas->width();
        height = canvas->height();
        
        srand(time(NULL));
        
        // Create multiple progress bars with different styles
        int bar_width = width - 8;
        int bar_height = 4;
        int spacing = 7;
        
        for (int i = 0; i < 4; i++) {
            ProgressBar pb;
            pb.x = 4;
            pb.y = 4 + i * spacing;
            pb.width = bar_width;
            pb.height = bar_height;
            pb.progress = 0.0;
            pb.style = i;
            pb.animation_phase = 0;
            progress_bars.push_back(pb);
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
    
    void drawBackground() {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                canvas->SetPixel(x, y, bg_dark.r, bg_dark.g, bg_dark.b);
            }
        }
    }
    
    void drawClassicProgressBar(ProgressBar& pb) {
        // Border
        for (int x = pb.x; x < pb.x + pb.width; x++) {
            if (x >= 0 && x < width) {
                // Top border
                if (pb.y >= 0 && pb.y < height) {
                    canvas->SetPixel(x, pb.y, border_gray.r, border_gray.g, border_gray.b);
                }
                // Bottom border
                if (pb.y + pb.height - 1 < height) {
                    canvas->SetPixel(x, pb.y + pb.height - 1, border_gray.r, border_gray.g, border_gray.b);
                }
            }
        }
        
        for (int y = pb.y; y < pb.y + pb.height; y++) {
            if (y >= 0 && y < height) {
                // Left border
                if (pb.x >= 0 && pb.x < width) {
                    canvas->SetPixel(pb.x, y, border_gray.r, border_gray.g, border_gray.b);
                }
                // Right border
                if (pb.x + pb.width - 1 < width) {
                    canvas->SetPixel(pb.x + pb.width - 1, y, border_gray.r, border_gray.g, border_gray.b);
                }
            }
        }
        
        // Interior
        int fill_width = (pb.width - 2) * pb.progress;
        
        for (int y = pb.y + 1; y < pb.y + pb.height - 1; y++) {
            for (int x = pb.x + 1; x < pb.x + pb.width - 1; x++) {
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    if (x < pb.x + 1 + fill_width) {
                        // Filled portion - green
                        canvas->SetPixel(x, y, bar_green.r, bar_green.g, bar_green.b);
                    } else {
                        // Empty portion - dark gray
                        canvas->SetPixel(x, y, empty_gray.r, empty_gray.g, empty_gray.b);
                    }
                }
            }
        }
    }
    
    void drawGradientProgressBar(ProgressBar& pb) {
        // Border
        for (int x = pb.x; x < pb.x + pb.width; x++) {
            if (x >= 0 && x < width) {
                if (pb.y >= 0 && pb.y < height) {
                    canvas->SetPixel(x, pb.y, border_gray.r, border_gray.g, border_gray.b);
                }
                if (pb.y + pb.height - 1 < height) {
                    canvas->SetPixel(x, pb.y + pb.height - 1, border_gray.r, border_gray.g, border_gray.b);
                }
            }
        }
        
        for (int y = pb.y; y < pb.y + pb.height; y++) {
            if (y >= 0 && y < height) {
                if (pb.x >= 0 && pb.x < width) {
                    canvas->SetPixel(pb.x, y, border_gray.r, border_gray.g, border_gray.b);
                }
                if (pb.x + pb.width - 1 < width) {
                    canvas->SetPixel(pb.x + pb.width - 1, y, border_gray.r, border_gray.g, border_gray.b);
                }
            }
        }
        
        // Interior with gradient
        int fill_width = (pb.width - 2) * pb.progress;
        
        for (int y = pb.y + 1; y < pb.y + pb.height - 1; y++) {
            for (int x = pb.x + 1; x < pb.x + pb.width - 1; x++) {
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    if (x < pb.x + 1 + fill_width) {
                        // Gradient from blue to green
                        float ratio = (float)(x - pb.x - 1) / (pb.width - 2);
                        int r = bar_blue.r + (bar_green.r - bar_blue.r) * ratio;
                        int g = bar_blue.g + (bar_green.g - bar_blue.g) * ratio;
                        int b = bar_blue.b + (bar_green.b - bar_blue.b) * ratio;
                        canvas->SetPixel(x, y, r, g, b);
                    } else {
                        canvas->SetPixel(x, y, empty_gray.r, empty_gray.g, empty_gray.b);
                    }
                }
            }
        }
    }
    
    void drawAnimatedProgressBar(ProgressBar& pb) {
        pb.animation_phase += 0.2;
        
        // Border
        for (int x = pb.x; x < pb.x + pb.width; x++) {
            if (x >= 0 && x < width) {
                if (pb.y >= 0 && pb.y < height) {
                    canvas->SetPixel(x, pb.y, border_gray.r, border_gray.g, border_gray.b);
                }
                if (pb.y + pb.height - 1 < height) {
                    canvas->SetPixel(x, pb.y + pb.height - 1, border_gray.r, border_gray.g, border_gray.b);
                }
            }
        }
        
        for (int y = pb.y; y < pb.y + pb.height; y++) {
            if (y >= 0 && y < height) {
                if (pb.x >= 0 && pb.x < width) {
                    canvas->SetPixel(pb.x, y, border_gray.r, border_gray.g, border_gray.b);
                }
                if (pb.x + pb.width - 1 < width) {
                    canvas->SetPixel(pb.x + pb.width - 1, y, border_gray.r, border_gray.g, border_gray.b);
                }
            }
        }
        
        // Interior with animated stripes
        int fill_width = (pb.width - 2) * pb.progress;
        
        for (int y = pb.y + 1; y < pb.y + pb.height - 1; y++) {
            for (int x = pb.x + 1; x < pb.x + pb.width - 1; x++) {
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    if (x < pb.x + 1 + fill_width) {
                        // Animated diagonal stripes
                        int stripe = (x + y + (int)pb.animation_phase) % 4;
                        if (stripe < 2) {
                            canvas->SetPixel(x, y, bar_yellow.r, bar_yellow.g, bar_yellow.b);
                        } else {
                            canvas->SetPixel(x, y, bar_yellow.r * 0.7, bar_yellow.g * 0.7, bar_yellow.b * 0.7);
                        }
                    } else {
                        canvas->SetPixel(x, y, empty_gray.r, empty_gray.g, empty_gray.b);
                    }
                }
            }
        }
    }
    
    void drawRainbowProgressBar(ProgressBar& pb) {
        // Border
        for (int x = pb.x; x < pb.x + pb.width; x++) {
            if (x >= 0 && x < width) {
                if (pb.y >= 0 && pb.y < height) {
                    canvas->SetPixel(x, pb.y, border_gray.r, border_gray.g, border_gray.b);
                }
                if (pb.y + pb.height - 1 < height) {
                    canvas->SetPixel(x, pb.y + pb.height - 1, border_gray.r, border_gray.g, border_gray.b);
                }
            }
        }
        
        for (int y = pb.y; y < pb.y + pb.height; y++) {
            if (y >= 0 && y < height) {
                if (pb.x >= 0 && pb.x < width) {
                    canvas->SetPixel(pb.x, y, border_gray.r, border_gray.g, border_gray.b);
                }
                if (pb.x + pb.width - 1 < width) {
                    canvas->SetPixel(pb.x + pb.width - 1, y, border_gray.r, border_gray.g, border_gray.b);
                }
            }
        }
        
        // Interior with rainbow
        int fill_width = (pb.width - 2) * pb.progress;
        
        for (int y = pb.y + 1; y < pb.y + pb.height - 1; y++) {
            for (int x = pb.x + 1; x < pb.x + pb.width - 1; x++) {
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    if (x < pb.x + 1 + fill_width) {
                        // Rainbow colors
                        float hue = ((float)(x - pb.x - 1) / (pb.width - 2)) * 360 + time_counter * 50;
                        hue = fmod(hue, 360);
                        int r, g, b;
                        HSVtoRGB(hue, 1.0, 0.9, r, g, b);
                        canvas->SetPixel(x, y, r, g, b);
                    } else {
                        canvas->SetPixel(x, y, empty_gray.r, empty_gray.g, empty_gray.b);
                    }
                }
            }
        }
    }
    
    void drawProgressBar(ProgressBar& pb) {
        switch(pb.style) {
            case 0:
                drawClassicProgressBar(pb);
                break;
            case 1:
                drawGradientProgressBar(pb);
                break;
            case 2:
                drawAnimatedProgressBar(pb);
                break;
            case 3:
                drawRainbowProgressBar(pb);
                break;
        }
        
        // Emit particles when complete
        if (pb.progress >= 0.99 && rand() % 5 < 2) {
            Particle p;
            p.x = pb.x + pb.width - 2;
            p.y = pb.y + pb.height / 2;
            p.vx = 0.3 + (float)rand() / RAND_MAX * 0.3;
            p.vy = ((float)rand() / RAND_MAX - 0.5) * 0.4;
            p.life = 0;
            p.max_life = 20 + rand() % 20;
            
            // Color based on bar style
            switch(pb.style) {
                case 0:
                    p.r = bar_green.r; p.g = bar_green.g; p.b = bar_green.b;
                    break;
                case 1:
                    p.r = bar_blue.r; p.g = bar_blue.g; p.b = bar_blue.b;
                    break;
                case 2:
                    p.r = bar_yellow.r; p.g = bar_yellow.g; p.b = bar_yellow.b;
                    break;
                case 3:
                    HSVtoRGB(rand() % 360, 1.0, 0.9, p.r, p.g, p.b);
                    break;
            }
            
            particles.push_back(p);
        }
    }
    
    void updateParticles() {
        for (auto it = particles.begin(); it != particles.end();) {
            it->life++;
            
            if (it->life < it->max_life) {
                float fade = 1.0 - (float)it->life / it->max_life;
                
                int px = (int)it->x;
                int py = (int)it->y;
                
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    canvas->SetPixel(px, py, it->r * fade, it->g * fade, it->b * fade);
                }
                
                it->x += it->vx;
                it->y += it->vy;
                it->vy += 0.02; // Gravity
                
                ++it;
            } else {
                it = particles.erase(it);
            }
        }
    }
    
    void drawPercentageText(ProgressBar& pb) {
        // Simple percentage display
        int percent = (int)(pb.progress * 100);
        int text_x = pb.x + pb.width + 2;
        int text_y = pb.y + 1;
        
        // Draw percentage as simple dots representing tens/ones
        // Just show completion status with simple indicator
        if (percent >= 100 && text_x < width && text_y < height) {
            // Checkmark or completion indicator
            canvas->SetPixel(text_x, text_y, 50, 250, 50);
            if (text_x + 1 < width) {
                canvas->SetPixel(text_x + 1, text_y, 50, 250, 50);
            }
        }
    }
    
    void update() {
        canvas->Clear();
        
        drawBackground();
        
        // Update progress bars
        for (auto& pb : progress_bars) {
            // Different speeds for each bar
            float speed = 0.003 + (pb.style * 0.002);
            pb.progress += speed;
            
            // Reset when complete
            if (pb.progress > 1.0) {
                pb.progress = 0.0;
            }
            
            drawProgressBar(pb);
            drawPercentageText(pb);
        }
        
        updateParticles();
        
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
    
    ProgressBarScene scene(matrix);
    
    while (!interrupt_received) {
        scene.update();
        usleep(50000); // 50ms = ~20 FPS
    }
    
    matrix->Clear();
    delete matrix;
    
    return 0;
}
