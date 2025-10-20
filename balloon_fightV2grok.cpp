#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <signal.h>

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
    exit(0); // Ensure clean exit
}

struct Enemy {
    float x;
    float y;
    float vx;
    float vy;
    bool active;
    Color balloon_color;
};

void drawBalloon(FrameCanvas* canvas, int x, int y, const Color& main_color, const Color& shadow_color, bool inflate) {
    canvas->SetPixel(x - 1, y - (inflate ? 4 : 3), main_color.r, main_color.g, main_color.b);
    canvas->SetPixel(x - 1, y - (inflate ? 5 : 4), main_color.r, main_color.g, main_color.b);
    canvas->SetPixel(x + 1, y - (inflate ? 4 : 3), main_color.r, main_color.g, main_color.b);
    canvas->SetPixel(x + 1, y - (inflate ? 5 : 4), main_color.r, main_color.g, main_color.b);
    canvas->SetPixel(x - 1, y - (inflate ? 3 : 2), shadow_color.r, shadow_color.g, shadow_color.b);
    canvas->SetPixel(x + 1, y - (inflate ? 3 : 2), shadow_color.r, shadow_color.g, shadow_color.b);
    canvas->SetPixel(x, y - (inflate ? 2 : 1), 255, 255, 255); // String
}

void drawPlayer(FrameCanvas* canvas, float x, float y, float vy, bool flapping) {
    int px = (int)x;
    int py = (int)y;
    bool inflate = vy < 0; // Balloon inflates when moving up

    // Balloons
    drawBalloon(canvas, px, py, Color(100, 255, 100), Color(50, 150, 50), inflate);

    // Head
    canvas->SetPixel(px, py, 255, 200, 150);

    // Body (red shirt)
    canvas->SetPixel(px, py + 1, 255, 80, 80);
    canvas->SetPixel(px, py + 2, 255, 80, 80);

    // Legs (blue)
    canvas->SetPixel(px - 1, py + 3, 80, 120, 255);
    canvas->SetPixel(px + 1, py + 3, 80, 120, 255);

    // Arms (flapping)
    if (flapping || vy < 0) {
        canvas->SetPixel(px - 1, py + 1, 255, 200, 150);
        canvas->SetPixel(px + 1, py + 1, 255, 200, 150);
        canvas->SetPixel(px - 2, py, 255, 200, 150);
        canvas->SetPixel(px + 2, py, 255, 200, 150);
    } else {
        canvas->SetPixel(px - 1, py + 1, 255, 200, 150);
        canvas->SetPixel(px + 1, py + 1, 255, 200, 150);
    }
}

void drawEnemy(FrameCanvas* canvas, float x, float y, const Color& balloon_color) {
    int ex = (int)x;
    int ey = (int)y;

    // Balloons (random color)
    drawBalloon(canvas, ex, ey, balloon_color, Color(balloon_color.r / 2, balloon_color.g / 2, balloon_color.b / 2), true);

    // Body
    canvas->SetPixel(ex, ey, 255, 150, 80);
    canvas->SetPixel(ex, ey + 1, 255, 150, 80);

    // Arms (simple flapping)
    if ((rand() % 10) < 5) {
        canvas->SetPixel(ex - 1, ey, 255, 150, 80);
        canvas->SetPixel(ex + 1, ey, 255, 150, 80);
    }
}

bool checkCollision(float x1, float y1, float x2, float y2) {
    return (abs(x1 - x2) < 2 && abs(y1 - y2) < 3);
}

