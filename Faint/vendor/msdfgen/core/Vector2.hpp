
#pragma once

#include <cmath>
#include "base.h"

namespace msdfgen {

/**
 * A 2-dimensional euclidean floating-point vector.
 * @author Viktor Chlumsky
 */
struct Vec2 {

    double x, y;

    inline Vec2(double val = 0) : x(val), y(val) { }

    inline Vec2(double x, double y) : x(x), y(y) { }

    /// Sets the vector to zero.
    inline void reset() {
        x = 0, y = 0;
    }

    /// Sets individual elements of the vector.
    inline void set(double newX, double newY) {
        x = newX, y = newY;
    }

    /// Returns the vector's squared length.
    inline double squaredLength() const {
        return x*x+y*y;
    }

    /// Returns the vector's length.
    inline double length() const {
        return sqrt(x*x+y*y);
    }

    /// Returns the normalized vector - one that has the same direction but unit length.
    inline Vec2 normalize(bool allowZero = false) const {
        if (double len = length())
            return Vec2(x/len, y/len);
        return Vec2(0, !allowZero);
    }

    /// Returns a vector with the same length that is orthogonal to this one.
    inline Vec2 getOrthogonal(bool polarity = true) const {
        return polarity ? Vec2(-y, x) : Vec2(y, -x);
    }

    /// Returns a vector with unit length that is orthogonal to this one.
    inline Vec2 getOrthonormal(bool polarity = true, bool allowZero = false) const {
        if (double len = length())
            return polarity ? Vec2(-y/len, x/len) : Vec2(y/len, -x/len);
        return polarity ? Vec2(0, !allowZero) : Vec2(0, -!allowZero);
    }

#ifdef MSDFGEN_USE_CPP11
    inline explicit operator bool() const {
        return x || y;
    }
#else
    inline operator const void *() const {
        return x || y ? this : NULL;
    }
#endif

    inline Vec2 &operator+=(const Vec2 other) {
        x += other.x, y += other.y;
        return *this;
    }

    inline Vec2 &operator-=(const Vec2 other) {
        x -= other.x, y -= other.y;
        return *this;
    }

    inline Vec2 &operator*=(const Vec2 other) {
        x *= other.x, y *= other.y;
        return *this;
    }

    inline Vec2 &operator/=(const Vec2 other) {
        x /= other.x, y /= other.y;
        return *this;
    }

    inline Vec2 &operator*=(double value) {
        x *= value, y *= value;
        return *this;
    }

    inline Vec2 &operator/=(double value) {
        x /= value, y /= value;
        return *this;
    }

};

/// A vector may also represent a point, which shall be differentiated semantically using the alias Point2.
typedef Vec2 Point2;

/// Dot product of two vectors.
inline double dotProduct(const Vec2 a, const Vec2 b) {
    return a.x*b.x+a.y*b.y;
}

/// A special version of the cross product for 2D vectors (returns scalar value).
inline double crossProduct(const Vec2 a, const Vec2 b) {
    return a.x*b.y-a.y*b.x;
}

inline bool operator==(const Vec2 a, const Vec2 b) {
    return a.x == b.x && a.y == b.y;
}

inline bool operator!=(const Vec2 a, const Vec2 b) {
    return a.x != b.x || a.y != b.y;
}

inline Vec2 operator+(const Vec2 v) {
    return v;
}

inline Vec2 operator-(const Vec2 v) {
    return Vec2(-v.x, -v.y);
}

inline bool operator!(const Vec2 v) {
    return !v.x && !v.y;
}

inline Vec2 operator+(const Vec2 a, const Vec2 b) {
    return Vec2(a.x+b.x, a.y+b.y);
}

inline Vec2 operator-(const Vec2 a, const Vec2 b) {
    return Vec2(a.x-b.x, a.y-b.y);
}

inline Vec2 operator*(const Vec2 a, const Vec2 b) {
    return Vec2(a.x*b.x, a.y*b.y);
}

inline Vec2 operator/(const Vec2 a, const Vec2 b) {
    return Vec2(a.x/b.x, a.y/b.y);
}

inline Vec2 operator*(double a, const Vec2 b) {
    return Vec2(a*b.x, a*b.y);
}

inline Vec2 operator/(double a, const Vec2 b) {
    return Vec2(a/b.x, a/b.y);
}

inline Vec2 operator*(const Vec2 a, double b) {
    return Vec2(a.x*b, a.y*b);
}

inline Vec2 operator/(const Vec2 a, double b) {
    return Vec2(a.x/b, a.y/b);
}

}
