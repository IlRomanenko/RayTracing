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
#include "../utilities/ThreadPool.h"
#include "../geometry/Ray.h"

#include <future>
#include <thread>
#include <queue>
#include <mutex>
#include <cmath>
#include <atomic>

using namespace std;

template<typename SceneParser>
class Scene {

    /// MAGIC CONSTANTS
    const ldb ANTIALIASING_CONST = 0.2;
    const int ANTIALIASING_POINT_COUNT = 5;
    const int MAX_RAY_TRACING_DEPTH = 10;
    const int LIGHT_RAYS_COUNT = 15; //maybe later
    const ldb BIAS = 1e-7;


    /// GEOMETRY, VIEWPORT, MATERIALS, LIGHTS
    MaterialsFactory materialsFactory;
    vector<IGeometryObject *> geometry;
    vector<Light> lights;
    Viewport viewport;


    /// KD-TREE
    KD_Tree kd_tree;


    /// RENDER's WORKERS VARIABLES
    ThreadPool<void> threadPool;
    vector<size_t> needToCalcPixels;
    mutex workerMutex;

    /// PIXELS VARIABLES
    const size_t workerPixelsCount = 10;
    size_t width, height;
    float *pixels;

    /// TIME VARIABLES
    chrono::steady_clock::time_point begin_render_time_point;
    chrono::steady_clock::time_point end_render_time_point;

    chrono::steady_clock::time_point begin_antialiasing_time_point;
    chrono::steady_clock::time_point end_antialiasing_time_point;


    Intersection castRayKD(const Ray &ray) {
        Ray nray = ray;
        nray.begin += nray.direction * BIAS;
        return kd_tree.castRay(nray);
    }

    ldb getLightIntensity(Point pnt, const IGeometryObject *object) {
        ldb lightIntensity = 0;

        for (const Light &light : lights) {

            Ray newRay = LineFromTwoPoints(pnt, light.getPosition());
            Intersection lightIntersection = castRayKD(newRay);
            ldb lightPositionCoef = newRay.getLineCoef(light.getPosition());

            if (!lightIntersection || Double::greater(lightIntersection.rayIntersectionCoef, lightPositionCoef)) {

                Vector normLightPos = light.getPosition() - pnt;
                ldb sqr_length = normLightPos.sqrLength();
                normLightPos.normalize();

                ldb currentPointLightContribution = (object->getNormal(pnt) * normLightPos);

                ldb distance_for_origin_power = light.getReference().distance / light.getReference().power;
                ldb cur_power = light.getPower() * distance_for_origin_power / sqr_length;

                currentPointLightContribution *= cur_power;

                lightIntensity += max(currentPointLightContribution, (ldb) 0.0);
            }
        }
        return min(lightIntensity + 0.2, 1.0);
    }

    inline ldb clamp(const ldb &low, const ldb &high, const ldb &value) {
        return std::max(low, std::min(high, value));
    }

    Vector refract(const Ray &ray, const Vector &normal, ldb ior) {
        ldb cosi = clamp(-1, 1, normal * ray.direction);
        ldb etai = 1, etat = ior;
        Vector n = normal;
        if (Double::less(cosi, 0)) {
            cosi *= -1;
        } else {
            std::swap(etai, etat);
            n = n * -1;
        }
        ldb eta = etai / etat;
        ldb k = 1 - eta * eta * (1 - cosi * cosi);
        if (Double::less(k, 0)) {
            // total internal reflection
            return 0;
        }
        return ray.direction * eta + n * (eta * cosi - sqrt(k));
    }

    void fresnel(const Vector &I, const Vector &N, const ldb &ior, ldb &Kr) {
        ldb cosi = clamp(-1, 1, N * I);
        ldb etai = 1, etat = ior;
        if (Double::greater(cosi, 0)) {
            swap(etai, etat);
        }
        ldb sint = etai / etat * sqrt(max((ldb) 0, 1 - cosi * cosi));

        if (Double::greaterEqual(sint, 0)) {
            //total inherit reflection
            Kr = 1;
        } else {
            ldb cost = sqrt(max((ldb) 0, 1 - sint * sint));
            cosi = abs(cosi);
            ldb Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
            ldb Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
            Kr = (Rs * Rs + Rp * Rp) / 2;
        }
    }

