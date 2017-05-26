//
// Created by ilya on 25.05.17.
//
#pragma once
#include "Material.h"
#include <png++/png.hpp>

class Material;


class TextureMaterial : public Material {

    png::image<png::rgb_pixel> texture;

    size_t width, height;

public:
    TextureMaterial(size_t materialId, const string &imageName, ldb alpha, ldb reflect, ldb refract, const string &materialName)
            : Material(materialId, Color(0, 0, 0), alpha, reflect, refract, materialName) {
        texture = png::image<png::rgb_pixel>(imageName);
        width = texture.get_width();
        height = texture.get_height();
    }

    Color getColor(const Vector &pos) const override {
        int tx = max(0, (int)(pos.x * width));
        int ty = max(0, (int)(pos.y * height));
        if (ty == height) {
            ty--;
        }
        if (tx == width) {
            tx--;
        }
        const auto& pixel = texture[ty][tx];
        return {pixel.red, pixel.green, pixel.blue};
    }

};