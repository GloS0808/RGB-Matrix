#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <cmath>
#include <signal.h>
using namespace rgb_matrix;

// Flag to control the main loop
static volatile bool running = true;

// Signal handler for Ctrl+C
void handle_interrupt(int sig) {
    running = false;
}

void DrawKaleidoscopeScene(FrameCanvas *canvas, int frame_count) {
    float center_x = 16.0f;
    float center_y = 16.0f;
    float time = frame_count * 0.05f;
    float pulse = sin(time * 2.0f) * 0.5f + 0.5f;  // 0 to 1 pulsing
    float rotation = time * 0.3f;

    // Draw radial kaleidoscope pattern
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {
            float dx = x - center_x;
            float dy = y - center_y;
            float distance = sqrt(dx*dx + dy*dy);
            float angle = atan2(dy, dx);

            // Create rotating spiral pattern
            float spiral = angle + distance * 0.3f + rotation;
            float radial_waves = sin(distance * 0.5f - time * 2.0f) * 0.5f + 0.5f;

            // Kaleidoscope effect - mirror across multiple axes
            float kaleidoscope = sin(angle * 6.0f + time) * 0.5f + 0.5f;

            // Combine patterns
            float intensity = (radial_waves * 0.5f + kaleidoscope * 0.5f);
            intensity = intensity * (1.0f - distance / 23.0f);  // Fade at edges

            // Pulsing rings
            float rings = fmod(distance + time * 3.0f, 8.0f);
            if (rings < 1.0f) {
                intensity += 0.5f;
            }

            // Clamp intensity
            if (intensity > 1.0f) intensity = 1.0f;
            if (intensity < 0.0f) intensity = 0.0f;

            // Color based on angle and time (psychedelic colors)
            float hue = fmod(angle / (2.0f * M_PI) + time * 0.2f, 1.0f);

            int r, g, b;
            if (hue < 0.33f) {
                // Purple to blue
                float t = hue * 3.0f;
                r = (int)((1.0f - t) * 200 * intensity);
                g = (int)(t * 100 * intensity);
                b = (int)(255 * intensity);
            } else if (hue < 0.66f) {
                // Blue to cyan/green
                float t = (hue - 0.33f) * 3.0f;
                r = 0;
                g = (int)((100 + t * 155) * intensity);
                b = (int)((255 - t * 155) * intensity);
            } else {
                // Green to purple
                float t = (hue - 0.66f) * 3.0f;
                r = (int)(t * 200 * intensity);
                g = (int)((255 - t * 155) * intensity);
                b = (int)(t * 255 * intensity);
            }

            canvas->SetPixel(x, y, r, g, b);
        }
    }

    // Add pulsing geometric overlays
    // Center bright spot that pulses
    int pulse_radius = (int)(3.0f + pulse * 3.0f);
    for (int y = -pulse_radius; y <= pulse_radius; ++y) {
        for (int x = -pulse_radius; x <= pulse_radius; ++x) {
            if (x*x + y*y <= pulse_radius * pulse_radius) {
                int px = (int)center_x + x;
                int py = (int)center_y + y;
                if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                    int bright = (int)(255 * pulse);
                    canvas->SetPixel(px, py, bright, bright, bright);
                }
            }
        }
    }

    // Rotating lines emanating from center
    int num_lines = 8;
    for (int i = 0; i < num_lines; ++i) {
        float line_angle = (i * 2.0f * M_PI / num_lines) + rotation;
        for (int r = 5; r < 20; ++r) {
            int px = (int)(center_x + cos(line_angle) * r);
            int py = (int)(center_y + sin(line_angle) * r);

            if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                // Fade lines based on pulse
                int brightness = (int)(200 * pulse);
                canvas->SetPixel(px, py, brightness, brightness / 2, brightness);
            }
        }
    }
}

