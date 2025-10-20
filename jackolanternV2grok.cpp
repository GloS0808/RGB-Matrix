#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <cmath>
using namespace rgb_matrix;

void DrawPumpkinBody(Canvas *canvas, Color pumpkin_orange, Color pumpkin_dark, Color stem, Color shadow) {
    // Draw shadow underneath for depth
    for (int y = 28; y < 32; ++y) {
        int width = 28 - (y - 28) * 2;
        int start_x = 16 - width / 2;
        int end_x = 16 + width / 2;
        for (int x = start_x; x <= end_x; ++x) {
            if (x >= 0 && x < 32) {
                canvas->SetPixel(x, y, shadow.r, shadow.g, shadow.b);
            }
        }
    }

    // Curved stem with texture
    for (int y = 0; y < 5; ++y) {
        int start_x = 14 - (y / 2);
        int end_x = 18 + (y / 2);
        for (int x = start_x; x <= end_x; ++x) {
            if (x >= 0 && x < 32) {
                // Add some texture by darkening some pixels
                Color stem_color = ((x + y) % 2 == 0) ? stem : Color(stem.r * 0.8, stem.g * 0.8, stem.b * 0.8);
                canvas->SetPixel(x, y, stem_color.r, stem_color.g, stem_color.b);
            }
        }
    }

    // Pumpkin body with gradient
    for (int y = 5; y < 29; ++y) {
        float center_y = 17.0f;
        float radius = 12.5f;
        float dy = (y - center_y) / radius;
        float width_ratio = 1.0f - (dy * dy);
        if (width_ratio < 0) width_ratio = 0;

        int width = (int)(radius * 2.0f * sqrt(width_ratio) * 1.1f);
        if (width > 28) width = 28;

        int start_x = 16 - width / 2;
        int end_x = 16 + width / 2;

        for (int x = start_x; x <= end_x; ++x) {
            if (x >= 0 && x < 32) {
                // Gradient based on distance from center
                float dist = sqrt(pow(x - 16.0f, 2) + pow(y - 17.0f, 2)) / radius;
                float shade = 1.0f - dist * 0.3f;
                if (shade < 0.7f) shade = 0.7f;
                Color pixel_color(
                    pumpkin_orange.r * shade,
                    pumpkin_orange.g * shade,
                    pumpkin_orange.b * shade
                );
                canvas->SetPixel(x, y, pixel_color.r, pixel_color.g, pixel_color.b);
            }
        }
    }

    // Enhanced vertical segments for ribs
    int segments[] = {5, 8, 11, 14, 17, 20, 23, 26};
    for (int s = 0; s < 8; ++s) {
        int seg_x = segments[s];
        for (int y = 7; y < 28; ++y) {
            float center_y = 17.0f;
            float radius = 12.5f;
            float dy = (y - center_y) / radius;
            float width_ratio = 1.0f - (dy * dy);
            if (width_ratio > 0) {
                int width = (int)(radius * 2.0f * sqrt(width_ratio) * 1.1f);
                int start_x = 16 - width / 2;
                int end_x = 16 + width / 2;
                if (seg_x >= start_x && seg_x <= end_x) {
                    Color rib_color = Color(
                        pumpkin_dark.r * 0.9,
                        pumpkin_dark.g * 0.9,
                        pumpkin_dark.b * 0.9
                    );
                    canvas->SetPixel(seg_x, y, rib_color.r, rib_color.g, rib_color.b);
                }
            }
        }
    }
}

