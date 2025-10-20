// May Flowers Scene
// Compilation: g++ -o mayflowers mayflowers.cpp -lrgbmatrix -std=c++11

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

struct Flower {
    int x;
    int y;
    int type; // 0=tulip, 1=daisy, 2=rose, 3=sunflower
    int color_variant;
    float sway_phase;
    float sway_speed;
};

struct Butterfly {
    float x;
    float y;
    float target_x;
    float target_y;
    float speed;
    int wing_frame;
    int color_type;
};

struct Bee {
    float x;
    float y;
    float angle;
    int frame;
};

struct Petal {
    float x;
    float y;
    float vx;
    float vy;
    int color_r, color_g, color_b;
    float rotation;
};

class MayFlowersScene {
private:
    RGBMatrix *matrix;
    FrameCanvas *canvas;
    int width, height;
    std::vector<Flower> flowers;
    std::vector<Butterfly> butterflies;
    std::vector<Bee> bees;
    std::vector<Petal> petals;
    float time_counter;
    
    // Flower colors
    Color tulip_red = Color(160, 20, 20);
    Color tulip_pink = Color(180, 60, 100);
    Color tulip_yellow = Color(180, 160, 20);
    Color tulip_purple = Color(100, 40, 120);
    
    Color daisy_white = Color(180, 180, 180);
    Color daisy_yellow_center = Color(200, 180, 0);
    
    Color rose_red = Color(140, 0, 0);
    Color rose_pink = Color(160, 80, 100);
    Color rose_dark = Color(80, 0, 0);
    
    Color sunflower_yellow = Color(200, 180, 0);
    Color sunflower_center = Color(80, 50, 0);
    
    // Scene colors
    Color stem_green = Color(20, 100, 20);
    Color leaf_green = Color(40, 120, 40);
    Color grass_dark = Color(20, 80, 20);
    Color grass_light = Color(40, 100, 30);
    Color sky_blue = Color(100, 150, 200);
    Color sky_light = Color(120, 170, 220);
    
    Color butterfly_orange = Color(180, 100, 20);
    Color butterfly_blue = Color(60, 100, 160);
    Color butterfly_yellow = Color(180, 160, 40);
    
    Color bee_yellow = Color(180, 160, 0);
    Color bee_black = Color(20, 20, 20);
    
public:
    MayFlowersScene(RGBMatrix *m) : matrix(m), time_counter(0) {
        canvas = matrix->CreateFrameCanvas();
        width = canvas->width();
        height = canvas->height();
        
        srand(time(NULL));
        
        // Plant flowers in garden
        int num_flowers = width / 4; // Spacing for flowers
        for (int i = 0; i < num_flowers; i++) {
            Flower f;
            f.x = 4 + (i * width / num_flowers) + (rand() % 3);
            f.y = height - 8 - (rand() % 4);
            f.type = rand() % 4;
            f.color_variant = rand() % 4;
            f.sway_phase = (float)rand() / RAND_MAX * 2 * M_PI;
            f.sway_speed = 0.5 + (float)rand() / RAND_MAX * 0.5;
            flowers.push_back(f);
        }
        
        // Add butterflies
        for (int i = 0; i < 2; i++) {
            Butterfly b;
            b.x = rand() % width;
            b.y = 5 + rand() % 10;
            b.target_x = rand() % width;
            b.target_y = 5 + rand() % 10;
            b.speed = 0.2 + (float)rand() / RAND_MAX * 0.2;
            b.wing_frame = 0;
            b.color_type = rand() % 3;
            butterflies.push_back(b);
        }
        
        // Add bees
        for (int i = 0; i < 2; i++) {
            Bee bee;
            bee.x = rand() % width;
            bee.y = 8 + rand() % 8;
            bee.angle = (float)rand() / RAND_MAX * 2 * M_PI;
            bee.frame = 0;
            bees.push_back(bee);
        }
    }
    
    void drawSky() {
        // Sky gradient
        for (int y = 0; y < height * 0.65; y++) {
            float ratio = (float)y / (height * 0.65);
            int r = sky_blue.r + (sky_light.r - sky_blue.r) * ratio;
            int g = sky_blue.g + (sky_light.g - sky_blue.g) * ratio;
            int b = sky_blue.b + (sky_light.b - sky_blue.b) * ratio;
            
            for (int x = 0; x < width; x++) {
                canvas->SetPixel(x, y, r, g, b);
            }
        }
    }
    
