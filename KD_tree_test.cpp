#include <iostream>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#include <GL/glu.h>
#include "base_headers.h"
#include "objects/Scene.h"
#include "ray-casting/KD_Tree.h"

Scene scene;
const size_t width = 600, height = 600;
const float *pixels;
KD_Tree *tree = nullptr;
GLuint textureID;

void drawBbox(BoundingBox box) {

    const auto &coords = box.getMinMaxCoordinates();
    glBegin(GL_LINE_STRIP);
    //1
    glVertex3d(coords[0][0], coords[1][0], coords[2][0]);
    glVertex3d(coords[0][1], coords[1][0], coords[2][0]);
    glVertex3d(coords[0][1], coords[1][1], coords[2][0]);
    glVertex3d(coords[0][0], coords[1][1], coords[2][0]);
    glVertex3d(coords[0][0], coords[1][0], coords[2][0]);
    glEnd();

    glBegin(GL_LINE_STRIP);
    //2
    glVertex3d(coords[0][0], coords[1][0], coords[2][0]);
    glVertex3d(coords[0][1], coords[1][0], coords[2][0]);
    glVertex3d(coords[0][1], coords[1][0], coords[2][1]);
    glVertex3d(coords[0][0], coords[1][0], coords[2][1]);
    glVertex3d(coords[0][0], coords[1][0], coords[2][0]);
    glEnd();

    glBegin(GL_LINE_STRIP);
    //3
    glVertex3d(coords[0][0], coords[1][0], coords[2][0]);
    glVertex3d(coords[0][0], coords[1][1], coords[2][0]);
    glVertex3d(coords[0][0], coords[1][1], coords[2][1]);
    glVertex3d(coords[0][0], coords[1][0], coords[2][1]);
    glVertex3d(coords[0][0], coords[1][0], coords[2][0]);
    glEnd();

    glBegin(GL_LINE_STRIP);
    //4
    glVertex3d(coords[0][0], coords[1][0], coords[2][1]);
    glVertex3d(coords[0][1], coords[1][0], coords[2][1]);
    glVertex3d(coords[0][1], coords[1][1], coords[2][1]);
    glVertex3d(coords[0][0], coords[1][1], coords[2][1]);
    glVertex3d(coords[0][0], coords[1][0], coords[2][1]);
    glEnd();


    glBegin(GL_LINE_STRIP);
    //5
    glVertex3d(coords[0][1], coords[1][0], coords[2][0]);
    glVertex3d(coords[0][1], coords[1][1], coords[2][0]);
    glVertex3d(coords[0][1], coords[1][1], coords[2][1]);
    glVertex3d(coords[0][1], coords[1][0], coords[2][1]);
    glVertex3d(coords[0][1], coords[1][0], coords[2][0]);
    glEnd();


    glBegin(GL_LINE_STRIP);
    //6
    glVertex3d(coords[0][0], coords[1][1], coords[2][0]);
    glVertex3d(coords[0][1], coords[1][1], coords[2][0]);
    glVertex3d(coords[0][1], coords[1][1], coords[2][1]);
    glVertex3d(coords[0][0], coords[1][1], coords[2][1]);
    glVertex3d(coords[0][0], coords[1][1], coords[2][0]);
    glEnd();
}

int depth = 0;

void printTree(KD_Tree::pnode root, bool needDraw = false, int tdepth = 0) {

    glColor3d(0, 1, 0);


    if (root->left != nullptr) {
        printTree(root->left, needDraw, tdepth + 1);
    }
    if (root->right != nullptr) {
        printTree(root->right, needDraw, tdepth + 1);
    }

    if (!needDraw) {
        drawBbox(root->boundingBox);
    }
    if (needDraw) {
        glColor3d(0, 0, 1);
        if (root->isLeaf != nullptr) {
            for (auto obj : root->isLeaf->objects) {
                if (obj->getTag()) {
                    auto pos = ((Sphere *) obj)->getPosition();
                    glTranslated(pos.x, pos.y, pos.z);
                    glutSolidSphere(((Sphere *) obj)->getRadius(), 100, 100);
                    glTranslated(-pos.x, -pos.y, -pos.z);
                } else {
                    auto pnt = ((Triangle *) obj)->getPoints();
                    glBegin(GL_POLYGON);
                    for (auto pn : pnt) {
                        glVertex3d(pn.x, pn.y, pn.z);
                    }
                    glEnd();
                }
            }
        }
    }

}

double cam_x = 5, cam_y = 5, cam_z = 5;
double scale = 1;

void output(int x, int y, float r, float g, float b, const char *string) {
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    int len, i;
    len = (int) strlen(string);
    for (i = 0; i < len; i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, string[i]);
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    output(1, 5, 0, 0, 0, to_string(cam_x).c_str());
    output(2, 5, 0, 0, 0, to_string(cam_y).c_str());
    output(3, 5, 0, 0, 0, to_string(cam_z).c_str());

    printTree(tree->root, true);
    printTree(tree->root);

    glutSwapBuffers();
}

void reshapeEvent(int x_size, int y_size) {
    glViewport(0, 0, x_size, y_size);


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //gluPerspective(45.0, (double)x_size / y_size, -10, 100);

    glOrtho(-10, 10, -10, 10, -10, 10);

    gluLookAt(5, 5, 5,
              0, 0, 0,
              0, 1, 0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void glInit() {
    glClearColor(1.0, 1.0, 1.0, 1.0);

    //glEnable(GL_DEPTH);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

void initRayCasting() {

    if (tree != nullptr) {
        delete tree;
    }
    MaterialsFactory materialsFactory;
    Viewport viewport;
    vector<Light> lights;
    vector<IGeometryObject *> geometry;
    ObjLoader rtFile("examples/obj_examples/buggy2.1.obj", "examples/obj_examples/", materialsFactory, viewport, lights, geometry);
    //RT_file rtFile("examples/scene.rt", materialsFactory, viewport, lights, geometry);

    tree = new KD_Tree(geometry);
}

void timer_redisplay(int) {
    //glutPostRedisplay();
}

void keyboardFunc(unsigned char chr, int x, int y) {
    if (chr == 'w') {
        cam_x += 0.2;
    } else if (chr == 's') {
        cam_x -= 0.2;
    } else if (chr == 'a') {
        cam_y -= 0.2;
    } else if (chr == 'd') {
        cam_y += 0.2;
    } else if (chr == 'q') {
        cam_z -= 0.2;
    } else if (chr == 'e') {
        cam_z += 0.2;
    } else if (chr == '+') {
        scale += 0.1;
    } else if (chr == '-') {
        scale -= 0.1;
    } else if (chr == ' ') {
        initRayCasting();
    } else if (chr == '[') {
        depth++;
    } else if (chr == ']') {
        depth--;
    }
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //gluPerspective(45.0, (double)x_size / y_size, -10, 100);

    glOrtho(-10, 10, -10, 10, -200, 200);

    gluLookAt(cam_x, cam_y, cam_z, 0, 0, 0, 0, 1, 0);

    glScaled(scale, scale, scale);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glutPostRedisplay();
}

int main(int argc, char **argv) {


    glutInit(&argc, argv);
    glutInitWindowSize(1000, 1000);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow("Ray-tracing by Ilya Romanenko");


    glInit();

    initRayCasting();

    glutKeyboardFunc(keyboardFunc);
    glutReshapeFunc(reshapeEvent);
    glutTimerFunc(100, timer_redisplay, 0);
    glutDisplayFunc(render);
    glutMainLoop();

    return 0;
}