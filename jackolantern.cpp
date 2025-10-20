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

void DrawPumpkinBody(Canvas *canvas, Color pumpkin_orange, Color pumpkin_dark, Color stem) {
    // Draw large pumpkin with more realistic rounded shape

    // Stem at top
    for (int y = 1; y < 4; ++y) {
        for (int x = 14; x <= 17; ++x) {
            canvas->SetPixel(x, y, stem.r, stem.g, stem.b);
        }
    }
    // Stem top
    for (int x = 13; x <= 18; ++x) {
        canvas->SetPixel(x, 0, stem.r, stem.g, stem.b);
    }

    // Pumpkin body - more circular/rounded shape
    for (int y = 4; y < 31; ++y) {
        // Calculate width based on distance from center (y=17)
        float center_y = 17.0f;
        float radius = 13.0f;
        float dy = (y - center_y) / radius;

        // Circular equation: width based on distance from center
        float width_ratio = 1.0f - (dy * dy);
        if (width_ratio < 0) width_ratio = 0;

        int width = (int)(radius * 2.0f * sqrt(width_ratio));

        // Make it slightly wider than tall (pumpkin proportions)
        width = (int)(width * 1.1f);
        if (width > 28) width = 28;

        int start_x = 16 - width / 2;
        int end_x = 16 + width / 2;

        for (int x = start_x; x <= end_x; ++x) {
            if (x >= 0 && x < 32) {
                canvas->SetPixel(x, y, pumpkin_orange.r, pumpkin_orange.g, pumpkin_orange.b);
            }
        }
    }

    // Add vertical segments for texture (ribs)
    int segments[] = {5, 9, 13, 16, 19, 23, 27};
    for (int s = 0; s < 7; ++s) {
        int seg_x = segments[s];
        for (int y = 6; y < 29; ++y) {
            // Only draw segment if there's pumpkin at this position
            // Check by approximating width at this y
            float center_y = 17.0f;
            float radius = 13.0f;
            float dy = (y - center_y) / radius;
            float width_ratio = 1.0f - (dy * dy);
            if (width_ratio > 0) {
                int width = (int)(radius * 2.0f * sqrt(width_ratio) * 1.1f);
                int start_x = 16 - width / 2;
                int end_x = 16 + width / 2;

                if (seg_x >= start_x && seg_x <= end_x) {
                    canvas->SetPixel(seg_x, y, pumpkin_dark.r, pumpkin_dark.g, pumpkin_dark.b);
                }
            }
        }
    }
}

void DrawHappyFace(Canvas *canvas, Color glow) {
    // Happy triangle eyes
    for (int y = 10; y <= 13; ++y) {
        int width = 14 - y;
        for (int x = 0; x < width; ++x) {
            canvas->SetPixel(9 + x, y, glow.r, glow.g, glow.b);
            canvas->SetPixel(19 + x, y, glow.r, glow.g, glow.b);
        }
    }

    // Big smile
    for (int x = 8; x <= 24; ++x) {
        canvas->SetPixel(x, 20, glow.r, glow.g, glow.b);
    }
    for (int x = 10; x <= 22; ++x) {
        canvas->SetPixel(x, 21, glow.r, glow.g, glow.b);
    }
    for (int x = 12; x <= 20; ++x) {
        canvas->SetPixel(x, 22, glow.r, glow.g, glow.b);
    }
}

