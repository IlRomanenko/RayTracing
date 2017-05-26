//
// Created by Ilya on 03.03.17.
//
#pragma once

#include "../base_headers.h"
#include "ISceneParser.h"
#include "Scanners.h"

#include "../scene/Material.h"
#include "../scene/Viewport.h"
#include "../scene/Light.h"
#include "../geometry/IGeometryObject.h"
#include "../geometry/Sphere.h"
#include "../geometry/Triangle.h"
#include "../geometry/Quadrangle.h"

using namespace geometry;

class RT_file : public ISceneParser {

    string currentDirectory;

protected:
    void checkEofScanner(FileScanner &scanner, const string &group) {
        if (scanner.eof()) {
            fprintf(stderr, "Unsupported file format.\nBug with unexpected end of file in group %s", group.c_str());
            throw exception();
        }
    }

    virtual void parseFile(const string &filename) {
        FileScanner scanner(currentDirectory + filename);

        string line;
        while (!scanner.eof()) {

            line = scanner.nextLine();

            if (line == "viewport") {
                viewportSection(scanner);
            } else if (line == "materials") {
                materialsSection(scanner);
            } else if (line == "lights") {
                lightsSection(scanner);
            } else if (line == "geometry") {
                geometrySection(scanner);
            } else if (line != "") {
                fprintf(stderr, "Unsupported file format.\nBug with line %s", line.c_str());
                throw exception();
            }
        }

        scanner.close();
    }

    virtual void viewportSection(FileScanner &scanner) {
        StringScanner stringScanner;
        string optionName;

        Vector origin, topLeft, bottomLeft, topRight;
        string line;

        while ((line = scanner.nextLine()) != "endviewport") {

            checkEofScanner(scanner, "viewport");

            stringScanner.setBuffer(line);
            optionName = stringScanner.nextString();

            if (optionName == "origin") {
                origin = stringScanner.nextVector();
            } else if (optionName == "topleft") {
                topLeft = stringScanner.nextVector();
            } else if (optionName == "bottomleft") {
                bottomLeft = stringScanner.nextVector();
            } else if (optionName == "topright") {
                topRight = stringScanner.nextVector();
            } else {
                fprintf(stderr, "Unsupported file format.\nBug with optionName %s", optionName.c_str());
                throw exception();
            }
        }
        viewport = Viewport(origin, topLeft, topRight, bottomLeft);
    }

    virtual void materialsSection(FileScanner &scanner) {
        string line;

        StringScanner stringScanner;
        while ((line = scanner.nextLine()) != "endmaterials") {

            checkEofScanner(scanner, "materials");

            if (line != "entry") {
                fprintf(stderr, "Unsupported file format.\nBug with entry %s", line.c_str());
                throw exception();
            }
            readMaterialEntry(scanner);
        }
    }

    virtual void readMaterialEntry(FileScanner &scanner) {
        string line;

        string material_name;
        Color color;
        ldb alpha = 0, reflect = 0, refract = 0;

        StringScanner stringScanner;
        string optionName;

        string imageName;
        bool isTexture = false;

        while ((line = scanner.nextLine()) != "endentry") {

            checkEofScanner(scanner, "materials -> entry");

            stringScanner.setBuffer(line);
            optionName = stringScanner.nextString();

            if (optionName == "name") {
                material_name = stringScanner.nextString();
            } else if (optionName == "alpha") {
                alpha = stringScanner.nextDouble();
            } else if (optionName == "reflect") {
                reflect = stringScanner.nextDouble();
            } else if (optionName == "refract") {
                refract = stringScanner.nextDouble();
            } else if (optionName == "color") {
                int r, g, b;
                r = stringScanner.nextInt();
                g = stringScanner.nextInt();
                b = stringScanner.nextInt();
                color = Color(r, g, b);
            } else if (optionName == "image") {
                imageName = stringScanner.nextString();
                isTexture = true;
            } else {
                fprintf(stderr, "Unsupported file format.\nBug with materials -> entry -> optionName %s",
                        optionName.c_str());
                throw exception();
            }
        }
        if (isTexture) {
            materialsFactory.constructTextureMaterial(material_name, currentDirectory + imageName, alpha, reflect, refract);
        } else {
            materialsFactory.constructMaterial(material_name, color, alpha, reflect, refract);
        }
    }

    void lightsSection(FileScanner &scanner) {
        string line;

        StringScanner stringScanner;
        while ((line = scanner.nextLine()) != "endlights") {
            if (scanner.eof()) {
                fprintf(stderr, "Unsupported file format.\nBug with endlights %s", line.c_str());
                throw exception();
            }

            if (line != "reference") {
                fprintf(stderr, "Unsupported file format.\nBug with entry %s", line.c_str());
                throw exception();
            }

            readReferenceAndLightEntry(scanner);

        }
    }

