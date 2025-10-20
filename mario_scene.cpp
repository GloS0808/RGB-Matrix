// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Mario-inspired scene for 32x32 RGB matrix

#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <signal.h>
#include <cmath>

using namespace rgb_matrix;

volatile bool interrupt_received = false;
RGBMatrix* global_matrix = nullptr;
FrameCanvas* global_canvas = nullptr;

static void InterruptHandler(int signo) {
    interrupt_received = true;
    if (global_canvas) {
        global_canvas->Clear();
        global_canvas = global_matrix->SwapOnVSync(global_canvas);
    }
    if (global_matrix) {
        delete global_matrix;
        global_matrix = nullptr;
    }
    exit(0);
}

struct Goomba {
    float x;
    float y;
    float vx;
    bool active;
};

void drawMario(FrameCanvas* canvas, float x, float y, bool jumping, bool facing_right) {
    int mx = (int)x;
    int my = (int)y;

    // Hat (red)
    canvas->SetPixel(mx, my - 4, 255, 0, 0);
    canvas->SetPixel(mx - 1, my - 4, 255, 0, 0);
    canvas->SetPixel(mx + 1, my - 4, 255, 0, 0);
    canvas->SetPixel(mx - 2, my - 4, 255, 0, 0);
    canvas->SetPixel(mx + 2, my - 4, 255, 0, 0);

    // Face (skin)
    canvas->SetPixel(mx, my - 3, 255, 200, 150);
    canvas->SetPixel(mx - 1, my - 3, 255, 200, 150);
    canvas->SetPixel(mx + 1, my - 3, 255, 200, 150);
    canvas->SetPixel(mx - 2, my - 3, 0, 0, 0); // Hair
    canvas->SetPixel(mx + 2, my - 3, 0, 0, 0); // Hair

    // Eyes and mustache
    canvas->SetPixel(mx - 1, my - 2, 0, 0, 0);
    canvas->SetPixel(mx + 1, my - 2, 0, 0, 0);
    canvas->SetPixel(mx, my - 2, 255, 200, 150);

    // Body (red shirt)
    canvas->SetPixel(mx, my - 1, 255, 0, 0);
    canvas->SetPixel(mx - 1, my - 1, 255, 0, 0);
    canvas->SetPixel(mx + 1, my - 1, 255, 0, 0);
    canvas->SetPixel(mx, my, 0, 0, 255); // Overalls (blue)
    canvas->SetPixel(mx - 1, my, 0, 0, 255);
    canvas->SetPixel(mx + 1, my, 0, 0, 255);

    // Arms
    if (jumping) {
        canvas->SetPixel(mx - 2, my - 1, 255, 200, 150); // Left arm up
        canvas->SetPixel(mx + 2, my - 1, 255, 200, 150); // Right arm up
    } else {
        canvas->SetPixel(mx - 2, my, 255, 200, 150); // Left arm down
        canvas->SetPixel(mx + 2, my, 255, 200, 150); // Right arm down
    }

    // Legs (blue overalls, brown shoes)
    canvas->SetPixel(mx - 1, my + 1, 0, 0, 255);
    canvas->SetPixel(mx + 1, my + 1, 0, 0, 255);
    canvas->SetPixel(mx - 1, my + 2, 139, 69, 19); // Shoes
    canvas->SetPixel(mx + 1, my + 2, 139, 69, 19);

    // Mirror if facing left
    if (!facing_right) {
        // Swap left/right pixels (simplified)
        // Note: For accuracy, redraw with mirrored positions
    }
}

void drawGoomba(FrameCanvas* canvas, float x, float y) {
    int gx = (int)x;
    int gy = (int)y;

    // Body (brown)
    canvas->SetPixel(gx, gy, 139, 69, 19);
    canvas->SetPixel(gx - 1, gy, 139, 69, 19);
    canvas->SetPixel(gx + 1, gy, 139, 69, 19);
    canvas->SetPixel(gx, gy - 1, 139, 69, 19);
    canvas->SetPixel(gx - 1, gy - 1, 139, 69, 19);
    canvas->SetPixel(gx + 1, gy - 1, 139, 69, 19);

    // Eyes (white with black pupils)
    canvas->SetPixel(gx - 1, gy - 2, 255, 255, 255);
    canvas->SetPixel(gx + 1, gy - 2, 255, 255, 255);
    canvas->SetPixel(gx - 1, gy - 3, 0, 0, 0);
    canvas->SetPixel(gx + 1, gy - 3, 0, 0, 0);

    // Feet (black)
    canvas->SetPixel(gx - 1, gy + 1, 0, 0, 0);
    canvas->SetPixel(gx + 1, gy + 1, 0, 0, 0);
}

void drawPipe(FrameCanvas* canvas, int x, int height) {
    for (int y = 31 - height; y < 32; ++y) {
        canvas->SetPixel(x, y, 0, 255, 0);
        canvas->SetPixel(x - 1, y, 0, 255, 0);
        canvas->SetPixel(x + 1, y, 0, 255, 0);
        canvas->SetPixel(x - 2, y, 0, 128, 0); // Shadow
        canvas->SetPixel(x + 2, y, 0, 128, 0);
    }
}

