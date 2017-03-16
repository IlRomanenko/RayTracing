cmake_minimum_required(VERSION 3.6)
project(Ray_Tracing)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-g -O0")

set(SOURCE_FILES
        main.cpp
        base_headers.h
        KD_tree_test.h
        scene/Material.cpp
        scene/Material.h
        scene/Viewport.h
        scene/Light.h
        scene/Scene.h
        geometry/Primitives.h
        geometry/Triangle.h
        geometry/IGeometryObject.h
        geometry/Polygon.h
        geometry/Sphere.h
        geometry/Quadrangle.h
        geometry/BoundingBox.h
        geometry/Ray.h
        parsers/RT_file.h
        parsers/tinyobjloader/tiny_obj_loader.h
        parsers/tinyobjloader/tiny_obj_loader.cc
        parsers/ObjLoader.h
        parsers/ISceneParser.h
        parsers/MyParser.h
        parsers/Scanners.h
        utilities/ThreadPool.h
        ray-casting/KD_Tree.h
        )


add_executable(Ray_Tracing ${SOURCE_FILES})
target_link_libraries(Ray_Tracing glut GL GLU X11 pthread)