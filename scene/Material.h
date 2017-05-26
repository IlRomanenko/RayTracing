//
// Created by ilya on 03.03.17.
//
#pragma once

#include <iostream>
#include "../base_headers.h"
using namespace geometry;
using namespace std;

enum MaterialType {
    ReflectDiffuse, ReflectRefract, Diffuse, Transparent
};

class Material {

    size_t materialId;

    ldb reflect, refract, alpha;
    Color color;

    MaterialType type;

    string materialName;

public:

    Material() = delete;

    Material(size_t materialId, Color color, ldb alpha, ldb reflect, ldb refract, const string &materialName) {
        this->materialId = materialId;
        this->color = color;
        this->alpha = alpha;
        this->reflect = reflect;
        this->refract = refract;
        this->materialName = materialName;

        if (Double::notEqual(alpha, 1)) {
            type = MaterialType::Transparent;
        } else if (Double::equal(reflect, 0) && Double::equal(refract, 0)) {
            type = MaterialType::Diffuse;
        } else if (Double::equal(refract, 0)) {
            type = MaterialType::ReflectDiffuse;
        } else {
            type = MaterialType::ReflectRefract;
        }
    }

    virtual ~Material() {}

    ldb getRefract() const {
        return refract;
    }

    ldb getReflect() const {
        return reflect;
    }

    ldb getAlpha() const {
        return alpha;
    }

    virtual Color getColor(const Vector &pos) const {
        return color;
    }

    MaterialType getType() const {
        return type;
    }

    const string &getMaterialName() const {
        return materialName;
    }

//    friend ostream &operator<<(ostream &stream, const Material &material);

    virtual ostream &operator<<(ostream &stream) const {

        stream << "\tentry" << endl;
        stream << "\t\tname " << materialName << endl;
        stream << "\t\tcolor " << (u_int) (color.r * 255)
               << ' ' << (u_int) (color.g * 255)
               << ' ' << (u_int) (color.b * 255) << endl;

        stream << "\t\talpha " << alpha << endl;
        stream << "\t\treflect " << reflect << endl;
        stream << "\t\trefract " << refract << endl;
        stream << "\tendentry" << endl;

        return stream;
    }

};