    Intersection castRay(const Ray &ray, ldb additionalLight = 0, int depth = 0) {
        if (depth > MAX_RAY_TRACING_DEPTH) {
            return Intersection();
        }
        additionalLight = 0;
        Intersection current = castRayKD(ray);

        if (!current) {
            return current;
        }
        const Material *currentMaterial = current.object->getMaterial();
        Vector intersectionPoint = current.intersectionPoint;
        Vector intersectionNormal = current.object->getNormal(intersectionPoint);

        Color materialColor;

        Color refractColor;
        Color reflectColor;
        ldb Kr, Kt; // reflection and refraction mix value

        fresnel(ray.direction, intersectionNormal, currentMaterial->getRefract(), Kr);
        Kt = 1 - Kr;

        ldb lightIntensity = getLightIntensity(current.intersectionPoint, current.object) + additionalLight;

        ldb normalizedLight = min((ldb) 1, lightIntensity);

        Vector texturePoint = current.object->getTexturePoint(intersectionPoint);

        switch (currentMaterial->getType()) {
            case ReflectDiffuse: {
                materialColor = current.object->getMaterial()->getColor(texturePoint) * (1 - currentMaterial->getReflect());
                Ray reflectRay = ray.getReflectRay(intersectionPoint, intersectionNormal);
                Intersection reflectInter = castRay(reflectRay, lightIntensity, depth + 1);
                if (reflectInter) {
                    reflectColor = reflectInter.color * currentMaterial->getReflect();
                }
                current.color = materialColor * normalizedLight + reflectColor;
                break;
            }
            case ReflectRefract: {
                //reflection
                Ray reflectRay = ray.getReflectRay(intersectionPoint, intersectionNormal);
                Intersection reflectInter = castRay(reflectRay, lightIntensity, depth + 1);
                if (reflectInter) {
                    reflectColor = reflectInter.color * Kr;
                }

                //refraction
                Vector refractDirection = refract(ray, intersectionNormal, currentMaterial->getRefract());
                Ray refractRay = LineFromTwoPoints(intersectionPoint, intersectionPoint + refractDirection);
                Intersection refractInter = castRay(refractRay, lightIntensity, depth + 1);
                if (refractInter) {
                    refractColor = refractInter.color * Kt;
                }

                current.color = reflectColor + refractColor;
                break;
            }
            case Diffuse: {
                materialColor = current.object->getMaterial()->getColor(texturePoint) * normalizedLight;
                current.color = materialColor;
                break;
            }
            case Transparent: {
                materialColor = currentMaterial->getColor(texturePoint) * currentMaterial->getAlpha();

                Vector refractDirection = refract(ray, intersectionNormal, currentMaterial->getRefract());

                Ray refractRay = LineFromTwoPoints(intersectionPoint, intersectionPoint + refractDirection);
                Intersection refractInter = castRay(refractRay, lightIntensity, depth + 1);
                if (refractInter) {
                    refractColor = refractInter.color * (1 - currentMaterial->getAlpha());
                }

                current.color = materialColor * normalizedLight + refractColor;
                break;
            }
        }

        current.color = current.color.normalize();
        return current;
    }

    Color traceRay(const Ray &ray) {
        Color color(0, 0, 0);

        Intersection intersection = castRay(ray);
        if (!intersection) {
            return Color(0.2, 0.2, 0.2);
        }
        return intersection.color.normalize();
    }

    void getSomePixelsToRender(vector<size_t> &v) {
        unique_lock<mutex> lock(workerMutex);

        for (size_t i = 0; i < min(workerPixelsCount, needToCalcPixels.size()); i++) {
            v.push_back(needToCalcPixels.back());
            needToCalcPixels.pop_back();
        }
    }

