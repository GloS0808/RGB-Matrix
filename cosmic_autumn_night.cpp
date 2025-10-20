// Cosmic Autumn Night Scene (Static)
// Compilation: g++ -o cosmic_autumn_night cosmic_autumn_night.cpp -I ~/rgbMatrix/rpi-rgb-led-matrix/include -L ~/rgbMatrix/rpi-rgb-led-matrix/lib -lrgbmatrix -lpthread -lm

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

class CosmicAutumnNight {
private:
    RGBMatrix *matrix;
    FrameCanvas *canvas;
    int width, height;

    // Colors
    Color sky_night = Color(10, 20, 40);          // Deep night sky
    Color aurora_green = Color(0, 150, 50);       // Green aurora
    Color aurora_purple = Color(100, 0, 150);     // Purple aurora
    Color star_white = Color(255, 255, 255);      // White stars
    Color balloon_orange = Color(255, 100, 0);    // Orange balloon
    Color basket_brown = Color(101, 67, 33);      // Brown pumpkin basket
    Color pumpkin_orange = Color(255, 80, 0);     // Bright pumpkin orange
    Color text_silver = Color(192, 192, 192);     // Silver for "xAI Night" text

public:
    CosmicAutumnNight(RGBMatrix *m) : matrix(m) {
        canvas = matrix->CreateFrameCanvas();
        width = canvas->width();
        height = canvas->height();
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

    void drawAurora() {
        // Static aurora waves
        for (int y = 0; y < height * 0.4; y++) {
            for (int x = 0; x < width; x++) {
                int intensity = static_cast<int>(50 * sin(x * 0.2 + y * 0.1));
                if (y < height * 0.2) {
                    int r = aurora_purple.r + intensity;
                    int g = aurora_purple.g + intensity / 2;
                    int b = aurora_purple.b + intensity;
                    if (r > 0 && g > 0 && b > 0) canvas->SetPixel(x, y, r, g, b);
                } else {
                    int r = aurora_green.r + intensity / 2;
                    int g = aurora_green.g + intensity;
                    int b = aurora_green.b + intensity / 2;
                    if (r > 0 && g > 0 && b > 0) canvas->SetPixel(x, y, r, g, b);
                }
            }
        }
    }

    void drawStars() {
        // Static stars
        canvas->SetPixel(5, 3, star_white.r, star_white.g, star_white.b);
        canvas->SetPixel(12, 6, star_white.r, star_white.g, star_white.b);
        canvas->SetPixel(18, 4, star_white.r, star_white.g, star_white.b);
        canvas->SetPixel(25, 8, star_white.r, star_white.g, star_white.b);
        canvas->SetPixel(8, 10, star_white.r, star_white.g, star_white.b);
    }

    void drawHotAirBalloon() {
        // Balloon body (orange ellipse)
        int center_x = width / 2; // 16
        int center_y = height * 0.5; // 16
        int radius_x = 6;
        int radius_y = 4;
        for (int x = center_x - radius_x; x <= center_x + radius_x; x++) {
            for (int y = center_y - radius_y; y <= center_y + radius_y; y++) {
                if ((pow((x - center_x) / static_cast<float>(radius_x), 2) + pow((y - center_y) / static_cast<float>(radius_y), 2)) <= 1) {
                    if (x >= 0 && x < width && y >= 0 && y < height) {
                        canvas->SetPixel(x, y, balloon_orange.r, balloon_orange.g, balloon_orange.b);
                    }
                }
            }
        }

        // Pumpkin basket (brown with orange accents)
        int basket_y = center_y + radius_y + 2;
        for (int x = center_x - 3; x <= center_x + 3; x++) {
            for (int y = basket_y; y < basket_y + 3; y++) {
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    if (x == center_x - 3 || x == center_x + 3 || y == basket_y + 2) {
                        canvas->SetPixel(x, y, pumpkin_orange.r, pumpkin_orange.g, pumpkin_orange.b);
                    } else {
                        canvas->SetPixel(x, y, basket_brown.r, basket_brown.g, basket_brown.b);
                    }
                }
            }
        }

        // Basket lines
        canvas->SetPixel(center_x - 2, basket_y, basket_brown.r, basket_brown.g, basket_brown.b);
        canvas->SetPixel(center_x + 2, basket_y, basket_brown.r, basket_brown.g, basket_brown.b);
    }

    void drawText() {
        // "xAI Night" in silver, simplified pixel art
        int text_x = 5;
        int text_y = 25;

        // x
        canvas->SetPixel(text_x, text_y, text_silver.r, text_silver.g, text_silver.b);
        canvas->SetPixel(text_x + 1, text_y, text_silver.r, text_silver.g, text_silver.b);
        canvas->SetPixel(text_x, text_y + 1, text_silver.r, text_silver.g, text_silver.b);
        canvas->SetPixel(text_x + 1, text_y + 1, text_silver.r, text_silver.g, text_silver.b);

        // A
        canvas->SetPixel(text_x + 3, text_y, text_silver.r, text_silver.g, text_silver.b);
        canvas->SetPixel(text_x + 4, text_y, text_silver.r, text_silver.g, text_silver.b);
        canvas->SetPixel(text_x + 3, text_y + 1, text_silver.r, text_silver.g, text_silver.b);
        canvas->SetPixel(text_x + 4, text_y + 1, text_silver.r, text_silver.g, text_silver.b);

        // I
        canvas->SetPixel(text_x + 6, text_y, text_silver.r, text_silver.g, text_silver.b);
        canvas->SetPixel(text_x + 6, text_y + 1, text_silver.r, text_silver.g, text_silver.b);

        // (space)

        // N
        canvas->SetPixel(text_x + 8, text_y, text_silver.r, text_silver.g, text_silver.b);
        canvas->SetPixel(text_x + 9, text_y, text_silver.r, text_silver.g, text_silver.b);
        canvas->SetPixel(text_x + 8, text_y + 1, text_silver.r, text_silver.g, text_silver.b);
        canvas->SetPixel(text_x + 9, text_y + 1, text_silver.r, text_silver.g, text_silver.b);

        // i
        canvas->SetPixel(text_x + 11, text_y + 1, text_silver.r, text_silver.g, text_silver.b);

        // g
        canvas->SetPixel(text_x + 13, text_y, text_silver.r, text_silver.g, text_silver.b);
        canvas->SetPixel(text_x + 14, text_y, text_silver.r, text_silver.g, text_silver.b);
        canvas->SetPixel(text_x + 13, text_y + 1, text_silver.r, text_silver.g, text_silver.b);
        canvas->SetPixel(text_x + 14, text_y + 1, text_silver.r, text_silver.g, text_silver.b);
    }

    void draw() {
        canvas->Clear();
        
        drawSky();
        drawAurora();
        drawStars();
        drawHotAirBalloon();
        drawText();
        
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
    
    CosmicAutumnNight scene(matrix);
    scene.draw();
    
    while (!interrupt_received) {
        usleep(1000000); // Hold display
    }
    
    matrix->Clear();
    delete matrix;
    
    return 0;
}
