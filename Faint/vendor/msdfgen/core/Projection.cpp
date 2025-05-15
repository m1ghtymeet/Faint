
#include "Projection.h"

namespace msdfgen {

Projection::Projection() : scale(1), translate(0) { }

Projection::Projection(const Vec2 &scale, const Vec2 &translate) : scale(scale), translate(translate) { }

Point2 Projection::project(const Point2 &coord) const {
    return scale*(coord+translate);
}

Point2 Projection::unproject(const Point2 &coord) const {
    return coord/scale-translate;
}

Vec2 Projection::projectVector(const Vec2 &vector) const {
    return scale*vector;
}

Vec2 Projection::unprojectVector(const Vec2 &vector) const {
    return vector/scale;
}

double Projection::projectX(double x) const {
    return scale.x*(x+translate.x);
}

double Projection::projectY(double y) const {
    return scale.y*(y+translate.y);
}

double Projection::unprojectX(double x) const {
    return x/scale.x-translate.x;
}

double Projection::unprojectY(double y) const {
    return y/scale.y-translate.y;
}

}
