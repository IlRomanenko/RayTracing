//
// Created by ilya on 10.03.17.
//
#pragma once

#include "../geometry/IGeometryObject.h"
#include "../scene/Light.h"
#include "../scene/Viewport.h"
#include "../scene/MaterialFactory.h"

class ISceneParser {

protected:

    MaterialsFactory &materialsFactory;
    Viewport &viewport;
    vector<Light> &lights;
    vector<IGeometryObject *> &geometry;

public:

    ISceneParser() = delete;

    ISceneParser(MaterialsFactory &factory,
                 Viewport &viewport,
                 vector<Light> &lights,
                 vector<IGeometryObject *> &geometry) :
            materialsFactory(factory), viewport(viewport), lights(lights), geometry(geometry) {}

    virtual void openScene(const string &filename,
                           const string &directory) = 0;

    virtual ~ISceneParser() { }
};