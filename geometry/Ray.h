//
// Created by ilya on 08.03.17.
//
#pragma once

#include "Primitives.h"
#include "IGeometryObject.h"


using namespace geometry;

class IGeometryObject;

struct Ray {
    Point begin;
    Vector direction;

    Ray() {
        begin = 0;
        direction = Vector(1, 0, 0);
    }

    Ray(Point begin, Point end) {
        direction = end - begin;
        direction.normalize();
        this->begin = begin;
    }

    //use with any point
    ldb getLineCoef(const Point &point) const {
        return (point - begin) * direction;
    }

    //use with any point
    ldb distance(const Point &point) const {
        Point npoint = point - begin;
        return (npoint - direction * (direction * npoint)).length();
    }

    Ray getReflectRay(const Point &point, const Vector &normal) const {
        Vector dir = direction * (-1);
        Vector norm_dir = dir - normal * (dir * normal);
        Vector newDir = dir - norm_dir * 2;
        return Ray(point, point + newDir);
    }
};

static Ray LineFromTwoPoints(Point begin, Point end) {
    return Ray(begin, end);
}

class RayCoefIntersection {
    bool has_intersection;

    ldb intersection_coef;

public:
    RayCoefIntersection() {
        has_intersection = false;
    }

    RayCoefIntersection(ldb intersectionPointRayCoef) {
        has_intersection = true;
        intersection_coef = intersectionPointRayCoef;
    }

    operator bool () const {
        return has_intersection;
    }

    ldb getIntersectionPointCoef() const {
        return intersection_coef;
    }
};

struct Intersection {
    ldb rayIntersectionCoef;
    Point intersectionPoint;
    IGeometryObject* object;
    Color color;

    bool has_intersection;

    Intersection() {
        object = nullptr;
        rayIntersectionCoef = 0;
        intersectionPoint = 0;
        has_intersection = false;
    }

    operator bool() {
        return has_intersection;
    }

    bool operator ! () {
        return !has_intersection;
    }
};