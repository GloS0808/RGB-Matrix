// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Clock displaying day of week, time (24-hour), and date (MM/DD) without scrolling

#include "led-matrix.h"
#include "graphics.h"
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

using namespace rgb_matrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s [options]\n", progname);
  fprintf(stderr, "Displays day of week (e.g., Mon), time (HH:MM), and date (MM/DD) vertically.\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr,
          "\t-f <font-file>    : Use given BDF font file (e.g., 5x8.bdf).\n"
          "\t-w <day-format>   : Day of week format (default: %%a)\n"
          "\t-t <time-format>  : Time format (default: %%H:%%M)\n"
          "\t-d <date-format>  : Date format (default: %%m/%%d)\n"
          "\t-x <x-origin>     : X-Origin of text (default: 0)\n"
          "\t-y <y-origin>     : Y-Origin of text (default: 0)\n"
          "\t-s <line-spacing> : Spacing between lines (default: 1)\n"
          "\t-S <letter-spacing> : Spacing between letters (default: 0)\n"
          "\t-C <r,g,b>        : Text color (default: 255,255,0)\n"
          "\t-B <r,g,b>        : Background gradient start color (default: 0,0,20)\n"
          "\t-G <r,g,b>        : Background gradient end color (default: 0,20,0)\n");
  rgb_matrix::PrintMatrixFlags(stderr);
  return 1;
}

static bool parseColor(Color *c, const char *str) {
  return sscanf(str, "%hhu,%hhu,%hhu", &c->r, &c->g, &c->b) == 3;
}

static bool FullSaturation(const Color &c) {
  return (c.r == 0 || c.r == 255) && (c.g == 0 || c.g == 255) && (c.b == 0 || c.b == 255);
}

void DrawGradientBackground(FrameCanvas *canvas, const Color &start_color, const Color &end_color, int rows) {
  for (int y = 0; y < rows; ++y) {
    float t = (float)y / (rows - 1);
    uint8_t r = start_color.r + t * (end_color.r - start_color.r);
    uint8_t g = start_color.g + t * (end_color.g - start_color.g);
    uint8_t b = start_color.b + t * (end_color.b - start_color.b);
    for (int x = 0; x < canvas->width(); ++x) {
      canvas->SetPixel(x, y, r, g, b);
    }
  }
}

