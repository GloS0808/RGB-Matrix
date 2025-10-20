// Arcade Game Scene - Space Invaders Style
// Compilation: g++ -o arcade arcade.cpp -lrgbmatrix -std=c++11

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

struct Invader {
    int x;
    int y;
    int type; // 0, 1, 2 for different alien types
    bool alive;
    int animation_frame;
};

struct Bullet {
    float x;
    float y;
    float vy;
    bool is_player; // true = player bullet, false = enemy bullet
};

struct Explosion {
    int x;
    int y;
    int life;
    int max_life;
};

struct Star {
    int x;
    int y;
    int brightness;
    float twinkle_phase;
};

struct Player {
    int x;
    int y;
    int lives;
    int direction; // -1 left, 0 stop, 1 right
};

class ArcadeScene {
private:
    RGBMatrix *matrix;
    FrameCanvas *canvas;
    int width, height;
    std::vector<Invader> invaders;
    std::vector<Bullet> bullets;
    std::vector<Explosion> explosions;
    std::vector<Star> stars;
    Player player;
    float time_counter;
    int invader_direction; // 1 = right, -1 = left
    int invader_drop_counter;
    int score;
    int frame_counter;
    
    // Colors
    Color space_black = Color(5, 5, 15);
    Color invader_green = Color(50, 220, 50);
    Color invader_blue = Color(50, 150, 250);
    Color invader_purple = Color(200, 50, 200);
    Color player_cyan = Color(50, 250, 250);
    Color bullet_yellow = Color(250, 250, 100);
    Color bullet_red = Color(250, 80, 80);
    Color explosion_orange = Color(250, 150, 50);
    Color explosion_yellow = Color(250, 250, 100);
    Color star_white = Color(200, 200, 220);
    
public:
    ArcadeScene(RGBMatrix *m) : matrix(m), time_counter(0), invader_direction(1), 
                                 invader_drop_counter(0), score(0), frame_counter(0) {
        canvas = matrix->CreateFrameCanvas();
        width = canvas->width();
        height = canvas->height();
        
        srand(time(NULL));
        
        // Create invader formation
        int start_x = 8;
        int start_y = 3;
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 8; col++) {
                Invader inv;
                inv.x = start_x + col * 6;
                inv.y = start_y + row * 5;
                inv.type = row / 2; // Different types by row
                inv.alive = true;
                inv.animation_frame = 0;
                invaders.push_back(inv);
            }
        }
        
        // Create player
        player.x = width / 2;
        player.y = height - 4;
        player.lives = 3;
        player.direction = 0;
        
        // Create starfield
        for (int i = 0; i < 30; i++) {
            Star s;
            s.x = rand() % width;
            s.y = rand() % height;
            s.brightness = 100 + rand() % 155;
            s.twinkle_phase = (float)rand() / RAND_MAX * 2 * M_PI;
            stars.push_back(s);
        }
    }
    
    Color getInvaderColor(int type) {
        switch(type) {
            case 0: return invader_green;
            case 1: return invader_blue;
            default: return invader_purple;
        }
    }
    
    void drawBackground() {
        // Space background
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                canvas->SetPixel(x, y, space_black.r, space_black.g, space_black.b);
            }
        }
        
        // Twinkling stars
        for (auto& star : stars) {
            star.twinkle_phase += 0.05;
            float twinkle = sin(star.twinkle_phase) * 0.3 + 0.7;
            int brightness = star.brightness * twinkle;
            
            if (star.x >= 0 && star.x < width && star.y >= 0 && star.y < height) {
                canvas->SetPixel(star.x, star.y, brightness, brightness, brightness);
            }
        }
    }
    
    void drawInvader(int x, int y, int type, int frame) {
        Color invader_color = getInvaderColor(type);
        
        // Different invader designs based on type
        if (type == 0) {
            // Squid alien
            if (frame % 2 == 0) {
                // Frame 1
                if (y >= 0 && y < height) {
                    if (x >= 0 && x < width) canvas->SetPixel(x, y, invader_color.r, invader_color.g, invader_color.b);
                    if (x + 2 >= 0 && x + 2 < width) canvas->SetPixel(x + 2, y, invader_color.r, invader_color.g, invader_color.b);
                }
                if (y + 1 < height) {
                    if (x >= 0 && x < width) canvas->SetPixel(x, y + 1, invader_color.r, invader_color.g, invader_color.b);
                    if (x + 1 >= 0 && x + 1 < width) canvas->SetPixel(x + 1, y + 1, invader_color.r, invader_color.g, invader_color.b);
                    if (x + 2 >= 0 && x + 2 < width) canvas->SetPixel(x + 2, y + 1, invader_color.r, invader_color.g, invader_color.b);
                }
                if (y + 2 < height) {
                    if (x + 1 >= 0 && x + 1 < width) canvas->SetPixel(x + 1, y + 2, invader_color.r, invader_color.g, invader_color.b);
                }
            } else {
                // Frame 2
                if (y >= 0 && y < height) {
                    if (x + 1 >= 0 && x + 1 < width) canvas->SetPixel(x + 1, y, invader_color.r, invader_color.g, invader_color.b);
                }
                if (y + 1 < height) {
                    if (x >= 0 && x < width) canvas->SetPixel(x, y + 1, invader_color.r, invader_color.g, invader_color.b);
                    if (x + 1 >= 0 && x + 1 < width) canvas->SetPixel(x + 1, y + 1, invader_color.r, invader_color.g, invader_color.b);
                    if (x + 2 >= 0 && x + 2 < width) canvas->SetPixel(x + 2, y + 1, invader_color.r, invader_color.g, invader_color.b);
                }
                if (y + 2 < height) {
                    if (x >= 0 && x < width) canvas->SetPixel(x, y + 2, invader_color.r, invader_color.g, invader_color.b);
                    if (x + 2 >= 0 && x + 2 < width) canvas->SetPixel(x + 2, y + 2, invader_color.r, invader_color.g, invader_color.b);
                }
            }
        } else {
            // Simple alien (crab/octopus)
            if (y >= 0 && y < height) {
                if (x >= 0 && x < width) canvas->SetPixel(x, y, invader_color.r, invader_color.g, invader_color.b);
                if (x + 1 >= 0 && x + 1 < width) canvas->SetPixel(x + 1, y, invader_color.r, invader_color.g, invader_color.b);
                if (x + 2 >= 0 && x + 2 < width) canvas->SetPixel(x + 2, y, invader_color.r, invader_color.g, invader_color.b);
            }
            if (y + 1 < height && frame % 2 == 0) {
                if (x >= 0 && x < width) canvas->SetPixel(x, y + 1, invader_color.r, invader_color.g, invader_color.b);
                if (x + 2 >= 0 && x + 2 < width) canvas->SetPixel(x + 2, y + 1, invader_color.r, invader_color.g, invader_color.b);
            } else if (y + 1 < height) {
                if (x + 1 >= 0 && x + 1 < width) canvas->SetPixel(x + 1, y + 1, invader_color.r, invader_color.g, invader_color.b);
            }
        }
    }
    
    void drawPlayer() {
        // Player ship (tank-like)
        int px = player.x;
        int py = player.y;
        
        // Cannon
        if (py - 1 >= 0 && px >= 0 && px < width) {
            canvas->SetPixel(px, py - 1, player_cyan.r, player_cyan.g, player_cyan.b);
        }
        
        // Body
        if (py >= 0 && py < height) {
            if (px - 1 >= 0) canvas->SetPixel(px - 1, py, player_cyan.r, player_cyan.g, player_cyan.b);
            if (px >= 0 && px < width) canvas->SetPixel(px, py, player_cyan.r * 1.2, player_cyan.g * 1.2, player_cyan.b * 1.2);
            if (px + 1 < width) canvas->SetPixel(px + 1, py, player_cyan.r, player_cyan.g, player_cyan.b);
        }
        
        // Base
        if (py + 1 < height) {
            if (px - 2 >= 0) canvas->SetPixel(px - 2, py + 1, player_cyan.r * 0.7, player_cyan.g * 0.7, player_cyan.b * 0.7);
            if (px - 1 >= 0) canvas->SetPixel(px - 1, py + 1, player_cyan.r * 0.7, player_cyan.g * 0.7, player_cyan.b * 0.7);
            if (px >= 0 && px < width) canvas->SetPixel(px, py + 1, player_cyan.r * 0.7, player_cyan.g * 0.7, player_cyan.b * 0.7);
            if (px + 1 < width) canvas->SetPixel(px + 1, py + 1, player_cyan.r * 0.7, player_cyan.g * 0.7, player_cyan.b * 0.7);
            if (px + 2 < width) canvas->SetPixel(px + 2, py + 1, player_cyan.r * 0.7, player_cyan.g * 0.7, player_cyan.b * 0.7);
        }
    }
    
    void updateInvaders() {
        frame_counter++;
        
        // Animate invaders
        if (frame_counter % 20 == 0) {
            for (auto& inv : invaders) {
                inv.animation_frame++;
            }
        }
        
        // Move invaders
        if (frame_counter % 30 == 0) {
            bool should_drop = false;
            
            // Check if any invader hit edge
            for (auto& inv : invaders) {
                if (inv.alive) {
                    if ((inv.x + invader_direction * 2 < 2) || (inv.x + invader_direction * 2 > width - 5)) {
                        should_drop = true;
                        break;
                    }
                }
            }
            
            if (should_drop) {
                invader_direction *= -1;
                for (auto& inv : invaders) {
                    if (inv.alive) {
                        inv.y += 2;
                    }
                }
            } else {
                for (auto& inv : invaders) {
                    if (inv.alive) {
                        inv.x += invader_direction * 2;
                    }
                }
            }
        }
        
        // Draw invaders
        for (auto& inv : invaders) {
            if (inv.alive) {
                drawInvader(inv.x, inv.y, inv.type, inv.animation_frame);
            }
        }
        
        // Invaders shoot randomly
        if (frame_counter % 60 == 0 && rand() % 3 == 0) {
            // Pick a random alive invader
            std::vector<int> alive_indices;
            for (size_t i = 0; i < invaders.size(); i++) {
                if (invaders[i].alive) {
                    alive_indices.push_back(i);
                }
            }
            
            if (!alive_indices.empty()) {
                int idx = alive_indices[rand() % alive_indices.size()];
                Bullet b;
                b.x = invaders[idx].x + 1;
                b.y = invaders[idx].y + 3;
                b.vy = 0.5;
                b.is_player = false;
                bullets.push_back(b);
            }
        }
    }
    
    void updatePlayer() {
        // Auto-move player
        if (frame_counter % 40 == 0) {
            player.direction = (rand() % 3) - 1; // -1, 0, or 1
        }
        
        player.x += player.direction;
        
        // Keep player in bounds
        if (player.x < 3) player.x = 3;
        if (player.x > width - 3) player.x = width - 3;
        
        drawPlayer();
        
        // Player shoots periodically
        if (frame_counter % 45 == 0) {
            Bullet b;
            b.x = player.x;
            b.y = player.y - 2;
            b.vy = -1.0;
            b.is_player = true;
            bullets.push_back(b);
        }
    }
    
    void updateBullets() {
        for (auto it = bullets.begin(); it != bullets.end();) {
            it->y += it->vy;
            
            // Draw bullet
            int bx = (int)it->x;
            int by = (int)it->y;
            
            if (by >= 0 && by < height && bx >= 0 && bx < width) {
                if (it->is_player) {
                    canvas->SetPixel(bx, by, bullet_yellow.r, bullet_yellow.g, bullet_yellow.b);
                    if (by - 1 >= 0) {
                        canvas->SetPixel(bx, by - 1, bullet_yellow.r * 0.7, bullet_yellow.g * 0.7, bullet_yellow.b * 0.7);
                    }
                } else {
                    canvas->SetPixel(bx, by, bullet_red.r, bullet_red.g, bullet_red.b);
                }
            }
            
            // Check collisions
            bool hit = false;
            
            if (it->is_player) {
                // Check player bullet vs invaders
                for (auto& inv : invaders) {
                    if (inv.alive && bx >= inv.x && bx <= inv.x + 3 && by >= inv.y && by <= inv.y + 2) {
                        inv.alive = false;
                        hit = true;
                        score += 10;
                        
                        // Create explosion
                        Explosion ex;
                        ex.x = inv.x;
                        ex.y = inv.y;
                        ex.life = 0;
                        ex.max_life = 10;
                        explosions.push_back(ex);
                        break;
                    }
                }
            }
            
            // Remove if off screen or hit
            if (it->y < -2 || it->y > height + 2 || hit) {
                it = bullets.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    void updateExplosions() {
        for (auto it = explosions.begin(); it != explosions.end();) {
            it->life++;
            
            if (it->life < it->max_life) {
                float fade = 1.0 - (float)it->life / it->max_life;
                
                // Expanding explosion
                int radius = 1 + it->life / 3;
                for (int dy = -radius; dy <= radius; dy++) {
                    for (int dx = -radius; dx <= radius; dx++) {
                        if (dx*dx + dy*dy <= radius*radius) {
                            int px = it->x + dx;
                            int py = it->y + dy;
                            if (px >= 0 && px < width && py >= 0 && py < height) {
                                if (it->life < it->max_life / 2) {
                                    canvas->SetPixel(px, py, 
                                                   explosion_yellow.r * fade,
                                                   explosion_yellow.g * fade,
                                                   explosion_yellow.b * fade);
                                } else {
                                    canvas->SetPixel(px, py,
                                                   explosion_orange.r * fade,
                                                   explosion_orange.g * fade,
                                                   explosion_orange.b * fade);
                                }
                            }
                        }
                    }
                }
                
                ++it;
            } else {
                it = explosions.erase(it);
            }
        }
    }
    
    void drawScore() {
        // Simple score display as dots/bars at top
        int num_bars = std::min(score / 10, width - 2);
        for (int x = 1; x < 1 + num_bars && x < width - 1; x++) {
            canvas->SetPixel(x, 0, 100, 250, 100);
        }
    }
    
    void update() {
        canvas->Clear();
        
        drawBackground();
        updateInvaders();
        updatePlayer();
        updateBullets();
        updateExplosions();
        drawScore();
        
        canvas = matrix->SwapOnVSync(canvas);
        
        time_counter += 0.05;
    }
};

int main(int argc, char *argv[]) {
    RGBMatrix::Options matrix_options;
    rgb_matrix::RuntimeOptions runtime_opt;
    
    // Default settings
    matrix_options.rows = 32;
    matrix_options.cols = 32;
    matrix_options.chain_length = 1;
    matrix_options.parallel = 1;
    matrix_options.hardware_mapping = "adafruit-hat";
    
    // Parse flags
    if (!ParseOptionsFromFlags(&argc, &argv, &matrix_options, &runtime_opt)) {
        return 1;
    }
    
    // Create matrix
    RGBMatrix *matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
    if (matrix == NULL) {
        return 1;
    }
    
    // Set up interrupt handler
    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);
    
    ArcadeScene scene(matrix);
    
    while (!interrupt_received) {
        scene.update();
        usleep(50000); // 50ms = ~20 FPS
    }
    
    matrix->Clear();
    delete matrix;
    
    return 0;
}