    void drawGrass() {
        int grass_start = height * 0.65;
        
        // Grass with texture
        for (int y = grass_start; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // Create grass texture
                if ((x + y) % 3 == 0 || (rand() % 10 < 2)) {
                    canvas->SetPixel(x, y, grass_light.r, grass_light.g, grass_light.b);
                } else {
                    canvas->SetPixel(x, y, grass_dark.r, grass_dark.g, grass_dark.b);
                }
            }
        }
        
        // Add some grass blades
        for (int x = 0; x < width; x += 3) {
            int blade_y = grass_start + (rand() % 3);
            if (blade_y < height) {
                canvas->SetPixel(x, blade_y, leaf_green.r, leaf_green.g, leaf_green.b);
            }
        }
    }
    
    void drawTulip(int x, int y, int color_variant, float sway) {
        int sway_offset = sin(sway) * 2;
        
        // Get color
        Color petal_color;
        switch(color_variant) {
            case 0: petal_color = tulip_red; break;
            case 1: petal_color = tulip_pink; break;
            case 2: petal_color = tulip_yellow; break;
            default: petal_color = tulip_purple; break;
        }
        
        // Stem
        for (int i = 0; i < 6; i++) {
            int stem_x = x + (sway_offset * i / 6);
            if (stem_x >= 0 && stem_x < width && y + i < height) {
                canvas->SetPixel(stem_x, y + i, stem_green.r, stem_green.g, stem_green.b);
            }
        }
        
        // Leaf
        int leaf_y = y + 3;
        if (x - 1 >= 0 && leaf_y < height) {
            canvas->SetPixel(x - 1, leaf_y, leaf_green.r, leaf_green.g, leaf_green.b);
        }
        
        // Tulip petals (cup shape)
        int bloom_x = x + sway_offset;
        if (bloom_x >= 1 && bloom_x < width - 1 && y - 1 >= 0) {
            canvas->SetPixel(bloom_x, y - 1, petal_color.r, petal_color.g, petal_color.b);
            canvas->SetPixel(bloom_x - 1, y, petal_color.r, petal_color.g, petal_color.b);
            canvas->SetPixel(bloom_x, y, petal_color.r * 0.8, petal_color.g * 0.8, petal_color.b * 0.8);
            canvas->SetPixel(bloom_x + 1, y, petal_color.r, petal_color.g, petal_color.b);
        }
    }
    
    void drawDaisy(int x, int y, float sway) {
        int sway_offset = sin(sway) * 2;
        
        // Stem
        for (int i = 0; i < 6; i++) {
            int stem_x = x + (sway_offset * i / 6);
            if (stem_x >= 0 && stem_x < width && y + i < height) {
                canvas->SetPixel(stem_x, y + i, stem_green.r, stem_green.g, stem_green.b);
            }
        }
        
        // Leaf
        int leaf_y = y + 3;
        if (x + 1 < width && leaf_y < height) {
            canvas->SetPixel(x + 1, leaf_y, leaf_green.r, leaf_green.g, leaf_green.b);
        }
        
        // Daisy petals (star pattern)
        int bloom_x = x + sway_offset;
        if (bloom_x >= 1 && bloom_x < width - 1 && y - 1 >= 0) {
            // Center
            canvas->SetPixel(bloom_x, y, daisy_yellow_center.r, daisy_yellow_center.g, daisy_yellow_center.b);
            
            // White petals around
            canvas->SetPixel(bloom_x, y - 1, daisy_white.r, daisy_white.g, daisy_white.b);
            canvas->SetPixel(bloom_x - 1, y, daisy_white.r, daisy_white.g, daisy_white.b);
            canvas->SetPixel(bloom_x + 1, y, daisy_white.r, daisy_white.g, daisy_white.b);
            canvas->SetPixel(bloom_x, y + 1, daisy_white.r, daisy_white.g, daisy_white.b);
        }
    }
    
    void drawRose(int x, int y, int color_variant, float sway) {
        int sway_offset = sin(sway) * 2;
        
        Color petal_color = (color_variant % 2 == 0) ? rose_red : rose_pink;
        
        // Stem with thorns
        for (int i = 0; i < 7; i++) {
            int stem_x = x + (sway_offset * i / 7);
            if (stem_x >= 0 && stem_x < width && y + i < height) {
                canvas->SetPixel(stem_x, y + i, stem_green.r, stem_green.g, stem_green.b);
            }
        }
        
        // Leaves
        int leaf_y = y + 2;
        if (x - 1 >= 0 && leaf_y < height) {
            canvas->SetPixel(x - 1, leaf_y, leaf_green.r, leaf_green.g, leaf_green.b);
        }
        if (x + 1 < width && leaf_y + 2 < height) {
            canvas->SetPixel(x + 1, leaf_y + 2, leaf_green.r, leaf_green.g, leaf_green.b);
        }
        
        // Rose bloom (layered)
        int bloom_x = x + sway_offset;
        if (bloom_x >= 1 && bloom_x < width - 1 && y - 1 >= 0) {
            canvas->SetPixel(bloom_x, y - 1, petal_color.r, petal_color.g, petal_color.b);
            canvas->SetPixel(bloom_x - 1, y, petal_color.r, petal_color.g, petal_color.b);
            canvas->SetPixel(bloom_x, y, rose_dark.r, rose_dark.g, rose_dark.b); // Center darker
            canvas->SetPixel(bloom_x + 1, y, petal_color.r, petal_color.g, petal_color.b);
            canvas->SetPixel(bloom_x, y + 1, petal_color.r * 0.7, petal_color.g * 0.7, petal_color.b * 0.7);
        }
    }
    
    void drawSunflower(int x, int y, float sway) {
        int sway_offset = sin(sway) * 2;
        
        // Thick stem
        for (int i = 0; i < 7; i++) {
            int stem_x = x + (sway_offset * i / 7);
            if (stem_x >= 0 && stem_x < width && y + i < height) {
                canvas->SetPixel(stem_x, y + i, stem_green.r, stem_green.g, stem_green.b);
            }
        }
        
        // Large leaves
        int leaf_y = y + 3;
        if (x - 1 >= 0 && leaf_y < height) {
            canvas->SetPixel(x - 1, leaf_y, leaf_green.r, leaf_green.g, leaf_green.b);
            canvas->SetPixel(x - 1, leaf_y + 1, leaf_green.r, leaf_green.g, leaf_green.b);
        }
        
        // Sunflower head (larger)
        int bloom_x = x + sway_offset;
        if (bloom_x >= 2 && bloom_x < width - 2 && y - 2 >= 0) {
            // Center (dark)
            canvas->SetPixel(bloom_x, y, sunflower_center.r, sunflower_center.g, sunflower_center.b);
            canvas->SetPixel(bloom_x, y - 1, sunflower_center.r, sunflower_center.g, sunflower_center.b);
            
            // Yellow petals
            canvas->SetPixel(bloom_x - 1, y - 1, sunflower_yellow.r, sunflower_yellow.g, sunflower_yellow.b);
            canvas->SetPixel(bloom_x + 1, y - 1, sunflower_yellow.r, sunflower_yellow.g, sunflower_yellow.b);
            canvas->SetPixel(bloom_x - 1, y, sunflower_yellow.r, sunflower_yellow.g, sunflower_yellow.b);
            canvas->SetPixel(bloom_x + 1, y, sunflower_yellow.r, sunflower_yellow.g, sunflower_yellow.b);
            canvas->SetPixel(bloom_x, y - 2, sunflower_yellow.r, sunflower_yellow.g, sunflower_yellow.b);
            canvas->SetPixel(bloom_x, y + 1, sunflower_yellow.r, sunflower_yellow.g, sunflower_yellow.b);
        }
    }
    
    void drawFlowers() {
        for (auto& flower : flowers) {
            float sway = time_counter * flower.sway_speed + flower.sway_phase;
            
            switch(flower.type) {
                case 0:
                    drawTulip(flower.x, flower.y, flower.color_variant, sway);
                    break;
                case 1:
                    drawDaisy(flower.x, flower.y, sway);
                    break;
                case 2:
                    drawRose(flower.x, flower.y, flower.color_variant, sway);
                    break;
                case 3:
                    drawSunflower(flower.x, flower.y, sway);
                    break;
            }
        }
    }
    
    void drawButterfly(int x, int y, int frame, int color_type) {
        Color wing_color;
        switch(color_type) {
            case 0: wing_color = butterfly_orange; break;
            case 1: wing_color = butterfly_blue; break;
            default: wing_color = butterfly_yellow; break;
        }
        
        if (x >= 1 && x < width - 1 && y >= 0 && y < height) {
            // Body
            canvas->SetPixel(x, y, 40, 40, 40);
            
            // Wings (flap animation)
            int wing_offset = (frame % 2 == 0) ? 0 : 1;
            canvas->SetPixel(x - 1, y - wing_offset, wing_color.r, wing_color.g, wing_color.b);
            canvas->SetPixel(x + 1, y - wing_offset, wing_color.r, wing_color.g, wing_color.b);
            
            // Wing tips (darker)
            if (wing_offset == 0) {
                canvas->SetPixel(x - 1, y + 1, wing_color.r * 0.6, wing_color.g * 0.6, wing_color.b * 0.6);
                canvas->SetPixel(x + 1, y + 1, wing_color.r * 0.6, wing_color.g * 0.6, wing_color.b * 0.6);
            }
        }
    }
    
    void updateButterflies() {
        for (auto& butterfly : butterflies) {
            drawButterfly((int)butterfly.x, (int)butterfly.y, butterfly.wing_frame, butterfly.color_type);
            
            // Move towards target
            float dx = butterfly.target_x - butterfly.x;
            float dy = butterfly.target_y - butterfly.y;
            float dist = sqrt(dx*dx + dy*dy);
            
            if (dist > 1) {
                butterfly.x += (dx / dist) * butterfly.speed;
                butterfly.y += (dy / dist) * butterfly.speed;
            } else {
                // Pick new target
                butterfly.target_x = rand() % width;
                butterfly.target_y = 5 + rand() % (int)(height * 0.4);
            }
            
            if ((int)(time_counter * 10) % 3 == 0) {
                butterfly.wing_frame++;
            }
        }
    }
    
    void drawBee(int x, int y, int frame) {
        if (x >= 1 && x < width - 1 && y >= 0 && y < height) {
            // Bee body (yellow and black stripes)
            canvas->SetPixel(x, y, bee_yellow.r, bee_yellow.g, bee_yellow.b);
            canvas->SetPixel(x + 1, y, bee_black.r, bee_black.g, bee_black.b);
            
            // Wings
            int wing_y = y - (frame % 2);
            if (wing_y >= 0) {
                canvas->SetPixel(x, wing_y, 140, 140, 140);
            }
        }
    }
    
    void updateBees() {
        for (auto& bee : bees) {
            drawBee((int)bee.x, (int)bee.y, bee.frame);
            
            // Fly in figure-8 pattern
            bee.angle += 0.05;
            bee.x += cos(bee.angle) * 0.3;
            bee.y += sin(bee.angle * 2) * 0.15;
            
            // Keep in bounds
            if (bee.x < 0) bee.x = width - 1;
            if (bee.x >= width) bee.x = 0;
            if (bee.y < 5) bee.y = 5;
            if (bee.y > height * 0.6) bee.y = height * 0.6;
            
            bee.frame++;
        }
    }
    
    void updatePetals() {
        // Occasionally drop a petal
        if (rand() % 30 == 0 && flowers.size() > 0) {
            int flower_idx = rand() % flowers.size();
            Petal p;
            p.x = flowers[flower_idx].x;
            p.y = flowers[flower_idx].y - 2;
            p.vx = ((float)rand() / RAND_MAX - 0.5) * 0.3;
            p.vy = 0.1 + (float)rand() / RAND_MAX * 0.1;
            p.rotation = 0;
            
            // Random petal color
            if (rand() % 2 == 0) {
                p.color_r = tulip_pink.r;
                p.color_g = tulip_pink.g;
                p.color_b = tulip_pink.b;
            } else {
                p.color_r = daisy_white.r;
                p.color_g = daisy_white.g;
                p.color_b = daisy_white.b;
            }
            
            petals.push_back(p);
        }
        
        // Update and draw petals
        for (auto it = petals.begin(); it != petals.end();) {
            canvas->SetPixel((int)it->x, (int)it->y, it->color_r, it->color_g, it->color_b);
            
            it->x += it->vx;
            it->y += it->vy;
            it->rotation += 0.1;
            
            if (it->y > height) {
                it = petals.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    void update() {
        canvas->Clear();
        
        drawSky();
        drawGrass();
        drawFlowers();
        updateButterflies();
        updateBees();
        updatePetals();
        
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
    
    MayFlowersScene scene(matrix);
    
    while (!interrupt_received) {
        scene.update();
        usleep(50000); // 50ms = ~20 FPS
    }
    
    matrix->Clear();
    delete matrix;
    
    return 0;
}
