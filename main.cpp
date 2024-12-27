#include <time.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <X11/cursorfont.h>
#include <vector>

clock_t start_time = clock();
double GetTime() {
    clock_t current = clock();
    return (double)(current - start_time) / CLOCKS_PER_SEC;
}

#include "Window.cpp"

using namespace std;

void panic(string msg) {
    puts(msg.c_str());
    exit(EXIT_FAILURE);
}

vector<MinWindow> windows;

Display * display;
int screen;
Window root;
XftFont *font;

void DoEvent(XEvent e) {
    switch (e.type) {
        case Expose : {
            if(e.xexpose.window == root) return;
            for(MinWindow win : windows) {
                if(win.titlebar != root && win.titlebar == e.xexpose.window)  {
                    win.DrawTitlebar(display);
                }
            }
            break;
        }
        case ConfigureNotify : break;
        case ClientMessage : {
            cout << "WM:\tMessage from id " << e.xclient.window << " : " << XGetAtomName(display, e.xclient.message_type) << endl;
            break;
        }
        default:
            cout << "WM:\tUnexpected event id " << e.type << endl;
            break;
        case ButtonPress:
            cout << "PRESS START\n";
            for(MinWindow & win : windows) {
                if(win.titlebar != root && win.titlebar == e.xbutton.window) {
                    cout << "PRESS DEFAQ\n";
                    win.dragging = true;
                    win.dragStart = (fvec2){(float)e.xbutton.x_root, (float)e.xbutton.y_root} - win.position.Get();
                    XAllowEvents(display, ReplayPointer, CurrentTime);
                    XSync(display, 0);
                    return;
                }
            }
            cout << "PRESS END\n";
            break;
        case ButtonRelease:
            for(MinWindow & win : windows) {
                win.dragging = false;
            }
            break;
        case MotionNotify:
            for(MinWindow & win : windows) {
                if(win.dragging) {
                    fvec2 offset = (fvec2){(float)e.xbutton.x_root, (float)e.xbutton.y_root} - win.position.Get();
                    fvec2 delta = offset - win.dragStart;
                    win.position.Set(win.position.Get() + delta, 0.03);
                    return;
                }
            }
            break;
        case CreateNotify: {
            if(e.xcreatewindow.window == root) return;
            // Check if is a titlebar
            for(MinWindow win : windows) {
                if(win.titlebar == e.xcreatewindow.window) return;
            }
            MinWindow window = MinWindow(display, root, e.xcreatewindow.window, e.xcreatewindow, font);
            windows.push_back(window);

            // Debug log
            char * windowName = NULL;
            XFetchName(display, e.xcreatewindow.window, &windowName);
            cout << "WM:\tNew window '" << (windowName ? windowName : "NULL") << "' with id " << e.xcreatewindow.window << endl;
            break;
        }
        case MapRequest:
            cout << "WM:\tMapped id " << e.xmaprequest.window << endl;
            XMapWindow(display, e.xmaprequest.window);
            break;
        case UnmapNotify:
            cout << "WM:\tUnmapped id " << e.xmaprequest.window << endl;
            break;
        case ConfigureRequest :
            // Get window
            for(MinWindow & win : windows) {
                if(win.window == e.xcreatewindow.window) {
                    win.size.Set((fvec2){e.xconfigurerequest.width, e.xconfigurerequest.height}, 0.2);
                }
            }
            break;
    }
}

int main() {
    display = XOpenDisplay(NULL);
    if(display == nullptr) {
        panic("Failed to open a display");
    }
    screen = DefaultScreen(display);
    root = DefaultRootWindow(display);

    LoadColors(display, screen);

    XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask);
    XSync(display, 0);
    Cursor cursor = XCreateFontCursor(display, XC_left_ptr);
    XDefineCursor(display, root, cursor);
    XSync(display, 0);
    XGrabButton(display, Button1, 0, root, 0, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, GrabModeSync,
              GrabModeAsync, root, cursor);
    XEvent e;

    font = XftFontOpenName(display, screen, "FiraCode:medium:size=12");
    if(!font) {
        cout << "WM:\tFailed to load font\n";
        exit(EXIT_FAILURE);
    }

    XWindowAttributes rootAttrib;
    XGetWindowAttributes(display, root, &rootAttrib);

    fd_set read_fds;

    timeval timeout;
    timeout.tv_usec = 1000 / 60;

    while(1) {
        FD_ZERO(&read_fds);
        FD_SET(ConnectionNumber(display), &read_fds);

        int result = select(ConnectionNumber(display) + 1, &read_fds, NULL, NULL, &timeout);
        if (result > 0) {
            // Check if there are events to process
            while (XPending(display) > 0) {
                XNextEvent(display, &e);
                DoEvent(e);
            }
            XSync(display, 0);
        } else {
            // Timeout or no events to process, handle other tasks here
            for(MinWindow & win : windows) win.Update(display, root);
        }
    }
    XftFontClose(display, font);
    XCloseDisplay(display);
}