// Spinning Progress Wheel - Old School Mac Style
// Compilation: g++ -o spinningwheel spinningwheel.cpp -lrgbmatrix -std=c++11

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

struct Segment {
    float angle;
    int hue;
    int brightness;
};

class SpinningWheelScene {
private:
    RGBMatrix *matrix;
    FrameCanvas *canvas;
    int width, height;
    std::vector<Segment> segments;
    float rotation;
    float rotation_speed;
    int num_segments;
    int radius;
    
public:
    SpinningWheelScene(RGBMatrix *m) : matrix(m), rotation(0), rotation_speed(0.15) {
        canvas = matrix->CreateFrameCanvas();
        width = canvas->width();
        height = canvas->height();
        
        srand(time(NULL));
        
        // Classic spinning beach ball has segments
        num_segments = 12;
        radius = std::min(width, height) / 2 - 2;
        
        // Create segments with rainbow colors
        for (int i = 0; i < num_segments; i++) {
            Segment s;
            s.angle = (float)i / num_segments * 2 * M_PI;
            s.hue = (i * 360 / num_segments) % 360;
            s.brightness = 255;
            segments.push_back(s);
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
    
    void drawSpinningWheel() {
        int center_x = width / 2;
        int center_y = height / 2;
        
        // Draw each segment as a wedge
        for (int seg = 0; seg < num_segments; seg++) {
            float start_angle = segments[seg].angle + rotation;
            float end_angle = start_angle + (2 * M_PI / num_segments);
            
            // Fade effect - segments get dimmer as they trail
            float fade = 1.0 - (float)seg / num_segments;
            fade = fade * fade; // Squared for more dramatic fade
            
            int r, g, b;
            HSVtoRGB(segments[seg].hue, 1.0, fade, r, g, b);
            
            // Draw the wedge by filling from center outward
            for (float angle = start_angle; angle < end_angle; angle += 0.05) {
                for (int rad = 2; rad <= radius; rad++) {
                    int px = center_x + cos(angle) * rad;
                    int py = center_y + sin(angle) * rad;
                    
                    if (px >= 0 && px < width && py >= 0 && py < height) {
                        // Add radial gradient
                        float radial_fade = (float)rad / radius;
                        canvas->SetPixel(px, py, 
                                       r * radial_fade, 
                                       g * radial_fade, 
                                       b * radial_fade);
                    }
                }
            }
        }
        
        // Draw center circle (darker)
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int px = center_x + dx;
                int py = center_y + dy;
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    canvas->SetPixel(px, py, 40, 40, 50);
                }
            }
        }
    }
    
    void drawClassicSpinner() {
        int center_x = width / 2;
        int center_y = height / 2;
        
        // Draw 8 spokes with fading trail
        for (int spoke = 0; spoke < 8; spoke++) {
            float angle = rotation + spoke * M_PI / 4;
            
            // Fade based on position (trailing effect)
            float fade = 1.0 - (float)spoke / 8.0;
            fade = fade * fade; // Squared for more dramatic fade
            
            int brightness = 200 * fade + 55;
            
            // Draw spoke from center outward
            for (int len = 3; len <= radius; len++) {
                int px = center_x + cos(angle) * len;
                int py = center_y + sin(angle) * len;
                
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    // Fade along length too
                    float length_fade = 1.0 - (float)(len - 3) / (radius - 3) * 0.5;
                    int b = brightness * length_fade;
                    canvas->SetPixel(px, py, b, b, b);
                }
            }
            
            // Thicker at the end (blob)
            int blob_x = center_x + cos(angle) * radius;
            int blob_y = center_y + sin(angle) * radius;
            
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int px = blob_x + dx;
                    int py = blob_y + dy;
                    if (px >= 0 && px < width && py >= 0 && py < height) {
                        int b = brightness;
                        canvas->SetPixel(px, py, b, b, b);
                    }
                }
            }
        }
    }
    
    void drawRainbowWheel() {
        int center_x = width / 2;
        int center_y = height / 2;
        
        // Classic rainbow beach ball style
        for (int seg = 0; seg < num_segments; seg++) {
            float start_angle = rotation + seg * 2 * M_PI / num_segments;
            float end_angle = start_angle + 2 * M_PI / num_segments;
            
            int hue = (seg * 360 / num_segments) % 360;
            
            // Draw filled wedge
            for (float angle = start_angle; angle < end_angle; angle += 0.03) {
                for (int rad = 0; rad <= radius; rad++) {
                    int px = center_x + cos(angle) * rad;
                    int py = center_y + sin(angle) * rad;
                    
                    if (px >= 0 && px < width && py >= 0 && py < height) {
                        int r, g, b;
                        // Brightness based on radius for 3D effect
                        float brightness = 0.7 + (1.0 - (float)rad / radius) * 0.3;
                        HSVtoRGB(hue, 0.9, brightness, r, g, b);
                        canvas->SetPixel(px, py, r, g, b);
                    }
                }
            }
        }
        
        // White outline
        for (float angle = 0; angle < 2 * M_PI; angle += 0.1) {
            int px = center_x + cos(angle) * radius;
            int py = center_y + sin(angle) * radius;
            if (px >= 0 && px < width && py >= 0 && py < height) {
                canvas->SetPixel(px, py, 200, 200, 200);
            }
        }
        
        // Segment dividers (white lines from center)
        for (int seg = 0; seg < num_segments; seg++) {
            float angle = rotation + seg * 2 * M_PI / num_segments;
            for (int rad = 0; rad <= radius; rad++) {
                int px = center_x + cos(angle) * rad;
                int py = center_y + sin(angle) * rad;
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    canvas->SetPixel(px, py, 220, 220, 220);
                }
            }
        }
    }
    
    void drawModernSpinner() {
        int center_x = width / 2;
        int center_y = height / 2;
        
        // Modern iOS-style spinner with arc
        int num_dots = 12;
        
        for (int i = 0; i < num_dots; i++) {
            float angle = rotation + i * 2 * M_PI / num_dots;
            
            // Fade trail
            float fade = 1.0 - (float)i / num_dots;
            fade = fade * fade * fade; // Cubic for smoother fade
            
            // Blue-ish modern color
            int r = 100 * fade;
            int g = 150 * fade;
            int b = 250 * fade;
            
            // Size varies
            int dot_size = (i == 0) ? 2 : 1;
            
            int dot_x = center_x + cos(angle) * (radius - 1);
            int dot_y = center_y + sin(angle) * (radius - 1);
            
            // Draw dot
            for (int dy = -dot_size/2; dy <= dot_size/2; dy++) {
                for (int dx = -dot_size/2; dx <= dot_size/2; dx++) {
                    int px = dot_x + dx;
                    int py = dot_y + dy;
                    if (px >= 0 && px < width && py >= 0 && py < height) {
                        canvas->SetPixel(px, py, r, g, b);
                    }
                }
            }
        }
    }
    
    void update() {
        canvas->Clear();
        
        // Clear to dark background
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                canvas->SetPixel(x, y, 20, 20, 25);
            }
        }
        
        // Choose spinner style (change this to try different styles)
        // Style 1: Rainbow beach ball (classic Mac)
        drawRainbowWheel();
        
        // Style 2: Monochrome spinner (uncomment to use)
        // drawClassicSpinner();
        
        // Style 3: Modern dots (uncomment to use)
        // drawModernSpinner();
        
        // Rotate
        rotation += rotation_speed;
        if (rotation > 2 * M_PI) {
            rotation -= 2 * M_PI;
        }
        
        canvas = matrix->SwapOnVSync(canvas);
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
    
    SpinningWheelScene scene(matrix);
    
    while (!interrupt_received) {
        scene.update();
        usleep(30000); // ~33 FPS for smooth spinning
    }
    
    matrix->Clear();
    delete matrix;
    
    return 0;
}