    void renderWorker(bool antialiasing = false) {
        size_t current_w, current_h;

        Vector base_w = viewport.getWidthBase() / width, base_h = viewport.getHeightBase() / height;
        Vector origin = viewport.getOrigin(), defaultOffset = base_w / 2 + base_h / 2;
        vector<size_t> pixelToRender;

        getSomePixelsToRender(pixelToRender);

        while (pixelToRender.size() > 0) {
            size_t pixel = pixelToRender.back();
            pixelToRender.pop_back();

            current_h = pixel / width;
            current_w = pixel % width;

            Point basePoint = viewport.getTopLeft() + base_w * current_w + base_h * current_h;

            Color color;

            if (!antialiasing) {

                Point screenPoint = basePoint + defaultOffset;
                Ray newRay = LineFromTwoPoints(origin, screenPoint);
                color = traceRay(newRay);

            } else {

                ldb r = 0, g = 0, b = 0;

                Point base_w_anti_al = base_w / ANTIALIASING_POINT_COUNT;
                Point base_h_anti_al = base_h / ANTIALIASING_POINT_COUNT;

                for (int i = 0; i < ANTIALIASING_POINT_COUNT; i++) {
                    for (int j = 0; j < ANTIALIASING_POINT_COUNT; j++) {

                        Point newPoint = basePoint + base_w_anti_al * j + base_h_anti_al * i;
                        Ray newRay = LineFromTwoPoints(origin, newPoint);
                        Color clr = traceRay(newRay);
                        r += clr.r;
                        g += clr.g;
                        b += clr.b;
                    }
                }

                r /= ANTIALIASING_POINT_COUNT * ANTIALIASING_POINT_COUNT;
                g /= ANTIALIASING_POINT_COUNT * ANTIALIASING_POINT_COUNT;
                b /= ANTIALIASING_POINT_COUNT * ANTIALIASING_POINT_COUNT;

                color = Color(r, g, b).normalize();
            }

            pixels[pixel * 3] = (float) color.r;
            pixels[pixel * 3 + 1] = (float) color.g;
            pixels[pixel * 3 + 2] = (float) color.b;
        }
        if (needToCalcPixels.size() > 0) {
            threadPool.submit(bind(render_lambda, antialiasing));
        }
        if (antialiasing) {
            end_antialiasing_time_point = chrono::steady_clock::now();
        } else {
            end_render_time_point = chrono::steady_clock::now();
        }
    }

    ISceneParser *createSceneParser() {
        return new SceneParser(materialsFactory, viewport, lights, geometry);
    }

    size_t getPixelPos(size_t i, size_t j) const {
        return i * width + j;
    }

public:

    Scene() {
        width = height = 0;
    }

    void openScene(const string &filename, const string &directory,
                   size_t width, size_t height,
                   size_t numberOfThreads = 8) {

        clear();
        ISceneParser *parser = createSceneParser();
        parser->openScene(filename, directory);
        delete parser;

        this->width = width;
        this->height = height;
        pixels = new float[width * height * 3];
        kd_tree.buildTree(geometry, numberOfThreads);
    }

    void render(size_t numberOfThreads = 8) {

        begin_render_time_point = chrono::steady_clock::now();

        threadPool.setWorkersNumber(numberOfThreads);

        for (size_t i = 0; i < width * height; i++) {
            needToCalcPixels.push_back(i);
        }

        for (size_t i = 0; i < numberOfThreads; i++) {
            threadPool.submit(bind(render_lambda, false));
        }
    }

    bool isBusy() const {
        return !needToCalcPixels.empty();
    }

    bool antialiasing(size_t numberOfThreads = 8) {

        if (isBusy()) {
            return false;
        }

        begin_antialiasing_time_point = chrono::steady_clock::now();

        int offsets[3] = {-1, 0, 1};

        size_t nx, ny;

        for (size_t i = 0; i < height; i++) {
            for (size_t j = 0; j < width; j++) {
                ldb resultDifference = 0;
                Color currentColor(pixels + getPixelPos(i, j) * 3);

                for (int x_p = 0; x_p < 3; x_p++) {
                    for (int y_p = 0; y_p < 3; y_p++) {
                        nx = i + offsets[x_p];
                        ny = j + offsets[y_p];
                        if (nx < height && ny < width) {
                            resultDifference += Color(pixels + getPixelPos(nx, ny) * 3).L1Norm(currentColor);
                        }
                    }
                }
                if (Double::less(ANTIALIASING_CONST, resultDifference)) {
                    needToCalcPixels.push_back(getPixelPos(i, j));
                }
            }
        }

        for (auto pixel : needToCalcPixels) {
            pixels[pixel * 3] = 1;
            pixels[pixel * 3 + 1] = 1;
            pixels[pixel * 3 + 2] = 1;
        }

        threadPool.setWorkersNumber(numberOfThreads);

        for (size_t i = 0; i < numberOfThreads; i++) {
            threadPool.submit(bind(render_lambda, true));
        }

        return true;
    }

    const float *getPixels() const {
        return pixels;
    }

    void clear() {
        for (IGeometryObject *object : geometry) {
            delete object;
        }
        lights.clear();
    }

    auto getRenderDuration() const {
        return end_render_time_point - begin_render_time_point;
    }

    auto getAntialiasingDuration() const {
        return end_antialiasing_time_point - begin_antialiasing_time_point;
    }

    auto getTotalRaysIntersections() const {
        return kd_tree.total_rays_intersections.load();
    }

    auto getTotalKDTreeBuildTime() const {
        return kd_tree.total_build_time;
    }

    ~Scene() {
        threadPool.shutdown();
    }

private:
    const function<void(bool)> render_lambda = [this](bool need_antialiasing) {
        renderWorker(need_antialiasing);
    };
};

