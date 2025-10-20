#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
using namespace rgb_matrix;

// Simple noise function for organic cloud patterns
float noise2d(float x, float y, int seed) {
    int ix = (int)x;
    int iy = (int)y;
    float fx = x - ix;
    float fy = y - iy;
    
    // Hash function for pseudo-random values
    int n = ix + iy * 57 + seed * 131;
    n = (n << 13) ^ n;
    float val = 1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f;
    
    return (val + 1.0f) * 0.5f; // Normalize to 0-1
}

float smoothNoise(float x, float y, int seed) {
    float corners = (noise2d(x-1, y-1, seed) + noise2d(x+1, y-1, seed) + 
                    noise2d(x-1, y+1, seed) + noise2d(x+1, y+1, seed)) / 16.0f;
    float sides = (noise2d(x-1, y, seed) + noise2d(x+1, y, seed) + 
                  noise2d(x, y-1, seed) + noise2d(x, y+1, seed)) / 8.0f;
    float center = noise2d(x, y, seed) / 4.0f;
    
    return corners + sides + center;
}

float turbulence(float x, float y, float size, int seed) {
    float value = 0.0f;
    float initialSize = size;
    
    while (size >= 1) {
        value += smoothNoise(x / size, y / size, seed) * size;
        size /= 2.0f;
    }
    
    return value / initialSize;
}

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
    
    int frame_count = 0;
    int seed = rand() % 1000;
    
    // Display continuously
    while (true) {
        float time = frame_count * 0.02f;
        
        // Draw nebula
        for (int y = 0; y < 32; ++y) {
            for (int x = 0; x < 32; ++x) {
                float fx = x + time * 2.0f;
                float fy = y + time * 1.5f;
                
                // Multi-layer turbulence for cloud-like appearance
                float cloud1 = turbulence(fx, fy, 32.0f, seed);
                float cloud2 = turbulence(fx * 0.5f, fy * 0.5f, 16.0f, seed + 1);
                float cloud3 = turbulence(fx * 2.0f, fy * 2.0f, 8.0f, seed + 2);
                
                // Combine layers
                float density = (cloud1 * 0.5f + cloud2 * 0.3f + cloud3 * 0.2f);
                density = density / 20.0f; // Normalize
                
                // Add distance from center for bright core
                float dx = x - 16.0f;
                float dy = y - 16.0f;
                float dist = sqrt(dx*dx + dy*dy);
                float core_brightness = 1.0f - (dist / 23.0f);
                if (core_brightness < 0) core_brightness = 0;
                core_brightness = core_brightness * core_brightness; // Sharper falloff
                
                // Combine density with core
                float total_density = density * 0.7f + core_brightness * 0.6f;
                if (total_density > 1.0f) total_density = 1.0f;
                
                // Color mapping - create nebula-like colors
                // Use density to determine color region
                int r, g, b;
                
                if (total_density < 0.2f) {
                    // Deep space - very dark with hints of purple
                    r = (int)(total_density * 50);
                    g = 0;
                    b = (int)(total_density * 80);
                } else if (total_density < 0.4f) {
                    // Purple/magenta regions
                    float t = (total_density - 0.2f) / 0.2f;
                    r = (int)(100 + t * 155);
                    g = (int)(t * 50);
                    b = (int)(150 + t * 105);
                } else if (total_density < 0.6f) {
                    // Pink/red regions
                    float t = (total_density - 0.4f) / 0.2f;
                    r = (int)(200 + t * 55);
                    g = (int)(50 + t * 100);
                    b = (int)(100 + t * 50);
                } else if (total_density < 0.8f) {
                    // Orange/yellow regions (hot gas)
                    float t = (total_density - 0.6f) / 0.2f;
                    r = (int)(255);
                    g = (int)(150 + t * 80);
                    b = (int)(50 + t * 50);
                } else {
                    // Bright core - white/cyan (hottest)
                    float t = (total_density - 0.8f) / 0.2f;
                    r = (int)(255);
                    g = (int)(230 + t * 25);
                    b = (int)(200 + t * 55);
                }
                
                // Add subtle pulsing to bright regions
                if (total_density > 0.5f) {
                    float pulse = sin(time * 2.0f + dist * 0.3f) * 0.1f + 1.0f;
                    r = (int)(r * pulse);
                    g = (int)(g * pulse);
                    b = (int)(b * pulse);
                    
                    if (r > 255) r = 255;
                    if (g > 255) g = 255;
                    if (b > 255) b = 255;
                }
                
                canvas->SetPixel(x, y, r, g, b);
            }
        }
        
        // Add stars in the background (sparse)
        for (int i = 0; i < 15; ++i) {
            int sx = (int)((i * 7 + frame_count / 10) % 32);
            int sy = (int)((i * 13) % 32);
            
            // Check if this position is dark enough for a visible star
            // Only show stars in darker regions
            float fx = sx + time * 2.0f;
            float fy = sy + time * 1.5f;
            float density = turbulence(fx, fy, 32.0f, seed) / 20.0f;
            
            if (density < 0.3f) {
                // Twinkling stars
                int twinkle = (int)(200 + sin(time * 3.0f + i) * 55);
                canvas->SetPixel(sx, sy, twinkle, twinkle, twinkle * 0.9f);
            }
        }
        
        // Add a few bright star clusters
        int cluster_x[] = {5, 26, 8, 24};
        int cluster_y[] = {4, 28, 26, 6};
        
        for (int c = 0; c < 4; ++c) {
            int cx = cluster_x[c];
            int cy = cluster_y[c];
            
            // Check if position is in dark region
            float fx = cx + time * 2.0f;
            float fy = cy + time * 1.5f;
            float density = turbulence(fx, fy, 32.0f, seed) / 20.0f;
            
            if (density < 0.25f) {
                // Bright star
                canvas->SetPixel(cx, cy, 255, 255, 230);
                // Cross pattern for brightness
                if (cx > 0) canvas->SetPixel(cx - 1, cy, 150, 150, 130);
                if (cx < 31) canvas->SetPixel(cx + 1, cy, 150, 150, 130);
                if (cy > 0) canvas->SetPixel(cx, cy - 1, 150, 150, 130);
                if (cy < 31) canvas->SetPixel(cx, cy + 1, 150, 150, 130);
            }
        }
        
        frame_count++;
        canvas = matrix->SwapOnVSync(canvas);
        usleep(50000);  // ~20 fps
    }
    
    delete matrix;
    return 0;
}
