#!/bin/bash
# Set GPIO config and matrix size
LED_OPTS="--led-rows=32 --led-cols=32 --led-gpio-mapping=adafruit-hat --led-slowdown-gpio=2"

# Trap Ctrl+C only once
trap "echo -e '\n🛑 Gracefully exiting...'; pkill -f 'clock\|demo\|minimal-example\|floating'; exit 0" SIGINT

# Helper: run the clock
run_clock() {
  echo "🕒 Starting clock..."
  sudo ./clock -f ~/rgbMatrix/rpi-rgb-led-matrix/fonts/6x13.bdf \
    -C 0,0,255 -B 16,0,32 -O 0,0,0 -x 1 -y 9 -d "%H:%M" $LED_OPTS &
  CLOCK_PID=$!
}

# Helper: stop clock before demos
stop_clock() {
  echo "🛑 Stopping clock..."
  sudo kill "$CLOCK_PID" 2>/dev/null
  wait "$CLOCK_PID" 2>/dev/null
}

# Helper: run one of the demos for 10 seconds
run_demo_timed() {
  local cmd=$1
  local args=$2
  echo "▶️  Running $cmd..."
  timeout 10s sudo ./$cmd $LED_OPTS $args >/dev/null 2>&1
}

# Run a sequence of demos
run_demos() {
  run_demo_timed "demo" "-D 0 -m 10"
  run_demo_timed "demo" "-D 7 -m 10"
  run_demo_timed "minimal-example" ""
  
  # Run floating balloons animation for 5 minutes
  echo "🎈 Running floating balloons..."
  timeout 300s sudo ./floating >/dev/null 2>&1
}

# Main loop
while true; do
  MINUTE=$(date +%M)
  if [[ "$MINUTE" =~ ^(00|15|30|45)$ ]]; then
    stop_clock
    run_demos
    echo "🕒 Returning to clock..."
    run_clock
    # Sleep past the quarter-hour mark to avoid retriggering
    # Total demo time: ~330 seconds (10+10+10+300)
    sleep 360
  else
    if ! ps -p "$CLOCK_PID" >/dev/null 2>&1; then
      run_clock
    fi
    sleep 10
  fi
done
