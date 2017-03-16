//
// Created by ilya on 03.03.17.
//
#pragma once

#include "../base_headers.h"

using geometry::Vector;
using geometry::Point;

struct Reference {
    ldb power, distance;
    Reference() {
        power = 0;
        distance = 1;
    }

    Reference(ldb power, ldb distance) {
        this->power = power;
        this->distance = distance;
    }
};

class Light {

    Reference ref;

    ldb power;
    Point position;

public:
    Light(Reference reference, ldb power, Point position) {
        ref = reference;
        this->power = power;
        this->position = position;
    }

    const Reference &getReference() const {
        return ref;
    }

    const ldb &getPower() const {
        return power;
    }

    const Point &getPosition() const {
        return position;
    }
};
