
#pragma once

#include "Vector2.hpp"

namespace msdfgen {

/// A transformation from shape coordinates to pixel coordinates.
class Projection {

public:
    Projection();
    Projection(const Vec2 &scale, const Vec2 &translate);
    /// Converts the shape coordinate to pixel coordinate.
    Point2 project(const Point2 &coord) const;
    /// Converts the pixel coordinate to shape coordinate.
    Point2 unproject(const Point2 &coord) const;
    /// Converts the vector to pixel coordinate space.
    Vec2 projectVector(const Vec2 &vector) const;
    /// Converts the vector from pixel coordinate space.
    Vec2 unprojectVector(const Vec2 &vector) const;
    /// Converts the X-coordinate from shape to pixel coordinate space.
    double projectX(double x) const;
    /// Converts the Y-coordinate from shape to pixel coordinate space.
    double projectY(double y) const;
    /// Converts the X-coordinate from pixel to shape coordinate space.
    double unprojectX(double x) const;
    /// Converts the Y-coordinate from pixel to shape coordinate space.
    double unprojectY(double y) const;

private:
    Vec2 scale;
    Vec2 translate;

};

}
