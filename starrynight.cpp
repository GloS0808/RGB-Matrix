// Starry Night - Van Gogh Inspired Scene
// Compilation: g++ -o starrynight starrynight.cpp -lrgbmatrix -std=c++11

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

struct Star {
    int x;
    int y;
    float pulse_phase;
    float pulse_speed;
    int brightness;
    int size;
};

struct SkySwirl {
    int center_x;
    int center_y;
    float rotation;
    float rotation_speed;
    int radius;
};

struct Moon {
    int x;
    int y;
    float glow_phase;
};

class StarryNightScene {
private:
    RGBMatrix *matrix;
    FrameCanvas *canvas;
    int width, height;
    std::vector<Star> stars;
    std::vector<SkySwirl> swirls;
    Moon moon;
    float time_counter;
    
    // Van Gogh color palette
    Color sky_deep_blue = Color(20, 40, 100);
    Color sky_blue = Color(40, 60, 140);
    Color sky_light = Color(60, 80, 160);
    Color swirl_blue = Color(80, 100, 180);
    Color swirl_light = Color(100, 120, 200);
    
    Color star_yellow = Color(220, 200, 80);
    Color star_white = Color(240, 230, 180);
    Color star_orange = Color(200, 160, 60);
    
    Color moon_yellow = Color(220, 200, 100);
    Color moon_bright = Color(240, 220, 140);
    Color moon_glow = Color(180, 160, 80);
    
    Color cypress_dark = Color(20, 40, 20);
    Color cypress_green = Color(30, 60, 30);
    
    Color village_dark = Color(30, 30, 50);
    Color village_blue = Color(40, 50, 80);
    Color village_light = Color(60, 70, 100);
    Color window_yellow = Color(200, 180, 60);
    
    Color hill_dark = Color(40, 60, 50);
    Color hill_green = Color(50, 80, 60);
    
public:
    StarryNightScene(RGBMatrix *m) : matrix(m), time_counter(0) {
        canvas = matrix->CreateFrameCanvas();
        width = canvas->width();
        height = canvas->height();
        
        srand(time(NULL));
        
        // Create prominent stars
        for (int i = 0; i < 8; i++) {
            Star s;
            s.x = 3 + rand() % (width - 6);
            s.y = 2 + rand() % (int)(height * 0.4);
            s.pulse_phase = (float)rand() / RAND_MAX * 2 * M_PI;
            s.pulse_speed = 0.5 + (float)rand() / RAND_MAX * 0.5;
            s.brightness = 180 + rand() % 60;
            s.size = 2 + (rand() % 3 == 0 ? 1 : 0); // Mostly size 2, some size 3
            stars.push_back(s);
        }
        
        // Create sky swirls
        for (int i = 0; i < 3; i++) {
            SkySwirl sw;
            sw.center_x = 5 + i * (width / 3);
            sw.center_y = 5 + rand() % 8;
            sw.rotation = (float)rand() / RAND_MAX * 2 * M_PI;
            sw.rotation_speed = 0.02 + (float)rand() / RAND_MAX * 0.02;
            sw.radius = 4 + rand() % 3;
            swirls.push_back(sw);
        }
        
        // Create crescent moon
        moon.x = width - 8;
        moon.y = 6;
        moon.glow_phase = 0;
    }
    
    void drawSky() {
        // Van Gogh style gradient sky
        for (int y = 0; y < height * 0.65; y++) {
            float ratio = (float)y / (height * 0.65);
            
            // Blend from deep blue to lighter blue
            int r = sky_deep_blue.r + (sky_light.r - sky_deep_blue.r) * ratio;
            int g = sky_deep_blue.g + (sky_light.g - sky_deep_blue.g) * ratio;
            int b = sky_deep_blue.b + (sky_light.b - sky_deep_blue.b) * ratio;
            
            // Add brush stroke texture
            int texture = sin(y * 0.5 + time_counter) * 8;
            r = std::min(255, std::max(0, r + texture));
            g = std::min(255, std::max(0, g + texture));
            b = std::min(255, std::max(0, b + texture));
            
            for (int x = 0; x < width; x++) {
                canvas->SetPixel(x, y, r, g, b);
            }
        }
    }
    
