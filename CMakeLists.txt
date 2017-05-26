cmake_minimum_required(VERSION 3.6)
project(Ray_Tracing)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "-O0 -g -Wall -Wextra -Wunreachable-code")
#-pg


# LibPNG
option(WITH_PNG "PNG file support" TRUE)
if(WITH_PNG)
    # search for pkg-config
    include (FindPkgConfig)
    if (NOT PKG_CONFIG_FOUND)
        message (FATAL_ERROR "pkg-config not found")
    endif ()

    # check for libpng
    pkg_check_modules (LIBPNG libpng16 REQUIRED)
    if (NOT LIBPNG_FOUND)
        message(FATAL_ERROR "You don't seem to have libpng16 development libraries installed")
    else ()
        include_directories (${LIBPNG_INCLUDE_DIRS})
        link_directories (${LIBPNG_LIBRARY_DIRS})
        link_libraries (${LIBPNG_LIBRARIES})
    endif ()
endif(WITH_PNG)


set(COMMON_FILES
        base_headers.h
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
        scene/TextureMaterial.h scene/MaterialFactory.h)

set(SOURCE_FILES
        main.cpp
        ${COMMON_FILES} scene/TextureMaterial.h)
add_executable(Ray_Tracing ${SOURCE_FILES})
target_link_libraries(Ray_Tracing glut GL GLU X11 pthread)



set(RT_PARSER_FILES
        rt_parser.cpp
        ${COMMON_FILES})
add_executable(RT_parser ${RT_PARSER_FILES})

set(KD_TREE_TEST
        KD_tree_test.cpp
        ${COMMON_FILES})
add_executable(KD_tree_test ${KD_TREE_TEST})
target_link_libraries(KD_tree_test glut GL GLU X11 pthread png)