void drawCloud(FrameCanvas* canvas, int x, int y) {
    canvas->SetPixel(x, y, 255, 255, 255);
    canvas->SetPixel(x - 1, y, 255, 255, 255);
    canvas->SetPixel(x + 1, y, 255, 255, 255);
    canvas->SetPixel(x - 2, y + 1, 255, 255, 255);
    canvas->SetPixel(x + 2, y + 1, 255, 255, 255);
    canvas->SetPixel(x, y + 1, 255, 255, 255);
}

bool checkCollision(float x1, float y1, float x2, float y2) {
    return (abs(x1 - x2) < 2 && abs(y1 - y2) < 3);
}

int main() {
    srand(time(NULL));

    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);

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
    global_matrix = matrix;
    FrameCanvas *canvas = matrix->CreateFrameCanvas();
    global_canvas = canvas;

    Color bg_sky(135, 206, 235); // Light blue sky
    Color ground_green(0, 255, 0);
    Color brick_brown(165, 42, 42);
    Color coin_gold(255, 215, 0);

    // Mario
    float mario_x = 8.0f;
    float mario_y = 24.0f;
    float mario_vx = 0.2f;
    float mario_vy = 0.0f;
    bool jumping = false;
    bool facing_right = true;
    int jump_timer = 0;

    // Goombas
    const int num_goombas = 2;
    Goomba goombas[num_goombas];
    for (int i = 0; i < num_goombas; ++i) {
        goombas[i].x = 20 + i * 10;
        goombas[i].y = 24.0f;
        goombas[i].vx = -0.1f;
        goombas[i].active = true;
    }

    // Pipes
    int pipe_x = 24;
    int pipe_height = 8;

    // Clouds
    int cloud_x[3] = {4, 14, 28};
    int cloud_y[3] = {4, 6, 5};

    // Coins
    int coin_x = 16;
    int coin_y = 16;
    bool coin_collected = false;

    int frame_count = 0;

    while (!interrupt_received) {
        canvas->Fill(bg_sky.r, bg_sky.g, bg_sky.b);

        // Draw ground
        for (int x = 0; x < 32; ++x) {
            canvas->SetPixel(x, 31, ground_green.r, ground_green.g, ground_green.b);
            canvas->SetPixel(x, 30, ground_green.r, ground_green.g, ground_green.b);
        }

        // Draw bricks
        for (int x = 0; x < 32; x += 2) {
            canvas->SetPixel(x, 29, brick_brown.r, brick_brown.g, brick_brown.b);
        }

        // Draw clouds
        for (int i = 0; i < 3; ++i) {
            drawCloud(canvas, cloud_x[i], cloud_y[i]);
        }

        // Draw pipe
        drawPipe(canvas, pipe_x, pipe_height);

        // Draw coin if not collected
        if (!coin_collected) {
            canvas->SetPixel(coin_x, coin_y, coin_gold.r, coin_gold.g, coin_gold.b);
            canvas->SetPixel(coin_x + 1, coin_y, coin_gold.r, coin_gold.g, coin_gold.b);
        }

        // Update Goombas
        for (int i = 0; i < num_goombas; ++i) {
            if (goombas[i].active) {
                goombas[i].x += goombas[i].vx;

                if (goombas[i].x < 0 || goombas[i].x > 31) goombas[i].vx *= -1;

                drawGoomba(canvas, goombas[i].x, goombas[i].y);

                // Collision with Mario
                if (checkCollision(mario_x, mario_y, goombas[i].x, goombas[i].y)) {
                    if (mario_vy > 0) { // Mario stomps
                        goombas[i].active = false;
                    } else { // Mario hit
                        interrupt_received = true; // Game over
                    }
                }
            }
        }

        // Update Mario
        mario_x += mario_vx;
        mario_y += mario_vy;
        mario_vy += 0.05f; // Gravity

        if (mario_x > 31) {
            mario_x = 0;
        } else if (mario_x < 0) {
            mario_x = 31;
        }

        if (mario_y >= 24) {
            mario_y = 24;
            mario_vy = 0;
            jumping = false;
        }

        if (jump_timer++ > 50) {
            if (!jumping) {
                mario_vy = -0.8f;
                jumping = true;
            }
            jump_timer = 0;
        }

        drawMario(canvas, mario_x, mario_y, jumping, facing_right);

        // Collect coin
        if (!coin_collected && checkCollision(mario_x, mario_y, coin_x, coin_y)) {
            coin_collected = true;
        }

        frame_count++;
        canvas = matrix->SwapOnVSync(canvas);
        usleep(50000); // ~20 fps
    }

    // Cleanup
    if (global_canvas) {
        global_canvas->Clear();
        global_canvas = matrix->SwapOnVSync(global_canvas);
    }
    if (global_matrix) {
        delete global_matrix;
        global_matrix = nullptr;
    }

    return 0;
}
