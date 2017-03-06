cmake_minimum_required(VERSION 3.6)
project(Ray_Tracing)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-lGL -lglut -pthread")

set(SOURCE_FILES
        main.cpp
        base_headers.h
        objects/Material.cpp
        objects/Material.h
        objects/Viewport.h
        objects/Light.h
        objects/Scene.h
        geometry/Primitives.h
        geometry/Triangle.h
        geometry/Plane.h
        geometry/IGeometryObject.h
        geometry/Intersection.h
        geometry/Polygon.h
        geometry/Sphere.h
        geometry/Quadrangle.h
        geometry/BoundingBox.h
        parsers/RT_file.h
        parsers/tinyobjloader/tiny_obj_loader.h
        parsers/tinyobjloader/tiny_obj_loader.cc
        parsers/ObjLoader.h
        ray-casting/RayCaster.h
        ray-casting/KD_Tree.cpp
        ray-casting/KD_Tree.h)

add_executable(Ray_Tracing ${SOURCE_FILES})