void DrawHappyFace(Canvas *canvas, Color glow) {
    // Eyebrows
    for (int x = 8; x <= 11; ++x) {
        canvas->SetPixel(x, 8, glow.r * 0.8, glow.g * 0.8, glow.b * 0.8);
        canvas->SetPixel(x + 12, 8, glow.r * 0.8, glow.g * 0.8, glow.b * 0.8);
    }

    // Triangle eyes with inner glow
    for (int y = 10; y <= 13; ++y) {
        int width = 14 - y;
        for (int x = 0; x < width; ++x) {
            Color eye_color = (y == 10 || x == 0 || x == width - 1) ? 
                Color(glow.r * 0.8, glow.g * 0.8, glow.b * 0.8) : glow;
            canvas->SetPixel(9 + x, y, eye_color.r, eye_color.g, eye_color.b);
            canvas->SetPixel(19 + x, y, eye_color.r, eye_color.g, eye_color.b);
        }
    }

    // Nose
    canvas->SetPixel(16, 15, glow.r, glow.g, glow.b);
    canvas->SetPixel(15, 16, glow.r, glow.g, glow.b);
    canvas->SetPixel(16, 16, glow.r, glow.g, glow.b);
    canvas->SetPixel(17, 16, glow.r, glow.g, glow.b);

    // Enhanced smile
    for (int x = 8; x <= 24; ++x) {
        canvas->SetPixel(x, 20, glow.r, glow.g, glow.b);
    }
    for (int x = 10; x <= 22; ++x) {
        canvas->SetPixel(x, 21, glow.r, glow.g, glow.b);
    }
    for (int x = 12; x <= 20; ++x) {
        canvas->SetPixel(x, 22, glow.r, glow.g, glow.b);
    }
    // Teeth
    canvas->SetPixel(14, 21, glow.r * 0.9, glow.g * 0.9, glow.b * 0.9);
    canvas->SetPixel(18, 21, glow.r * 0.9, glow.g * 0.9, glow.b * 0.9);
}

void DrawScaryFace(Canvas *canvas, Color glow) {
    // Angled eyebrows
    canvas->SetPixel(8, 8, glow.r * 0.8, glow.g * 0.8, glow.b * 0.8);
    canvas->SetPixel(9, 9, glow.r * 0.8, glow.g * 0.8, glow.b * 0.8);
    canvas->SetPixel(23, 8, glow.r * 0.8, glow.g * 0.8, glow.b * 0.8);
    canvas->SetPixel(22, 9, glow.r * 0.8, glow.g * 0.8, glow.b * 0.8);

    // Angry triangle eyes
    for (int y = 10; y <= 13; ++y) {
        int width = y - 9;
        for (int x = 0; x < width; ++x) {
            Color eye_color = (y == 13 || x == 0 || x == width - 1) ? 
                Color(glow.r * 0.8, glow.g * 0.8, glow.b * 0.8) : glow;
            canvas->SetPixel(9 + x, y, eye_color.r, eye_color.g, eye_color.b);
            canvas->SetPixel(19 + x, y, eye_color.r, eye_color.g, eye_color.b);
        }
    }

    // Nose
    canvas->SetPixel(16, 15, glow.r, glow.g, glow.b);
    canvas->SetPixel(15, 16, glow.r, glow.g, glow.b);
    canvas->SetPixel(16, 16, glow.r, glow.g, glow.b);
    canvas->SetPixel(17, 16, glow.r, glow.g, glow.b);

    // Jagged mouth with more teeth
    for (int x = 8; x <= 24; ++x) {
        canvas->SetPixel(x, 20, glow.r, glow.g, glow.b);
    }
    for (int x = 10; x <= 22; x += 2) {
        canvas->SetPixel(x, 21, glow.r * 0.9, glow.g * 0.9, glow.b * 0.9);
        canvas->SetPixel(x, 22, glow.r * 0.9, glow.g * 0.9, glow.b * 0.9);
    }
}

