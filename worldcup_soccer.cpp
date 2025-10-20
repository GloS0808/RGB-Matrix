#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <signal.h>
#include <vector>
#include <cmath>

using namespace rgb_matrix;
using namespace std::chrono;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
    interrupt_received = true;
}

// Soccer field element structure
struct SoccerElement {
    float x, y;
    float speed;
    bool active;
    steady_clock::time_point last_active;
    Color color;
};

// Goal scoring structure
struct GoalEvent {
    int x;
    int frame;
    bool active;
};

// Simple circle drawing function (since your example used it)
void DrawCircle(FrameCanvas *canvas, int x, int y, int radius, Color color) {
    for (int i = -radius; i <= radius; ++i) {
        for (int j = -radius; j <= radius; ++j) {
            if (i*i + j*j <= radius*radius) {
                int px = x + i;
                int py = y + j;
                if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                    canvas->SetPixel(px, py, color.r, color.g, color.b);
                }
            }
        }
    }
}

int main() {
    // Set up signal handler for graceful exit
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
    FrameCanvas *canvas = matrix->CreateFrameCanvas();

    // World Cup colors
    Color field_green(0, 128, 0);
    Color white(255, 255, 255);
    Color team_a_blue(0, 0, 255);    // Team A (blue)
    Color team_b_yellow(255, 255, 0); // Team B (yellow)
    Color ball_white(255, 255, 255);
    Color goal_net(200, 200, 200);
    Color crowd(30, 144, 255);       // Crowd blue
    Color trophy_gold(255, 215, 0);

    // Soccer ball position and movement
    float ball_x = 16.0f, ball_y = 16.0f;
    float ball_dx = 0.8f, ball_dy = 0.6f;
    
    // Players
    std::vector<SoccerElement> players;
    for (int i = 0; i < 8; ++i) {
        SoccerElement player;
        player.x = rand() % 32;
        player.y = 8 + (rand() % 16);
        player.speed = 0.1f + (rand() % 100) / 500.0f;
        player.active = true;
        player.color = (i < 4) ? team_a_blue : team_b_yellow;
        players.push_back(player);
    }

    // Cheering crowd elements
    std::vector<SoccerElement> crowd_effects;
    for (int i = 0; i < 15; ++i) {
        SoccerElement effect;
        effect.x = rand() % 32;
        effect.y = rand() % 4;  // Top rows for crowd
        effect.speed = 0.05f + (rand() % 50) / 1000.0f;
        effect.active = true;
        effect.color = white;
        crowd_effects.push_back(effect);
    }

    // Goal event
    GoalEvent goal = {0, 0, false};
    int score_team_a = 0, score_team_b = 0;
    auto last_goal_time = steady_clock::now();
    
    // Celebration particles
    std::vector<SoccerElement> celebration;

    // Main display loop
    while (!interrupt_received) {
        canvas->Fill(0, 0, 0);  // Clear canvas

        // Draw soccer field
        for (int y = 4; y < 28; ++y) {
            for (int x = 0; x < 32; ++x) {
                canvas->SetPixel(x, y, field_green.r, field_green.g, field_green.b);
            }
        }

        // Draw field markings - center line
        for (int y = 4; y < 28; ++y) {
            canvas->SetPixel(16, y, white.r, white.g, white.b);
        }
        
        // Draw center circle
        DrawCircle(canvas, 16, 16, 4, white);

        // Draw goals
        for (int y = 10; y < 22; ++y) {
            canvas->SetPixel(0, y, goal_net.r, goal_net.g, goal_net.b);
            canvas->SetPixel(31, y, goal_net.r, goal_net.g, goal_net.b);
        }

        // Draw top and bottom crowd areas
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 32; ++x) {
                canvas->SetPixel(x, y, crowd.r, crowd.g, crowd.b);
            }
        }
        for (int y = 28; y < 32; ++y) {
            for (int x = 0; x < 32; ++x) {
                canvas->SetPixel(x, y, crowd.r, crowd.g, crowd.b);
            }
        }

        // Update and draw soccer ball
        ball_x += ball_dx;
        ball_y += ball_dy;
        
        // Ball collision with field boundaries
        if (ball_x <= 2 || ball_x >= 30) {
            ball_dx = -ball_dx;
            // Check for goals
            if ((ball_x <= 2 && ball_y >= 10 && ball_y <= 22) || 
                (ball_x >= 30 && ball_y >= 10 && ball_y <= 22)) {
                goal.active = true;
                goal.x = (ball_x <= 2) ? 0 : 31;
                goal.frame = 0;
                
                if (ball_x <= 2) score_team_b++;
                else score_team_a++;
                
                last_goal_time = steady_clock::now();
                
                // Create celebration particles
                for (int i = 0; i < 20; ++i) {
                    SoccerElement particle;
                    particle.x = ball_x;
                    particle.y = ball_y;
                    particle.speed = 0.5f + (rand() % 100) / 200.0f;
                    particle.active = true;
                    particle.color = (ball_x <= 2) ? team_b_yellow : team_a_blue;
                    celebration.push_back(particle);
                }
                
                // Reset ball after goal
                ball_x = 16.0f;
                ball_y = 16.0f;
                ball_dx = (rand() % 2 == 0) ? 0.8f : -0.8f;
                ball_dy = (rand() % 2 == 0) ? 0.6f : -0.6f;
            }
        }
        if (ball_y <= 6 || ball_y >= 26) {
            ball_dy = -ball_dy;
        }
        
        // Keep ball in reasonable bounds
        if (ball_x < 2) ball_x = 2;
        if (ball_x > 30) ball_x = 30;
        if (ball_y < 6) ball_y = 6;
        if (ball_y > 26) ball_y = 26;
        
        // Draw soccer ball
        DrawCircle(canvas, (int)ball_x, (int)ball_y, 1, ball_white);

        // Update and draw players
        for (size_t i = 0; i < players.size(); ++i) {
            SoccerElement& player = players[i];
            
            // Simple AI - move toward ball sometimes
            if (rand() % 100 < 20) {
                if (player.x < ball_x) player.x += player.speed;
                else if (player.x > ball_x) player.x -= player.speed;
                if (player.y < ball_y) player.y += player.speed;
                else if (player.y > ball_y) player.y -= player.speed;
            } else {
                // Random movement
                player.x += (rand() % 3 - 1) * player.speed;
                player.y += (rand() % 3 - 1) * player.speed;
            }
            
            // Keep players on field
            if (player.x < 2) player.x = 2;
            if (player.x > 30) player.x = 30;
            if (player.y < 6) player.y = 6;
            if (player.y > 26) player.y = 26;
            
            // Draw player
            canvas->SetPixel((int)player.x, (int)player.y, 
                           player.color.r, player.color.g, player.color.b);
        }

        // Update and draw crowd effects
        for (size_t i = 0; i < crowd_effects.size(); ++i) {
            SoccerElement& effect = crowd_effects[i];
            effect.x += effect.speed;
            if (effect.x >= 32) effect.x = 0;
            canvas->SetPixel((int)effect.x, (int)effect.y, 
                           effect.color.r, effect.color.g, effect.color.b);
        }

        // Handle goal celebration
        if (goal.active) {
            // Draw expanding circle for goal
            int radius = goal.frame / 2;
            DrawCircle(canvas, goal.x, 16, radius, white);
            goal.frame++;
            if (goal.frame > 20) {
                goal.active = false;
            }
        }

        // Update and draw celebration particles (using index to avoid iterator issues)
        for (size_t i = 0; i < celebration.size(); ) {
            SoccerElement& particle = celebration[i];
            particle.x += (rand() % 5 - 2) * particle.speed;
            particle.y += (rand() % 5 - 2) * particle.speed;
            
            if (particle.x >= 0 && particle.x < 32 && particle.y >= 0 && particle.y < 32) {
                canvas->SetPixel((int)particle.x, (int)particle.y, 
                               particle.color.r, particle.color.g, particle.color.b);
                ++i;
            } else {
                // Remove particle if it's off screen
                celebration[i] = celebration.back();
                celebration.pop_back();
            }
        }

        // Swap canvas and control frame rate
        canvas = matrix->SwapOnVSync(canvas);
        usleep(50000);  // ~20 fps
    }

    // Clean up on exit
    canvas->Clear();
    canvas = matrix->SwapOnVSync(canvas);
    delete matrix;

    std::cout << "\nWorld Cup display cleared. Exiting gracefully.\n";
    return 0;
}
