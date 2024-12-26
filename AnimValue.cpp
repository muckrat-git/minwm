#pragma once

#include <raylib.h>
#include <raymath.h>
#include <algorithm>

using namespace std;

// Generalized lerp
float lerp(float a, float b, float x) { return Lerp(a, b, x); }
Vector2 lerp(Vector2 a, Vector2 b, float x) { return Vector2Lerp(a, b, x); }
Rectangle lerp(Rectangle a, Rectangle b, float x) {
    return {
        .x =        Lerp(a.x, b.x, x),
        .y =        Lerp(a.y, b.y, x),
        .width =    Lerp(a.width, b.width, x),
        .height =   Lerp(a.height, b.height, x)
    };
}

template <typename type>
class AnimValue {
    private:
    type src, dest;
    double srcT, moveT;

    public:
    type Get() {
        if(moveT == 0) return dest;
        return lerp(src, dest, min(1.0,(GetTime() - srcT) / moveT));
    }
    void Set(type val, double time) {
        src = Get();
        dest = val;
        moveT = time;
        srcT = GetTime();
    }

    AnimValue() = default;

    // Constructor
    AnimValue(type val) {
        dest = val;
        moveT = 0;
    }
};