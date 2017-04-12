//
// Created by ilya on 06.03.17.
//
#pragma once

#include <cassert>
#include <array>

#include "Primitives.h"
#include "Ray.h"

using namespace geometry;

class BoundingBox {

    typedef array<array<ldb, 2>, 3> MinMaxCoords;

    MinMaxCoords currentCoords;


    RayCoefIntersection createPlaneAndIntersect(const Ray &ray, const Vector &p1,
                                                const Vector &p2, const Vector &p3) const {
        Vector normal = (p2 - p1) ^(p3 - p1);
        normal.normalize();

        if (Double::equal(ray.direction * normal, 0)) {
            return RayCoefIntersection();
        }

        ldb t = (normal * p1 - ray.begin * normal) / (ray.direction * normal);
        if (!contains(ray.begin + ray.direction * t)) {
            return RayCoefIntersection();
        }
        return RayCoefIntersection(t);
    }

public:

    BoundingBox() {
        currentCoords = {0, 0, 0, 0, 0, 0};
    }

    /***
     * Need half full size of boungind box!
     */
    BoundingBox(Point center, ldb x_size, ldb y_size, ldb z_size) {
        currentCoords[0][0] = center.x - x_size;
        currentCoords[0][1] = center.x + x_size;

        currentCoords[1][0] = center.y - y_size;
        currentCoords[1][1] = center.y + y_size;

        currentCoords[2][0] = center.z - z_size;
        currentCoords[2][1] = center.z + z_size;

    }

    BoundingBox(MinMaxCoords coords_min_max) {
        currentCoords = coords_min_max;
    }

    template<typename ItType>
    BoundingBox(ItType begin, ItType end) {

        currentCoords = {
                array<ldb, 2>{begin->x, begin->x},
                {begin->y, begin->y},
                {begin->z, begin->z}};

        while (begin != end) {
            const auto &coords = begin->getCoordArray();
            for (int i = 0; i < 3; i++) {
                if (Double::less(coords[i], currentCoords[i][0])) {
                    currentCoords[i][0] = coords[i];
                }
                if (Double::greater(coords[i], currentCoords[i][1])) {
                    currentCoords[i][1] = coords[i];
                }
            }
            begin++;
        }
    }

    template<int pointsCount>
    BoundingBox(const array<Point, pointsCount> &points) {

        currentCoords = {
                array<ldb, 2>{points[0].x, points[0].x},
                {points[0].y, points[0].y},
                {points[0].z, points[0].z}};

        for (const auto &pnt : points) {
            const auto &coords = pnt.getCoordArray();
            for (int i = 0; i < 3; i++) {
                if (Double::less(coords[i], currentCoords[i][0])) {
                    currentCoords[i][0] = coords[i];
                }
                if (Double::greater(coords[i], currentCoords[i][1])) {
                    currentCoords[i][1] = coords[i];
                }
            }
        }
    }

    const MinMaxCoords getMinMaxCoordinates() const {
        return currentCoords;
    }

    void expand(const BoundingBox &other) {
        const auto otherCoords = other.getMinMaxCoordinates();
        for (int i = 0; i < 3; i++) {
            currentCoords[i][0] = min(currentCoords[i][0], otherCoords[i][0]);
            currentCoords[i][1] = max(currentCoords[i][1], otherCoords[i][1]);
        }
    }

    bool contains(const Point &point) const {
        return Double::lessEqual(currentCoords[0][0], point.x) && Double::lessEqual(point.x, currentCoords[0][1]) &&
               Double::lessEqual(currentCoords[1][0], point.y) && Double::lessEqual(point.y, currentCoords[1][1]) &&
               Double::lessEqual(currentCoords[2][0], point.z) && Double::lessEqual(point.z, currentCoords[2][1]);
    }

    const array<BoundingBox, 2> split(size_t axisNumber, ldb value) const {

        assert((axisNumber < 3));

        MinMaxCoords leftCoords = currentCoords, rightCoords = currentCoords;

        leftCoords[axisNumber][1] = value;
        rightCoords[axisNumber][0] = value;

        return {BoundingBox(leftCoords), BoundingBox(rightCoords)};
    }

