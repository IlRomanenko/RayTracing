//
// Created by ilya on 03.03.17.
//
#pragma once

#include <cmath>
#include <cstdlib>
#include <array>

using std::array;

namespace geometry {

#define ACCURACY 1

#ifdef ACCURACY
    typedef double ldb;
    const ldb eps = 1e-9;
#else
    typedef float ldb;
    const ldb eps = 1e-6;
#endif

#define sqr(x) ((x) * (x))

    struct Color {

        ldb r, g, b;

        Color() : r(0), g(0), b(0) {}

        Color(int R, int G, int B) {
            R = min(abs(R), 255);
            G = min(abs(G), 255);
            B = min(abs(B), 255);
            r = R / (ldb) 255.0;
            g = G / (ldb) 255.0;
            b = B / (ldb) 255.0;
        }

        explicit Color(float *rgb) {
            r = *rgb;
            g = *(rgb + 1);
            b = *(rgb + 2);
        }

        Color(ldb R, ldb G, ldb B) {
            r = R;
            g = G;
            b = B;
        }

        Color operator*(const ldb &f) const {
            return Color(r * f, g * f, b * f);
        }

        Color operator+(const Color &other) const {
            return Color(r + other.r, g + other.g, b + other.b);
        }

        ldb L2Norm(const Color &other) {
            return sqrt(sqr(r - other.r) + sqr(g - other.g) + sqr(b - other.b));
        }

        ldb L1Norm(const Color &other) {
            return abs(r - other.r) + abs(g - other.g) + abs(b - other.b);
        }

        Color normalize() {
            return Color(min(abs(r), 1.0), min(abs(g), 1.0), min(abs(b), 1.0));
        }

    };

    namespace Double {
        static bool greater(const ldb &a, const ldb &b) {
            return a - eps > b;
        }

        static bool less(const ldb &a, const ldb &b) {
            return a + eps < b;
        }

        static bool equal(const ldb &a, const ldb &b) {
            return fabs(a - b) < eps;
        }

        static bool notEqual(const ldb &a, const ldb &b) {
            return !equal(a, b);
        }

        static bool lessEqual(const ldb &a, const ldb &b) {
            return !greater(a, b);
        }

        static bool greaterEqual(const ldb &a, const ldb &b) {
            return !less(a, b);
        }
    }

    struct Vector {
        ldb x, y, z;

        Vector() : x(0.0), y(0.0), z(0.0) {}

        Vector(ldb X) : x(X), y(0.0), z(0.0) {}

        Vector(ldb X, ldb Y) : x(X), y(Y), z(0.0) {}

        Vector(ldb X, ldb Y, ldb Z) : x(X), y(Y), z(Z) {}

        ldb sqrLength() const {
            return x * x + y * y + z * z;
        }

        ldb length() const {
            return sqrt(sqrLength());
        }

        Vector operator+(const Vector &other) const {
            return Vector(x + other.x, y + other.y, z + other.z);
        }

        Vector operator-(const Vector &other) const {
            return Vector(x - other.x, y - other.y, z - other.z);
        }

        Vector operator*(const ldb val) const {
            return Vector(x * val, y * val, z * val);
        }

        Vector operator/(const ldb val) const {
            return Vector(x / val, y / val, z / val);
        }

        Vector &operator+=(const Vector &other) {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        Vector &operator-=(const Vector &other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        Vector &operator*=(const ldb val) {
            x *= val;
            y *= val;
            z *= val;
            return *this;
        }

        ldb operator*(const Vector &other) const {
            return x * other.x + y * other.y + z * other.z;
        }

        Vector operator^(const Vector &other) const {
            return Vector(
                    y * other.z - z * other.y,
                    -(x * other.z - z * other.x),
                    x * other.y - y * other.x
            );
        }

        const array<ldb, 3> getCoordArray() const {
            return {x, y, z};
        }

        void normalize() {
            ldb len = length();
            x /= len;
            y /= len;
            z /= len;
        }
    };

    typedef Vector Point;
}