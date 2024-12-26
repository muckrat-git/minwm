#include <X11/Xlib.h>
#include <X11/X.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <X11/cursorfont.h>

using namespace std;

void panic(string msg) {
    puts(msg.c_str());
    exit(EXIT_FAILURE);
}

int main() {
    Display * display = XOpenDisplay(NULL);
    if(display == nullptr) {
        panic("Failed to open a display");
    }
    Window root = DefaultRootWindow(display);

    XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask | ButtonPress);
    XSync(display, false);
    Cursor cursor = XCreateFontCursor(display, XC_left_ptr);
    XDefineCursor(display, root, cursor);
    XSync(display, false);

    XEvent e;
    for (;;) {
        XNextEvent(display, &e);
        switch (e.type) {
            default:
                cout << "WM:\tUnexpected event id " << e.type << endl;
                break;
            case ButtonPress:
                XAllowEvents(display, ReplayPointer, CurrentTime);
                XSync(display, 0);
                puts("WM:\tButton pressed");
                break;
        }
        // Ensures that X will proccess the event.
        XSync(display, 0);
    }

    XCloseDisplay(display);
}