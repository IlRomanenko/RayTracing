//
// Created by ilya on 04.03.17.
//
#pragma once

#include "IGeometryObject.h"

template<int pointsSize>
class Polygon : public IGeometryObject {
protected:
    std::array<Point, pointsSize> points;
    std::array<Point, pointsSize> textureCoords;

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

    Polygon(const std::vector<Vector> &trianglePoints) {
        for (int i = 0; i < pointsSize; i++) {
            points[i] = trianglePoints[i];
        }
        calculateArea();
        calculateNormal();
    }

    Polygon(const std::vector<Vector> &trianglePoints, const std::vector<Vector> &texturePoints) :
            Polygon(trianglePoints) {
        for (int i = 0; i < pointsSize; i++) {
            textureCoords[i] = texturePoints[i];
        }
    }

    Vector getTexturePoint(const Vector &position) const override {
        Vector tempPos = position - points[1];
        static const Vector baseU = textureCoords[2] - textureCoords[1], baseV = textureCoords[0] - textureCoords[1];
        static const Vector baseX = points[2] - points[1], baseY = points[0] - points[1];
        static const ldb baseXLength = baseX.length(), baseYLength = baseY.length();

        return baseU * (tempPos * baseX) / baseXLength / baseXLength + baseV * (tempPos * baseY) / baseYLength / baseYLength;
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

    BoundingBox getBoundingBox() const override {
        return {points.begin(), points.end()};
    }
};