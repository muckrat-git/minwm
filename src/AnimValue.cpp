#pragma once

#include <algorithm>
#include <Vec2.cpp>

using namespace std;

// Generalized lerp
float lerp(float a, float b, float x) { return a + (b - a) * x; }
Vec2<float> lerp(Vec2<float> a, Vec2<float> b, float x) { return a.Lerp(b, x); }

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