    void drawSkySwirls() {
        for (auto& swirl : swirls) {
            swirl.rotation += swirl.rotation_speed;
            
            // Draw Van Gogh-style spiral brushstrokes
            for (float angle = 0; angle < 4 * M_PI; angle += 0.3) {
                float spiral_radius = (angle / (4 * M_PI)) * swirl.radius;
                int sx = swirl.center_x + cos(angle + swirl.rotation) * spiral_radius;
                int sy = swirl.center_y + sin(angle + swirl.rotation) * spiral_radius;
                
                if (sx >= 0 && sx < width && sy >= 0 && sy < height) {
                    // Vary colors along the swirl
                    float color_phase = angle / (4 * M_PI);
                    int r = swirl_blue.r + (swirl_light.r - swirl_blue.r) * color_phase;
                    int g = swirl_blue.g + (swirl_light.g - swirl_blue.g) * color_phase;
                    int b = swirl_blue.b + (swirl_light.b - swirl_blue.b) * color_phase;
                    
                    // Add brightness variation
                    float brightness = sin(angle * 2 + time_counter) * 0.2 + 0.9;
                    r *= brightness;
                    g *= brightness;
                    b *= brightness;
                    
                    canvas->SetPixel(sx, sy, r, g, b);
                    
                    // Thicker brushstrokes
                    if (sx + 1 < width) {
                        canvas->SetPixel(sx + 1, sy, r * 0.8, g * 0.8, b * 0.8);
                    }
                }
            }
        }
    }
    
