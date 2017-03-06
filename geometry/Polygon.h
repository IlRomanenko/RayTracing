//
// Created by ilya on 04.03.17.
//
#pragma once

#include "IGeometryObject.h"

template <int pointsSize>
class Polygon : public IGeometryObject{
protected:
    std::array<Point, pointsSize> points;

    ldb planeCoefficient;
    ldb surfaceArea;

    void calculateNormal() {
        normal = (points[1] - points[0]) ^ (points[2] - points[0]);
        normal.normalize();
        planeCoefficient = normal * points[0];
    }

    void calculateArea() {
        surfaceArea = 0;
        for (int i = 2; i < pointsSize; i++) {
            surfaceArea += ((points[i - 1] - points[0]) ^ (points[i] - points[0])).length();
        }
    }

public:

    Polygon(std::array<Vector, pointsSize> trianglePoints) {
        points = trianglePoints;
        calculateArea();
        calculateNormal();
    }

    Polygon(std::vector<Vector> trianglePoints) {
        for (int i = 0; i < pointsSize; i++) {
            points[i] = trianglePoints[i];
        }
        calculateArea();
        calculateNormal();
    }

    const array<Point, pointsSize> &getPoints() const {
        return points;
    }

    RayCoefIntersection intersect(const Ray &ray) const override {

        ldb t = (planeCoefficient - ray.begin * normal) / (ray.direction * normal);

        Point point = ray.begin + ray.direction * t;


        ldb area = 0;
        for (int i = 0; i < pointsSize - 1; i++) {
            area += ((point - points[i]) ^ (point - points[i + 1])).length();
        }
        area += ((point - points[pointsSize - 1]) ^ (point - points[0])).length();

        if (Double::notEqual(area, surfaceArea) || Double::less(area * surfaceArea, 0)) {
            return RayCoefIntersection();
        }

        return RayCoefIntersection(t);
    }
};