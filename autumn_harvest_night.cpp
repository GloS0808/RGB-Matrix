// Autumn Harvest Night Scene (Animated)
// Compilation: g++ -o autumn_harvest_night autumn_harvest_night.cpp -I ~/rgbMatrix/rpi-rgb-led-matrix/include -L ~/rgbMatrix/rpi-rgb-led-matrix/lib -lrgbmatrix -lpthread -lm

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

struct Firefly {
    int x;
    int y;
    bool active;
};

class AutumnHarvestNight {
private:
    RGBMatrix *matrix;
    FrameCanvas *canvas;
    int width, height;
    int frame_count;

    // Colors
    Color sky_night = Color(10, 20, 40);       // Deep night sky
    Color corn_gold = Color(200, 180, 50);     // Golden corn
    Color ground_brown = Color(101, 67, 33);   // Brown ground
    Color firefly_yellow = Color(255, 255, 0); // Bright yellow firefly
    Color moon_white = Color(255, 240, 200);   // Soft white moon
    Color text_orange = Color(255, 100, 0);    // Orange for "Harvest 2025" text
    std::vector<Firefly> fireflies;

public:
    AutumnHarvestNight(RGBMatrix *m) : matrix(m), frame_count(0) {
        canvas = matrix->CreateFrameCanvas();
        width = canvas->width();
        height = canvas->height();

        // Initialize fireflies
        srand(time(NULL));
        for (int i = 0; i < 10; i++) {
            fireflies.push_back({rand() % (width - 2) + 1, rand() % (height / 2) + 1, true});
        }
    }

    void drawSky() {
        for (int y = 0; y < height * 0.6; y++) {
            int r = sky_night.r + (sky_night.r * y) / (height * 0.6) / 3;
            int g = sky_night.g + (sky_night.g * y) / (height * 0.6) / 2;
            int b = sky_night.b + (sky_night.b * y) / (height * 0.6);
            for (int x = 0; x < width; x++) {
                canvas->SetPixel(x, y, r, g, b);
            }
        }
    }

    void drawCornfield() {
        // Animated swaying corn (simple wave effect)
        for (int x = 0; x < width; x++) {
            for (int y = height * 0.6; y < height * 0.9; y++) {
                int offset = static_cast<int>(5 * sin((x + frame_count * 0.1) * 0.3));
                if (y - offset >= height * 0.6 && y - offset < height * 0.9) {
                    canvas->SetPixel(x, y - offset, corn_gold.r, corn_gold.g, corn_gold.b);
                }
            }
        }
    }

    void drawGround() {
        for (int y = height * 0.9; y < height; y++) {
            for (int x = 0; x < width; x++) {
                canvas->SetPixel(x, y, ground_brown.r, ground_brown.g, ground_brown.b);
            }
        }
    }

    void drawMoon() {
        // Animated moon phase (simple waxing/waning)
        int center_x = width - 6;
        int center_y = 6;
        int radius = 4;
        int phase = (frame_count / 10) % 8; // 8-phase cycle
        for (int x = center_x - radius; x <= center_x + radius; x++) {
            for (int y = center_y - radius; y <= center_y + radius; y++) {
                if ((x - center_x) * (x - center_x) + (y - center_y) * (y - center_y) <= radius * radius) {
                    if (x >= 0 && x < width && y >= 0 && y < height) {
                        if (phase < 4) { // Waxing
                            if (x >= center_x - (phase * radius / 4)) {
                                canvas->SetPixel(x, y, moon_white.r, moon_white.g, moon_white.b);
                            }
                        } else { // Waning
                            if (x <= center_x + ((phase - 4) * radius / 4)) {
                                canvas->SetPixel(x, y, moon_white.r, moon_white.g, moon_white.b);
                            }
                        }
                    }
                }
            }
        }
    }

