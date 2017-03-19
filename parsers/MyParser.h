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
 * Version
 *      Version [rt|irt]
 * Endversion
 *
 * File
 *      Name <name of file>
 *      Directory <directory of file>
 * EndFile
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

    enum FileType {
        RT, OBJ, UNKNOWN
    };

    string fileName, fileDirectory;

    FileType geometryFileType = FileType::UNKNOWN;;

protected:

    void fileSection(FileScanner &scanner) {
        string line;

        StringScanner stringScanner;
        string optionName;

        while ((line = scanner.nextLine()) != "EndFile") {

            checkEofScanner(scanner, "File");

            stringScanner.setBuffer(line);
            optionName = stringScanner.nextString();

            if (optionName == "Name") {
                fileName = stringScanner.nextString();
            } else if (optionName == "Directory") {
                fileDirectory = stringScanner.nextString();
            } else {
                fprintf(stderr, "Unsupported file format.\nBug with File -> optionName %s", optionName);
                throw exception();
            }
        }
    }

    void versionSection(FileScanner &scanner) {
        string line;

        StringScanner stringScanner;
        string optionName, type;

        while ((line = scanner.nextLine()) != "EndVersion") {

            checkEofScanner(scanner, "Version");

            stringScanner.setBuffer(line);
            optionName = stringScanner.nextString();

            if (optionName == "Version") {
                type = stringScanner.nextString();
            } else {
                fprintf(stderr, "Unsupported file format.\nBug with Version -> optionName %s", optionName);
                throw exception();
            }
        }
        if (type == "rt") {
            geometryFileType = FileType::RT;
        } else if (type == "irt") {
            geometryFileType = FileType::OBJ;
        }
    }

    void parseFile(const string &filename) override {
        FileScanner scanner(filename);

        string line;
        while (!scanner.eof()) {
            line = scanner.nextLine();
            if (line == "Version") {
                versionSection(scanner);
            } else if (line == "File") {
                fileSection(scanner);
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

        Vector base_height = upNormal * (-1), base_width = direction ^upNormal;
        Point center = origin + direction * distance;

        viewport.setTopLeft(center - base_height * (height / 2) - base_width * (width / 2));
        viewport.setTopRight(center - base_height * (height / 2) + base_width * (width / 2));
        viewport.setBottomLeft(center + base_height * (height / 2) - base_width * (width / 2));
        viewport.setOrigin(origin);
    }

    ISceneParser *createParser() const {
        ISceneParser *result;
        Viewport tempViewport;
        switch (geometryFileType) {
            case RT:
                result = new RT_file(materialsFactory, tempViewport, lights, geometry);
                break;
            case OBJ:
                result = new ObjLoader(materialsFactory, tempViewport, lights, geometry);
                break;
            default:
                result = nullptr;
        }
        return result;
    }

public:
    MyParser(MaterialsFactory &factory,
             Viewport &viewport,
             vector<Light> &lights,
             vector<IGeometryObject *> &geometry)
            : RT_file(factory, viewport, lights, geometry) {}

    void openScene(const string &filename, const string &directory) override {
        parseFile(directory + filename);
        ISceneParser *parser = createParser();
        parser->openScene(fileName, directory + fileDirectory);
        delete parser;
    }
};

