#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xft/Xft.h>
#include <X11/Xatom.h>
#include "Colors.cpp"
#include <string.h>
#include "AnimValue.cpp"
#include <math.h>

#define TITLEBAR_SIZE 30

XftFont * font;
XftFont * iconFont;

fvec2 GetMousePosition(Display * display, Window root) {
    // Variables to store the pointer information
    Window root_return, child_return;
    int root_x, root_y, win_x, win_y;
    unsigned int mask_return;

    // Query the pointer position
    XQueryPointer(display, root, &root_return, &child_return, &root_x, &root_y, &win_x, &win_y, &mask_return);

    return (fvec2){root_x, root_y};
}

int has_titlebar(Display *display, Window window) {
    // Check override_redirect
    XWindowAttributes attrs;
    if (XGetWindowAttributes(display, window, &attrs)) {
        if (attrs.override_redirect) {
            return 0; // Override-redirect windows do not have a titlebar
        }
    }

    // Check if transient
    Window transient;
    if(XGetTransientForHint(display, window, &transient))
        return 0;

    // Check types
    Atom wm_window_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", True);
    if (wm_window_type != None) {
        Atom actual_type;
        int actual_format;
        unsigned long nitems, bytes_after;
        Atom *prop = NULL;

        if (XGetWindowProperty(display, window, wm_window_type, 0, 1024, False,
                               XA_ATOM, &actual_type, &actual_format,
                               &nitems, &bytes_after, (unsigned char **)&prop) == Success) {
            if (prop) {
                // Check window type
                Atom dialog_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", True);
                Atom utility_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE_UTILITY", True);
                Atom splash_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE_SPLASH", True);

                for (unsigned long i = 0; i < nitems; i++) {
                    cout << "ATOM:\t" << XGetAtomName(display, prop[i]) << "\n";
                    if (prop[i] == dialog_type || prop[i] == utility_type || prop[i] == splash_type) {
                        XFree(prop);
                        return 0; // These types do not need a titlebar
                    }
                }
                XFree(prop);
            }
        }
    }

    // Check states
    Atom wm_window_state = XInternAtom(display, "_NET_WM_STATE", True);
    if (wm_window_type != None) {
        Atom actual_type;
        int actual_format;
        unsigned long nitems, bytes_after;
        Atom *prop = NULL;

        if (XGetWindowProperty(display, window, wm_window_type, 0, 1024, False,
                               XA_ATOM, &actual_type, &actual_format,
                               &nitems, &bytes_after, (unsigned char **)&prop) == Success) {
            if (prop) {
                // Check window type
                Atom fullscreen_state = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", True);
                Atom above_state = XInternAtom(display, "_NET_WM_STATE_ABOVE", True);

                for (unsigned long i = 0; i < nitems; i++) {
                    if (prop[i] == fullscreen_state || prop[i] == above_state) {
                        XFree(prop);
                        return 0; // These types do not need a titlebar
                    }
                }
                XFree(prop);
            }
        }
    }
    return 1; // Assume it needs a titlebar by default
}

class MinWindow {
    public:
    Window window;
    Window titlebar;
    Window background;
    bool dragging = false;
    bool resizing = false;
    fvec2 dragStart;

    AnimValue<fvec2> position;
    AnimValue<fvec2> size;

    MinWindow(Display * display, Window root, Window window, XCreateWindowEvent e) {
        this->window = window;
        titlebar = root;
        window = root;

        // Set position and size
        position = AnimValue((fvec2){e.x, e.y});
        size = AnimValue((fvec2){e.width, e.height});
        XResizeWindow(display, window, e.width, e.height);
    }
    MinWindow() = default;

    void CreateTitlebar(Display * display, Window root) {
        if(!has_titlebar(display, window)) return;

        fvec2 currentPos = position.Get();
        fvec2 currentSize = size.Get();

        background = XCreateSimpleWindow(display, root, currentPos.x-1, currentPos.y, currentSize.x, currentSize.y + 30, 1, borderC.pixel, backgroundC.pixel);
        titlebar = XCreateSimpleWindow(display, root, currentPos.x-1, currentPos.y, currentSize.x, TITLEBAR_SIZE, 0, borderC.pixel, tilebarBgC.pixel);

        XSelectInput(display, titlebar, ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ExposureMask);
        XMapWindow(display, background);
        XMapWindow(display, titlebar);
        XMoveWindow(display, window, currentPos.x, currentPos.y + TITLEBAR_SIZE);
    }

