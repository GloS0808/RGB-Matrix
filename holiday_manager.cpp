#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sstream>
#include <cstring>

struct Holiday {
    int month;
    int day;
    int duration_hours;
    std::string program_name;
    int priority; // Higher priority holidays override lower ones
};

class HolidayManager {
private:
    std::vector<Holiday> holidays;
    std::string scripts_path;
    std::string default_program;
    std::string current_program;
    time_t program_start_time;
    std::string log_file;
    std::vector<std::string> default_args; // Arguments for all programs
    std::vector<std::string> clock_args;   // Additional arguments for clockV2grok
    
    // Weather display settings
    std::string weather_program;
    std::string weather_api_key;
    int weather_duration_seconds;
    bool weather_enabled;

    void log(const std::string& message) {
        std::ofstream log(log_file, std::ios::app);
        time_t now = time(nullptr);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
        log << "[" << timestamp << "] " << message << std::endl;
        std::cout << "[" << timestamp << "] " << message << std::endl;
    }

    void killAllInstances(const std::string& program_name) {
        log("Killing all instances of: " + program_name);
        std::string cmd = "killall -9 " + program_name + " 2>/dev/null";
        system(cmd.c_str());
        sleep(1); // Give it time to cleanup
    }

    bool isProgramRunning(const std::string& program_name) {
        std::string cmd = "pgrep -x " + program_name + " > /dev/null 2>&1";
        return (system(cmd.c_str()) == 0);
    }

    bool startProgram(const std::string& program_name, bool is_weather = false) {
        std::string full_path = scripts_path + "/" + program_name;

        // First, kill any existing instances to prevent doubling
        killAllInstances(program_name);

        log("Starting program: " + program_name);

        // Build command with arguments
        std::string cmd = full_path;

        if (is_weather) {
            // Weather-specific arguments
            cmd += " -f /home/seth/rgbMatrix/rpi-rgb-led-matrix/fonts/5x7.bdf";
            cmd += " -k " + weather_api_key;
            // Add default args for matrix control
            for (const auto& arg : default_args) {
                cmd += " " + arg;
            }
        } else {
            // Add default arguments for all programs
            for (const auto& arg : default_args) {
                cmd += " " + arg;
            }

            // Add clock-specific arguments only for clockV2grok
            if (program_name == default_program) {
                for (const auto& arg : clock_args) {
                    cmd += " " + arg;
                }
            }
        }

        cmd += " > /dev/null 2>&1 &";

        int result = system(cmd.c_str());

        if (result == 0) {
            sleep(2); // Give it time to start
            if (isProgramRunning(program_name)) {
                if (!is_weather) {
                    current_program = program_name;
                    program_start_time = time(nullptr);
                }
                log("Program started successfully");
                return true;
            } else {
                log("ERROR: Program failed to start");
                return false;
            }
        } else {
            log("ERROR: Failed to execute command");
            return false;
        }
    }

    void showWeather() {
        if (!weather_enabled || weather_api_key.empty()) {
            log("Weather display skipped (not configured)");
            return;
        }

        log("Showing weather display");
        std::string saved_program = current_program;

        // Kill current program and show weather
        killAllInstances(current_program);
        
        if (startProgram(weather_program, true)) {
            // Weather display runs for specified duration
            sleep(weather_duration_seconds);
            
            // Kill weather and restore previous program
            killAllInstances(weather_program);
            log("Weather display complete, restoring: " + saved_program);
            startProgram(saved_program);
        } else {
            log("ERROR: Failed to start weather display, restoring: " + saved_program);
            startProgram(saved_program);
        }
    }

    bool shouldShowWeather() {
        if (!weather_enabled) return false;

        time_t now = time(nullptr);
        struct tm* timeinfo = localtime(&now);
        int current_minute = timeinfo->tm_min;

        // Check if we're at :18 or :48
        return (current_minute == 18 || current_minute == 48);
    }