void DrawStarfieldScene(FrameCanvas *canvas, int frame_count) {
    // Colors for starfield scene
    Color nebula_base(20, 10, 40);      // Dark purple nebula base
    Color nebula_accent(100, 50, 150);  // Brighter purple accent
    Color star_bright(255, 255, 255);   // Bright white star
    Color star_dim(150, 150, 200);      // Dim star

    float time = frame_count * 0.05f;
    float pulse = sin(time * 1.5f) * 0.5f + 0.5f;  // Slower pulse for stars

    // Draw nebula background (flowing color gradients)
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {
            float dx = x - 16.0f;
            float dy = y - 16.0f;
            float distance = sqrt(dx*dx + dy*dy);
            float angle = atan2(dy, dx);

            // Create swirling nebula effect
            float nebula_flow = sin(angle * 4.0f + time * 0.8f) * cos(distance * 0.3f - time) * 0.5f + 0.5f;
            float intensity = nebula_flow * (1.0f - distance / 25.0f);  // Fade at edges

            if (intensity > 1.0f) intensity = 1.0f;
            if (intensity < 0.0f) intensity = 0.0f;

            // Blend between base and accent colors
            int r = (int)(nebula_base.r + (nebula_accent.r - nebula_base.r) * intensity);
            int g = (int)(nebula_base.g + (nebula_accent.g - nebula_base.g) * intensity);
            int b = (int)(nebula_base.b + (nebula_accent.b - nebula_base.b) * intensity);

            canvas->SetPixel(x, y, r, g, b);
        }
    }

    // Define a grid of star positions
    int star_positions[][2] = {
        {4, 4}, {12, 8}, {20, 6}, {28, 10},
        {8, 16}, {16, 14}, {24, 18},
        {6, 24}, {14, 28}, {22, 26}
    };
    int num_stars = 10;

    // Draw pulsing stars
    for (int i = 0; i < num_stars; ++i) {
        int star_x = star_positions[i][0];
        int star_y = star_positions[i][1];
        float star_pulse = sin(time + i * 0.5f) * 0.5f + 0.5f;

        // Choose between bright and dim star based on pulse
        Color star_color = (star_pulse > 0.7f) ? star_bright : star_dim;

        // Draw star (cross shape for more visibility)
        if (star_x >= 0 && star_x < 32 && star_y >= 0 && star_y < 32) {
            canvas->SetPixel(star_x, star_y, star_color.r, star_color.g, star_color.b);
            if (star_x + 1 < 32) canvas->SetPixel(star_x + 1, star_y, star_color.r / 2, star_color.g / 2, star_color.b / 2);
            if (star_x - 1 >= 0) canvas->SetPixel(star_x - 1, star_y, star_color.r / 2, star_color.g / 2, star_color.b / 2);
            if (star_y + 1 < 32) canvas->SetPixel(star_x, star_y + 1, star_color.r / 2, star_color.g / 2, star_color.b / 2);
            if (star_y - 1 >= 0) canvas->SetPixel(star_x, star_y - 1, star_color.r / 2, star_color.g / 2, star_color.b / 2);
        }
    }

    // Add occasional cosmic flares (inspired by sparkles in valentines.cpp)
    if ((frame_count % 50) < 5) {
        int flare_x = 8 + (frame_count % 16);
        int flare_y = 8 + ((frame_count / 2) % 16);
        if (flare_x >= 0 && flare_x < 32 && flare_y >= 0 && flare_y < 32) {
            int brightness = (frame_count % 50) < 3 ? 255 : 150;
            canvas->SetPixel(flare_x, flare_y, brightness, brightness, 200);
        }
    }
}

int main() {
    // Set up signal handler for Ctrl+C
    signal(SIGINT, handle_interrupt);

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

    int frame_count = 0;
    int scene = 0; // 0 for kaleidoscope, 1 for starfield
    const int scene_duration = 200; // ~10 seconds at 20 fps

    // Display until interrupted
    while (running) {
        // Clear canvas
        canvas->Clear();

        // Switch scenes every 10 seconds
        if (frame_count % scene_duration == 0 && frame_count > 0) {
            scene = (scene + 1) % 2;
        }

        // Draw appropriate scene
        if (scene == 0) {
            DrawKaleidoscopeScene(canvas, frame_count);
        } else {
            DrawStarfieldScene(canvas, frame_count);
        }

        frame_count++;
        canvas = matrix->SwapOnVSync(canvas);
        usleep(50000);  // ~20 fps
    }

    // Cleanup
    delete matrix;
    std::cout << "Program terminated gracefully." << std::endl;
    return 0;
}