    void DrawTitlebar(Display * display, Window root) {
        char * title;
        if(!XFetchName(display, window, &title)) title = "Window";

        XftDraw * drawCtx = XftDrawCreate(display, titlebar, DefaultVisual(display, 0), DefaultColormap(display, 0));
        if(!drawCtx) {
            cout << "WM:\tFailed to create draw context\n";
            exit(EXIT_FAILURE);
        }

        XftColor color;
        XftColorAllocName(display, DefaultVisual(display, 0), DefaultColormap(display, 0), "lightgray", &color);
        XftColor colorLight;
        XftColorAllocName(display, DefaultVisual(display, 0), DefaultColormap(display, 0), "white", &colorLight);

        XClearWindow(display, titlebar);

        XGlyphInfo attr;
        XftTextExtentsUtf8(display, font, (const FcChar8 *)title, strlen(title), &attr);
        int text_width = attr.width;
        fvec2 currentSize = size.Get();
        fvec2 currentPos = position.Get();

        XftDrawStringUtf8(drawCtx, &color, font, (currentSize.x - text_width) / 2, font->ascent + 5, (const XftChar8 *)title, strlen(title));

        fvec2 mouse = GetMousePosition(display, root) - currentPos;

        // Draw buttons
        XftColor c = mouse.Distance((fvec2){currentSize.x - 78, iconFont->ascent + 10}) < 15 ? colorLight : color;
        XftDrawString32(drawCtx, &c, iconFont, currentSize.x - 85, iconFont->ascent + 3, (const XftChar32 *)L"-", 1);
        c = mouse.Distance((fvec2){currentSize.x - 48, iconFont->ascent + 10}) < 15 ? colorLight : color;
        XftDrawString32(drawCtx, &c, iconFont, currentSize.x - 55, iconFont->ascent + 3, (const XftChar32 *)L"□", 1);
        c = mouse.Distance((fvec2){currentSize.x - 18, iconFont->ascent + 10}) < 15 ? colorLight : color;
        XftDrawString32(drawCtx, &c, iconFont, currentSize.x - 25, iconFont->ascent + 3, (const XftChar32 *)L"x", 1);

        XftDrawDestroy(drawCtx);
    }

    int GetButtonPressed(Display * display, Window root) {
        fvec2 currentSize = size.Get();
        fvec2 currentPos = position.Get();
        fvec2 mouse = GetMousePosition(display, root) - currentPos;

        if(mouse.Distance((fvec2){currentSize.x - 78, iconFont->ascent + 10}) < 15) return 1;
        if(mouse.Distance((fvec2){currentSize.x - 48, iconFont->ascent + 10}) < 15) return 2;
        if(mouse.Distance((fvec2){currentSize.x - 18, iconFont->ascent + 10}) < 15) return 3;
        return 0;
    }

    void Delete(Display * display) {
        XUnmapWindow(display, titlebar);
        XUnmapWindow(display, background);
    }

    void Update(Display * display, Window root) {
        fvec2 currentPos = position.Get();
        fvec2 currentSize = size.Get();

        // Check for change in size
        if(GetTime() <= size.srcT + size.moveT + 0.02) {
            XResizeWindow(display, window, currentSize.x, currentSize.y);
            if(titlebar != root) DrawTitlebar(display, root);
        }
        if(GetTime() <= position.srcT + position.moveT + 0.02) {
            XMoveWindow(display, window, currentPos.x, round(currentPos.y + 30.0));
            /*XEvent event;
            event.type = Expose;
            event.xexpose.window = window;
            event.xexpose.x = currentPos.x;
            event.xexpose.y = currentPos.y;
            event.xexpose.width = currentSize.x;   // Entire width
            event.xexpose.height = currentSize.y;  // Entire height
            event.xexpose.count = 0;   // No more expose events following

            // Send the event to the window
            XSendEvent(display, window, False, ExposureMask, &event);
            XFlush(display);*/
        }
        
        if(titlebar == root) return;

        XResizeWindow(display, titlebar, currentSize.x, 30);
        XMoveWindow(display, titlebar, currentPos.x, round(currentPos.y));
        XResizeWindow(display, background, currentSize.x + 1, currentSize.y + 31);
        XMoveWindow(display, background, currentPos.x-1, round(currentPos.y) - 1);
    }
};