void DrawSurprisedFace(Canvas *canvas, Color glow) {
    // Raised eyebrows
    for (int x = 8; x <= 11; ++x) {
        canvas->SetPixel(x, 7, glow.r * 0.8, glow.g * 0.8, glow.b * 0.8);
        canvas->SetPixel(x + 12, 7, glow.r * 0.8, glow.g * 0.8, glow.b * 0.8);
    }

    // Round eyes
    for (int y = 10; y <= 13; ++y) {
        for (int x = 8; x <= 11; ++x) {
            Color eye_color = (y == 10 || y == 13 || x == 8 || x == 11) ? 
                Color(glow.r * 0.8, glow.g * 0.8, glow.b * 0.8) : glow;
            canvas->SetPixel(x, y, eye_color.r, eye_color.g, eye_color.b);
            canvas->SetPixel(x + 12, y, eye_color.r, eye_color.g, eye_color.b);
        }
    }

    // Nose
    canvas->SetPixel(16, 15, glow.r, glow.g, glow.b);
    canvas->SetPixel(15, 16, glow.r, glow.g, glow.b);
    canvas->SetPixel(16, 16, glow.r, glow.g, glow.b);
    canvas->SetPixel(17, 16, glow.r, glow.g, glow.b);

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
    // Angled eyebrows
    canvas->SetPixel(8, 9, glow.r * 0.8, glow.g * 0.8, glow.b * 0.8);
    canvas->SetPixel(9, 8, glow.r * 0.8, glow.g * 0.8, glow.b * 0.8);
    canvas->SetPixel(22, 9, glow.r * 0.8, glow.g * 0.8, glow.b * 0.8);
    canvas->SetPixel(23, 8, glow.r * 0.8, glow.g * 0.8, glow.b * 0.8);

    // Narrow eyes
    for (int x = 8; x <= 12; ++x) {
        Color eye_color = (x == 8 || x == 12) ? 
            Color(glow.r * 0.8, glow.g * 0.8, glow.b * 0.8) : glow;
        canvas->SetPixel(x, 11, eye_color.r, eye_color.g, eye_color.b);
        canvas->SetPixel(x, 12, eye_color.r, eye_color.g, eye_color.b);
        canvas->SetPixel(x + 12, 11, eye_color.r, eye_color.g, eye_color.b);
        canvas->SetPixel(x + 12, 12, eye_color.r, eye_color.g, eye_color.b);
    }

    // Nose
    canvas->SetPixel(16, 15, glow.r, glow.g, glow.b);
    canvas->SetPixel(15, 16, glow.r, glow.g, glow.b);
    canvas->SetPixel(16, 16, glow.r, glow.g, glow.b);
    canvas->SetPixel(17, 16, glow.r, glow.g, glow.b);

    // Sinister grin
    for (int x = 10; x <= 22; ++x) {
        canvas->SetPixel(x, 20, glow.r, glow.g, glow.b);
    }
    canvas->SetPixel(9, 19, glow.r, glow.g, glow.b);
    canvas->SetPixel(23, 19, glow.r, glow.g, glow.b);
    for (int x = 12; x <= 20; x += 2) {
        canvas->SetPixel(x, 21, glow.r * 0.9, glow.g * 0.9, glow.b * 0.9);
    }
}

int main() {
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

    Color bg_night(5, 0, 20);
    Color pumpkin_orange(255, 120, 0);
    Color pumpkin_dark(180, 70, 0);
    Color stem(80, 50, 20);
    Color glow(255, 200, 0);
    Color shadow(20, 10, 5);

    int frame_count = 0;
    int face_type = 0;
    int face_duration = 120;

    while (true) {
        canvas->Fill(bg_night.r, bg_night.g, bg_night.b);

        // Smoother flickering effect
        float flicker = sin(frame_count * 0.1) * 0.15 + 0.85;
        if (flicker < 0.7) flicker = 0.7;
        Color glow_color(glow.r * flicker, glow.g * flicker, glow.b * flicker);

        DrawPumpkinBody(canvas, pumpkin_orange, pumpkin_dark, stem, shadow);

        switch(face_type) {
            case 0: DrawHappyFace(canvas, glow_color); break;
            case 1: DrawScaryFace(canvas, glow_color); break;
            case 2: DrawSurprisedFace(canvas, glow_color); break;
            case 3: DrawEvilFace(canvas, glow_color); break;
        }

        if (frame_count % face_duration == 0 && frame_count > 0) {
            face_type = (face_type + 1) % 4;
        }

        frame_count++;
        canvas = matrix->SwapOnVSync(canvas);
        usleep(50000);
    }

    delete matrix;
    return 0;
}
