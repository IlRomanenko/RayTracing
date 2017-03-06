//
// Created by ilya on 03.03.17.
//
#pragma once

#include "../base_headers.h"

using geometry::ldb;
using geometry::Color;

class MaterialsFactory;

class Material {

    MaterialsFactory* factory;

    size_t materialId;

    ldb reflect, refract, alpha;
    Color color;

protected:

    Material(size_t materialId, Color color, ldb alpha, ldb reflect, ldb refract) {
        this->materialId = materialId;
        this->color = color;
        this->alpha = alpha;
        this->reflect = reflect;
        this->refract = refract;
    }

    ~Material() {}
public:
    Material() = delete;

    ldb getRefract() const {
        return refract;
    }

    ldb getReflect() const {
        return reflect;
    }

    ldb getAlpha() const {
        return alpha;
    }

    Color getColor() const {
        return color;
    }

    void dispose();

    friend class MaterialsFactory;
};


class MaterialsFactory {


    map<size_t, Material*> materials;
    map<size_t, int> links; //yes, hand made links counter

    size_t getHashCode(const string& s) {
        const size_t base = 259;

        size_t hsh = 0;
        for (char chr : s) {
            hsh += hsh * base + chr;
        }
        return hsh;
    }

public:

    Material* constructMaterial(const string &materialName, const Color &color, ldb alpha, ldb reflect, ldb refract) {
        size_t hash = getHashCode(materialName);

        Material* currentMaterial = new Material(hash, color, alpha, reflect, refract);

        materials[hash] = currentMaterial;
        links[hash]++;

        return currentMaterial;
    }

    Material* getMaterial(const string& materialName) {
        size_t hash = getHashCode(materialName);
        Material* currentMaterial = materials[hash];
        links[hash]++;
        return currentMaterial;
    }


    void dispose(size_t materialId) {
        links[materialId]--;
        if (links[materialId] <= 0) {
            delete materials[materialId];
            materials.erase(materialId);
            links.erase(materialId);
        }
    }

    ~MaterialsFactory() {
        for (pair<size_t, Material*> k_v_pair : materials) {
            delete k_v_pair.second;
        }
    }
};