    void readReferenceAndLightEntry(FileScanner &scanner) {
        string line, optionName;

        StringScanner stringScanner;

        ldb power = 0, distance = 0;

        while ((line = scanner.nextLine()) != "endreference") {


            checkEofScanner(scanner, "lights -> reference");

            stringScanner.setBuffer(line);

            optionName = stringScanner.nextString();

            if (optionName == "power") {
                power = stringScanner.nextDouble();
            } else if (optionName == "distance") {
                distance = stringScanner.nextDouble();
            } else {
                fprintf(stderr, "Unsupported file format.\nBug with lights -> entry -> optionName %s",
                        optionName.c_str());
                throw exception();
            }
        }

        Reference ref(power, distance);


        Vector position;
        power = 0;

        line = scanner.nextLine();
        if (line != "point") {
            fprintf(stderr, "Unsupported file format.\nBug with lights -> entry -> point %s", optionName.c_str());
            throw exception();
        }
        while ((line = scanner.nextLine()) != "endpoint") {

            checkEofScanner(scanner, "lights -> point");

            stringScanner.setBuffer(line);

            optionName = stringScanner.nextString();

            if (optionName == "coords") {
                position = stringScanner.nextVector();
            } else if (optionName == "power") {
                power = stringScanner.nextDouble();
            } else {
                fprintf(stderr, "Unsupported file format.\nBug with lights -> entry -> optionName %s",
                        optionName.c_str());
                throw exception();
            }
        }
        Light light(ref, power, position);

        lights.push_back(light);
    }

    void geometrySection(FileScanner &scanner) {
        string line;
        StringScanner stringScanner;
        string optionName;

        while ((line = scanner.nextLine()) != "endgeometry") {

            checkEofScanner(scanner, "geometry");

            stringScanner.setBuffer(line);

            optionName = stringScanner.nextString();

            if (optionName == "sphere") {
                readSphere(scanner);
            } else if (optionName == "triangle") {
                readTriangle(scanner);
            } else if (optionName == "quadrangle") {
                readQuadrangle(scanner);
            } else {
                fprintf(stderr, "Unsupported file format.\nBug with entry %s", line.c_str());
                throw exception();
            }
        }
    }

    void readSphere(FileScanner &scanner) {
        string line;

        string material_name;
        Point position;
        ldb radius = 0;

        StringScanner stringScanner;
        string optionName;

        while ((line = scanner.nextLine()) != "endsphere") {

            checkEofScanner(scanner, "geometry -> sphere");

            stringScanner.setBuffer(line);
            optionName = stringScanner.nextString();

            if (optionName == "material") {
                material_name = stringScanner.nextString();
            } else if (optionName == "radius") {
                radius = stringScanner.nextDouble();
            } else if (optionName == "coords") {
                position = stringScanner.nextVector();
            } else {
                fprintf(stderr, "Unsupported file format.\nBug with materials -> entry -> %s", optionName.c_str());
                throw exception();
            }
        }

        //Todo add sphere
        geometry.push_back(new Sphere(position, radius, materialsFactory.getMaterial(material_name)));
    }

    void readTriangle(FileScanner &scanner) {
        string line;

        string material_name;

        StringScanner stringScanner;
        string optionName;

        vector<Point> vertexes;
        vector<Point> textureCoords;

        while ((line = scanner.nextLine()) != "endtriangle") {

            checkEofScanner(scanner, "geometry -> triangle");

            stringScanner.setBuffer(line);
            optionName = stringScanner.nextString();

            if (optionName == "vertex") {
                vertexes.push_back(stringScanner.nextVector());
            } else if (optionName == "textureCoord") {
                textureCoords.push_back(stringScanner.nextVector());
            } else if (optionName == "material") {
                material_name = stringScanner.nextString();
            } else {
                fprintf(stderr, "Unsupported file format.\nBug with materials -> entry -> %s", optionName.c_str());
                throw exception();
            }
        }

        if (vertexes.size() != 3) {
            fprintf(stderr, "Unsupported file format.\nBug with sphere vertex count -> %lu", vertexes.size());
            throw exception();
        }

        if (textureCoords.size() != 0) {
            geometry.push_back(new Triangle(vertexes, textureCoords, materialsFactory.getMaterial(material_name)));
        } else {
            geometry.push_back(new Triangle(vertexes, materialsFactory.getMaterial(material_name)));
        }
    }

    void readQuadrangle(FileScanner &scanner) {
        string line;

        string material_name;

        StringScanner stringScanner;
        string optionName;

        vector<Point> vertexes;
        vector<Point> textureCoords;

        while ((line = scanner.nextLine()) != "endquadrangle") {

            checkEofScanner(scanner, "geometry -> quadrangle");

            stringScanner.setBuffer(line);
            optionName = stringScanner.nextString();

            if (optionName == "vertex") {
                vertexes.push_back(stringScanner.nextVector());
            } else if (optionName == "textureCoord") {
                textureCoords.push_back(stringScanner.nextVector());
            } else if (optionName == "material") {
                material_name = stringScanner.nextString();
            } else {
                fprintf(stderr, "Unsupported file format.\nBug with materials -> entry -> %s", optionName.c_str());
                throw exception();
            }
        }

        if (vertexes.size() != 4) {
            fprintf(stderr, "Unsupported file format.\nBug with sphere vertex count -> %lu", vertexes.size());
            throw exception();
        }

        //Todo add quadrangle

        if (textureCoords.size() != 0) {
            geometry.push_back(new Quadrangle(vertexes, textureCoords, materialsFactory.getMaterial(material_name)));
        } else {
            geometry.push_back(new Quadrangle(vertexes, materialsFactory.getMaterial(material_name)));
        }
    }


public:
    RT_file() = delete;

    RT_file(MaterialsFactory &factory,
            Viewport &viewport,
            vector<Light> &lights,
            vector<IGeometryObject *> &geometry)
            : ISceneParser(factory, viewport, lights, geometry) {}

    void openScene(const string &filename, const string &directory) override {
        currentDirectory = directory;
        parseFile(filename);
    }

};