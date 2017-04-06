//
// Created by ilya on 04.03.17.
//
#pragma once

#include "IGeometryObject.h"

class Sphere : public IGeometryObject {

    Point center;
    ldb radius;


public:

    Sphere() = delete;

    Sphere(Point position, ldb radius, Material *material) {
        this->material = material;
        this->center = position;
        this->radius = radius;
    }

    Vector getNormal(Point point) const override {
        assert(Double::lessEqual((point - center).length(), radius));
        Vector norm = point - center;
        norm.normalize();
        return norm;
    }

    RayCoefIntersection intersect(const Ray &ray) const override {


        ldb distance = ray.distance(center);

        if (Double::greater(distance, radius)) {
            return RayCoefIntersection();
        }

        ldb scalar_distance = ray.getLineCoef(center);
        ldb half_sphere_distance = sqrt(abs(radius * radius - distance * distance));
        ldb ray_d = min(scalar_distance - half_sphere_distance, scalar_distance + half_sphere_distance);
        if (Double::less(ray_d, 0)) {
            ray_d = max(scalar_distance - half_sphere_distance, scalar_distance + half_sphere_distance);
        }
        return RayCoefIntersection(ray_d);
    }

    int getTag() const override {
        return 1;
    }

    BoundingBox getBoundingBox() const override {
        return BoundingBox(center, radius, radius, radius);
    }

    ldb getRadius() {
        return radius;
    }

    Vector getPosition() {
        return center;
    }

    ostream &operator<<(ostream &stream) override;

};

ostream &Sphere::operator<<(ostream &stream) {
    stream << "\tsphere" << endl;

    stream << "\t\tcoords " << center.x << ' ' << center.y << ' ' << center.z << endl;
    stream << "\t\tradius " << radius << endl;
    stream << "\t\tmaterial " << material->getMaterialName() << endl;

    stream << "\tendsphere" << endl;
    return stream;
}
