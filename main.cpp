#include <iostream>
#include <GL/freeglut.h>
#include "objects/Scene.h"
#include "geometry/BoundingBox.h"

Scene scene;
const size_t width = 600, height = 600;
const float* pixels;
GLuint textureID;

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, pixels);

    glBegin(GL_POLYGON);
    {
        glTexCoord2d(0, 0);
        glVertex2d(-1, -1);

        glTexCoord2d(1, 0);
        glVertex2d(1, -1);

        glTexCoord2d(1, 1);
        glVertex2d(1, 1);

        glTexCoord2d(0, 1);
        glVertex2d(-1, 1);
    }
    glEnd();

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

    scene.openScene("examples/scene.rt", width, height);
    scene.render();

    pixels = scene.getPixels();


    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);


    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, pixels);


}

void timer_redisplay(int) {
    glutPostRedisplay();
    if (scene.isBusy()) {
        glutTimerFunc(16, timer_redisplay, 0);
    }
}

void tempTest() {

    vector<Vector> points;
    for (int i = 0; i < 3; i++) {
        points.push_back(Vector(i, i, i));
    }
    BoundingBox box(Triangle(points, nullptr));

}

int main(int argc, char** argv) {


    glutInit(&argc, argv);
    glutInitWindowSize(1000, 1000);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow("Ray-tracing by Ilya Romanenko");


    glInit();

    tempTest();

    initRayCasting();

    glutReshapeFunc(reshapeEvent);
    glutTimerFunc(100, timer_redisplay, 0);
    glutDisplayFunc(render);
    glutMainLoop();

    return 0;
}