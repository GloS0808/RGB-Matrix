// St. Patrick's Day Scene
// Compilation: g++ -o stpatricksday stpatricksday.cpp -lrgbmatrix -std=c++11

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

struct Shamrock {
    float x;
    float y;
    float speed;
    float rotation;
    float rot_speed;
    int size;
};

struct Sparkle {
    int x;
    int y;
    int life;
    int max_life;
};

struct Coin {
    float x;
    float y;
    float speed;
    int brightness;
};

class StPatricksScene {
private:
    RGBMatrix *matrix;
    FrameCanvas *canvas;
    int width, height;
    std::vector<Shamrock> shamrocks;
    std::vector<Sparkle> sparkles;
    std::vector<Coin> coins;
    float time_counter;
    int rainbow_phase;
    
    // Colors
    Color green_dark = Color(0, 80, 0);
    Color green_med = Color(0, 120, 0);
    Color green_bright = Color(0, 180, 0);
    Color green_light = Color(100, 220, 100);
    Color gold = Color(180, 140, 0);
    Color gold_bright = Color(220, 180, 0);
    Color gold_dark = Color(120, 90, 0);
    Color orange = Color(200, 100, 0);
    Color white = Color(200, 200, 200);
    Color black = Color(0, 0, 0);
    
public:
    StPatricksScene(RGBMatrix *m) : matrix(m), time_counter(0), rainbow_phase(0) {
        canvas = matrix->CreateFrameCanvas();
        width = canvas->width();
        height = canvas->height();
        
        srand(time(NULL));
        
        // Initialize falling shamrocks
        for (int i = 0; i < 8; i++) {
            Shamrock s;
            s.x = rand() % width;
            s.y = -(rand() % height);
            s.speed = 0.1 + (float)rand() / RAND_MAX * 0.2;
            s.rotation = (float)rand() / RAND_MAX * 2 * M_PI;
            s.rot_speed = ((float)rand() / RAND_MAX - 0.5) * 0.1;
            s.size = 2 + rand() % 2;
            shamrocks.push_back(s);
        }
        
        // Initialize gold coins
        for (int i = 0; i < 5; i++) {
            Coin c;
            c.x = rand() % width;
            c.y = height + (rand() % 20);
            c.speed = 0.05 + (float)rand() / RAND_MAX * 0.1;
            c.brightness = rand() % 50;
            coins.push_back(c);
        }
    }
    
