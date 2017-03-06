//
// Created by ilya on 06.03.17.
//
#pragma once

#include "../base_headers.h"
#include <exception>

#include "tinyobjloader/tiny_obj_loader.h"

#include "../objects/Material.h"
#include "../objects/Viewport.h"
#include "../objects/Light.h"
#include "../geometry/IGeometryObject.h"
#include "../geometry/Sphere.h"
#include "../geometry/Triangle.h"
#include "../geometry/Quadrangle.h"

using namespace geometry;

class ObjLoader {

    MaterialsFactory &materialsFactory;
    Viewport &viewport;
    vector<Light> &lights;
    vector<IGeometryObject *> &geometry;


    void parseFile(const string &filename, const string &directory) {
        tinyobj::attrib_t attrib;
        vector<tinyobj::shape_t> shapes;
        vector<tinyobj::material_t> materials;
        string error;
        bool exitCode = tinyobj::LoadObj(&attrib, &shapes, &materials, &error, filename.c_str(), directory.c_str());

        if (!exitCode) {
            fprintf(stderr, "Error with obj file %s", error);
            throw exception();
        }

        vector<Material*> currentMaterials;
        for (size_t i = 0; i < materials.size(); i++) {
            Color clr(materials[i].diffuse);
            currentMaterials.push_back(materialsFactory.constructMaterial(
                    materials[i].name,
                    clr,
                    materials[i].dissolve,
                    materials[i].ambient[0],
                    0));
        }

        for (size_t s = 0; s < shapes.size(); s++) {
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                int fv = shapes[s].mesh.num_face_vertices[f];

                vector<Point> points;

                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    points.emplace_back(attrib.vertices[3 * idx.vertex_index + 0],
                                        attrib.vertices[3 * idx.vertex_index + 1],
                                        attrib.vertices[3 * idx.vertex_index + 2]);
                }

                geometry.push_back(new Triangle(points, currentMaterials[shapes[s].mesh.material_ids[f]]));

                index_offset += fv;

                // per-face material
                shapes[s].mesh.material_ids[f];
            }
        }
    }

public:
    ObjLoader() = delete;

    ObjLoader(const string &filename,
              const string &directory,
              MaterialsFactory &factory,
              Viewport &viewport,
              vector<Light> &lights,
              vector<IGeometryObject *> &geometry)
            : materialsFactory(factory),
              viewport(viewport),
              lights(lights),
              geometry(geometry) {
        parseFile(filename, directory);
        lights.push_back(Light(Reference(1, 1), 1000, Point(7, 0, 7)));
        viewport = Viewport(Vector(1000, 0, 0), Vector(10, -5, 5), Vector(10, 5, 5), Vector(10, -5, -5));
    }
};