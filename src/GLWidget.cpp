#include <QTimer>
#include <QMouseEvent>
#include <ctime>
#include <cmath>
#include <iostream>

#include "GLWidget.h"
#include "Drawable.h"

//Konstruktor ustawia podwójne buforowanie.
GLWidget::GLWidget(QWidget *parent) :
    QGLWidget(QGLFormat(QGL::DoubleBuffer),parent) {

    //Początkowe ustawienie kamery.
    rotation.setX(90);
    rotation.setY(180);
    rotation.setZ(100);
    //Wyliczamy współrzędne punktu.
    polarToCartesian();
    drawableObject_ = NULL;

}

//Przelicza współrzędne biegunowe na kartezjańskie
void GLWidget::polarToCartesian()
{
    camera.setX(rotation.z()*cos(rotation.y()/180.0*M_PI)*sin(rotation.x()/180.0*M_PI));
    camera.setZ(rotation.z()*sin(rotation.y()/180.0*M_PI)*sin(rotation.x()/180.0*M_PI));
    camera.setY(rotation.z()*cos(rotation.x()/180.0*M_PI));
}

//Wskazówki dotyczące rozmiaru okna.
QSize GLWidget::minimumSizeHint() const {
    return QSize(100,100);
}
QSize GLWidget::sizeHint() const {
    return QSize(640,480);
}

//Jeżeli naciśnięto uaktualnij ostatnią pozycję myszy.
void GLWidget::mousePressEvent(QMouseEvent *event) {
    lastMousePosition_ = event->pos();
}

//Sprawdzamy ruch myszy i obracamy scenę.
void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    int dx = event->x() - lastMousePosition_.x();
    int dy = event->y() - lastMousePosition_.y();

    if (event->buttons() & Qt::LeftButton) {
        //Uaktualniamy rotację.
        rotation.setX(rotation.x()-180.0*dy/width());
        rotation.setY(rotation.y()+180.0*dx/width());
        if( rotation.x() > 180.0 ) rotation.setX(180.0);
        if( rotation.x() <= 0 ) rotation.setX(0.001);

        //Wyliczamy współrzędne punktu.
        polarToCartesian();

        updateGL();
    }

    lastMousePosition_ = event->pos();
}

//Obsługujemy scroll by można było zoomować.
void GLWidget::wheelEvent(QWheelEvent *event) {

    //O ile stopni przesunął się scroll.
    int degrees = event->delta()/8;
    float radiusDelta = degrees/30.0*30;
    rotation.setZ(rotation.z()-radiusDelta);
    if( rotation.z() <= 0 ) rotation.setZ(0.0001);

    //Wyliczamy współrzędne punktu.
    polarToCartesian();

    updateGL();
}

//Inicjalizacja OpenGL.
void GLWidget::initializeGL() {
    glClearColor(0.1,0.1,0.1,1);

    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


}

//Funkcja rysująca scenę OpenGL (na razie testowy kwadrat)
void GLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Ustawiamy kamerę.
    glLoadIdentity();
    gluLookAt(camera.x(),camera.y(),camera.z(),0,0,0,0,1,0);

    if(drawableObject_ != NULL){
        drawableObject_->draw();
    }

}

//Zmieniamy rozmiar sceny i jego prespektywę.
void GLWidget::resizeGL(int w, int h) {
    if(h==0) h = 1;
    glViewport(0,0,w,h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0,w/h,1,10000);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

