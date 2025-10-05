#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
using namespace rgb_matrix;
int main() {
    // Matrix setup
    RGBMatrix::Options matrix_options;
    matrix_options.rows = 32;
    matrix_options.cols = 32;
    matrix_options.chain_length = 1;
    matrix_options.parallel = 1;
    matrix_options.hardware_mapping = "adafruit-hat";
    RuntimeOptions rt_opts;
    rt_opts.gpio_slowdown = 2;
    RGBMatrix *matrix = CreateMatrixFromOptions(matrix_options, rt_opts);
    if (!matrix) {
        std::cerr << "Could not initialize RGB matrix." << std::endl;
        return 1;
    }
    FrameCanvas *canvas = matrix->CreateFrameCanvas();
    // Colors
    Color sky(0, 0, 255);
    Color ground(0, 128, 0);
    Color cloud(255, 255, 255);
    Color balloon(255, 0, 0);
    Color string_color(255, 255, 255);
    // Draw sky
    canvas->Fill(sky.r, sky.g, sky.b);
    // Draw ground (bottom 8 rows)
    for (int y = 24; y < 32; ++y)
        for (int x = 0; x < 32; ++x)
            canvas->SetPixel(x, y, ground.r, ground.g, ground.b);
    // Draw cloud
    DrawCircle(canvas, 10, 6, 3, cloud);
    DrawCircle(canvas, 12, 7, 2, cloud);
    DrawCircle(canvas, 8, 7, 2, cloud);
    // Draw balloons with strings
    int balloon_x[] = {5, 20, 27};
    int balloon_y[] = {18, 16, 20};
    for (int i = 0; i < 3; ++i) {
        DrawCircle(canvas, balloon_x[i], balloon_y[i], 2, balloon);
        // string
        for (int s = 0; s < 6; ++s) {
            int y_pos = balloon_y[i] + 2 + s;
            if (y_pos < 32)
                canvas->SetPixel(balloon_x[i], y_pos, string_color.r, string_color.g, string_color.b);
        }
    }
    // Swap once to display the scene
    canvas = matrix->SwapOnVSync(canvas);
    
    // Just wait - no need to keep swapping since nothing is changing
    while (true) {
        sleep(1000);  // Sleep indefinitely
    }
    delete matrix;
    return 0;
}
