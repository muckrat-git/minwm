#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xft/Xft.h>
#include <X11/Xatom.h>
#include "Colors.cpp"
#include <string.h>
#include "AnimValue.cpp"

#define TITLEBAR_SIZE 30

int has_titlebar(Display *display, Window window) {
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
                    if (prop[i] == dialog_type || prop[i] == utility_type || prop[i] == splash_type) {
                        XFree(prop);
                        return 0; // These types do not need a titlebar
                    }
                }
                XFree(prop);
            }
        }
    }

    // Check override_redirect
    XWindowAttributes attrs;
    if (XGetWindowAttributes(display, window, &attrs)) {
        if (attrs.override_redirect) {
            return 0; // Override-redirect windows do not have a titlebar
        }
    }

    return 1; // Assume it needs a titlebar by default
}

class MinWindow {
    public:
    XftFont * font;
    Window window;
    Window titlebar;
    bool dragging = false;
    fvec2 dragStart;

    AnimValue<fvec2> position;
    AnimValue<fvec2> size;

    MinWindow(Display * display, Window root, Window window, XCreateWindowEvent e, XftFont * font) {
        this->font = font;
        this->window = window;
        titlebar = root;

        // Get stuff
        XWindowAttributes attr;
        XGetWindowAttributes(display, window, &attr);

        // Set position and size
        position = AnimValue((fvec2){attr.x, attr.y});
        size = AnimValue((fvec2){attr.width, attr.height});

        if(!has_titlebar(display, window)) return;

        titlebar = XCreateSimpleWindow(display, root, attr.x-1, attr.y, attr.width, TITLEBAR_SIZE, 1, borderC.pixel, tilebarBgC.pixel);
        XSelectInput(display, titlebar, ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ExposureMask);
        XMapWindow(display, titlebar);
        XMoveWindow(display, window, attr.x, attr.y + TITLEBAR_SIZE);
    }
    MinWindow() = default;

    void DrawTitlebar(Display * display) {
        char * title;
        if(!XFetchName(display, window, &title)) title = "Window";

        XftDraw * drawCtx = XftDrawCreate(display, titlebar, DefaultVisual(display, 0), DefaultColormap(display, 0));
        if(!drawCtx) {
            cout << "WM:\tFailed to create draw context\n";
            exit(EXIT_FAILURE);
        }

        XftColor color;
        XftColorAllocName(display, DefaultVisual(display, 0), DefaultColormap(display, 0), "white", &color);
        XClearWindow(display, titlebar);

        XGlyphInfo attr;
        XftTextExtentsUtf8(display, font, (const FcChar8 *)title, strlen(title), &attr);
        int text_width = attr.width;


        XftDrawStringUtf8(drawCtx, &color, font, 10, font->ascent + 5, (const XftChar8 *)title, strlen(title));
        XftDrawDestroy(drawCtx);
    }

    void Update(Display * display, Window root) {
        fvec2 currentPos = position.Get();
        fvec2 currentSize = size.Get();

        XResizeWindow(display, window, currentSize.x, currentSize.y);
        XMoveWindow(display, window, currentPos.x, currentPos.y+30);
        
        if(titlebar == root) return;

        XResizeWindow(display, titlebar, currentSize.x, 30);
        XMoveWindow(display, titlebar, currentPos.x, currentPos.y);
    }
};