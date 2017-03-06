//
// Created by ilya on 03.03.17.
//
#pragma once

#include "../base_headers.h"
#include "../geometry/Primitives.h"

using geometry::Vector;

class Viewport {
    Vector origin, t_left, b_left, t_right;
public:
    Viewport() { }

    Viewport(Vector Origin, Vector TopLeft, Vector TopRight, Vector BottomLeft) {
        origin = Origin;
        t_left = TopLeft;
        t_right = TopRight;
        b_left = BottomLeft;
    }

    void setOrigin(const Vector& vect) {
        origin = vect;
    }

    void setTopLeft(const Vector &vect) {
        t_left = vect;
    }

    void setBottomLeft(const Vector &vect) {
        b_left = vect;
    }

    void setTopRight(const Vector &vect) {
        t_right = vect;
    }

    const Vector getOrigin() const {
        return origin;
    }

    const Vector getWidthBase() const {
        return t_right - t_left;
    }

    const Vector getHeightBase() const {
        return b_left - t_left;
    }

    const Vector getTopLeft() const {
        return t_left;
    }
};