//
// Created by ilya on 03.03.17.
//
#pragma once

#include "Primitives.h"
#include "../objects/Material.h"
#include "Intersection.h"

using namespace geometry;

class IGeometryObject {
protected:

    Material *material;
    Vector normal;

public:
    virtual Vector getNormal(Point point) const {
        return normal;
    }

    virtual Material* getMaterial() const {
        return material;
    }

    virtual RayCoefIntersection intersect(const Ray &ray) const = 0;

    virtual ~IGeometryObject() {
        if (material != nullptr) {
            material->dispose();
        }
    }
};