    ldb calculateSurfaceArea() const {
        ldb x_size = currentCoords[0][1] - currentCoords[0][0];
        ldb y_size = currentCoords[1][1] - currentCoords[1][0];
        ldb z_size = currentCoords[2][1] - currentCoords[2][0];
        return 2 * (x_size * y_size + x_size * z_size + y_size * z_size);
    }

    ldb calculateVolume() const {
        ldb ans = 1;
        for (size_t i = 0; i < 3; i++) {
            ans *= currentCoords[i][1] - currentCoords[i][0];
        }
        return ans;
    }

    pair<bool, array<RayCoefIntersection, 2> > intersect2(const Ray &ray) const {

        RayCoefIntersection pu, pd, pl, pr, pn, pf;
        pd = createPlaneAndIntersect(ray,
                                     Vector(currentCoords[0][0], currentCoords[1][0], currentCoords[2][0]),
                                     Vector(currentCoords[0][1], currentCoords[1][0], currentCoords[2][0]),
                                     Vector(currentCoords[0][0], currentCoords[1][1], currentCoords[2][0]));
        pu = createPlaneAndIntersect(ray,
                                     Vector(currentCoords[0][0], currentCoords[1][0], currentCoords[2][1]),
                                     Vector(currentCoords[0][1], currentCoords[1][0], currentCoords[2][1]),
                                     Vector(currentCoords[0][0], currentCoords[1][1], currentCoords[2][1]));

        pl = createPlaneAndIntersect(ray,
                                     Vector(currentCoords[0][0], currentCoords[1][0], currentCoords[2][0]),
                                     Vector(currentCoords[0][0], currentCoords[1][1], currentCoords[2][0]),
                                     Vector(currentCoords[0][0], currentCoords[1][1], currentCoords[2][1]));

        pr = createPlaneAndIntersect(ray,
                                     Vector(currentCoords[0][1], currentCoords[1][0], currentCoords[2][0]),
                                     Vector(currentCoords[0][1], currentCoords[1][1], currentCoords[2][0]),
                                     Vector(currentCoords[0][1], currentCoords[1][1], currentCoords[2][1]));

        pn = createPlaneAndIntersect(ray,
                                     Vector(currentCoords[0][0], currentCoords[1][0], currentCoords[2][0]),
                                     Vector(currentCoords[0][1], currentCoords[1][0], currentCoords[2][0]),
                                     Vector(currentCoords[0][1], currentCoords[1][0], currentCoords[2][1]));
        pf = createPlaneAndIntersect(ray,
                                     Vector(currentCoords[0][0], currentCoords[1][1], currentCoords[2][0]),
                                     Vector(currentCoords[0][1], currentCoords[1][1], currentCoords[2][0]),
                                     Vector(currentCoords[0][1], currentCoords[1][1], currentCoords[2][1]));

        vector<ldb> coefs;
        if (pd) coefs.push_back(pd.getIntersectionPointCoef());
        if (pu) coefs.push_back(pu.getIntersectionPointCoef());

        if (pl) coefs.push_back(pl.getIntersectionPointCoef());
        if (pr) coefs.push_back(pr.getIntersectionPointCoef());

        if (pn) coefs.push_back(pn.getIntersectionPointCoef());
        if (pf) coefs.push_back(pf.getIntersectionPointCoef());
        sort(coefs.begin(), coefs.end());

        if (coefs.size() == 1) {
            return make_pair(false, array<RayCoefIntersection, 2>{coefs.front(), coefs.front()});
        } else if (coefs.size() == 0 || coefs.back() < 0) {
            return make_pair(false, array<RayCoefIntersection, 2>{-1, -1});
        }

        //assert(coefs.front() > 0);
        return make_pair(true, array<RayCoefIntersection, 2>{RayCoefIntersection(coefs.front()),
                                                             RayCoefIntersection(coefs.back())});
    }

