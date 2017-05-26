//
// Created by ilya on 25.05.17.
//
#pragma once
#include "../base_headers.h"
#include "Material.h"
#include "TextureMaterial.h"
using namespace geometry;
using namespace std;

class MaterialsFactory {

    map<size_t, Material *> materials;

    size_t getHashCode(const string &s) {
        const size_t base = 259;

        size_t hsh = 0;
        for (char chr : s) {
            hsh += hsh * base + chr;
        }
        return hsh;
    }

public:

    Material *constructMaterial(const string &materialName, const Color &color, ldb alpha, ldb reflect,
                                                  ldb refract) {
        size_t hash = getHashCode(materialName);

        Material *currentMaterial = new Material(hash, color, alpha, reflect, refract, materialName);

        materials[hash] = currentMaterial;

        return currentMaterial;
    }

    Material *constructTextureMaterial(const string &materialName, const string &imageName, ldb alpha, ldb reflect, ldb refract) {
        size_t hash = getHashCode(materialName);

        Material *currentMaterial = new TextureMaterial(hash, imageName, alpha, reflect, refract, materialName);

        materials[hash] = currentMaterial;

        return currentMaterial;
    }



    Material *getMaterial(const string &materialName) {
        size_t hash = getHashCode(materialName);
        Material *currentMaterial = materials[hash];
        return currentMaterial;
    }

    const auto getMaterialsVector() const {
        vector<Material*> mat_vt;
        for (auto mat_pair : materials) {
            mat_vt.push_back(mat_pair.second);
        }
        return mat_vt;
    }

    ~MaterialsFactory() {
        for (pair<size_t, Material *> k_v_pair : materials) {
            delete k_v_pair.second;
        }
    }
};