//
// Created by ilya on 03.03.17.
//
#pragma once

#include "Primitives.h"
#include "BoundingBox.h"
#include "Ray.h"

#include "../scene/Material.h"

using namespace geometry;

class IGeometryObject {
protected:

    Material *material;
    Vector normal;

public:
    virtual Vector getNormal(Point) const {
        return normal;
    }

    virtual Material* getMaterial() const {
        return material;
    }

    virtual BoundingBox getBoundingBox() const = 0;

    virtual RayCoefIntersection intersect(const Ray &ray) const = 0;

    virtual int getTag() const {
        return 0;
    }

    virtual ~IGeometryObject() {
        if (material != nullptr) {
            material->dispose();
        }
    }
};