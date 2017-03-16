//
// Created by ilya on 04.03.17.
//


#include "Material.h"

void Material::dispose() {
    if (factory != nullptr) {
        factory->dispose(materialId);
    }
}