    void drawRainbow() {
        int rainbow_x = width / 2;
        int rainbow_y = height;
        int rainbow_width = width * 0.6;
        
        // Rainbow colors (ROYGBIV)
        Color rainbow_colors[] = {
            Color(120, 0, 0),      // Red
            Color(140, 60, 0),     // Orange
            Color(140, 120, 0),    // Yellow
            Color(0, 100, 0),      // Green
            Color(0, 0, 120),      // Blue
            Color(40, 0, 80),      // Indigo
            Color(80, 0, 80)       // Violet
        };
        
        // Draw rainbow arcs
        for (int arc = 6; arc >= 0; arc--) {
            int radius = rainbow_width / 2 - arc * 2;
            for (int angle = 0; angle <= 180; angle += 2) {
                float rad = angle * M_PI / 180.0;
                int x = rainbow_x + cos(rad + M_PI) * radius;
                int y = rainbow_y + sin(rad + M_PI) * radius;
                
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    Color c = rainbow_colors[arc];
                    // Add shimmer effect
                    float shimmer = sin(time_counter * 3 + angle * 0.1) * 0.2 + 0.8;
                    canvas->SetPixel(x, y, c.r * shimmer, c.g * shimmer, c.b * shimmer);
                }
            }
        }
    }
    
    void drawPotOfGold(int pot_x, int pot_y) {
        // Pot (black/dark)
        for (int y = 0; y < 4; y++) {
            int pot_width = 4 + y;
            for (int x = -pot_width/2; x <= pot_width/2; x++) {
                int px = pot_x + x;
                int py = pot_y + y;
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    if (y == 0) {
                        canvas->SetPixel(px, py, 40, 40, 40);
                    } else {
                        canvas->SetPixel(px, py, 20, 20, 20);
                    }
                }
            }
        }
        
        // Gold coins on top
        for (int i = 0; i < 3; i++) {
            int coin_x = pot_x - 1 + i;
            int coin_y = pot_y - 1;
            if (coin_x >= 0 && coin_x < width && coin_y >= 0 && coin_y < height) {
                float sparkle = sin(time_counter * 4 + i) * 0.3 + 0.7;
                canvas->SetPixel(coin_x, coin_y, gold_bright.r * sparkle, 
                               gold_bright.g * sparkle, gold_bright.b * sparkle);
            }
        }
        
        // Overflow coins
        if (pot_x - 2 >= 0 && pot_y >= 0 && pot_y < height) {
            canvas->SetPixel(pot_x - 2, pot_y, gold.r, gold.g, gold.b);
        }
        if (pot_x + 2 < width && pot_y >= 0 && pot_y < height) {
            canvas->SetPixel(pot_x + 2, pot_y, gold.r, gold.g, gold.b);
        }
    }
    
    void drawShamrock(int cx, int cy, int size, float rotation) {
        // Draw a shamrock (3-leaf clover) with stem
        for (float angle = 0; angle < 2 * M_PI; angle += M_PI * 2 / 3) {
            float leaf_angle = angle + rotation;
            for (int r = 0; r <= size; r++) {
                int x = cx + cos(leaf_angle) * r;
                int y = cy + sin(leaf_angle) * r;
                
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    // Create rounded leaf effect
                    if (r < size - 1) {
                        canvas->SetPixel(x, y, green_bright.r, green_bright.g, green_bright.b);
                    } else {
                        canvas->SetPixel(x, y, green_med.r, green_med.g, green_med.b);
                    }
                }
            }
        }
        
        // Center of shamrock (brighter)
        if (cx >= 0 && cx < width && cy >= 0 && cy < height) {
            canvas->SetPixel(cx, cy, green_light.r, green_light.g, green_light.b);
        }
        
        // Stem
        for (int i = 1; i <= size; i++) {
            int stem_x = cx;
            int stem_y = cy + i;
            if (stem_x >= 0 && stem_x < width && stem_y >= 0 && stem_y < height) {
                canvas->SetPixel(stem_x, stem_y, green_dark.r, green_dark.g, green_dark.b);
            }
        }
    }
    
    void drawFallingShamrocks() {
        for (auto& shamrock : shamrocks) {
            drawShamrock((int)shamrock.x, (int)shamrock.y, shamrock.size, shamrock.rotation);
            
            shamrock.y += shamrock.speed;
            shamrock.rotation += shamrock.rot_speed;
            
            if (shamrock.y > height + 5) {
                shamrock.y = -5;
                shamrock.x = rand() % width;
                shamrock.speed = 0.1 + (float)rand() / RAND_MAX * 0.2;
            }
        }
    }
    
    void drawCoin(int cx, int cy, int brightness) {
        // Simple coin
        Color coin_color = Color(
            gold_bright.r * brightness / 100,
            gold_bright.g * brightness / 100,
            gold_bright.b * brightness / 100
        );
        
        // Coin body
        canvas->SetPixel(cx, cy, coin_color.r, coin_color.g, coin_color.b);
        
        // Coin highlight
        if (cx + 1 < width) {
            canvas->SetPixel(cx + 1, cy, gold_bright.r, gold_bright.g, gold_bright.b);
        }
        
        // Coin shadow
        if (cy + 1 < height) {
            canvas->SetPixel(cx, cy + 1, gold_dark.r, gold_dark.g, gold_dark.b);
        }
    }
    
    void updateCoins() {
        for (auto& coin : coins) {
            drawCoin((int)coin.x, (int)coin.y, coin.brightness);
            
            coin.y -= coin.speed;
            coin.brightness = 80 + sin(time_counter * 5 + coin.x) * 20;
            
            if (coin.y < -5) {
                coin.y = height + 5;
                coin.x = rand() % width;
            }
        }
    }
    
    void addSparkles() {
        // Randomly add sparkles
        if (rand() % 10 < 3) {
            Sparkle s;
            s.x = rand() % width;
            s.y = rand() % height;
            s.life = 0;
            s.max_life = 10 + rand() % 10;
            sparkles.push_back(s);
        }
        
        // Update and draw sparkles
        for (auto it = sparkles.begin(); it != sparkles.end();) {
            it->life++;
            
            if (it->life < it->max_life) {
                float brightness = 1.0 - (float)it->life / it->max_life;
                int b = 200 * brightness;
                
                canvas->SetPixel(it->x, it->y, b, b, b);
                
                // Cross sparkle
                if (it->x > 0) canvas->SetPixel(it->x - 1, it->y, b/2, b/2, b/2);
                if (it->x < width - 1) canvas->SetPixel(it->x + 1, it->y, b/2, b/2, b/2);
                if (it->y > 0) canvas->SetPixel(it->x, it->y - 1, b/2, b/2, b/2);
                if (it->y < height - 1) canvas->SetPixel(it->x, it->y + 1, b/2, b/2, b/2);
                
                ++it;
            } else {
                it = sparkles.erase(it);
            }
        }
    }
    
    void drawText() {
        // Draw "LUCKY" text in green at bottom if there's room
        if (height >= 32) {
            // Simple pixel text - just the word in a festive way
            int text_y = height - 4;
            int text_x = width / 2 - 6;
            
            // Draw shamrock instead of text for small displays
            if (text_x > 2) {
                drawShamrock(text_x, text_y, 2, 0);
            }
        }
    }
    
    void drawBackground() {
        // Dark green gradient background
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int r = 0;
                int g = 30 + (y * 20 / height);
                int b = 0;
                canvas->SetPixel(x, y, r, g, b);
            }
        }
    }
    
    void update() {
        canvas->Clear();
        
        drawBackground();
        drawRainbow();
        
        // Draw pots of gold at rainbow ends
        drawPotOfGold(5, height - 5);
        drawPotOfGold(width - 6, height - 5);
        
        updateCoins();
        drawFallingShamrocks();
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
    
    StPatricksScene scene(matrix);
    
    while (!interrupt_received) {
        scene.update();
        usleep(50000); // 50ms = ~20 FPS
    }
    
    matrix->Clear();
    delete matrix;
    
    return 0;
}
