//
// Created by ilya on 03.03.17.
//
#pragma once

#include <array>
#include "Polygon.h"

class Triangle : public Polygon<3> {

public:

    Triangle() = delete;

    Triangle(const vector<Vector> &trianglePoints, Material *triangleMaterial) : Polygon(trianglePoints) {
        material = triangleMaterial;
    }

    Triangle(const vector<Vector> &trianglePoints, const vector<Vector> &textureCoords, Material *triangleMaterial)
            : Polygon(trianglePoints, textureCoords) {
        material = triangleMaterial;
    }


    ostream &operator<<(ostream &stream) override;
};

ostream &Triangle::operator<<(ostream &stream) {
    stream << "\ttriangle" << endl;

    for (auto pnt : points) {
        stream << "\t\tvertex " <<  pnt.x << ' ' << pnt.y << ' ' << pnt.z << endl;
    }


    stream << "\t\tmaterial " << material->getMaterialName() << endl;
    stream << "\tendtriangle" << endl;
    return stream;
}