void DrawScaryFace(Canvas *canvas, Color glow) {
    // Angry triangle eyes (upside down)
    for (int y = 10; y <= 13; ++y) {
        int width = y - 9;
        for (int x = 0; x < width; ++x) {
            canvas->SetPixel(9 + x, y, glow.r, glow.g, glow.b);
            canvas->SetPixel(19 + x, y, glow.r, glow.g, glow.b);
        }
    }

    // Jagged scary mouth
    for (int x = 8; x <= 24; ++x) {
        canvas->SetPixel(x, 20, glow.r, glow.g, glow.b);
    }
    // Teeth
    canvas->SetPixel(10, 21, glow.r, glow.g, glow.b);
    canvas->SetPixel(10, 22, glow.r, glow.g, glow.b);
    canvas->SetPixel(14, 21, glow.r, glow.g, glow.b);
    canvas->SetPixel(14, 22, glow.r, glow.g, glow.b);
    canvas->SetPixel(18, 21, glow.r, glow.g, glow.b);
    canvas->SetPixel(18, 22, glow.r, glow.g, glow.b);
    canvas->SetPixel(22, 21, glow.r, glow.g, glow.b);
    canvas->SetPixel(22, 22, glow.r, glow.g, glow.b);
}

void DrawSurprisedFace(Canvas *canvas, Color glow) {
    // Round surprised eyes
    for (int y = 10; y <= 13; ++y) {
        for (int x = 8; x <= 11; ++x) {
            canvas->SetPixel(x, y, glow.r, glow.g, glow.b);
            canvas->SetPixel(x + 12, y, glow.r, glow.g, glow.b);
        }
    }

    // Round "O" mouth
    for (int y = 19; y <= 23; ++y) {
        for (int x = 14; x <= 18; ++x) {
            if ((y == 19 || y == 23) || (x == 14 || x == 18)) {
                canvas->SetPixel(x, y, glow.r, glow.g, glow.b);
            }
        }
    }
}

void DrawEvilFace(Canvas *canvas, Color glow) {
    // Narrow evil eyes
    for (int x = 8; x <= 12; ++x) {
        canvas->SetPixel(x, 11, glow.r, glow.g, glow.b);
        canvas->SetPixel(x, 12, glow.r, glow.g, glow.b);
        canvas->SetPixel(x + 12, 11, glow.r, glow.g, glow.b);
        canvas->SetPixel(x + 12, 12, glow.r, glow.g, glow.b);
    }

    // Sinister grin
    for (int x = 10; x <= 22; ++x) {
        canvas->SetPixel(x, 20, glow.r, glow.g, glow.b);
    }
    // Upward curve at ends
    canvas->SetPixel(9, 19, glow.r, glow.g, glow.b);
    canvas->SetPixel(23, 19, glow.r, glow.g, glow.b);
    // Gaps for teeth
    for (int x = 12; x <= 20; x += 2) {
        canvas->SetPixel(x, 21, glow.r, glow.g, glow.b);
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

    // Colors
    Color bg_night(5, 0, 20);           // Dark background
    Color pumpkin_orange(255, 120, 0);  // Orange pumpkin
    Color pumpkin_dark(180, 70, 0);     // Dark orange for shading
    Color stem(80, 50, 20);             // Brown stem
    Color glow_bright(255, 200, 0);     // Bright yellow glow
    Color glow_dark(80, 60, 0);         // Dark yellow for dim state

    int frame_count = 0;
    int face_type = 0;
    int face_duration = 120; // frames per face (~6 seconds at 20fps)

    // Display until interrupted
    while (running) {
        // Clear canvas
        canvas->Fill(bg_night.r, bg_night.g, bg_night.b);

        // Draw pumpkin body
        DrawPumpkinBody(canvas, pumpkin_orange, pumpkin_dark, stem);

        // Flickering glow effect: bright for ≤1s (20 frames), dark otherwise
        int flicker = frame_count % 100; // Cycle every ~5 seconds
        Color glow_color = (flicker < 20) ? glow_bright : glow_dark;

        // Draw face based on current face type
        switch(face_type) {
            case 0:
                DrawHappyFace(canvas, glow_color);
                break;
            case 1:
                DrawScaryFace(canvas, glow_color);
                break;
            case 2:
                DrawSurprisedFace(canvas, glow_color);
                break;
            case 3:
                DrawEvilFace(canvas, glow_color);
                break;
        }

        // Switch faces every few seconds
        if (frame_count % face_duration == 0 && frame_count > 0) {
            face_type = (face_type + 1) % 4;
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
