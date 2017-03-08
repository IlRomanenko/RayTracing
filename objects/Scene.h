//
// Created by Ilya on 04.03.17.
//
#pragma once

#include "../base_headers.h"
#include "../geometry/IGeometryObject.h"
#include "../parsers/RT_file.h"
#include "Viewport.h"
#include "../parsers/ObjLoader.h"
#include "../ray-casting/KD_Tree.h"

#include <future>
#include <thread>
#include <queue>
#include <mutex>

using namespace std;

class Scene {

    MaterialsFactory materialsFactory;
    vector<IGeometryObject*> geometry;
    vector<Light> lights;
    Viewport viewport;


    KD_Tree kd_tree;

    vector<future<void> > workers;
    queue<size_t> needToCalcPixels;
    mutex workerMutex;


    const size_t workerPixelsCount = 20;
    size_t width, height;
    float* pixels;

    Intersection castRay(const Ray& ray, IGeometryObject* skip = nullptr) {

        /*Intersection intersection;
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

        return intersection;*/
        return kd_tree.castRay(ray);
    }

    Intersection castRayKD(const Ray& ray, IGeometryObject* skip = nullptr) {
        return kd_tree.castRay(ray);
    }

    Color traceRay(const Ray &ray) {
        Color color(0, 0, 0);

        Intersection intersection = castRayKD(ray);

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

        return color * min(lightIntensity + 0.3, 1.0);
    }

    void getSomePixelsToRender(vector<size_t> &v) {
        unique_lock<mutex> lock(workerMutex);

        for (size_t i = 0; i < min(workerPixelsCount, needToCalcPixels.size()); i++) {
            v.push_back(needToCalcPixels.front());
            needToCalcPixels.pop();
        }
    }

    void renderWorker() {
        size_t current_w, current_h;

        Vector base_w = viewport.getWidthBase() / width, base_h = viewport.getHeightBase() / height;
        Vector origin = viewport.getOrigin();
        vector<size_t> pixelToRender;

        getSomePixelsToRender(pixelToRender);

        while (pixelToRender.size() > 0) {

            while (pixelToRender.size() > 0) {
                size_t pixel = pixelToRender.back();
                pixelToRender.pop_back();

                current_w = pixel / width;
                current_h = pixel % width;

                Color clr = traceRay(LineFromTwoPoints(origin, viewport.getTopLeft() + base_w * current_w + base_h * current_h));
                pixels[pixel * 3] = (float)clr.r;
                pixels[pixel * 3 + 1] = (float)clr.g;
                pixels[pixel * 3 + 2] = (float)clr.b;
            }

            getSomePixelsToRender(pixelToRender);
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
        kd_tree.buildTree(geometry);
    }

    void openScene(const string& filename, size_t width, size_t height) {
        {
            ObjLoader rtFile("examples/obj_examples/buggy2.1.obj", "examples/obj_examples/", materialsFactory, viewport, lights, geometry);
        }
       // clear();
       // RT_file rtFile(filename, materialsFactory, viewport, lights, geometry);
        this->width = width;
        this->height = height;
        pixels = new float[width * height * 3];
        kd_tree.buildTree(geometry);
    }

    void render(size_t numberOfThreads = 8) {
        //TODO change threads strategy

        for (size_t i = 0; i < width * height; i++) {
            needToCalcPixels.push(i);
        }

        size_t part = width * height / numberOfThreads;
        auto workerLambda = [&]() { renderWorker(); };
        for (size_t i = 0; i < numberOfThreads - 1; i++) {
            workers.emplace_back(async(std::launch::async, workerLambda));
        }
        workers.emplace_back(async(std::launch::async, workerLambda));
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

