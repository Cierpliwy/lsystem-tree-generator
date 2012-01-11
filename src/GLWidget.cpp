#include <QTimer>
#include <QMouseEvent>
#include <ctime>
#include <cmath>
#include <iostream>

#include "GLWidget.h"
#include "Drawable.h"

GLWidget::GLWidget(QWidget *parent) :
    QGLWidget(QGLFormat(QGL::DoubleBuffer),parent) {
    rotation_.setX(90);
    rotation_.setY(180);
    rotation_.setZ(100);

    polarToCartesian();

    drawableObject_ = NULL;
}

void GLWidget::polarToCartesian() {
    cameraPosition_.setX(rotation_.z()*cos(rotation_.y()/180.0*M_PI)*sin(rotation_.x()/180.0*M_PI));
    cameraPosition_.setZ(rotation_.z()*sin(rotation_.y()/180.0*M_PI)*sin(rotation_.x()/180.0*M_PI));
    cameraPosition_.setY(rotation_.z()*cos(rotation_.x()/180.0*M_PI));
}

QSize GLWidget::minimumSizeHint() const {
    return QSize(100,100);
}
QSize GLWidget::sizeHint() const {
    return QSize(640,480);
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
    lastMousePosition_ = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    int dx = event->x() - lastMousePosition_.x();
    int dy = event->y() - lastMousePosition_.y();

    if (event->buttons() & Qt::LeftButton) {

        rotation_.setX(rotation_.x()-180.0*dy/width());
        rotation_.setY(rotation_.y()+180.0*dx/width());

        if( rotation_.x() > 180.0 ) rotation_.setX(180.0);
        if( rotation_.x() <= 0 ) rotation_.setX(0.001);

        polarToCartesian();

        updateGL();
    }

    lastMousePosition_ = event->pos();
}

void GLWidget::wheelEvent(QWheelEvent *event) {
    int degrees = event->delta()/8;
    float radiusDelta = degrees/30.0*30;

    rotation_.setZ(rotation_.z()-radiusDelta);
    if( rotation_.z() <= 0 ) rotation_.setZ(0.0001);

    polarToCartesian();

    updateGL();
}

void GLWidget::initializeGL() {
    glClearColor(0.1,0.1,0.1,1);

    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void GLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    gluLookAt(cameraPosition_.x(),cameraPosition_.y(),cameraPosition_.z(),0,0,0,0,1,0);

    if(drawableObject_ != NULL){
        drawableObject_->draw();
    }
}

void GLWidget::resizeGL(int w, int h) {
    if(h==0) h = 1;
    glViewport(0,0,w,h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0,w/h,1,10000);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