int main(int argc, char *argv[]) {
  RGBMatrix::Options matrix_options;
  matrix_options.rows = 32;
  matrix_options.cols = 32;
  matrix_options.chain_length = 1;
  matrix_options.parallel = 1;
  matrix_options.hardware_mapping = "adafruit-hat";
  rgb_matrix::RuntimeOptions runtime_opt;
  runtime_opt.gpio_slowdown = 2;

  if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv, &matrix_options, &runtime_opt)) {
    return usage(argv[0]);
  }

  std::string day_format = "%a";
  std::string time_format = "%H:%M";
  std::string date_format = "%m/%d";
  Color text_color(255, 255, 0);
  Color bg_start_color(0, 0, 20);
  Color bg_end_color(0, 20, 0);
  const char *bdf_font_file = NULL;
  int x_orig = 0;
  int y_orig = 0;
  int line_spacing = 1;
  int letter_spacing = 0;

  int opt;
  while ((opt = getopt(argc, argv, "f:w:t:d:x:y:s:S:C:B:G:")) != -1) {
    switch (opt) {
    case 'f': bdf_font_file = strdup(optarg); break;
    case 'w': day_format = optarg; break;
    case 't': time_format = optarg; break;
    case 'd': date_format = optarg; break;
    case 'x': x_orig = atoi(optarg); break;
    case 'y': y_orig = atoi(optarg); break;
    case 's': line_spacing = atoi(optarg); break;
    case 'S': letter_spacing = atoi(optarg); break;
    case 'C':
      if (!parseColor(&text_color, optarg)) {
        fprintf(stderr, "Invalid text color spec: %s\n", optarg);
        return usage(argv[0]);
      }
      break;
    case 'B':
      if (!parseColor(&bg_start_color, optarg)) {
        fprintf(stderr, "Invalid background start color spec: %s\n", optarg);
        return usage(argv[0]);
      }
      break;
    case 'G':
      if (!parseColor(&bg_end_color, optarg)) {
        fprintf(stderr, "Invalid background end color spec: %s\n", optarg);
        return usage(argv[0]);
      }
      break;
    default:
      return usage(argv[0]);
    }
  }

  if (bdf_font_file == NULL) {
    fprintf(stderr, "Need to specify BDF font-file with -f (e.g., 5x8.bdf)\n");
    return usage(argv[0]);
  }

  rgb_matrix::Font font;
  if (!font.LoadFont(bdf_font_file)) {
    fprintf(stderr, "Couldn't load font '%s'\n", bdf_font_file);
    free((void*)bdf_font_file);
    return 1;
  }

  RGBMatrix *matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
  if (matrix == NULL) {
    free((void*)bdf_font_file);
    return 1;
  }

  const bool all_extreme_colors = (matrix_options.brightness == 100) &&
                                  FullSaturation(text_color) &&
                                  FullSaturation(bg_start_color) &&
                                  FullSaturation(bg_end_color);
  if (all_extreme_colors) {
    matrix->SetPWMBits(1);
  }

  FrameCanvas *offscreen = matrix->CreateFrameCanvas();
  char day_buffer[256];
  char time_buffer[256];
  char date_buffer[256];
  struct timespec next_time;
  next_time.tv_sec = time(NULL);
  next_time.tv_nsec = 0;
  struct tm tm;
  int frame_count = 0;

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  while (!interrupt_received) {
    localtime_r(&next_time.tv_sec, &tm);

    // Draw gradient background
    DrawGradientBackground(offscreen, bg_start_color, bg_end_color, matrix->height());

    // Blinking colon for time
    std::string current_time_format = time_format;
    if (frame_count % 2 == 0) {
      size_t pos = current_time_format.find(':');
      while (pos != std::string::npos) {
        current_time_format.replace(pos, 1, " ");
        pos = current_time_format.find(':', pos + 1);
      }
    }

    // Prepare text buffers
    strftime(day_buffer, sizeof(day_buffer), day_format.c_str(), &tm);
    strftime(time_buffer, sizeof(time_buffer), current_time_format.c_str(), &tm);
    strftime(date_buffer, sizeof(date_buffer), date_format.c_str(), &tm);

    // Draw day of week
    int y = y_orig;
    rgb_matrix::DrawText(offscreen, font, x_orig, y + font.baseline(),
                         text_color, NULL, day_buffer, letter_spacing);

    // Draw time
    y += font.height() + line_spacing;
    rgb_matrix::DrawText(offscreen, font, x_orig, y + font.baseline(),
                         text_color, NULL, time_buffer, letter_spacing);

    // Draw date
    y += font.height() + line_spacing;
    rgb_matrix::DrawText(offscreen, font, x_orig, y + font.baseline(),
                         text_color, NULL, date_buffer, letter_spacing);

    // Pulsing seconds indicator
    int sec_x = x_orig + font.CharacterWidth('0') * 6 + letter_spacing * 5;
    int sec_y = y_orig + font.height() + line_spacing + font.baseline();
    float pulse = 0.5f + 0.5f * sin(frame_count * 0.2f);
    Color pulse_color(text_color.r * pulse, text_color.g * pulse, text_color.b * pulse);
    offscreen->SetPixel(sec_x, sec_y, pulse_color.r, pulse_color.g, pulse_color.b);

    // Update display
    offscreen = matrix->SwapOnVSync(offscreen);
    frame_count++;

    // Sleep for ~1 second
    next_time.tv_sec += 1;
    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_time, NULL);
  }

  free((void*)bdf_font_file);
  delete matrix;
  std::cout << std::endl;
  return 0;
}
