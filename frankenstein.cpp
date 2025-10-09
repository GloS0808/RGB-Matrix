#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <signal.h>
using namespace rgb_matrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
    interrupt_received = true;
}

int main() {
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
    FrameCanvas *canvas = matrix->CreateFrameCanvas();
    
    // Colors
    Color bg_dark(15, 10, 20);           // Dark lab background
    Color skin_green(140, 160, 120);     // Greenish pale skin
    Color skin_shadow(100, 120, 90);     // Shadow areas
    Color hair_black(20, 20, 25);        // Black hair
    Color bolt_metal(180, 180, 190);     // Metal bolts
    Color bolt_dark(120, 120, 130);      // Bolt shadows
    Color scar_red(160, 80, 80);         // Reddish scars
    Color teeth_yellow(230, 220, 180);   // Yellowish teeth
    Color pupil_black(10, 10, 15);       // Eye pupils
    Color eye_white(240, 240, 230);      // Eye whites
    Color lightning_flash(220, 220, 255); // Lightning effect
    
    int frame_count = 0;
    
    // Display continuously
    while (!interrupt_received) {
        // Dark background
        canvas->Fill(bg_dark.r, bg_dark.g, bg_dark.b);
        
        // Lightning flash effect (occasional)
        bool lightning = (frame_count % 120) < 3;
        if (lightning) {
            for (int i = 0; i < 15; ++i) {
                int lx = rand() % 32;
                int ly = rand() % 10;
                canvas->SetPixel(lx, ly, lightning_flash.r, lightning_flash.g, lightning_flash.b);
            }
        }
        
        // Frankenstein's face (centered)
        int cx = 16;  // center x
        
        // NECK (bottom)
        for (int y = 28; y < 32; ++y) {
            for (int x = 12; x < 20; ++x) {
                canvas->SetPixel(x, y, skin_shadow.r, skin_shadow.g, skin_shadow.b);
            }
        }
        
        // Neck bolts (iconic!)
        // Left bolt
        canvas->SetPixel(11, 28, bolt_metal.r, bolt_metal.g, bolt_metal.b);
        canvas->SetPixel(11, 29, bolt_metal.r, bolt_metal.g, bolt_metal.b);
        canvas->SetPixel(12, 28, bolt_dark.r, bolt_dark.g, bolt_dark.b);
        canvas->SetPixel(12, 29, bolt_dark.r, bolt_dark.g, bolt_dark.b);
        
        // Right bolt
        canvas->SetPixel(19, 28, bolt_metal.r, bolt_metal.g, bolt_metal.b);
        canvas->SetPixel(19, 29, bolt_metal.r, bolt_metal.g, bolt_metal.b);
        canvas->SetPixel(20, 28, bolt_dark.r, bolt_dark.g, bolt_dark.b);
        canvas->SetPixel(20, 29, bolt_dark.r, bolt_dark.g, bolt_dark.b);
        
        // JAW/LOWER FACE
        for (int y = 24; y < 28; ++y) {
            for (int x = 10; x < 22; ++x) {
                canvas->SetPixel(x, y, skin_green.r, skin_green.g, skin_green.b);
            }
        }
        
        // Chin shadow
        for (int x = 13; x < 19; ++x) {
            canvas->SetPixel(x, 27, skin_shadow.r, skin_shadow.g, skin_shadow.b);
        }
        
        // Mouth (grim expression)
        for (int x = 13; x < 19; ++x) {
            canvas->SetPixel(x, 24, pupil_black.r, pupil_black.g, pupil_black.b);
        }
        
        // Teeth showing slightly
        canvas->SetPixel(14, 25, teeth_yellow.r, teeth_yellow.g, teeth_yellow.b);
        canvas->SetPixel(15, 25, teeth_yellow.r, teeth_yellow.g, teeth_yellow.b);
        canvas->SetPixel(16, 25, teeth_yellow.r, teeth_yellow.g, teeth_yellow.b);
        canvas->SetPixel(17, 25, teeth_yellow.r, teeth_yellow.g, teeth_yellow.b);
        
        // MID FACE / CHEEKS
        for (int y = 18; y < 24; ++y) {
            for (int x = 9; x < 23; ++x) {
                canvas->SetPixel(x, y, skin_green.r, skin_green.g, skin_green.b);
            }
        }
        
        // Nose
        canvas->SetPixel(cx, 20, skin_shadow.r, skin_shadow.g, skin_shadow.b);
        canvas->SetPixel(cx, 21, skin_shadow.r, skin_shadow.g, skin_shadow.b);
        canvas->SetPixel(cx - 1, 21, skin_shadow.r, skin_shadow.g, skin_shadow.b);
        canvas->SetPixel(cx + 1, 21, skin_shadow.r, skin_shadow.g, skin_shadow.b);
        
        // FOREHEAD
        for (int y = 12; y < 18; ++y) {
            for (int x = 9; x < 23; ++x) {
                canvas->SetPixel(x, y, skin_green.r, skin_green.g, skin_green.b);
            }
        }
        
        // Forehead is squared/flat (classic look)
        for (int x = 10; x < 22; ++x) {
            canvas->SetPixel(x, 12, skin_shadow.r, skin_shadow.g, skin_shadow.b);
        }
        
        // EYES (deep set, intense)
        // Left eye
        // Eye socket (dark)
        for (int y = 18; y < 21; ++y) {
            for (int x = 11; x < 14; ++x) {
                canvas->SetPixel(x, y, skin_shadow.r, skin_shadow.g, skin_shadow.b);
            }
        }
        // Eye white
        canvas->SetPixel(12, 19, eye_white.r, eye_white.g, eye_white.b);
        canvas->SetPixel(13, 19, eye_white.r, eye_white.g, eye_white.b);
        // Pupil
        canvas->SetPixel(12, 19, pupil_black.r, pupil_black.g, pupil_black.b);
        
        // Right eye
        // Eye socket
        for (int y = 18; y < 21; ++y) {
            for (int x = 18; x < 21; ++x) {
                canvas->SetPixel(x, y, skin_shadow.r, skin_shadow.g, skin_shadow.b);
            }
        }
        // Eye white
        canvas->SetPixel(18, 19, eye_white.r, eye_white.g, eye_white.b);
        canvas->SetPixel(19, 19, eye_white.r, eye_white.g, eye_white.b);
        // Pupil
        canvas->SetPixel(19, 19, pupil_black.r, pupil_black.g, pupil_black.b);
        
        // Heavy brow ridge (prominent forehead)
        for (int x = 11; x < 21; ++x) {
            canvas->SetPixel(x, 17, skin_shadow.r, skin_shadow.g, skin_shadow.b);
        }
        
        // HAIR (flat top, iconic)
        for (int y = 8; y < 12; ++y) {
            for (int x = 9; x < 23; ++x) {
                canvas->SetPixel(x, y, hair_black.r, hair_black.g, hair_black.b);
            }
        }
        
        // Hair extends down sides
        for (int y = 12; y < 20; ++y) {
            canvas->SetPixel(8, y, hair_black.r, hair_black.g, hair_black.b);
            canvas->SetPixel(9, y, hair_black.r, hair_black.g, hair_black.b);
            canvas->SetPixel(22, y, hair_black.r, hair_black.g, hair_black.b);
            canvas->SetPixel(23, y, hair_black.r, hair_black.g, hair_black.b);
        }
        
        // Widow's peak
        canvas->SetPixel(cx, 11, hair_black.r, hair_black.g, hair_black.b);
        canvas->SetPixel(cx, 12, hair_black.r, hair_black.g, hair_black.b);
        
        // SCARS (stitches across forehead - iconic!)
        // Horizontal scar across forehead
        for (int x = 10; x < 22; ++x) {
            canvas->SetPixel(x, 14, scar_red.r, scar_red.g, scar_red.b);
        }
        
        // Stitch marks (vertical lines)
        for (int x = 11; x < 21; x += 2) {
            canvas->SetPixel(x, 13, scar_red.r, scar_red.g, scar_red.b);
            canvas->SetPixel(x, 15, scar_red.r, scar_red.g, scar_red.b);
        }
        
        // Vertical scar on left cheek
        canvas->SetPixel(11, 21, scar_red.r, scar_red.g, scar_red.b);
        canvas->SetPixel(11, 22, scar_red.r, scar_red.g, scar_red.b);
        canvas->SetPixel(11, 23, scar_red.r, scar_red.g, scar_red.b);
        
        // Electrode bolt tops (on head)
        canvas->SetPixel(8, 10, bolt_metal.r, bolt_metal.g, bolt_metal.b);
        canvas->SetPixel(8, 11, bolt_metal.r, bolt_metal.g, bolt_metal.b);
        canvas->SetPixel(23, 10, bolt_metal.r, bolt_metal.g, bolt_metal.b);
        canvas->SetPixel(23, 11, bolt_metal.r, bolt_metal.g, bolt_metal.b);
        
        frame_count++;
        canvas = matrix->SwapOnVSync(canvas);
        usleep(50000);  // ~20 fs
    }
    
    // Clean up on exit
    canvas->Clear();
    canvas = matrix->SwapOnVSync(canvas);
    delete matrix;
    return 0;
}
