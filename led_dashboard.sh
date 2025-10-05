#!/bin/bash
# RGB LED Matrix Dashboard Rotation
# Rotates between clock, minimal example, color demo, and Game of Life every 10 seconds
# Press 'q' to quit gracefully

LED_OPTS="--led-gpio-mapping=adafruit-hat --led-slowdown-gpio=2 --led-brightness=80 --led-rows=32 --led-cols=32"

stop_demo() {
    if [ ! -z "$PID" ]; then
        sudo kill "$PID" 2>/dev/null
        wait "$PID" 2>/dev/null
        PID=""
    fi
}

run_mode() {
    local desc=$1
    shift
    echo "▶️  Showing $desc..."
    "$@" &
    PID=$!
    for ((i=0; i<10; i++)); do
        if read -t 1 -n 1 key && [[ $key = "q" ]]; then
            echo -e "\n🛑 Quitting dashboard..."
            stop_demo
            sudo ./demo $LED_OPTS -D 0 >/dev/null 2>&1 &
            sleep 1
            sudo killall demo 2>/dev/null
            exit 0
        fi
    done
    stop_demo
}

while true; do
    # 🕒 Clock
    run_mode "Clock" sudo ./clock -f ~/rgbMatrix/rpi-rgb-led-matrix/fonts/6x13.bdf \
        -C 0,0,255 -B 16,0,32 -O 0,0,0 -x 1 -y 9 -d "%H:%M" $LED_OPTS

    # 🟥 Minimal Example
    run_mode "Minimal Example" sudo ./minimal-example $LED_OPTS

    # 🌈 Color Demo
    run_mode "Color Demo" sudo ./demo $LED_OPTS -D 4

    # 🧬 Game of Life (demo 7)
    run_mode "Game of Life" sudo ./demo $LED_OPTS -D 7 -m 10
done