    Holiday* getCurrentHoliday() {
        time_t now = time(nullptr);
        struct tm* timeinfo = localtime(&now);
        int current_month = timeinfo->tm_mon + 1;
        int current_day = timeinfo->tm_mday;
        int current_hour = timeinfo->tm_hour;

        Holiday* active_holiday = nullptr;
        int highest_priority = -1;

        for (auto& holiday : holidays) {
            if (holiday.month == current_month && holiday.day == current_day) {
                // Check if we're within the duration window
                time_t holiday_start = now - (current_hour * 3600) - (timeinfo->tm_min * 60) - timeinfo->tm_sec;
                time_t holiday_end = holiday_start + (holiday.duration_hours * 3600);

                if (now >= holiday_start && now < holiday_end) {
                    if (holiday.priority > highest_priority) {
                        active_holiday = &holiday;
                        highest_priority = holiday.priority;
                    }
                }
            }
        }

        return active_holiday;
    }

public:
    HolidayManager(const std::string& config_file, const std::string& path, const std::string& default_prog, 
                   const std::vector<std::string>& args, const std::string& api_key = "")
        : scripts_path(path), default_program(default_prog), program_start_time(0), 
          weather_program("temp_display"), weather_api_key(api_key), 
          weather_duration_seconds(30), weather_enabled(!api_key.empty()) {
        log_file = path + "/holiday_manager.log";

        // Split args into default_args (for all programs) and clock_args (for clockV2grok)
        default_args = {
            "--led-gpio-mapping=adafruit-hat",
            "--led-slowdown-gpio=2",
            "--led-daemon"
        };

        clock_args = {
            "-f", "/home/seth/rgbMatrix/rpi-rgb-led-matrix/fonts/5x8.bdf",
            "-C", "0,0,255",
            "-B", "16,0,32"
        };

        // Override default_args with any provided command-line args
        if (!args.empty()) {
            default_args = args;
        }

        loadConfig(config_file);
        log("Holiday Manager initialized");
        if (weather_enabled) {
            log("Weather display enabled (shows at :18 and :48 for " + 
                std::to_string(weather_duration_seconds) + " seconds)");
        } else {
            log("Weather display disabled (no API key provided)");
        }
    }

    void loadConfig(const std::string& config_file) {
        std::ifstream file(config_file);
        if (!file.is_open()) {
            log("WARNING: Could not open config file: " + config_file);
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            // Skip comments and empty lines
            if (line.empty() || line[0] == '#') continue;

            // Parse: MM-DD,duration_hours,program_name,priority
            std::istringstream iss(line);
            std::string date_str, duration_str, program, priority_str;

            if (std::getline(iss, date_str, ',') &&
                std::getline(iss, duration_str, ',') &&
                std::getline(iss, program, ',') &&
                std::getline(iss, priority_str)) {

                Holiday h;
                sscanf(date_str.c_str(), "%d-%d", &h.month, &h.day);
                h.duration_hours = std::stoi(duration_str);
                h.program_name = program;
                h.priority = std::stoi(priority_str);

                holidays.push_back(h);
                log("Loaded holiday: " + std::to_string(h.month) + "/" + std::to_string(h.day) +
                    " - " + h.program_name + " (Priority: " + std::to_string(h.priority) + ")");
            }
        }

        log("Loaded " + std::to_string(holidays.size()) + " holidays");
    }

    void run() {
        log("Starting holiday manager main loop");

        // Clean up any existing processes first
        log("Cleaning up any existing matrix processes...");
        killAllInstances(default_program);
        for (const auto& h : holidays) {
            killAllInstances(h.program_name);
        }
        killAllInstances(weather_program);

        time_t last_weather_check = 0;

        while (true) {
            time_t now = time(nullptr);
            struct tm* timeinfo = localtime(&now);
            
            // Check if it's time to show weather (only once per minute)
            if (shouldShowWeather() && (now - last_weather_check) >= 60) {
                showWeather();
                last_weather_check = now;
                // After weather, continue with normal operations
            }

            // Determine what should be running
            Holiday* active_holiday = getCurrentHoliday();
            std::string target_program = active_holiday ? active_holiday->program_name : default_program;

            // Check if we need to switch programs
            if (current_program != target_program) {
                if (active_holiday) {
                    log("Holiday detected: " + active_holiday->program_name);
                } else {
                    log("Holiday period ended, switching to default: " + default_program);
                }
                // Kill the current program before starting the new one
                if (!current_program.empty()) {
                    killAllInstances(current_program);
                }
                startProgram(target_program);
            } else {
                // Verify the program is still running
                if (!isProgramRunning(current_program)) {
                    log("Program " + current_program + " stopped unexpectedly, restarting...");
                    startProgram(current_program);
                }
            }

            // Check every 30 seconds for better weather timing accuracy
            sleep(30);
        }
    }

    ~HolidayManager() {
        log("Holiday Manager shutting down");
        if (!current_program.empty()) {
            killAllInstances(current_program);
        }
        killAllInstances(weather_program);
    }
};

int main(int argc, char* argv[]) {
    std::string scripts_path = "/home/USER/rgbMatrix/rpi-rgb-led-matrix/examples-api-use/rgbScripts";
    std::string config_file = scripts_path + "/holidays.conf";
    std::string default_program = "clockV2grok";
    std::string weather_api_key = "";
    std::vector<std::string> additional_args;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--weather-api-key" && i + 1 < argc) {
            weather_api_key = argv[++i];
        } else {
            additional_args.push_back(arg);
        }
    }

    HolidayManager manager(config_file, scripts_path, default_program, additional_args, weather_api_key);
    manager.run();

    return 0;
}
