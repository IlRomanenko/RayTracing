//
// Created by ilya on 04.03.17.
//
#pragma once

#include "Polygon.h"

class Quadrangle : public Polygon<4> {

protected:

public:
    Quadrangle(const array<Vector, 4> &trianglePoints, Material* quadrangleMaterial) : Polygon(trianglePoints) {
        material = quadrangleMaterial;
    }

    Quadrangle(const vector<Vector> &trianglePoints, Material* quadrangleMaterial) : Polygon(trianglePoints) {
        material = quadrangleMaterial;
    }

};

