// Summer Beach Scene
// Compilation: g++ -o summer summer.cpp -lrgbmatrix -std=c++11

#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <signal.h>

using namespace rgb_matrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
    interrupt_received = true;
}

struct Wave {
    float x;
    float y;
    float phase;
    float amplitude;
};

struct Cloud {
    float x;
    float y;
    float speed;
    int size;
};

struct Seagull {
    float x;
    float y;
    float speed;
    int frame;
};

class SummerScene {
private:
    RGBMatrix *matrix;
    FrameCanvas *canvas;
    int width, height;
    std::vector<Wave> waves;
    std::vector<Cloud> clouds;
    std::vector<Seagull> seagulls;
    float time_counter;
    
    // Colors - darker and more contrasted
    Color sky_blue = Color(70, 130, 180);
    Color sky_light = Color(100, 150, 200);
    Color sun_yellow = Color(200, 180, 0);
    Color sun_orange = Color(180, 100, 0);
    Color sand_light = Color(150, 120, 80);
    Color sand_dark = Color(120, 90, 60);
    Color ocean_dark = Color(0, 60, 100);
    Color ocean_mid = Color(0, 80, 120);
    Color ocean_light = Color(30, 100, 140);
    Color wave_white = Color(150, 180, 200);
    Color wave_foam = Color(200, 220, 230);
    Color cloud_white = Color(180, 190, 200);
    Color cloud_shadow = Color(140, 150, 160);
    Color palm_trunk = Color(80, 50, 25);
    Color palm_trunk_dark = Color(50, 30, 15);
    Color palm_green = Color(20, 80, 20);
    Color palm_green_dark = Color(10, 50, 10);
    Color seagull_white = Color(150, 150, 150);
    Color seagull_gray = Color(100, 100, 100);
    
public:
    SummerScene(RGBMatrix *m) : matrix(m), time_counter(0) {
        canvas = matrix->CreateFrameCanvas();
        width = canvas->width();
        height = canvas->height();
        
        srand(time(NULL));
        
        // Initialize waves
        for (int i = 0; i < 5; i++) {
            Wave w;
            w.x = rand() % width;
            w.y = height * 0.6 + (rand() % 10);
            w.phase = (float)rand() / RAND_MAX * 2 * M_PI;
            w.amplitude = 2 + rand() % 3;
            waves.push_back(w);
        }
        
        // Initialize clouds
        for (int i = 0; i < 3; i++) {
            Cloud c;
            c.x = rand() % width;
            c.y = 2 + rand() % 8;
            c.speed = 0.1 + (float)rand() / RAND_MAX * 0.2;
            c.size = 3 + rand() % 3;
            clouds.push_back(c);
        }
        
        // Initialize seagulls
        for (int i = 0; i < 2; i++) {
            Seagull s;
            s.x = rand() % width;
            s.y = 8 + rand() % 10;
            s.speed = 0.3 + (float)rand() / RAND_MAX * 0.3;
            s.frame = rand() % 4;
            seagulls.push_back(s);
        }
    }
    
    void drawSky() {
        // Gradient sky
        for (int y = 0; y < height * 0.7; y++) {
            int r = 135 + (y * 30 / height);
            int g = 206 - (y * 40 / height);
            int b = 235 - (y * 20 / height);
            Color sky_color(r, g, b);
            for (int x = 0; x < width; x++) {
                canvas->SetPixel(x, y, sky_color.r, sky_color.g, sky_color.b);
            }
        }
    }
    
    void drawSun() {
        int sun_x = width - 8;
        int sun_y = 5;
        int sun_radius = 4;
        
        // Draw sun with glow
        for (int y = -sun_radius - 1; y <= sun_radius + 1; y++) {
            for (int x = -sun_radius - 1; x <= sun_radius + 1; x++) {
                float dist = sqrt(x*x + y*y);
                int px = sun_x + x;
                int py = sun_y + y;
                
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    if (dist <= sun_radius) {
                        // Core sun
                        canvas->SetPixel(px, py, sun_yellow.r, sun_yellow.g, sun_yellow.b);
                    } else if (dist <= sun_radius + 1) {
                        // Glow
                        canvas->SetPixel(px, py, sun_orange.r, sun_orange.g, sun_orange.b);
                    }
                }
            }
        }
        