int main() {
    srand(time(NULL));

    // Set up signal handler
    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);

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
    global_matrix = matrix;
    FrameCanvas *canvas = matrix->CreateFrameCanvas();
    global_canvas = canvas;

    // Colors
    Color bg_black(0, 0, 0);
    Color star_white(255, 255, 255);
    Color water_blue(80, 160, 255);
    Color water_light(120, 200, 255);
    Color cloud_white(200, 200, 200);

    // Player
    float player_x = 16.0f;
    float player_y = 20.0f;
    float player_vy = 0.0f;
    bool flapping = false;
    int flap_timer = 0;
    int lives = 3;
    int score = 0;

    // Enemies
    const int num_enemies = 4;
    Enemy enemies[num_enemies];
    Color enemy_colors[] = {Color(255, 255, 255), Color(255, 150, 80), Color(150, 200, 150), Color(200, 150, 255)};
    for (int i = 0; i < num_enemies; ++i) {
        enemies[i].x = rand() % 26 + 3;
        enemies[i].y = rand() % 12 + 4;
        enemies[i].vx = (rand() % 2 == 0 ? 1 : -1) * 0.15f;
        enemies[i].vy = 0.05f + (rand() % 10) / 100.0f;
        enemies[i].active = true;
        enemies[i].balloon_color = enemy_colors[rand() % 4];
    }

    // Stars
    int star_x[20];
    int star_y[20];
    for (int i = 0; i < 20; ++i) {
        star_x[i] = rand() % 32;
        star_y[i] = rand() % 26;
    }

    // Clouds
    int cloud_x[3] = {5, 15, 25};
    int cloud_y[3] = {2, 4, 3};

    int frame_count = 0;

    while (!interrupt_received) {
        canvas->Fill(bg_black.r, bg_black.g, bg_black.b);

        // Draw twinkling stars
        for (int i = 0; i < 20; ++i) {
            int twinkle = (frame_count + i * 7) % 20;
            if (twinkle < 2) {
                canvas->SetPixel(star_x[i], star_y[i], star_white.r, star_white.g, star_white.b);
            } else if (twinkle < 4) {
                canvas->SetPixel(star_x[i], star_y[i], 150, 150, 150);
            }
        }

        // Draw clouds
        for (int i = 0; i < 3; ++i) {
            canvas->SetPixel(cloud_x[i], cloud_y[i], cloud_white.r, cloud_white.g, cloud_white.b);
            canvas->SetPixel(cloud_x[i] + 1, cloud_y[i], cloud_white.r, cloud_white.g, cloud_white.b);
            canvas->SetPixel(cloud_x[i], cloud_y[i] + 1, cloud_white.r, cloud_white.g, cloud_white.b);
            canvas->SetPixel(cloud_x[i] + 1, cloud_y[i] + 1, cloud_white.r, cloud_white.g, cloud_white.b);
        }

        // Draw water
        for (int y = 28; y < 32; ++y) {
            for (int x = 0; x < 32; ++x) {
                if ((x + frame_count / 3) % 4 < 2) {
                    canvas->SetPixel(x, y, water_blue.r, water_blue.g, water_blue.b);
                } else {
                    canvas->SetPixel(x, y, water_light.r, water_light.g, water_light.b);
                }
            }
        }
        for (int x = 0; x < 32; x += 2) {
            int wave_offset = (frame_count / 2 + x) % 4;
            canvas->SetPixel(x + wave_offset, 27, water_light.r, water_light.g, water_light.b);
        }

        // Update and draw enemies
        for (int i = 0; i < num_enemies; ++i) {
            if (enemies[i].active) {
                enemies[i].x += enemies[i].vx;
                enemies[i].y += enemies[i].vy;

                if (enemies[i].x < 2 || enemies[i].x > 29) enemies[i].vx *= -1;
                if (enemies[i].y < 4 || enemies[i].y > 20) enemies[i].vy *= -0.8f; // Dampened bounce

                drawEnemy(canvas, enemies[i].x, enemies[i].y, enemies[i].balloon_color);

                // Collision with player
                if (checkCollision(player_x, player_y, enemies[i].x, enemies[i].y)) {
                    lives--;
                    enemies[i].active = false;
                    if (lives <= 0) interrupt_received = true; // Game over
                }
            }
        }

        // Player input simulation
        flap_timer++;
        if (flap_timer > 25) {
            flapping = true;
            player_vy = -0.3f;
            flap_timer = 0;
        } else {
            flapping = false;
        }

        // Apply gravity
        player_vy += 0.02f;
        player_y += player_vy;

        // Keep player on screen
        if (player_y < 6) player_y = 6;
        if (player_y > 24) {
            player_y = 24;
            player_vy = 0;
        }

        // Draw player
        drawPlayer(canvas, player_x, player_y, player_vy, flapping);

        // Update score (enemies popped)
        for (int i = 0; i < num_enemies; ++i) {
            if (!enemies[i].active) score++;
        }

        frame_count++;
        canvas = matrix->SwapOnVSync(canvas);
        usleep(50000); // ~20 fps
    }

    // Final cleanup (redundant with signal handler but ensures safety)
    if (global_canvas) {
        global_canvas->Clear();
        global_canvas = matrix->SwapOnVSync(global_canvas);
    }
    if (global_matrix) {
        delete global_matrix;
        global_matrix = nullptr;
    }

    std::cout << "Game Over! Score: " << score << ", Lives Left: " << lives << std::endl;
    return 0;
}
