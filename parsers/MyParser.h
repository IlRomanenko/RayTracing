//
// Created by ilya on 13.03.17.
//
#pragma once

#include "ObjLoader.h"
#include "RT_file.h"

/***
 *
 * .irt     type format:
 *
 * ObjFile
 *      Name <name of ObjFile>
 *      Directory <directory of ObjFile>
 * EndObjFile
 *
 * Viewport
 *      Origin      <vector(position)>
 *      UpNormal    <vector>
 *      LookAt      <vector(position)>
 *      Width       <double>
 *      Height      <double>
 *      Distance    <double>    # distance from origin to viewport
 * EndViewport
 *
 * lights
 *	    # Описание нормировочных коэффициентов
 *	    reference
 *		    # Мощность эталонного источника
 *		    power       <double>
 *		    # Расстояние от него до эталонной плоскости
 *		    distance    <double>
 *	    endreference
 *	    # Описание одного источника света
 *	    point
 *		    # Координаты
 *		    coords      <vector>
 *		    # Мощность
 *		    power       <double>
 *	    endpoint
 *
 *	    <...>
 *
 *  endlights
 */

class MyParser : public RT_file {

    string objFileName, objFileDirectory;

protected:
    void objFileSection(FileScanner &scanner) {
        string line;

        StringScanner stringScanner;
        string optionName;

        while ((line = scanner.nextLine()) != "EndObjFile") {

            checkEofScanner(scanner, "ObjFile");

            stringScanner.setBuffer(line);
            optionName = stringScanner.nextString();

            if (optionName == "Name") {
                objFileName = stringScanner.nextString();
            } else if (optionName == "Directory") {
                objFileDirectory = stringScanner.nextString();
            } else {
                fprintf(stderr, "Unsupported file format.\nBug with ObjFile -> optionName %s", optionName);
                throw exception();
            }
        }
    }

    void parseFile(const string &filename) override {
        FileScanner scanner(filename);

        string line;
        while (!scanner.eof()) {

            line = scanner.nextLine();
            if (line == "ObjFile") {
                objFileSection(scanner);
            } else if (line == "Viewport") {
                viewportSection(scanner);
            } else if (line == "lights") {
                RT_file::lightsSection(scanner);
            } else if (line != "") {
                fprintf(stderr, "Unsupported file format.\nBug with line %s", line);
                throw exception();
            }
        }

        scanner.close();
    }

    void viewportSection(FileScanner &scanner) override {
        string line;

        StringScanner stringScanner;
        string optionName;

        Vector origin, upNormal, lookAt;
        ldb width = 0, height = 0, distance = 0;

        while ((line = scanner.nextLine()) != "EndViewport") {

            checkEofScanner(scanner, "materials -> entry");

            stringScanner.setBuffer(line);
            optionName = stringScanner.nextString();

            if (optionName == "Origin") {
                origin = stringScanner.nextVector();
            } else if (optionName == "UpNormal") {
                upNormal = stringScanner.nextVector();
            } else if (optionName == "LookAt") {
                lookAt = stringScanner.nextVector();
            } else if (optionName == "Height") {
                height = stringScanner.nextDouble();
            } else if (optionName == "Width") {
                width = stringScanner.nextDouble();
            } else if (optionName == "Distance") {
                distance = stringScanner.nextDouble();
            } else {
                fprintf(stderr, "Unsupported file format.\nBug with viewport -> optionName %s", optionName);
                throw exception();
            }
        }
        Vector direction = lookAt - origin;
        upNormal.normalize();
        direction.normalize();

        Vector base_height = upNormal * (-1), base_width = direction ^ upNormal;
        Point center = origin + direction * distance;

        viewport.setTopLeft(center - base_height * (height / 2) - base_width * (width / 2));
        viewport.setTopRight(center - base_height * (height / 2) + base_width * (width / 2));
        viewport.setBottomLeft(center + base_height * (height / 2) - base_width * (width / 2));
        viewport.setOrigin(origin);
    }

public:
    MyParser(MaterialsFactory &factory,
             Viewport &viewport,
             vector<Light> &lights,
             vector<IGeometryObject *> &geometry)
            : RT_file(factory, viewport, lights, geometry) { }

    void openScene(const string &filename, const string &directory) override {
        parseFile(directory + filename);
        ObjLoader loader(materialsFactory, viewport, lights, geometry);
        loader.openScene(objFileName, directory + objFileDirectory);
    }
};

