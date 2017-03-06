//
// Created by Ilya on 04.03.17.
//
#pragma once

#include "../base_headers.h"
#include "../geometry/IGeometryObject.h"
#include "../parsers/RT_file.h"
#include "Viewport.h"
#include "../parsers/ObjLoader.h"

#include <future>
#include <thread>
using namespace std;

class Scene {

    MaterialsFactory materialsFactory;
    vector<IGeometryObject*> geometry;
    vector<Light> lights;
    Viewport viewport;

    vector<future<void>> workers;

    size_t width, height;

    float* pixels;

    struct Intersection {
        ldb rayIntersectionCoef;
        Point intersectionPoint;
        IGeometryObject* object;

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

    Intersection castRay(const Ray& ray, IGeometryObject* skip = nullptr) {

        Intersection intersection;
        RayCoefIntersection rayIntersection;

        ldb last_point = 1e30, current_point;

        for (IGeometryObject* object : geometry) {

            if (object == skip) {
                continue;
            }

            rayIntersection = object->intersect(ray);
            if (!rayIntersection) {
                continue;
            }

            current_point = rayIntersection.getIntersectionPointCoef();

            if (Double::greater(current_point, 0) && Double::less(current_point, last_point)) {
                last_point = current_point;
                intersection.object = object;
                intersection.has_intersection = true;
            }
        }

        intersection.rayIntersectionCoef = last_point;
        intersection.intersectionPoint = ray.begin + ray.direction * last_point;

        return intersection;
    }

    Color traceRay(const Ray &ray) {
        Color color(0, 0, 0);

        Intersection intersection = castRay(ray);

        if (!intersection) {
            return color;
        }

        ldb lightIntensity = 0;

        for (const Light &light : lights) {
            ldb lightPositionCoef = ray.getLineCoef(light.getPosition());

            Ray newRay = LineFromTwoPoints(intersection.intersectionPoint, light.getPosition());
            newRay.begin += newRay.direction * 1e-3;
            Intersection lightIntersection = castRay(newRay,
                                                        intersection.object);

            if (!lightIntersection || Double::greater(lightIntersection.rayIntersectionCoef, lightPositionCoef)) {

                Vector normLightPos = light.getPosition() - intersection.intersectionPoint;
                ldb inv_length = normLightPos.sqrLength();
                normLightPos.normalize();

                ldb currentPointLightContribution = intersection.object->getNormal(intersection.intersectionPoint) * normLightPos;
                lightIntensity += max(currentPointLightContribution , (ldb)0.0);
                //TODO add light
            }
        }

        color = intersection.object->getMaterial()->getColor();

        return color * min(lightIntensity + 0.2, 1.0);
    }

    void renderWorker(size_t firstPixel, size_t lastPixel) {
        size_t current_w, current_h;

        Vector base_w = viewport.getWidthBase() / width, base_h = viewport.getHeightBase() / height;
        Vector origin = viewport.getOrigin();


        while (firstPixel != lastPixel) {
            current_w = firstPixel / width;
            current_h = firstPixel % width;

            Color clr = traceRay(LineFromTwoPoints(origin, viewport.getTopLeft() + base_w * current_w + base_h * current_h));

            pixels[firstPixel * 3] = (float)clr.r;
            pixels[firstPixel * 3 + 1] = (float)clr.g;
            pixels[firstPixel * 3 + 2] = (float)clr.b;
            firstPixel++;
        }
    }

public:

    Scene() {
        width = height = 0;
    }

    Scene(const string& filename, size_t width, size_t height) {
        RT_file rtFile(filename, materialsFactory, viewport, lights, geometry);
        this->width = width;
        this->height = height;
        pixels = new float[width * height * 3];
    }

    void openScene(const string& filename, size_t width, size_t height) {
        {
            ObjLoader rtFile("examples/obj_examples/scene.obj", "examples/obj_examples/", materialsFactory, viewport, lights, geometry);
        }
        /*clear();
        RT_file rtFile(filename, materialsFactory, viewport, lights, geometry);*/
        this->width = width;
        this->height = height;
        pixels = new float[width * height * 3];
    }

    void render(size_t numberOfThreads = 16) {
        size_t part = width * height / numberOfThreads;
        auto workerLambda = [&](size_t first, size_t last) {renderWorker(first, last); };
        for (size_t i = 0; i < numberOfThreads - 1; i++) {
            workers.emplace_back(async(std::launch::async, workerLambda, part * i, part * (i + 1)));
        }
        workers.emplace_back(async(std::launch::async, workerLambda, part * (numberOfThreads - 1), width * height));
    }

    bool isBusy() const {
        bool busy = false;
        for (const auto &th : workers) {
            if (th.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
                busy = true;
            }
        }
        return busy;
    }

    const float* getPixels() const {
        return pixels;
    }

    void clear() {
        for (IGeometryObject *object : geometry) {
            delete object;
        }
        lights.clear();
    }

    ~Scene() {
        for (auto &th : workers) {
            th.get();
        }
    }
};