    void drawStar(int cx, int cy, int size, float pulse, int brightness) {
        // Van Gogh style radiating star
        float pulse_brightness = sin(pulse) * 0.3 + 0.7;
        int r = star_yellow.r * pulse_brightness * brightness / 255;
        int g = star_yellow.g * pulse_brightness * brightness / 255;
        int b = star_yellow.b * pulse_brightness * brightness / 255;
        
        // Center
        if (cx >= 0 && cx < width && cy >= 0 && cy < height) {
            canvas->SetPixel(cx, cy, star_white.r, star_white.g, star_white.b);
        }
        
        // Radiating points (Van Gogh style)
        for (int ray = 0; ray < 8; ray++) {
            float angle = ray * M_PI / 4;
            for (int len = 1; len <= size; len++) {
                int sx = cx + cos(angle) * len;
                int sy = cy + sin(angle) * len;
                
                if (sx >= 0 && sx < width && sy >= 0 && sy < height) {
                    float fade = 1.0 - (float)len / (size + 1);
                    canvas->SetPixel(sx, sy, r * fade, g * fade, b * fade);
                }
            }
        }
        
        // Add halo/glow
        if (size >= 3) {
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int hx = cx + dx;
                    int hy = cy + dy;
                    if (hx >= 0 && hx < width && hy >= 0 && hy < height && (dx != 0 || dy != 0)) {
                        canvas->SetPixel(hx, hy, star_orange.r * 0.5, star_orange.g * 0.5, star_orange.b * 0.5);
                    }
                }
            }
        }
    }
    
    void drawStars() {
        for (auto& star : stars) {
            star.pulse_phase += star.pulse_speed * 0.05;
            drawStar(star.x, star.y, star.size, star.pulse_phase, star.brightness);
        }
    }
    
    void drawMoon() {
        moon.glow_phase += 0.03;
        float glow = sin(moon.glow_phase) * 0.2 + 0.8;
        
        int mx = moon.x;
        int my = moon.y;
        
        // Moon glow (outer)
        for (int dy = -4; dy <= 4; dy++) {
            for (int dx = -4; dx <= 4; dx++) {
                float dist = sqrt(dx*dx + dy*dy);
                if (dist > 2 && dist < 4.5) {
                    int px = mx + dx;
                    int py = my + dy;
                    if (px >= 0 && px < width && py >= 0 && py < height) {
                        float glow_intensity = (1.0 - (dist - 2) / 2.5) * glow;
                        canvas->SetPixel(px, py, 
                                       moon_glow.r * glow_intensity,
                                       moon_glow.g * glow_intensity,
                                       moon_glow.b * glow_intensity);
                    }
                }
            }
        }
        
        // Moon body (crescent shape)
        for (int dy = -3; dy <= 3; dy++) {
            for (int dx = -3; dx <= 3; dx++) {
                float dist = sqrt(dx*dx + dy*dy);
                if (dist <= 3) {
                    // Create crescent by cutting out a circle
                    float dist2 = sqrt((dx+1.5)*(dx+1.5) + dy*dy);
                    if (dist2 > 2.5) {
                        int px = mx + dx;
                        int py = my + dy;
                        if (px >= 0 && px < width && py >= 0 && py < height) {
                            // Vary brightness across moon
                            float brightness = 1.0 - dist / 4.0;
                            canvas->SetPixel(px, py, 
                                           moon_bright.r * brightness * glow,
                                           moon_bright.g * brightness * glow,
                                           moon_bright.b * brightness * glow);
                        }
                    }
                }
            }
        }
    }
    
    void drawCypress() {
        // Iconic tall dark cypress tree (left side)
        int tree_x = 3;
        int tree_base_y = height - 1;
        int tree_height = height * 0.55;
        
        // Tree trunk/body - flame-like shape
        for (int y = 0; y < tree_height; y++) {
            float width_ratio = sin((float)y / tree_height * M_PI);
            int tree_width = 2 + width_ratio * 2;
            
            for (int x = -tree_width; x <= tree_width; x++) {
                int px = tree_x + x + sin(y * 0.3 + time_counter * 0.5) * 0.5;
                int py = tree_base_y - y;
                
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    // Dark cypress color with slight variation
                    if (abs(x) == tree_width) {
                        canvas->SetPixel(px, py, cypress_dark.r, cypress_dark.g, cypress_dark.b);
                    } else {
                        canvas->SetPixel(px, py, cypress_green.r, cypress_green.g, cypress_green.b);
                    }
                }
            }
        }
    }
    
    void drawVillage() {
        int village_y = height * 0.65;
        int ground_y = height - 1;
        
        // Rolling hills
        for (int x = 0; x < width; x++) {
            int hill_height = 2 + sin(x * 0.3) * 1.5;
            for (int y = 0; y < hill_height; y++) {
                int py = village_y + y;
                if (py < height) {
                    Color hill_color = (y % 2 == 0) ? hill_dark : hill_green;
                    canvas->SetPixel(x, py, hill_color.r, hill_color.g, hill_color.b);
                }
            }
        }
        
        // Village buildings
        drawBuilding(width / 2 - 6, village_y - 6, 4, 6, true);  // Church with steeple
        drawBuilding(width / 2 + 2, village_y - 4, 3, 4, false); // House
        drawBuilding(width / 2 + 7, village_y - 3, 3, 3, false); // Small house
        drawBuilding(width / 2 - 12, village_y - 5, 4, 5, false); // House on left
    }
    
    void drawBuilding(int x, int y, int width_b, int height_b, bool is_church) {
        // Building body
        for (int by = 0; by < height_b; by++) {
            for (int bx = 0; bx < width_b; bx++) {
                int px = x + bx;
                int py = y + by;
                
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    canvas->SetPixel(px, py, village_blue.r, village_blue.g, village_blue.b);
                }
            }
        }
        
        // Roof
        for (int rx = -1; rx <= width_b; rx++) {
            int px = x + rx;
            int py = y - 1;
            if (px >= 0 && px < width && py >= 0) {
                canvas->SetPixel(px, py, village_dark.r, village_dark.g, village_dark.b);
            }
        }
        
        // Windows (lit)
        if (height_b >= 3) {
            int window_x = x + width_b / 2;
            int window_y = y + height_b - 2;
            if (window_x >= 0 && window_x < width && window_y < height) {
                float flicker = sin(time_counter * 3 + x) * 0.2 + 0.8;
                canvas->SetPixel(window_x, window_y, 
                               window_yellow.r * flicker,
                               window_yellow.g * flicker,
                               window_yellow.b * flicker);
            }
        }
        
        // Church steeple
        if (is_church) {
            for (int s = 0; s < 3; s++) {
                int sx = x + width_b / 2;
                int sy = y - 2 - s;
                if (sx >= 0 && sx < width && sy >= 0) {
                    canvas->SetPixel(sx, sy, village_dark.r, village_dark.g, village_dark.b);
                }
            }
        }
    }
    
    void update() {
        canvas->Clear();
        
        drawSky();
        drawSkySwirls();
        drawMoon();
        drawStars();
        drawVillage();
        drawCypress();
        
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
    
    StarryNightScene scene(matrix);
    
    while (!interrupt_received) {
        scene.update();
        usleep(50000); // 50ms = ~20 FPS
    }
    
    matrix->Clear();
    delete matrix;
    
    return 0;
}
