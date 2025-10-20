// link_display.cpp
#include "led-matrix.h"
#include "graphics.h"
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>

using namespace rgb_matrix;

static volatile bool running = true;
void handle_interrupt(int /*sig*/) { running = false; }

int main(int argc, char** argv) {
  // Ctrl+C handler
  signal(SIGINT, handle_interrupt);

  // Matrix setup (match your working example)
  RGBMatrix::Options matrix_options;
  matrix_options.rows = 32;
  matrix_options.cols = 32;
  matrix_options.chain_length = 1;
  matrix_options.parallel = 1;
  matrix_options.hardware_mapping = "adafruit-hat"; // change if you use different mapping

  RuntimeOptions rt_opts;
  rt_opts.gpio_slowdown = 2; // tune if you need (2 is common for adafruit-hat)

  RGBMatrix *matrix = CreateMatrixFromOptions(matrix_options, rt_opts);
  if (!matrix) {
    std::cerr << "Could not initialize RGB matrix." << std::endl;
    return 1;
  }

  FrameCanvas *canvas = matrix->CreateFrameCanvas();

  // Palette (tweak RGB if you want closer matching)
  Color GREEN(0, 200, 0);
  Color BROWN(150, 75, 0);
  Color SKIN(255, 200, 150);
  Color YELLOW(255, 220, 0);
  Color BLACK(0, 0, 0);

  // 16x16 Link sprite ('.' = transparent / background)
  // Each string MUST be length 16
  std::vector<std::string> sprite = {
    "..GGGG..........",  // row 0
    ".GGBBGG.........",  // row 1
    "GBSSSSBG........",  // row 2
    "GSBSSBSG........",  // row 3
    "GSSSSSSG........",  // row 4
    "GGSSSSGG........",  // row 5
    ".GGGGGGG........",  // row 6
    "..BBBB..........",  // row 7
    "..SSSS..........",  // row 8
    "..BYBB..........",  // row 9 (Y = yellow belt highlight)
    "...GG...........",  // row 10
    "..B..B..........",  // row 11
    "..S..S..........",  // row 12
    "..B..B..........",  // row 13
    "...BB...........",  // row 14
    "................"   // row 15
  };

  const int W = 16;
  const int H = 16;
  const int offsetX = (32 - W) / 2; // center horizontally
  const int offsetY = (32 - H) / 2; // center vertically

  int frame_count = 0;

  // Main loop: draw sprite every frame, swap, sleep, repeat
  while (running) {
    // Clear canvas (black background)
    canvas->Fill(BLACK.r, BLACK.g, BLACK.b);

    // Draw sprite
    for (int y = 0; y < H; ++y) {
      const std::string &row = sprite[y];
      for (int x = 0; x < W; ++x) {
        char c = row[x];
        int px = offsetX + x;
        int py = offsetY + y;
        switch (c) {
          case 'G':
            canvas->SetPixel(px, py, GREEN.r, GREEN.g, GREEN.b);
            break;
          case 'B':
            canvas->SetPixel(px, py, BROWN.r, BROWN.g, BROWN.b);
            break;
          case 'S':
            canvas->SetPixel(px, py, SKIN.r, SKIN.g, SKIN.b);
            break;
          case 'Y':
            canvas->SetPixel(px, py, YELLOW.r, YELLOW.g, YELLOW.b);
            break;
          default:
            // '.' = transparent / background - do nothing
            break;
        }
      }
    }

    // Swap on VSync (returns next canvas to draw into)
    canvas = matrix->SwapOnVSync(canvas);

    // Keep modest framerate (20 fps)
    usleep(50000);
    ++frame_count;
  }

  // Cleanup
  delete matrix;
  std::cout << "Exiting link_display (clean shutdown)." << std::endl;
  return 0;
}
