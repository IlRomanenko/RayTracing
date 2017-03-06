//
// Created by ilya on 06.03.17.
//
#pragma once

#include "Primitives.h"
#include "Triangle.h"

using namespace geometry;

class BoundingBox {

    Point center;
    ldb x_size, y_size, z_size;

public:

    /***
     * Need half full size of boungind box!
     */
    BoundingBox(Point center, ldb x_size, ldb y_size, ldb z_size) {
        this->center = center;
        this->x_size = x_size;
        this->y_size = y_size;
        this->z_size = z_size;
    }

    template<int pointsCount>
    BoundingBox(Polygon<pointsCount> triangle) {

        x_size = y_size = z_size = 0;

        const auto& points = triangle.getPoints();
        center = 0;
        for (int i = 0; i < pointsCount; i++) {
            center += points[i];
        }
        center = center / pointsCount;

        Vector tpoint;
        for (const auto& p : points) {
            tpoint = center - p;
            x_size = max(x_size, abs(tpoint.x));
            y_size = max(y_size, abs(tpoint.y));
            z_size = max(z_size, abs(tpoint.z));
        }
    }
};

