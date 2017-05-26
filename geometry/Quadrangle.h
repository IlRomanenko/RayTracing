//
// Created by ilya on 04.03.17.
//
#pragma once

#include "Polygon.h"

class Quadrangle : public Polygon<4> {

protected:

public:

    Quadrangle(const vector<Vector> &trianglePoints, Material *quadrangleMaterial) : Polygon(trianglePoints) {
        material = quadrangleMaterial;
    }

    Quadrangle(const vector<Vector> &trianglePoints, const vector<Vector> &textureCoords, Material *quadrangleMaterial)
            : Polygon(trianglePoints, textureCoords) {
        material = quadrangleMaterial;
    }


    ostream &operator<<(ostream &stream) override;

};

ostream &Quadrangle::operator<<(ostream &stream) {
    stream << "\tquadrangle" << endl;

    for (auto pnt : points) {
        stream << "\t\tvertex " <<  pnt.x << ' ' << pnt.y << ' ' << pnt.z << endl;
    }

    stream << "\t\tmaterial " << material->getMaterialName() << endl;
    stream << "\tendquadrangle" << endl;
    return stream;
}
