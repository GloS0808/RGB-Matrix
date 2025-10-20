#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <cmath>
using namespace rgb_matrix;

struct Particle {
    float x;
    float y;
    float vx;
    float vy;
    float lifetime;
    int brightness;
};

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
    
    // Particles for light rays
    const int num_particles = 40;
    Particle particles[num_particles];
    for (int i = 0; i < num_particles; ++i) {
        particles[i].x = 16.0f;
        particles[i].y = 16.0f;
        particles[i].vx = (rand() % 200 - 100) / 100.0f;
        particles[i].vy = (rand() % 200 - 100) / 100.0f;
        particles[i].lifetime = rand() % 100;
        particles[i].brightness = 255;
    }
    
    int frame_count = 0;
    float center_x = 16.0f;
    float center_y = 16.0f;
    
    // Display continuously
    while (true) {
        float time = frame_count * 0.05f;
        float pulse = sin(time) * 0.5f + 0.5f;
        float breath = sin(time * 0.5f) * 0.5f + 0.5f;
        
        // Create gradient background - dark to light (journey theme)
        for (int y = 0; y < 32; ++y) {
            for (int x = 0; x < 32; ++x) {
                // Horizontal gradient - darkness on left, light on right
                float gradient = (x / 32.0f) * 0.3f;
                
                // Distance from center for radial glow
                float dx = x - center_x;
                float dy = y - center_y;
                float dist = sqrt(dx*dx + dy*dy);
                float radial_glow = 1.0f - (dist / 23.0f);
                if (radial_glow < 0) radial_glow = 0;
                
                // Combine with pulsing
                float total_brightness = (gradient + radial_glow * pulse * 0.5f);
                if (total_brightness > 1.0f) total_brightness = 1.0f;
                
                // Warm colors - amber/gold tones for "walking toward light"
                int r = (int)(total_brightness * 80 + pulse * 30);
                int g = (int)(total_brightness * 60 + pulse * 20);
                int b = (int)(total_brightness * 20);
                
                canvas->SetPixel(x, y, r, g, b);
            }
        }
        
        // Draw central light source (the destination)
        int light_radius = (int)(8.0f + breath * 4.0f);
        for (int y = -light_radius; y <= light_radius; ++y) {
            for (int x = -light_radius; x <= light_radius; ++x) {
                float dist = sqrt(x*x + y*y);
                if (dist <= light_radius) {
                    int px = (int)center_x + x;
                    int py = (int)center_y + y;
                    if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                        // Bright center fading outward
                        float intensity = 1.0f - (dist / light_radius);
                        intensity = intensity * intensity; // Squared for sharper falloff
                        
                        // Warm white/golden light
                        int r = (int)(255 * intensity);
                        int g = (int)(240 * intensity);
                        int b = (int)(180 * intensity);
                        
                        canvas->SetPixel(px, py, r, g, b);
                    }
                }
            }
        }
        
        // Update and draw light particles (photons radiating outward)
        for (int i = 0; i < num_particles; ++i) {
            particles[i].lifetime++;
            
            // Reset particle if it's too old or off screen
            if (particles[i].lifetime > 60 || 
                particles[i].x < 0 || particles[i].x > 31 ||
                particles[i].y < 0 || particles[i].y > 31) {
                
                particles[i].x = center_x;
                particles[i].y = center_y;
                
                // Radiate outward in all directions
                float angle = (rand() % 360) * M_PI / 180.0f;
                float speed = 0.5f + (rand() % 100) / 100.0f;
                particles[i].vx = cos(angle) * speed;
                particles[i].vy = sin(angle) * speed;
                particles[i].lifetime = 0;
                particles[i].brightness = 200 + rand() % 55;
            }
            
            // Move particle
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            
            // Draw particle with fading trail
            int px = (int)particles[i].x;
            int py = (int)particles[i].y;
            
            if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                float fade = 1.0f - (particles[i].lifetime / 60.0f);
                int brightness = (int)(particles[i].brightness * fade);
                
                // Golden/white particles
                canvas->SetPixel(px, py, brightness, brightness * 0.9f, brightness * 0.7f);
            }
        }
        
        // Draw path/rays pointing toward center (guiding lines)
        int num_rays = 12;
        for (int i = 0; i < num_rays; ++i) {
            float angle = (i * 2.0f * M_PI / num_rays) + time * 0.2f;
            
            // Dotted lines pointing inward
            for (int r = 18; r < 30; r += 3) {
                int px = (int)(center_x + cos(angle) * r);
                int py = (int)(center_y + sin(angle) * r);
                
                if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                    // Subtle guiding rays
                    int brightness = (int)(80 * pulse);
                    canvas->SetPixel(px, py, brightness, brightness * 0.8f, brightness * 0.5f);
                }
            }
        }
        
        // Beacon effect - bright flashes from center
        if ((frame_count % 60) < 5) {
            float beacon_intensity = 1.0f - ((frame_count % 60) / 5.0f);
            int beacon_radius = 4 + (frame_count % 60);
            
            for (int y = -beacon_radius; y <= beacon_radius; ++y) {
                for (int x = -beacon_radius; x <= beacon_radius; ++x) {
                    if (x*x + y*y <= beacon_radius * beacon_radius) {
                        int px = (int)center_x + x;
                        int py = (int)center_y + y;
                        if (px >= 0 && px < 32 && py >= 0 && py < 32) {
                            int bright = (int)(255 * beacon_intensity);
                            canvas->SetPixel(px, py, bright, bright, bright * 0.8f);
                        }
                    }
                }
            }
        }
        
        frame_count++;
        canvas = matrix->SwapOnVSync(canvas);
        usleep(50000);  // ~20 fps
    }
    
    delete matrix;
    return 0;
}
