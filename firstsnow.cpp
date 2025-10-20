// First Snowfall Scene
// Compilation: g++ -o firstsnow firstsnow.cpp -lrgbmatrix -std=c++11

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

struct Snowflake {
    float x;
    float y;
    float speed;
    float drift;
    float drift_phase;
    int size;
    int brightness;
    float rotation;
    float rot_speed;
};

struct GroundSnow {
    int x;
    int height;
    float accumulation_rate;
};

struct Tree {
    int x;
    int height;
    int snow_amount;
};

struct Child {
    int x;
    int y;
    bool catching_snow;
    int animation_frame;
};

struct Sparkle {
    int x;
    int y;
    int life;
    int max_life;
    int brightness;
};

class FirstSnowScene {
private:
    RGBMatrix *matrix;
    FrameCanvas *canvas;
    int width, height;
    std::vector<Snowflake> snowflakes;
    std::vector<GroundSnow> ground_snow;
    std::vector<Tree> trees;
    std::vector<Sparkle> sparkles;
    Child child;
    float time_counter;
    int total_snowfall;
    
    // Colors
    Color sky_evening = Color(40, 50, 80);
    Color sky_dark = Color(30, 40, 70);
    Color snow_white = Color(220, 230, 240);
    Color snow_blue = Color(180, 200, 220);
    Color snow_shadow = Color(150, 160, 180);
    Color ground_brown = Color(60, 50, 40);
    Color ground_dark = Color(40, 35, 30);
    Color snow_on_ground = Color(200, 210, 220);
    Color tree_bark = Color(60, 40, 30);
    Color tree_green = Color(20, 60, 30);
    Color tree_dark_green = Color(10, 40, 20);
    Color child_coat_red = Color(140, 20, 20);
    Color child_skin = Color(160, 120, 100);
    
public:
    FirstSnowScene(RGBMatrix *m) : matrix(m), time_counter(0), total_snowfall(0) {
        canvas = matrix->CreateFrameCanvas();
        width = canvas->width();
        height = canvas->height();
        
        srand(time(NULL));
        
        // Create gentle snowfall
        for (int i = 0; i < 30; i++) {
            Snowflake s;
            s.x = rand() % width;
            s.y = -(rand() % height);
            s.speed = 0.15 + (float)rand() / RAND_MAX * 0.25;
            s.drift = ((float)rand() / RAND_MAX - 0.5) * 0.3;
            s.drift_phase = (float)rand() / RAND_MAX * 2 * M_PI;
            s.size = 1 + (rand() % 3 == 0 ? 1 : 0); // Mostly size 1, occasional size 2
            s.brightness = 180 + rand() % 60;
            s.rotation = (float)rand() / RAND_MAX * 2 * M_PI;
            s.rot_speed = ((float)rand() / RAND_MAX - 0.5) * 0.05;
            snowflakes.push_back(s);
        }
        
        // Initialize ground snow accumulation
        for (int x = 0; x < width; x++) {
            GroundSnow gs;
            gs.x = x;
            gs.height = 0;
            gs.accumulation_rate = 0.002 + (float)rand() / RAND_MAX * 0.001;
            ground_snow.push_back(gs);
        }
        
        // Create trees
        if (width >= 32) {
            Tree t1;
            t1.x = 5;
            t1.height = 8 + rand() % 3;
            t1.snow_amount = 0;
            trees.push_back(t1);
            
            Tree t2;
            t2.x = width - 7;
            t2.height = 7 + rand() % 3;
            t2.snow_amount = 0;
            trees.push_back(t2);
        }
        
        // Create child catching snowflakes
        child.x = width / 2;
        child.y = height - 8;
        child.catching_snow = true;
        child.animation_frame = 0;
    }
    
    void drawSky() {
        // Evening sky with slight gradient
        for (int y = 0; y < height * 0.75; y++) {
            float ratio = (float)y / (height * 0.75);
            int r = sky_evening.r - (sky_evening.r - sky_dark.r) * ratio;
            int g = sky_evening.g - (sky_evening.g - sky_dark.g) * ratio;
            int b = sky_evening.b - (sky_evening.b - sky_dark.b) * ratio;
            
            for (int x = 0; x < width; x++) {
                canvas->SetPixel(x, y, r, g, b);
            }
        }
    }
    