    void drawFireflies() {
        // Animated fireflies with random blinking
        for (auto& fly : fireflies) {
            if (fly.active && rand() % 10 < 8) { // 80% chance to show
                canvas->SetPixel(fly.x, fly.y, firefly_yellow.r, firefly_yellow.g, firefly_yellow.b);
            }
            // Small random movement
            if (frame_count % 20 == 0) {
                fly.x += (rand() % 3) - 1;
                fly.y += (rand() % 3) - 1;
                if (fly.x < 1 || fly.x >= width - 1) fly.x = rand() % (width - 2) + 1;
                if (fly.y < 1 || fly.y >= height / 2) fly.y = rand() % (height / 2) + 1;
            }
        }
    }

    void drawText() {
        // Static "Harvest 2025" in orange
        int text_x = 2;
        int text_y = height - 4;

        // H
        canvas->SetPixel(text_x, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 1, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x, text_y + 1, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 1, text_y + 1, text_orange.r, text_orange.g, text_orange.b);

        // a
        canvas->SetPixel(text_x + 3, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 4, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 3, text_y + 1, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 4, text_y + 1, text_orange.r, text_orange.g, text_orange.b);

        // r
        canvas->SetPixel(text_x + 6, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 7, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 6, text_y + 1, text_orange.r, text_orange.g, text_orange.b);

        // v
        canvas->SetPixel(text_x + 9, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 10, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 9, text_y + 1, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 10, text_y + 1, text_orange.r, text_orange.g, text_orange.b);

        // e
        canvas->SetPixel(text_x + 12, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 13, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 12, text_y + 1, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 13, text_y + 1, text_orange.r, text_orange.g, text_orange.b);

        // s
        canvas->SetPixel(text_x + 15, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 16, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 15, text_y + 1, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 16, text_y + 1, text_orange.r, text_orange.g, text_orange.b);

        // (space)

        // 2
        canvas->SetPixel(text_x + 18, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 19, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 20, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 18, text_y + 1, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 20, text_y + 1, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 18, text_y + 2, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 19, text_y + 2, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 20, text_y + 2, text_orange.r, text_orange.g, text_orange.b);

        // 0
        canvas->SetPixel(text_x + 22, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 23, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 24, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 22, text_y + 1, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 24, text_y + 1, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 22, text_y + 2, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 23, text_y + 2, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 24, text_y + 2, text_orange.r, text_orange.g, text_orange.b);

        // 2
        canvas->SetPixel(text_x + 26, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 27, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 28, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 26, text_y + 1, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 28, text_y + 1, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 26, text_y + 2, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 27, text_y + 2, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 28, text_y + 2, text_orange.r, text_orange.g, text_orange.b);

        // 5
        canvas->SetPixel(text_x + 30, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 31, text_y, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 30, text_y + 1, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 31, text_y + 1, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 30, text_y + 2, text_orange.r, text_orange.g, text_orange.b);
        canvas->SetPixel(text_x + 31, text_y + 2, text_orange.r, text_orange.g, text_orange.b);
    }

    void draw() {
        canvas->Clear();
        
        drawSky();
        drawCornfield();
        drawGround();
        drawMoon();
        drawFireflies();
        drawText();
        
        frame_count++;
        canvas = matrix->SwapOnVSync(canvas);
    }
};

int main(int argc, char *argv[]) {
    RGBMatrix::Options matrix_options;
    rgb_matrix::RuntimeOptions runtime_opt;
    
    matrix_options.rows = 32;
    matrix_options.cols = 32;
    matrix_options.chain_length = 1;
    matrix_options.parallel = 1;
    matrix_options.hardware_mapping = "adafruit-hat";
    
    if (!ParseOptionsFromFlags(&argc, &argv, &matrix_options, &runtime_opt)) {
        return 1;
    }
    
    RGBMatrix *matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
    if (matrix == NULL) {
        return 1;
    }
    
    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);
    
    AutumnHarvestNight scene(matrix);
    
    while (!interrupt_received) {
        scene.draw();
        usleep(100000); // ~10 fps for smooth animation
    }
    
    matrix->Clear();
    delete matrix;
    
    return 0;
}
