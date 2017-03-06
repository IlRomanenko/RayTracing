//
// Created by ilya on 04.03.17.
//
#pragma once

#include "IGeometryObject.h"

class Sphere : public IGeometryObject {

    Point position;
    ldb radius;


public:

    Sphere() = delete;

    Sphere(Point position, ldb radius, Material *material) {
        this->material = material;
        this->position = position;
        this->radius = radius;
    }

    Vector getNormal(Point point) const override {
        assert(Double::lessEqual((point - position).length(), radius));
        Vector norm = point - position;
        norm.normalize();
        return norm;
    }

    RayCoefIntersection intersect(const Ray &ray) const override {


        ldb distance = ray.distance(position);

        if (Double::greater(distance, radius)) {
            return RayCoefIntersection();
        }

        ldb scalar_distance = ray.getLineCoef(position);
        ldb half_sphere_distance = sqrt(abs(radius * radius - distance * distance));
        ldb ray_d = min(scalar_distance - half_sphere_distance, scalar_distance + half_sphere_distance);

        return RayCoefIntersection(ray_d);
    }
};