//
// Created by ilya on 07.04.17.
//

#include "scene/Material.h"
#include "scene/Light.h"
#include "geometry/IGeometryObject.h"
#include "scene/Viewport.h"
#include "parsers/MyParser.h"
#include <chrono>
#include <ratio>

using namespace chrono;


int main(int argc, char **argv) {

    if (argc != 4) {
        cout << "Usage <filename without extension> <directory> <result directory>\nOnly for irt files!" << endl;
        return 0;
    }

    string filename(argv[1]), directory(argv[2]), result_directory(argv[3]);

    MaterialsFactory factory;
    Viewport viewport;
    vector<Light> lights;
    vector<IGeometryObject*> geometry;

    MyParser parser(factory, viewport, lights, geometry);

    auto begin_time = steady_clock::now();

    parser.openScene(filename + ".irt", directory);
    parser.saveAsRTFile(filename + ".rt", result_directory);

    auto end_time = steady_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(end_time - begin_time);

    cout << endl;
    cout << "Completed with time " << time_span.count() << " seconds";
    cout << endl;

    return 0;
}