    pair<bool, array<RayCoefIntersection, 2> > intersect(const Ray &ray) const {

        RayCoefIntersection pu, pd, pl, pr, pn, pf;

        pd = createPlaneAndIntersect(ray,
                                     Vector(currentCoords[0][0], currentCoords[1][0], currentCoords[2][0]),
                                     Vector(currentCoords[0][1], currentCoords[1][0], currentCoords[2][0]),
                                     Vector(currentCoords[0][0], currentCoords[1][1], currentCoords[2][0]));
        pu = createPlaneAndIntersect(ray,
                                     Vector(currentCoords[0][0], currentCoords[1][0], currentCoords[2][1]),
                                     Vector(currentCoords[0][1], currentCoords[1][0], currentCoords[2][1]),
                                     Vector(currentCoords[0][0], currentCoords[1][1], currentCoords[2][1]));

        pl = createPlaneAndIntersect(ray,
                                     Vector(currentCoords[0][0], currentCoords[1][0], currentCoords[2][0]),
                                     Vector(currentCoords[0][0], currentCoords[1][1], currentCoords[2][0]),
                                     Vector(currentCoords[0][0], currentCoords[1][1], currentCoords[2][1]));

        pr = createPlaneAndIntersect(ray,
                                     Vector(currentCoords[0][1], currentCoords[1][0], currentCoords[2][0]),
                                     Vector(currentCoords[0][1], currentCoords[1][1], currentCoords[2][0]),
                                     Vector(currentCoords[0][1], currentCoords[1][1], currentCoords[2][1]));

        pn = createPlaneAndIntersect(ray,
                                     Vector(currentCoords[0][0], currentCoords[1][0], currentCoords[2][0]),
                                     Vector(currentCoords[0][1], currentCoords[1][0], currentCoords[2][0]),
                                     Vector(currentCoords[0][1], currentCoords[1][0], currentCoords[2][1]));
        pf = createPlaneAndIntersect(ray,
                                     Vector(currentCoords[0][0], currentCoords[1][1], currentCoords[2][0]),
                                     Vector(currentCoords[0][1], currentCoords[1][1], currentCoords[2][0]),
                                     Vector(currentCoords[0][1], currentCoords[1][1], currentCoords[2][1]));

        vector<ldb> coefs;
        if (pd) coefs.push_back(pd.getIntersectionPointCoef());
        if (pu) coefs.push_back(pu.getIntersectionPointCoef());

        if (pl) coefs.push_back(pl.getIntersectionPointCoef());
        if (pr) coefs.push_back(pr.getIntersectionPointCoef());

        if (pn) coefs.push_back(pn.getIntersectionPointCoef());
        if (pf) coefs.push_back(pf.getIntersectionPointCoef());
        sort(coefs.begin(), coefs.end());

        if (coefs.size() == 1) {
            assert(false);
            return make_pair(false, array<RayCoefIntersection, 2>{coefs.front(), coefs.front()});
        } else if (coefs.size() == 0 || coefs.back() < 0) {
            return make_pair(false, array<RayCoefIntersection, 2>{-1, -1});
        }

        auto first_coef = *find_if(coefs.begin(), coefs.end(), [](ldb x) { return x > 0; });

        //assert(coefs.front() > 0);
        //return make_pair(true, array<RayCoefIntersection, 2>{RayCoefIntersection(coefs.front()),
        //                                                     RayCoefIntersection(coefs.back())});
        return make_pair(true, array<RayCoefIntersection, 2>{RayCoefIntersection(first_coef),
                                                             RayCoefIntersection(coefs.back())});
    }


    ldb getMin(const size_t axisNumber) const {
        assert(axisNumber < 3);
        return currentCoords[axisNumber][0];
    }

    ldb getMax(const size_t axisNumber) const {
        assert(axisNumber < 3);
        return currentCoords[axisNumber][1];
    }
};

