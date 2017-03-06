//
// Created by ilya on 03.03.17.
//
#pragma once

#include <array>
#include "IGeometryObject.h"
#include "Polygon.h"

#include <cassert>
class Triangle : public Polygon<3> {

public:

    Triangle() = delete;

    Triangle(const array<Vector, 3> &trianglePoints, Material *triangleMaterial) : Polygon(trianglePoints) {
        material = triangleMaterial;
    }

    Triangle(const vector<Vector> &trianglePoints, Material *triangleMaterial) : Polygon(trianglePoints) {
        material = triangleMaterial;
    }

};