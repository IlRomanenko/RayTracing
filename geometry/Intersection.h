//
// Created by ilya on 04.03.17.
//
#pragma once

#include "Primitives.h"
using namespace geometry;

class RayCoefIntersection {
    bool has_intersection;

    ldb intersection_coef;

public:
    RayCoefIntersection() {
        has_intersection = false;
    }

    RayCoefIntersection(ldb intersectionPointRayCoef) {
        has_intersection = true;
        intersection_coef = intersectionPointRayCoef;
    }

    operator bool () const {
        return has_intersection;
    }

    ldb getIntersectionPointCoef() const {
        return intersection_coef;
    }
};