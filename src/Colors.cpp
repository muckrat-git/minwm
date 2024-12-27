#include <X11/Xlib.h>
#include <X11/X.h>
#include <iostream>

using namespace std;

#define u8 unsigned char
#define u16 unsigned short

XColor borderC;
XColor backgroundC;
XColor tilebarBgC;

XColor FromRGB(u8 r, u8 g, u8 b) {
    XColor color = {.red = (u16)((u16)r * 257), .green = (u16)((u16)g * 257), .blue = (u16)((u16)b * 257)};
    color.flags = DoRed | DoGreen | DoBlue;
    return color;
}

void LoadColors(Display * display, int screen) {
    borderC = FromRGB(102, 102, 102);
    backgroundC = FromRGB(20, 20, 20);
    tilebarBgC = FromRGB(33, 33, 33);

    Colormap colormap = DefaultColormap(display, screen);

    if(!XAllocColor(display, colormap, &borderC)) {
        cout << "WM\tFailed to allocate color 'borderC'\n";
        exit(EXIT_FAILURE);
    }
    if(!XAllocColor(display, colormap, &backgroundC)) {
        cout << "WM\tFailed to allocate color 'backgroundC'\n";
        exit(EXIT_FAILURE);
    }
    if(!XAllocColor(display, colormap, &tilebarBgC)) {
        cout << "WM\tFailed to allocate color 'tilebarBgC'\n";
        exit(EXIT_FAILURE);
    }
}