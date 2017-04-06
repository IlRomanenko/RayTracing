#include <iostream>
#include <GL/freeglut.h>
#include "base_headers.h"
#include "scene/Scene.h"


#include "KD_tree_test.h"
#include "parsers/MyParser.h"
#include <png++/png.hpp>

Scene<RT_file> scene;

const string SCENENAME = "transparent";
const string SCENEFORMAT = ".rt";
const string DIRECTORY = "examples/rt_examples/";


const size_t width = 600, height = 600;
const float *pixels;
int currentWindow;
GLuint textureID;

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, pixels);

    glBegin(GL_POLYGON);
    {
        glTexCoord2d(0, 0);
        glVertex2d(-1, 1);

        glTexCoord2d(1, 0);
        glVertex2d(1, 1);

        glTexCoord2d(1, 1);
        glVertex2d(1, -1);

        glTexCoord2d(0, 1);
        glVertex2d(-1, -1);
    }
    glEnd();

    glRasterPos2d(0, 0);

    glutSwapBuffers();
}

void reshapeEvent(int x_size, int y_size) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glViewport(0, 0, x_size, y_size);
    glOrtho(-1, 1, -1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void glInit() {
    glClearColor(1.0, 1.0, 1.0, 1.0);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

void initRayCasting() {

    //scene.openScene("examples/obj_examples/buggy.obj", "examples/obj_examples/", width, height);
    //scene.openScene("examples/scene.rt", "examples/obj_examples/", width, height);
    scene.openScene(SCENENAME + SCENEFORMAT, DIRECTORY, width, height);
    //scene.openScene("refract.rt", "examples/rt_examples/", width, height);
    //transparent
    scene.render();

    pixels = scene.getPixels();


    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, pixels);


}

void timer_redisplay(int) {
    glutPostWindowRedisplay(currentWindow);
    glutTimerFunc(16, timer_redisplay, 0);
}

void tempTest() {

    vector<Vector> points;
    for (int i = 0; i < 3; i++) {
        points.push_back(Vector(i, i, i));
    }
    BoundingBox box(points.begin(), points.end());

}

void keyboard(unsigned char chr, int x, int y) {
    if (chr == ' ') {
        scene.antialiasing();
        glutTimerFunc(16, timer_redisplay, 0);
    } else if (chr == '\r') {
        scene.openScene("input.rt", "examples/", width, height);
        scene.render();
        glutTimerFunc(16, timer_redisplay, 0);
    }
}

void writeImage(const string &s) {

    png::image<png::rgb_pixel> image(width, height);
    const float *pixels = scene.getPixels();

    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            image[i][j] = png::rgb_pixel(
                    (png_byte) (pixels[(i * width + j) * 3] * 255),
                    (png_byte) (pixels[(i * width + j) * 3 + 1] * 255),
                    (png_byte) (pixels[(i * width + j) * 3 + 2] * 255)
            );
        }
    }
    image.write(s);
}

void closeEvent() {
    writeImage("results/" + SCENENAME + ".png");
}


int main(int argc, char **argv) {

    //kd_tree_test::MainTest(argc, argv);
    //thread([&](){kd_tree_test::MainTest(argc, argv);}).join();


    glutInit(&argc, argv);
    glutInitWindowSize(1000, 1000);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    currentWindow = glutCreateWindow("Ray-tracing by Ilya Romanenko");


    glInit();

    tempTest();

    initRayCasting();

    glutReshapeFunc(reshapeEvent);
    glutTimerFunc(100, timer_redisplay, 0);
    glutDisplayFunc(render);
    glutKeyboardFunc(keyboard);
    glutCloseFunc(closeEvent);
    glutMainLoop();

    return 0;
}