    void drawGround() {
        int ground_start = height * 0.75;
        
        // Ground before snow coverage
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
    
    void drawGroundSnow() {
        int ground_level = height - 1;
        
        for (auto& snow : ground_snow) {
            // Draw accumulated snow from ground up
            for (int h = 0; h < snow.height; h++) {
                int y = ground_level - h;
                if (y >= 0 && y < height && snow.x >= 0 && snow.x < width) {
                    // Snow gets slightly darker lower down
                    float depth_ratio = 1.0 - (float)h / (snow.height + 1) * 0.2;
                    int r = snow_on_ground.r * depth_ratio;
                    int g = snow_on_ground.g * depth_ratio;
                    int b = snow_on_ground.b * depth_ratio;
                    
                    // Add sparkle effect on surface
                    if (h == snow.height - 1 && rand() % 100 < 2) {
                        canvas->SetPixel(snow.x, y, 240, 245, 250);
                    } else {
                        canvas->SetPixel(snow.x, y, r, g, b);
                    }
                }
            }
            
            // Slow accumulation
            if (snow.height < 4) {
                snow.height += snow.accumulation_rate;
            }
        }
    }
    
    void drawSnowflake(int cx, int cy, int size, float rotation, int brightness) {
        if (cx < 0 || cx >= width || cy < 0 || cy >= height) return;
        
        int r = snow_white.r * brightness / 255;
        int g = snow_white.g * brightness / 255;
        int b = snow_white.b * brightness / 255;
        
        if (size == 1) {
            // Simple snowflake
            canvas->SetPixel(cx, cy, r, g, b);
        } else {
            // Larger snowflake with pattern
            canvas->SetPixel(cx, cy, r, g, b);
            
            // Arms of snowflake
            for (int arm = 0; arm < 4; arm++) {
                float angle = rotation + arm * M_PI / 2;
                int ax = cx + cos(angle);
                int ay = cy + sin(angle);
                if (ax >= 0 && ax < width && ay >= 0 && ay < height) {
                    canvas->SetPixel(ax, ay, r * 0.7, g * 0.7, b * 0.7);
                }
            }
        }
    }
    
    void updateSnowfall() {
        for (auto& flake : snowflakes) {
            drawSnowflake((int)flake.x, (int)flake.y, flake.size, flake.rotation, flake.brightness);
            
            // Gentle falling with drift
            flake.y += flake.speed;
            flake.x += sin(time_counter * 2 + flake.drift_phase) * flake.drift;
            flake.rotation += flake.rot_speed;
            
            // Wrap around horizontally
            if (flake.x < 0) flake.x = width - 1;
            if (flake.x >= width) flake.x = 0;
            
            // Reset when hitting ground
            if (flake.y > height * 0.75) {
                // Accumulate snow on ground
                int ground_x = (int)flake.x;
                if (ground_x >= 0 && ground_x < ground_snow.size()) {
                    ground_snow[ground_x].height += 0.1;
                    if (ground_snow[ground_x].height > 5) {
                        ground_snow[ground_x].height = 5;
                    }
                }
                
                // Accumulate on trees
                for (auto& tree : trees) {
                    if (abs((int)flake.x - tree.x) < 3 && flake.y < height - tree.height) {
                        tree.snow_amount++;
                    }
                }
                
                // Reset snowflake
                flake.y = -2;
                flake.x = rand() % width;
                total_snowfall++;
                
                // Add sparkle where it lands occasionally
                if (rand() % 5 == 0) {
                    Sparkle sp;
                    sp.x = (int)flake.x;
                    sp.y = height - 2;
                    sp.life = 0;
                    sp.max_life = 15 + rand() % 10;
                    sp.brightness = 200 + rand() % 55;
                    sparkles.push_back(sp);
                }
            }
        }
    }
    
    void drawTree(int base_x, int base_y, int tree_height, int snow_amount) {
        // Simple evergreen tree
        int tree_width = tree_height / 2 + 1;
        
        // Trunk
        for (int y = 0; y < 3; y++) {
            int ty = base_y + y;
            if (ty < height) {
                canvas->SetPixel(base_x, ty, tree_bark.r, tree_bark.g, tree_bark.b);
            }
        }
        
        // Tree triangular shape
        for (int y = 0; y < tree_height; y++) {
            int width_at_height = tree_width - (y * tree_width / tree_height);
            int ty = base_y - y;
            
            for (int x = -width_at_height; x <= width_at_height; x++) {
                int tx = base_x + x;
                if (tx >= 0 && tx < width && ty >= 0 && ty < height) {
                    // Snow on top of tree
                    if (y < 2 && snow_amount > 20) {
                        canvas->SetPixel(tx, ty, snow_on_ground.r, snow_on_ground.g, snow_on_ground.b);
                    } else {
                        // Vary green shades
                        Color tree_color = (x % 2 == 0) ? tree_green : tree_dark_green;
                        canvas->SetPixel(tx, ty, tree_color.r, tree_color.g, tree_color.b);
                    }
                }
            }
        }
        
        // Star on top (optional, for festive feel)
        if (base_y - tree_height >= 0) {
            canvas->SetPixel(base_x, base_y - tree_height, 200, 200, 100);
        }
    }
    
    void drawTrees() {
        for (auto& tree : trees) {
            drawTree(tree.x, height - 4, tree.height, tree.snow_amount);
        }
    }
    
    void drawChild() {
        int cx = child.x;
        int cy = child.y;
        
        child.animation_frame = (int)(time_counter * 5) % 20;
        
        // Head
        if (cy >= 0 && cy < height && cx >= 0 && cx < width) {
            canvas->SetPixel(cx, cy, child_skin.r, child_skin.g, child_skin.b);
            
            // Hat
            if (cy - 1 >= 0) {
                canvas->SetPixel(cx, cy - 1, 100, 20, 20);
            }
        }
        
        // Body with coat
        if (cy + 1 < height) {
            canvas->SetPixel(cx, cy + 1, child_coat_red.r, child_coat_red.g, child_coat_red.b);
        }
        if (cy + 2 < height) {
            canvas->SetPixel(cx, cy + 2, child_coat_red.r, child_coat_red.g, child_coat_red.b);
        }
        
        // Arms reaching up (catching snow)
        if (child.animation_frame < 10) {
            if (cx - 1 >= 0 && cy >= 0) {
                canvas->SetPixel(cx - 1, cy, child_coat_red.r, child_coat_red.g, child_coat_red.b);
            }
            if (cx + 1 < width && cy >= 0) {
                canvas->SetPixel(cx + 1, cy, child_coat_red.r, child_coat_red.g, child_coat_red.b);
            }
        }
        
        // Legs
        if (cy + 3 < height) {
            if (cx - 1 >= 0) {
                canvas->SetPixel(cx - 1, cy + 3, 40, 40, 80);
            }
            if (cx + 1 < width) {
                canvas->SetPixel(cx + 1, cy + 3, 40, 40, 80);
            }
        }
    }
    
    void updateSparkles() {
        for (auto it = sparkles.begin(); it != sparkles.end();) {
            it->life++;
            
            if (it->life < it->max_life) {
                float fade = 1.0 - (float)it->life / it->max_life;
                int b = it->brightness * fade;
                
                canvas->SetPixel(it->x, it->y, b, b, b);
                
                // Twinkle effect
                if (it->life % 3 == 0) {
                    if (it->x > 0) canvas->SetPixel(it->x - 1, it->y, b/2, b/2, b/2);
                    if (it->x < width - 1) canvas->SetPixel(it->x + 1, it->y, b/2, b/2, b/2);
                }
                
                ++it;
            } else {
                it = sparkles.erase(it);
            }
        }
    }
    
    void drawSnowText() {
        // Draw "FIRST SNOW" message if there's room (32x64 or larger)
        if (width >= 48 && total_snowfall > 50) {
            int text_y = 5;
            int text_x = width / 2 - 8;
            
            // Simple "SNOW" text using pixels
            // Just draw a snowflake symbol instead
            if (text_x >= 0 && text_x < width - 3 && text_y < height) {
                drawSnowflake(text_x, text_y, 2, 0, 255);
            }
        }
    }
    
    void update() {
        canvas->Clear();
        
        drawSky();
        drawGround();
        drawGroundSnow();
        drawTrees();
        drawChild();
        updateSnowfall();
        updateSparkles();
        drawSnowText();
        
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
    
    FirstSnowScene scene(matrix);
    
    while (!interrupt_received) {
        scene.update();
        usleep(50000); // 50ms = ~20 FPS
    }
    
    matrix->Clear();
    delete matrix;
    
    return 0;
}
