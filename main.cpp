#include <time.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <X11/cursorfont.h>
#include <unistd.h>

double GetTime() {
    static timespec start = {0, 0};
    timespec current;

    if (start.tv_sec == 0 && start.tv_nsec == 0) {
        // Initialize start time
        clock_gettime(CLOCK_MONOTONIC, &start);
    }

    clock_gettime(CLOCK_MONOTONIC, &current);

    return (current.tv_sec - start.tv_sec) + 
           (current.tv_nsec - start.tv_nsec) / 1e9;
}

#include "Window.cpp"
#include "List.cpp"

using namespace std;

void panic(string msg) {
    puts(msg.c_str());
    exit(EXIT_FAILURE);
}

List<MinWindow> windows;

Cursor cursor;
Display * display;
int screen;
Window root;

void DoEvent(XEvent e) {
    switch (e.type) {
        case Expose : {
            if(e.xexpose.window == root) return;
            for(MinWindow win : windows) {
                if(win.titlebar != root && win.titlebar == e.xexpose.window)  {
                    win.DrawTitlebar(display, root);
                }
            }
            break;
        }
        case ConfigureNotify : break;
        case ClientMessage : {
            cout << "WM:\tMessage from id " << e.xclient.window << " : " << XGetAtomName(display, e.xclient.message_type) << endl;
            //cout << "WM:\tl[0] = " << e.xclient.data.l[0] << endl;
            //cout << "WM:\tl[1] = " << XGetAtomName(display, e.xclient.data.l[1]) << endl;
            break;
        }
        default:
            cout << "WM:\tUnexpected event id " << e.type << endl;
            break;
        case ButtonPress:
            for(MinWindow & win : windows) {
                if(win.window == e.xbutton.subwindow) {
                    // Check for corner
                    fvec2 corner = win.position.Get() + win.size.Get() + (fvec2){0, 30};
                    float dist = corner.Distance((fvec2){e.xbutton.x_root, e.xbutton.y_root});
                    if(dist < 30) {
                        win.resizing = true;
                        win.dragStart = (fvec2){(float)e.xbutton.x_root, (float)e.xbutton.y_root} - corner;
                        XGrabPointer(display, win.window, False, ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
                        XAllowEvents(display, ReplayPointer, CurrentTime);
                        XSync(display, 0);
                        return;
                    }
                }
                else if(win.titlebar != root && win.titlebar == e.xbutton.window) {
                    // Check for button interaction
                    int button = win.GetButtonPressed(display, root);
                    if(button) {
                        if(button == 3) {
                            XUnmapWindow(display, win.background);
                            XDestroyWindow(display, win.window);
                        }
                        return;
                    }

                    // Start dragging
                    win.dragging = true;
                    win.dragStart = (fvec2){(float)e.xbutton.x_root, (float)e.xbutton.y_root} - win.position.Get();
                    XAllowEvents(display, ReplayPointer, CurrentTime);
                    XSync(display, 0);

                    // Reorder everything
                    XWindowChanges a;
                    a.stack_mode = Above;
                    XConfigureWindow(display, win.window,       CWStackMode, &a);
                    XConfigureWindow(display, win.titlebar,     CWStackMode, &a);
                    a.sibling = win.window;
                    a.stack_mode = Below;
                    XConfigureWindow(display, win.background,   CWStackMode | CWSibling, &a);
                    return;
                }
            }
            XAllowEvents(display, ReplayPointer, CurrentTime);
            XSync(display, 0);
            break;
        case ButtonRelease:
            XUngrabPointer(display, CurrentTime);
            for(MinWindow & win : windows) {
                win.dragging = false;
                win.resizing = false;
            }
            break;
        case MotionNotify:
            for(MinWindow & win : windows) {
                if(win.dragging) {
                    fvec2 offset = (fvec2){(float)e.xbutton.x_root, (float)e.xbutton.y_root} - win.position.Get();
                    fvec2 delta = offset - win.dragStart;
                    win.position.Set(win.position.Get() + delta, 0.06);
                }
                if(win.resizing) {
                    fvec2 corner = win.position.Get() + win.size.Get() + (fvec2){0, 30};
                    fvec2 offset = (fvec2){(float)e.xbutton.x_root, (float)e.xbutton.y_root} - corner;
                    fvec2 delta = offset - win.dragStart;
                    win.size.Set(win.size.Get() + delta, 0.06);
                }
            }
            break;
        case CreateNotify: {
            if(e.xcreatewindow.window == root) return;
            // Check if is a titlebar
            for(MinWindow win : windows) {
                if(win.titlebar == e.xcreatewindow.window) return;
            }
            // Debug log
            char * windowName = NULL;
            XFetchName(display, e.xcreatewindow.window, &windowName);

            MinWindow window = MinWindow(display, root, e.xcreatewindow.window, e.xcreatewindow);
            windows.Append(window);
            
            cout << "WM:\tNew window '" << (windowName ? windowName : "NULL") << "' with id " << e.xcreatewindow.window << endl;
            break;
        }
        case MapRequest: {
            cout << "WM:\tMapped id " << e.xmaprequest.window << endl;
            XMapWindow(display, e.xmaprequest.window);
            
            if(e.xmaprequest.window == root) return;
            for(MinWindow & win : windows) {
                if(win.window == e.xmaprequest.window) {
                    win.CreateTitlebar(display, root);

                    // Reorder everything
                    XWindowChanges a;
                    a.stack_mode = Above;
                    XConfigureWindow(display, win.window,       CWStackMode, &a);
                    XConfigureWindow(display, win.titlebar,     CWStackMode, &a);
                    a.sibling = win.window;
                    a.stack_mode = Below;
                    XConfigureWindow(display, win.background,   CWStackMode | CWSibling, &a);
                    return;
                }
            }
            break;
        }
        case MapNotify : {
            break;
        }
        case DestroyNotify : {
            cout << "WM:\tDestroyed window id " << e.xdestroywindow.window << endl;
            break;
        }
        case UnmapNotify: {
            cout << "WM:\tUnmapped id " << e.xunmap.window << endl;
            int i = 0;
            for(MinWindow & win : windows) {
                if(win.titlebar == root) continue;

                if(win.window == e.xunmap.window) {
                    win.Delete(display);
                    // Delete item
                    windows.Remove(i);
                    return;
                }
                ++i;
            }
            break;
        }
        case ConfigureRequest :
            // Get window
            for(MinWindow & win : windows) {
                if(win.window == e.xcreatewindow.window) {
                    win.size.Set((fvec2){e.xconfigurerequest.width, e.xconfigurerequest.height}, 0.06);
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
    cursor = XCreateFontCursor(display, XC_left_ptr);
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
    iconFont = XftFontOpenName(display, screen, "FiraCode:medium:size=15");
    if(!iconFont) {
        cout << "WM:\tFailed to load font\n";
        exit(EXIT_FAILURE);
    }

    XWindowAttributes rootAttrib;
    XGetWindowAttributes(display, root, &rootAttrib);

    fd_set read_fds;

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000 / 120;

    double frameStart = GetTime();

    while(1) {
        frameStart = GetTime();

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
        } 
        else {
            // Timeout or no events to process, handle other tasks here
            for(MinWindow & win : windows) {
                win.Update(display, root);
            }
        }

        // Limit to 60 fps
        double deltaT = GetTime() - frameStart;
        double remainder = (1.0 / 120.0) - deltaT;
        if(remainder > 0) {
            usleep((useconds_t)(remainder * 1000.0));
        }
    }
    XftFontClose(display, font);
    XCloseDisplay(display);
}