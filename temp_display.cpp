// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Temperature display for Centennial, CO on 32x32 RGB matrix

#include "led-matrix.h"
#include "graphics.h"
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <cmath>

using namespace rgb_matrix;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

// Callback for CURL to write response data
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

// Simple JSON parser to extract temperature and condition
bool ParseWeatherJson(const std::string &json, double &temp_c, std::string &condition) {
  // Find "temp":value
  size_t temp_pos = json.find("\"temp\":");
  if (temp_pos == std::string::npos) return false;
  
  temp_pos += 7; // Move past "temp":
  while (temp_pos < json.length() && (json[temp_pos] == ' ' || json[temp_pos] == '\t')) {
    temp_pos++;
  }
  
  char *end;
  temp_c = strtod(json.c_str() + temp_pos, &end);
  if (end == json.c_str() + temp_pos) return false;
  
  // Find "main":"condition"
  size_t main_pos = json.find("\"main\":\"");
  if (main_pos != std::string::npos) {
    main_pos += 8; // Move past "main":"
    size_t end_pos = json.find("\"", main_pos);
    if (end_pos != std::string::npos) {
      condition = json.substr(main_pos, end_pos - main_pos);
    }
  }
  
  return true;
}

// Fetch temperature from OpenWeatherMap API
bool FetchTemperature(const std::string &api_key, double &temp_c, std::string &condition) {
  CURL *curl;
  CURLcode res;
  std::string readBuffer;
  bool success = false;

  curl = curl_easy_init();
  if(curl) {
    // Centennial, CO coordinates: 39.5794, -104.8772
    std::string url = "https://api.openweathermap.org/data/2.5/weather?lat=39.5794&lon=-104.8772&appid=" + 
                      api_key + "&units=metric";
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    
    res = curl_easy_perform(curl);
    
    if(res == CURLE_OK) {
      success = ParseWeatherJson(readBuffer, temp_c, condition);
    }
    curl_easy_cleanup(curl);
  }
  return success;
}

static int usage(const char *progname) {
  fprintf(stderr, "usage: %s [options]\n", progname);
  fprintf(stderr, "Displays current temperature for Centennial, CO.\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "\t-f <font-file>    : Use given BDF font file (e.g., 7x13.bdf).\n");
  fprintf(stderr, "\t-k <api-key>      : OpenWeatherMap API key (required).\n");
  fprintf(stderr, "\t-r <refresh-sec>  : Refresh interval in seconds (default: 600).\n");
  fprintf(stderr, "\t-x <x-origin>     : X-Origin of text (default: 2).\n");
  fprintf(stderr, "\t-y <y-origin>     : Y-Origin of text (default: 0).\n");
  fprintf(stderr, "\t-s <line-spacing> : Spacing between lines (default: 1).\n");
  fprintf(stderr, "\t-C <r,g,b>        : Text color (default: 255,100,0).\n");
  fprintf(stderr, "\t-B <r,g,b>        : Background start color (default: 0,0,20).\n");
  fprintf(stderr, "\t-G <r,g,b>        : Background end color (default: 0,20,40).\n");
  rgb_matrix::PrintMatrixFlags(stderr);
  return 1;
}

static bool parseColor(Color *c, const char *str) {
  return sscanf(str, "%hhu,%hhu,%hhu", &c->r, &c->g, &c->b) == 3;
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

  Color text_color(255, 100, 0);
  Color bg_start_color(0, 0, 20);
  Color bg_end_color(0, 20, 40);
  const char *bdf_font_file = NULL;
  std::string api_key = "";
  int x_orig = 2;
  int y_orig = 0;
  int line_spacing = 1;
  int refresh_interval = 600; // 10 minutes default

  int opt;
  while ((opt = getopt(argc, argv, "f:k:r:x:y:s:C:B:G:")) != -1) {
    switch (opt) {
    case 'f': bdf_font_file = strdup(optarg); break;
    case 'k': api_key = optarg; break;
    case 'r': refresh_interval = atoi(optarg); break;
    case 'x': x_orig = atoi(optarg); break;
    case 'y': y_orig = atoi(optarg); break;
    case 's': line_spacing = atoi(optarg); break;
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
    fprintf(stderr, "Need to specify BDF font-file with -f\n");
    return usage(argv[0]);
  }

  if (api_key.empty()) {
    fprintf(stderr, "Need to specify OpenWeatherMap API key with -k\n");
    fprintf(stderr, "Get a free API key at: https://openweathermap.org/api\n");
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

  curl_global_init(CURL_GLOBAL_DEFAULT);

  FrameCanvas *offscreen = matrix->CreateFrameCanvas();
  double temp_celsius = 0.0;
  std::string condition = "---";
  time_t last_fetch = 0;
  bool first_fetch = true;

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  while (!interrupt_received) {
    time_t current_time = time(NULL);
    
    // Fetch temperature data at specified interval
    if (first_fetch || (current_time - last_fetch) >= refresh_interval) {
      if (FetchTemperature(api_key, temp_celsius, condition)) {
        last_fetch = current_time;
        first_fetch = false;
        printf("Temperature: %.1f°C (%.1f°F) - %s\n", 
               temp_celsius, temp_celsius * 9.0 / 5.0 + 32.0, condition.c_str());
      } else {
        fprintf(stderr, "Failed to fetch temperature data\n");
      }
    }

    // Draw gradient background
    DrawGradientBackground(offscreen, bg_start_color, bg_end_color, matrix->height());

    // Convert to Fahrenheit
    double temp_fahrenheit = temp_celsius * 9.0 / 5.0 + 32.0;
    
    // Format temperature strings
    char temp_buffer[32];
    char condition_buffer[32];
    snprintf(temp_buffer, sizeof(temp_buffer), "%.0f F", temp_fahrenheit);
    snprintf(condition_buffer, sizeof(condition_buffer), "%s", condition.c_str());

    // Draw location
    int y = y_orig + font.baseline();
    rgb_matrix::DrawText(offscreen, font, x_orig, y, text_color, NULL, "Cent'l");

    // Draw temperature
    y += font.height() + line_spacing;
    rgb_matrix::DrawText(offscreen, font, x_orig, y, text_color, NULL, temp_buffer);

    // Draw condition
    y += font.height() + line_spacing;
    rgb_matrix::DrawText(offscreen, font, x_orig, y, text_color, NULL, condition_buffer);

    // Update display
    offscreen = matrix->SwapOnVSync(offscreen);

    // Sleep for 1 second
    sleep(1);
  }

  curl_global_cleanup();
  free((void*)bdf_font_file);
  delete matrix;
  std::cout << std::endl;
  return 0;
}