        // Sun rays
        float ray_angle = time_counter * 0.5;
        for (int i = 0; i < 8; i++) {
            float angle = (i * M_PI / 4) + ray_angle;
            int ray_len = 3;
            for (int r = sun_radius + 1; r < sun_radius + ray_len; r++) {
                int rx = sun_x + cos(angle) * r;
                int ry = sun_y + sin(angle) * r;
                if (rx >= 0 && rx < width && ry >= 0 && ry < height) {
                    canvas->SetPixel(rx, ry, sun_orange.r, sun_orange.g, sun_orange.b);
                }
            }
        }
    }
    
    void drawClouds() {
        for (auto& cloud : clouds) {
            // Simple cloud shape
            for (int i = 0; i < cloud.size; i++) {
                int cx = (int)cloud.x + i;
                int cy = (int)cloud.y;
                if (cx >= 0 && cx < width && cy >= 0 && cy < height) {
                    canvas->SetPixel(cx, cy, cloud_white.r, cloud_white.g, cloud_white.b);
                    if (cy + 1 < height && i > 0 && i < cloud.size - 1) {
                        canvas->SetPixel(cx, cy + 1, cloud_white.r, cloud_white.g, cloud_white.b);
                    }
                }
            }
            
            // Move cloud
            cloud.x += cloud.speed;
            if (cloud.x > width) {
                cloud.x = -cloud.size;
                cloud.y = 2 + rand() % 8;
            }
        }
    }
    
    void drawOcean() {
        int ocean_start = height * 0.6;
        int beach_start = height * 0.75;
        
        // Ocean
        for (int y = ocean_start; y < beach_start; y++) {
            for (int x = 0; x < width; x++) {
                // Wave pattern
                float wave_val = sin((x * 0.3) + (time_counter * 2)) * 0.5 + 0.5;
                int r = ocean_dark.r + (ocean_light.r - ocean_dark.r) * wave_val;
                int g = ocean_dark.g + (ocean_light.g - ocean_dark.g) * wave_val;
                int b = ocean_dark.b + (ocean_light.b - ocean_dark.b) * wave_val;
                canvas->SetPixel(x, y, r, g, b);
            }
        }
        
        // Animated waves
        for (auto& wave : waves) {
            int wx = (int)wave.x;
            int wy = (int)wave.y + sin(time_counter * 2 + wave.phase) * wave.amplitude;
            
            if (wx >= 0 && wx < width && wy >= ocean_start && wy < beach_start) {
                canvas->SetPixel(wx, wy, wave_white.r, wave_white.g, wave_white.b);
                if (wx + 1 < width) {
                    canvas->SetPixel(wx + 1, wy, wave_white.r, wave_white.g, wave_white.b);
                }
            }
            
            wave.x += 0.5;
            if (wave.x > width) {
                wave.x = 0;
                wave.y = ocean_start + (rand() % (beach_start - ocean_start));
            }
        }
    }
    
    void drawBeach() {
        int beach_start = height * 0.75;
        
        // Sand with texture
        for (int y = beach_start; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // Add some variation to sand
                if (rand() % 10 > 7) {
                    canvas->SetPixel(x, y, sand_dark.r, sand_dark.g, sand_dark.b);
                } else {
                    canvas->SetPixel(x, y, sand_light.r, sand_light.g, sand_light.b);
                }
            }
        }
    }
    
    void drawPalmTree(int base_x, int base_y) {
        // Trunk
        for (int y = 0; y < 8; y++) {
            canvas->SetPixel(base_x, base_y - y, palm_trunk.r, palm_trunk.g, palm_trunk.b);
            if (y > 2 && y < 6) {
                if (y % 2 == 0) {
                    canvas->SetPixel(base_x + 1, base_y - y, palm_trunk.r, palm_trunk.g, palm_trunk.b);
                } else {
                    canvas->SetPixel(base_x - 1, base_y - y, palm_trunk.r, palm_trunk.g, palm_trunk.b);
                }
            }
        }
        
        // Palm leaves
        int leaf_top = base_y - 8;
        // Left leaves
        for (int i = 0; i < 4; i++) {
            canvas->SetPixel(base_x - i - 1, leaf_top + i/2, palm_green.r, palm_green.g, palm_green.b);
        }
        // Right leaves
        for (int i = 0; i < 4; i++) {
            canvas->SetPixel(base_x + i + 1, leaf_top + i/2, palm_green.r, palm_green.g, palm_green.b);
        }
        // Top leaves
        for (int i = 0; i < 3; i++) {
            canvas->SetPixel(base_x, leaf_top - i, palm_green.r, palm_green.g, palm_green.b);
        }
        // Diagonal leaves
        canvas->SetPixel(base_x - 1, leaf_top - 1, palm_green.r, palm_green.g, palm_green.b);
        canvas->SetPixel(base_x + 1, leaf_top - 1, palm_green.r, palm_green.g, palm_green.b);
    }
    
    void drawSeagulls() {
        for (auto& seagull : seagulls) {
            int sx = (int)seagull.x;
            int sy = (int)seagull.y;
            
            // Simple seagull - V shape with animation
            if (sx >= 1 && sx < width - 1 && sy >= 0 && sy < height) {
                Color seagull_color(255, 255, 255);
                
                // Animated wing flap
                int wing_offset = (seagull.frame % 2 == 0) ? 0 : 1;
                canvas->SetPixel(sx - 1, sy + wing_offset, seagull_color.r, seagull_color.g, seagull_color.b);
                canvas->SetPixel(sx, sy, seagull_color.r, seagull_color.g, seagull_color.b);
                canvas->SetPixel(sx + 1, sy + wing_offset, seagull_color.r, seagull_color.g, seagull_color.b);
            }
            
            seagull.x += seagull.speed;
            if ((int)(time_counter * 10) % 3 == 0) {
                seagull.frame++;
            }
            
            if (seagull.x > width + 2) {
                seagull.x = -2;
                seagull.y = 8 + rand() % 10;
            }
        }
    }
    
    void update() {
        canvas->Clear();
        
        drawSky();
        drawSun();
        drawClouds();
        drawOcean();
        drawBeach();
        
        // Palm trees on beach
        if (width >= 32) {
            drawPalmTree(5, height - 2);
            drawPalmTree(width - 6, height - 2);
        } else {
            drawPalmTree(width / 2, height - 2);
        }
        
        drawSeagulls();
        
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
    
    SummerScene scene(matrix);
    
    while (!interrupt_received) {
        scene.update();
        usleep(50000); // 50ms = ~20 FPS
    }
    
    matrix->Clear();
    delete matrix;
    
    return 0;
}
