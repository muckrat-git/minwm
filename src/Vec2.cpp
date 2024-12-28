#pragma once

#include <iostream>
#include <unordered_map>
#include <math.h>

using namespace std;

#define vec2(x, y) (Vec2<__typeof__(x)>{x,y})

template<typename type>
struct Vec2 {
    type x;
    type y;

    // Basic arithmatic operators
    Vec2 operator-(const Vec2 &rhs) const {
        return {
            x - rhs.x,
            y - rhs.y
        };
    }
    Vec2 operator+(const Vec2 &rhs) const {
        return {
            x + rhs.x,
            y + rhs.y
        };
    }
    Vec2 operator/(const Vec2 &rhs) const {
        return {
            x / rhs.x,
            y / rhs.y
        };
    }
    Vec2 operator/(const type &rhs) const {
        return {
            x / rhs,
            y / rhs
        };
    }
    Vec2 operator*(const Vec2 &rhs) const {
        return {
            x * rhs.x,
            y * rhs.y
        };
    }
    Vec2 operator*(const type &rhs) const {
        return {
            x * rhs,
            y * rhs
        };
    }

    Vec2 operator+=(const Vec2 &rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    Vec2 operator-=(const Vec2 &rhs) {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    bool operator==(const Vec2 &rhs) const {
        return x == rhs.x && y == rhs.y;
    }

    bool operator!=(const Vec2 &rhs) const {
        return !operator==(rhs);
    }

    float Distance(Vec2 b) {
        return sqrtf(
            powf((float)x - (float)b.x, 2) + powf((float)y - (float)b.y, 2)
        );
    }

    string Str() {
        return to_string(x) + "," + to_string(y);
    }

    bool operator <(const Vec2 rhs) const {
        hash<string> hasher;
        return hasher(to_string(x) + "," + to_string(y)) < hasher(to_string(rhs.x) + "," + to_string(rhs.y));
    }

    // Convert any Vec2 to a floating point Vec2
    Vec2<float> Float() {
        return Vec2<float>{(float)x, (float)y};
    }

    // Convert any Vec2 to an integer Vec2
    Vec2<int> Int() {
        return Vec2<int>{(int)x, (int)y};
    }

    // Rounds a Vec2 to the closest whole number
    Vec2<type> Round() {
        return Vec2<type>{round(x), round(y)};
    }

    // Provides a rounded integer Vec2
    Vec2<int> iRound() {
        return Round().Int();
    }

    // Provides a Vec2 rounded down to the nearest whole number
    Vec2<type> Floor() {
        return Vec2<type>{floor(x), floor(y)};
    }

    Vec2<type> Lerp(Vec2<type> rhs, float alpha) {
        return *this + (rhs - *this) * alpha;
    }
};

using fvec2 = Vec2<float>;
using ivec2 = Vec2<int>;