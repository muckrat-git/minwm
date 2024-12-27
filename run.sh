g++ main.cpp -o main -lraylib -lX11 -Isrc -I/usr/include/freetype2 -lXft -Wno-narrowing -Wno-write-strings

XEPHYR=$(command -v Xephyr) # Absolute path of Xephyr's bin
xinit ./xinitrc -- \
    "$XEPHYR" \
        :100 \
        -ac \
        -screen 1380x720\
        -host-cursor