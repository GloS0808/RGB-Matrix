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

// Football field element structure
struct FootballElement {
    float x, y;
    float dx, dy;
    bool active;
    Color color;
};

// Game state structure
struct GameState {
    int down = 1;
    int yards_to_go = 10;
    int yard_line = 20;
    int score_home = 0;
    int score_away = 0;
    bool home_has_ball = true;
};

// Simple drawing functions
void DrawRectangle(FrameCanvas *canvas, int x, int y, int width, int height, Color color) {
    for (int i = x; i < x + width && i < 32; ++i) {
        for (int j = y; j < y + height && j < 32; ++j) {
            if (i >= 0 && j >= 0) {
                canvas->SetPixel(i, j, color.r, color.g, color.b);
            }
        }
    }
}

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

void DrawLine(FrameCanvas *canvas, int x1, int y1, int x2, int y2, Color color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    
    while (true) {
        canvas->SetPixel(x1, y1, color.r, color.g, color.b);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
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

    // American football colors
    Color field_green(0, 128, 0);
    Color white(255, 255, 255);
    Color yellow(255, 255, 0);
    Color brown(139, 69, 19);
    Color home_red(255, 0, 0);      // Home team (red)
    Color away_blue(0, 0, 255);     // Away team (blue)
    Color football_brown(160, 120, 80);
    Color goalpost_yellow(255, 255, 0);
    Color crowd_dark(30, 30, 100);

    // Game state
    GameState game;
    
    // Football position and movement
    FootballElement football;
    football.x = 16.0f;
    football.y = 16.0f;
    football.dx = 0.0f;
    football.dy = 0.0f;
    football.active = false;
    football.color = football_brown;

    // Players
    std::vector<FootballElement> players;
    
    // Initialize offensive team (7 players)
    for (int i = 0; i < 7; ++i) {
        FootballElement player;
        player.x = 10.0f + (i % 3) * 3.0f;
        player.y = 8.0f + (i / 3) * 8.0f;
        player.dx = 0.0f;
        player.dy = 0.0f;
        player.active = true;
        player.color = game.home_has_ball ? home_red : away_blue;
        players.push_back(player);
    }
    
    // Initialize defensive team (7 players)
    for (int i = 0; i < 7; ++i) {
        FootballElement player;
        player.x = 20.0f + (i % 3) * 3.0f;
        player.y = 8.0f + (i / 3) * 8.0f;
        player.dx = 0.0f;
        player.dy = 0.0f;
        player.active = true;
        player.color = game.home_has_ball ? away_blue : home_red;
        players.push_back(player);
    }

    // Game cycle variables
    enum GamePhase { PRE_SNAP, PLAY_IN_PROGRESS, SCORE, TURNOVER };
    GamePhase current_phase = PRE_SNAP;
    int play_clock = 0;
    int max_play_clock = 100;
    auto last_phase_change = steady_clock::now();
    
    // Celebration particles
    std::vector<FootballElement> celebration;

    // Main display loop
    while (!interrupt_received) {
        canvas->Fill(0, 0, 0);  // Clear canvas

        // Draw football field
        for (int y = 4; y < 28; ++y) {
            for (int x = 0; x < 32; ++x) {
                canvas->SetPixel(x, y, field_green.r, field_green.g, field_green.b);
            }
        }

        // Draw yard lines every 5 yards (scaled for 32x32)
        for (int x = 2; x < 30; x += 3) {
            for (int y = 4; y < 28; y++) {
                canvas->SetPixel(x, y, white.r, white.g, white.b);
            }
        }

        // Draw end zones
        DrawRectangle(canvas, 0, 4, 2, 24, home_red);  // Left end zone
        DrawRectangle(canvas, 30, 4, 2, 24, away_blue); // Right end zone

        // Draw goalposts
        DrawLine(canvas, 1, 10, 1, 22, goalpost_yellow);
        DrawLine(canvas, 31, 10, 31, 22, goalpost_yellow);

        // Draw 50-yard line
        for (int y = 4; y < 28; y += 2) {
            canvas->SetPixel(16, y, white.r, white.g, white.b);
        }

        // Draw crowd in top and bottom areas
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 32; ++x) {
                canvas->SetPixel(x, y, crowd_dark.r, crowd_dark.g, crowd_dark.b);
            }
        }
        for (int y = 28; y < 32; ++y) {
            for (int x = 0; x < 32; ++x) {
                canvas->SetPixel(x, y, crowd_dark.r, crowd_dark.g, crowd_dark.b);
            }
        }

        // Game logic based on current phase
        switch (current_phase) {
            case PRE_SNAP:
                // Position players for snap
                if (!football.active) {
                    // Position quarterback (offensive team's center player)
                    players[3].x = 12.0f;
                    players[3].y = 16.0f;
                    football.x = players[3].x - 1.0f;
                    football.y = players[3].y;
                    football.active = true;
                }
                
                // Start play after a delay
                auto now = steady_clock::now();
                if (duration_cast<milliseconds>(now - last_phase_change).count() > 2000) {
                    current_phase = PLAY_IN_PROGRESS;
                    play_clock = max_play_clock;
                    last_phase_change = now;
                    
                    // Set initial football movement
                    football.dx = 1.0f;
                    football.dy = (rand() % 3 - 1) * 0.5f;
                }
                break;
                
            case PLAY_IN_PROGRESS:
                // Update football position
                football.x += football.dx;
                football.y += football.dy;
                
                // Football field boundaries
                if (football.y <= 5 || football.y >= 27) {
                    football.dy = -football.dy;
                }
                
                // Update player positions (simple AI)
                for (size_t i = 0; i < players.size(); ++i) {
                    FootballElement& player = players[i];
                    
                    // Offensive players try to advance
                    if (i < 7) {
                        player.dx = 0.3f;
                        // Try to get open for pass
                        if (rand() % 100 < 10) {
                            player.dy = (rand() % 3 - 1) * 0.4f;
                        }
                    } 
                    // Defensive players try to tackle
                    else {
                        // Move toward football
                        if (player.x > football.x) player.dx = -0.4f;
                        else player.dx = 0.4f;
                        if (player.y > football.y) player.dy = -0.3f;
                        else player.dy = 0.3f;
                    }
                    
                    player.x += player.dx;
                    player.y += player.dy;
                    
                    // Keep players on field
                    if (player.x < 2) player.x = 2;
                    if (player.x > 30) player.x = 30;
                    if (player.y < 6) player.y = 6;
                    if (player.y > 26) player.y = 26;
                    
                    // Check for tackles (collision with football)
                    float dist_to_ball = sqrt(pow(player.x - football.x, 2) + pow(player.y - football.y, 2));
                    if (dist_to_ball < 2.0f && i >= 7) { // Defensive player tackles
                        football.dx = 0.0f;
                        football.dy = 0.0f;
                    }
                }
                
                // Check for scoring
                if (football.x <= 2) { // Touchdown for away team
                    game.score_away += 7;
                    current_phase = SCORE;
                    last_phase_change = steady_clock::now();
                    
                    // Celebration for away team
                    for (int i = 0; i < 15; ++i) {
                        FootballElement particle;
                        particle.x = football.x;
                        particle.y = football.y;
                        particle.dx = (rand() % 10 - 5) * 0.1f;
                        particle.dy = (rand() % 10 - 5) * 0.1f;
                        particle.active = true;
                        particle.color = away_blue;
                        celebration.push_back(particle);
                    }
                } else if (football.x >= 30) { // Touchdown for home team
                    game.score_home += 7;
                    current_phase = SCORE;
                    last_phase_change = steady_clock::now();
                    
                    // Celebration for home team
                    for (int i = 0; i < 15; ++i) {
                        FootballElement particle;
                        particle.x = football.x;
                        particle.y = football.y;
                        particle.dx = (rand() % 10 - 5) * 0.1f;
                        particle.dy = (rand() % 10 - 5) * 0.1f;
                        particle.active = true;
                        particle.color = home_red;
                        celebration.push_back(particle);
                    }
                }
                
                play_clock--;
                if (play_clock <= 0) {
                    current_phase = TURNOVER;
                    last_phase_change = steady_clock::now();
                    game.home_has_ball = !game.home_has_ball;
                }
                break;
                
            case SCORE:
                // Show score celebration
                if (duration_cast<milliseconds>(steady_clock::now() - last_phase_change).count() > 3000) {
                    current_phase = PRE_SNAP;
                    last_phase_change = steady_clock::now();
                    game.home_has_ball = !game.home_has_ball; // Switch possession after score
                    
                    // Reset football position
                    football.x = 16.0f;
                    football.y = 16.0f;
                    football.dx = 0.0f;
                    football.dy = 0.0f;
                    
                    // Reset player positions
                    players.clear();
                    // Reinitialize teams with new possession
                    for (int i = 0; i < 7; ++i) {
                        FootballElement player;
                        player.x = 10.0f + (i % 3) * 3.0f;
                        player.y = 8.0f + (i / 3) * 8.0f;
                        player.dx = 0.0f;
                        player.dy = 0.0f;
                        player.active = true;
                        player.color = game.home_has_ball ? home_red : away_blue;
                        players.push_back(player);
                    }
                    for (int i = 0; i < 7; ++i) {
                        FootballElement player;
                        player.x = 20.0f + (i % 3) * 3.0f;
                        player.y = 8.0f + (i / 3) * 8.0f;
                        player.dx = 0.0f;
                        player.dy = 0.0f;
                        player.active = true;
                        player.color = game.home_has_ball ? away_blue : home_red;
                        players.push_back(player);
                    }
                }
                break;
                
            case TURNOVER:
                // Show turnover state briefly
                if (duration_cast<milliseconds>(steady_clock::now() - last_phase_change).count() > 1500) {
                    current_phase = PRE_SNAP;
                    last_phase_change = steady_clock::now();
                    
                    // Reset positions for new possession
                    football.x = 16.0f;
                    football.y = 16.0f;
                    football.dx = 0.0f;
                    football.dy = 0.0f;
                }
                break;
        }

        // Draw players
        for (size_t i = 0; i < players.size(); ++i) {
            const FootballElement& player = players[i];
            DrawCircle(canvas, (int)player.x, (int)player.y, 1, player.color);
        }

        // Draw football
        if (football.active) {
            // Draw football as brown oval
            canvas->SetPixel((int)football.x, (int)football.y, 
                           football.color.r, football.color.g, football.color.b);
            canvas->SetPixel((int)football.x + 1, (int)football.y, 
                           football.color.r, football.color.g, football.color.b);
            canvas->SetPixel((int)football.x - 1, (int)football.y, 
                           football.color.r, football.color.g, football.color.b);
        }

        // Draw celebration particles
        for (size_t i = 0; i < celebration.size(); ) {
            FootballElement& particle = celebration[i];
            particle.x += particle.dx;
            particle.y += particle.dy;
            
            if (particle.x >= 0 && particle.x < 32 && particle.y >= 0 && particle.y < 32) {
                canvas->SetPixel((int)particle.x, (int)partball.y, 
                               particle.color.r, particle.color.g, particle.color.b);
                ++i;
            } else {
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

    std::cout << "\nFootball display cleared. Exiting gracefully.\n";
    return 0;
}
