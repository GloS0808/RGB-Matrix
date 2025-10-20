#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
using namespace rgb_matrix;

enum Scene {
    GRAVEYARD,
    TRANSITION_1,
    HAND_EMERGING,
    TRANSITION_2,
    ZOMBIE_STANDING
};

struct Particle {
    float x;
    float y;
    float speed;
    bool active;
};

int main() {
    srand(time(NULL));
    
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
    Color sky_night(10, 5, 30);
    Color moon(255, 220, 150);
    Color ground(25, 20, 15);
    Color tombstone_gray(100, 100, 110);
    Color tombstone_dark(60, 60, 70);
    Color grass_dead(30, 40, 20);
    Color zombie_green(100, 140, 80);
    Color zombie_dark(60, 90, 50);
    Color bone_white(220, 220, 200);
    Color dirt_brown(70, 50, 30);
    Color mist_purple(80, 60, 100);
    
    // Mist particles
    const int num_mist = 10;
    Particle mist[num_mist];
    for (int i = 0; i < num_mist; ++i) {
        mist[i].x = rand() % 32;
        mist[i].y = 20 + rand() % 10;
        mist[i].speed = 0.05f + (rand() % 5) / 50.0f;
        mist[i].active = true;
    }
    
    Scene current_scene = GRAVEYARD;
    int frame_count = 0;
    int scene_timer = 0;
    float hand_progress = 0.0f;
    float transition_alpha = 0.0f;
    
    while (true) {
        scene_timer++;
        
        // Scene transitions
        if (current_scene == GRAVEYARD && scene_timer > 120) {
            current_scene = TRANSITION_1;
            scene_timer = 0;
        } else if (current_scene == TRANSITION_1 && scene_timer > 20) {
            current_scene = HAND_EMERGING;
            scene_timer = 0;
            hand_progress = 0.0f;
        } else if (current_scene == HAND_EMERGING && scene_timer > 100) {
            current_scene = TRANSITION_2;
            scene_timer = 0;
        } else if (current_scene == TRANSITION_2 && scene_timer > 20) {
            current_scene = ZOMBIE_STANDING;
            scene_timer = 0;
        } else if (current_scene == ZOMBIE_STANDING && scene_timer > 120) {
            current_scene = GRAVEYARD;
            scene_timer = 0;
        }
        
        // Calculate transition
        if (current_scene == TRANSITION_1 || current_scene == TRANSITION_2) {
            transition_alpha = scene_timer / 20.0f;
        }
        
        // SCENE 1: GRAVEYARD
        if (current_scene == GRAVEYARD || current_scene == TRANSITION_1) {
            float alpha = (current_scene == TRANSITION_1) ? (1.0f - transition_alpha) : 1.0f;
            
            // Night sky
            canvas->Fill(sky_night.r, sky_night.g, sky_night.b);
            
            // Full moon
            for (int y = 4; y <= 9; ++y) {
                for (int x = 22; x <= 27; ++x) {
                    int dx = x - 24;
                    int dy = y - 6;
                    if (dx*dx + dy*dy <= 9) {
                        canvas->SetPixel(x, y,
                                       (int)(moon.r * alpha),
                                       (int)(moon.g * alpha),
                                       (int)(moon.b * alpha));
                    }
                }
            }
            
            // Ground
            for (int y = 22; y < 32; ++y) {
                for (int x = 0; x < 32; ++x) {
                    canvas->SetPixel(x, y,
                                   (int)(ground.r * alpha),
                                   (int)(ground.g * alpha),
                                   (int)(ground.b * alpha));
                }
            }
            
            // Dead grass
            for (int x = 0; x < 32; x += 3) {
                int offset = (frame_count / 20 + x) % 3;
                canvas->SetPixel(x + offset, 21,
                               (int)(grass_dead.r * alpha),
                               (int)(grass_dead.g * alpha),
                               (int)(grass_dead.b * alpha));
            }
            
            // Three tombstones
            // Left tombstone
            for (int y = 18; y <= 26; ++y) {
                for (int x = 6; x <= 9; ++x) {
                    canvas->SetPixel(x, y,
                                   (int)(tombstone_gray.r * alpha),
                                   (int)(tombstone_gray.g * alpha),
                                   (int)(tombstone_gray.b * alpha));
                }
            }
            canvas->SetPixel(7, 17, (int)(tombstone_gray.r * alpha), (int)(tombstone_gray.g * alpha), (int)(tombstone_gray.b * alpha));
            canvas->SetPixel(8, 17, (int)(tombstone_gray.r * alpha), (int)(tombstone_gray.g * alpha), (int)(tombstone_gray.b * alpha));
            
            // Center tombstone (taller, with cross)
            for (int y = 15; y <= 26; ++y) {
                for (int x = 14; x <= 17; ++x) {
                    canvas->SetPixel(x, y,
                                   (int)(tombstone_gray.r * alpha),
                                   (int)(tombstone_gray.g * alpha),
                                   (int)(tombstone_gray.b * alpha));
                }
            }
            // Cross
            canvas->SetPixel(15, 13, (int)(tombstone_gray.r * alpha), (int)(tombstone_gray.g * alpha), (int)(tombstone_gray.b * alpha));
            canvas->SetPixel(16, 13, (int)(tombstone_gray.r * alpha), (int)(tombstone_gray.g * alpha), (int)(tombstone_gray.b * alpha));
            canvas->SetPixel(15, 14, (int)(tombstone_gray.r * alpha), (int)(tombstone_gray.g * alpha), (int)(tombstone_gray.b * alpha));
            canvas->SetPixel(16, 14, (int)(tombstone_gray.r * alpha), (int)(tombstone_gray.g * alpha), (int)(tombstone_gray.b * alpha));
            canvas->SetPixel(14, 14, (int)(tombstone_gray.r * alpha), (int)(tombstone_gray.g * alpha), (int)(tombstone_gray.b * alpha));
            canvas->SetPixel(17, 14, (int)(tombstone_gray.r * alpha), (int)(tombstone_gray.g * alpha), (int)(tombstone_gray.b * alpha));
            
            // Right tombstone
            for (int y = 19; y <= 26; ++y) {
                for (int x = 23; x <= 26; ++x) {
                    canvas->SetPixel(x, y,
                                   (int)(tombstone_gray.r * alpha),
                                   (int)(tombstone_gray.g * alpha),
                                   (int)(tombstone_gray.b * alpha));
                }
            }
            canvas->SetPixel(24, 18, (int)(tombstone_gray.r * alpha), (int)(tombstone_gray.g * alpha), (int)(tombstone_gray.b * alpha));
            canvas->SetPixel(25, 18, (int)(tombstone_gray.r * alpha), (int)(tombstone_gray.g * alpha), (int)(tombstone_gray.b * alpha));
            
            // Mist
            for (int i = 0; i < num_mist; ++i) {
                mist[i].x += mist[i].speed;
                if (mist[i].x > 32) mist[i].x = -2;
                
                int mx = (int)mist[i].x;
                int my = (int)mist[i].y;
                if (mx >= 0 && mx < 32 && my >= 0 && my < 32) {
                    canvas->SetPixel(mx, my,
                                   (int)(mist_purple.r * alpha),
                                   (int)(mist_purple.g * alpha),
                                   (int)(mist_purple.b * alpha));
                }
            }
        }
        
        // SCENE 2: HAND EMERGING
        if (current_scene == HAND_EMERGING || 
            current_scene == TRANSITION_1 || 
            current_scene == TRANSITION_2) {
            
            float alpha = 1.0f;
            if (current_scene == TRANSITION_1) alpha = transition_alpha;
            if (current_scene == TRANSITION_2) alpha = 1.0f - transition_alpha;
            
            if (current_scene == HAND_EMERGING) {
                hand_progress += 0.015f;
                if (hand_progress > 1.0f) hand_progress = 1.0f;
            }
            
            // Dark background
            for (int y = 0; y < 32; ++y) {
                for (int x = 0; x < 32; ++x) {
                    canvas->SetPixel(x, y,
                                   (int)(sky_night.r * alpha),
                                   (int)(sky_night.g * alpha),
                                   (int)(sky_night.b * alpha));
                }
            }
            
            // Ground close-up
            for (int y = 16; y < 32; ++y) {
                for (int x = 0; x < 32; ++x) {
                    canvas->SetPixel(x, y,
                                   (int)(dirt_brown.r * alpha),
                                   (int)(dirt_brown.g * alpha),
                                   (int)(dirt_brown.b * alpha));
                }
            }
            
            // Cracks in ground
            int crack_y = 16;
            for (int x = 10; x < 22; ++x) {
                canvas->SetPixel(x, crack_y, (int)(ground.r * alpha), (int)(ground.g * alpha), (int)(ground.b * alpha));
            }
            canvas->SetPixel(11, crack_y + 1, (int)(ground.r * alpha), (int)(ground.g * alpha), (int)(ground.b * alpha));
            canvas->SetPixel(20, crack_y + 1, (int)(ground.r * alpha), (int)(ground.g * alpha), (int)(ground.b * alpha));
            
            // Emerging hand (rises based on progress)
            int hand_base_y = 16 - (int)(hand_progress * 6);
            int hand_x = 16;
            
            // Wrist/forearm
            for (int y = 0; y < 4; ++y) {
                int wy = hand_base_y + 6 + y;
                if (wy >= 0 && wy < 32) {
                    canvas->SetPixel(hand_x - 1, wy,
                                   (int)(zombie_green.r * alpha),
                                   (int)(zombie_green.g * alpha),
                                   (int)(zombie_green.b * alpha));
                    canvas->SetPixel(hand_x, wy,
                                   (int)(zombie_green.r * alpha),
                                   (int)(zombie_green.g * alpha),
                                   (int)(zombie_green.b * alpha));
                }
            }
            
            // Palm
            for (int y = 0; y < 3; ++y) {
                int py = hand_base_y + 3 + y;
                if (py >= 0 && py < 32) {
                    for (int x = -1; x <= 1; ++x) {
                        canvas->SetPixel(hand_x + x, py,
                                       (int)(zombie_green.r * alpha),
                                       (int)(zombie_green.g * alpha),
                                       (int)(zombie_green.b * alpha));
                    }
                }
            }
            
            // Fingers (reaching up)
            // Middle finger (longest)
            for (int y = 0; y < 4; ++y) {
                int fy = hand_base_y + y;
                if (fy >= 0 && fy < 32) {
                    canvas->SetPixel(hand_x, fy,
                                   (int)(zombie_green.r * alpha),
                                   (int)(zombie_green.g * alpha),
                                   (int)(zombie_green.b * alpha));
                }
            }
            
            // Index finger
            for (int y = 1; y < 4; ++y) {
                int fy = hand_base_y + y;
                if (fy >= 0 && fy < 32) {
                    canvas->SetPixel(hand_x - 1, fy,
                                   (int)(zombie_dark.r * alpha),
                                   (int)(zombie_dark.g * alpha),
                                   (int)(zombie_dark.b * alpha));
                }
            }
            
            // Ring finger
            for (int y = 1; y < 4; ++y) {
                int fy = hand_base_y + y;
                if (fy >= 0 && fy < 32) {
                    canvas->SetPixel(hand_x + 1, fy,
                                   (int)(zombie_dark.r * alpha),
                                   (int)(zombie_dark.g * alpha),
                                   (int)(zombie_dark.b * alpha));
                }
            }
            
            // Thumb
            int thumb_y = hand_base_y + 4;
            if (thumb_y >= 0 && thumb_y < 32) {
                canvas->SetPixel(hand_x - 2, thumb_y,
                               (int)(zombie_dark.r * alpha),
                               (int)(zombie_dark.g * alpha),
                               (int)(zombie_dark.b * alpha));
                canvas->SetPixel(hand_x - 2, thumb_y + 1,
                               (int)(zombie_dark.r * alpha),
                               (int)(zombie_dark.g * alpha),
                               (int)(zombie_dark.b * alpha));
            }
            
            // Bone showing through (knuckle)
            if (hand_base_y + 5 >= 0 && hand_base_y + 5 < 32) {
                canvas->SetPixel(hand_x, hand_base_y + 5,
                               (int)(bone_white.r * alpha),
                               (int)(bone_white.g * alpha),
                               (int)(bone_white.b * alpha));
            }
        }
        
        // SCENE 3: ZOMBIE STANDING
        if (current_scene == ZOMBIE_STANDING || current_scene == TRANSITION_2) {
            float alpha = (current_scene == TRANSITION_2) ? transition_alpha : 1.0f;
            
            // Night sky
            canvas->Fill((int)(sky_night.r * alpha),
                        (int)(sky_night.g * alpha),
                        (int)(sky_night.b * alpha));
            
            // Moon (eerie)
            for (int y = 3; y <= 7; ++y) {
                for (int x = 4; x <= 8; ++x) {
                    int dx = x - 6;
                    int dy = y - 5;
                    if (dx*dx + dy*dy <= 6) {
                        int flicker = (frame_count / 10) % 2;
                        int brightness = flicker ? 255 : 200;
                        canvas->SetPixel(x, y,
                                       (int)(brightness * alpha),
                                       (int)((brightness - 50) * alpha),
                                       (int)((brightness - 100) * alpha));
                    }
                }
            }
            
            // Ground
            for (int y = 24; y < 32; ++y) {
                for (int x = 0; x < 32; ++x) {
                    canvas->SetPixel(x, y,
                                   (int)(ground.r * alpha),
                                   (int)(ground.g * alpha),
                                   (int)(ground.b * alpha));
                }
            }
            
            // Full zombie figure (center)
            int zx = 16;
            int zy = 24;
            
            // Legs
            canvas->SetPixel(zx - 1, zy, (int)(zombie_dark.r * alpha), (int)(zombie_dark.g * alpha), (int)(zombie_dark.b * alpha));
            canvas->SetPixel(zx + 1, zy, (int)(zombie_dark.r * alpha), (int)(zombie_dark.g * alpha), (int)(zombie_dark.b * alpha));
            canvas->SetPixel(zx - 1, zy + 1, (int)(zombie_dark.r * alpha), (int)(zombie_dark.g * alpha), (int)(zombie_dark.b * alpha));
            canvas->SetPixel(zx + 1, zy + 1, (int)(zombie_dark.r * alpha), (int)(zombie_dark.g * alpha), (int)(zombie_dark.b * alpha));
            
            // Body
            for (int y = -6; y <= -1; ++y) {
                for (int x = -1; x <= 1; ++x) {
                    canvas->SetPixel(zx + x, zy + y,
                                   (int)(zombie_green.r * alpha),
                                   (int)(zombie_green.g * alpha),
                                   (int)(zombie_green.b * alpha));
                }
            }
            
            // Arms (reaching forward menacingly)
            bool arms_up = (frame_count / 15) % 2 == 0;
            if (arms_up) {
                // Left arm up
                canvas->SetPixel(zx - 2, zy - 5, (int)(zombie_green.r * alpha), (int)(zombie_green.g * alpha), (int)(zombie_green.b * alpha));
                canvas->SetPixel(zx - 2, zy - 4, (int)(zombie_green.r * alpha), (int)(zombie_green.g * alpha), (int)(zombie_green.b * alpha));
                canvas->SetPixel(zx - 3, zy - 4, (int)(zombie_dark.r * alpha), (int)(zombie_dark.g * alpha), (int)(zombie_dark.b * alpha));
                // Right arm up
                canvas->SetPixel(zx + 2, zy - 5, (int)(zombie_green.r * alpha), (int)(zombie_green.g * alpha), (int)(zombie_green.b * alpha));
                canvas->SetPixel(zx + 2, zy - 4, (int)(zombie_green.r * alpha), (int)(zombie_green.g * alpha), (int)(zombie_green.b * alpha));
                canvas->SetPixel(zx + 3, zy - 4, (int)(zombie_dark.r * alpha), (int)(zombie_dark.g * alpha), (int)(zombie_dark.b * alpha));
            } else {
                // Arms forward
                canvas->SetPixel(zx - 2, zy - 4, (int)(zombie_green.r * alpha), (int)(zombie_green.g * alpha), (int)(zombie_green.b * alpha));
                canvas->SetPixel(zx - 3, zy - 4, (int)(zombie_dark.r * alpha), (int)(zombie_dark.g * alpha), (int)(zombie_dark.b * alpha));
                canvas->SetPixel(zx + 2, zy - 4, (int)(zombie_green.r * alpha), (int)(zombie_green.g * alpha), (int)(zombie_green.b * alpha));
                canvas->SetPixel(zx + 3, zy - 4, (int)(zombie_dark.r * alpha), (int)(zombie_dark.g * alpha), (int)(zombie_dark.b * alpha));
            }
            
            // Head
            canvas->SetPixel(zx - 1, zy - 7, (int)(zombie_green.r * alpha), (int)(zombie_green.g * alpha), (int)(zombie_green.b * alpha));
            canvas->SetPixel(zx, zy - 7, (int)(zombie_green.r * alpha), (int)(zombie_green.g * alpha), (int)(zombie_green.b * alpha));
            canvas->SetPixel(zx + 1, zy - 7, (int)(zombie_green.r * alpha), (int)(zombie_green.g * alpha), (int)(zombie_green.b * alpha));
            canvas->SetPixel(zx - 1, zy - 8, (int)(zombie_green.r * alpha), (int)(zombie_green.g * alpha), (int)(zombie_green.b * alpha));
            canvas->SetPixel(zx, zy - 8, (int)(zombie_green.r * alpha), (int)(zombie_green.g * alpha), (int)(zombie_green.b * alpha));
            canvas->SetPixel(zx + 1, zy - 8, (int)(zombie_green.r * alpha), (int)(zombie_green.g * alpha), (int)(zombie_green.b * alpha));
            
            // Glowing red eyes
            canvas->SetPixel(zx - 1, zy - 7, (int)(255 * alpha), (int)(50 * alpha), (int)(50 * alpha));
            canvas->SetPixel(zx + 1, zy - 7, (int)(255 * alpha), (int)(50 * alpha), (int)(50 * alpha));
            
            // Torn clothes
            canvas->SetPixel(zx, zy - 3, (int)(100 * alpha), (int)(80 * alpha), (int)(70 * alpha));
            canvas->SetPixel(zx - 1, zy - 2, (int)(100 * alpha), (int)(80 * alpha), (int)(70 * alpha));
        }
        
        frame_count++;
        canvas = matrix->SwapOnVSync(canvas);
        usleep(50000);  // ~20 fps
    }
    
    delete matrix;
    return 0;
}
