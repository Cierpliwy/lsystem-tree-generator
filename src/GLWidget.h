#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QVector3D>
#include <QVector4D>
#include <QGLWidget>

class Drawable;

// Okno, które zapewnia akcelerację graficzną dotyczącą wyświetlania
// trójwymiarowej grafiki.
class GLWidget : public QGLWidget
{
    Q_OBJECT

public:

    explicit GLWidget(QWidget *parent = 0);

    void setDrawable(Drawable* d) { drawableObject_ = d;}

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    void setCameraRotation(float x, float y, float z);
    void setLookAtPosition(float x, float y, float z);
    void setLookAtDistance(float distance);
    void setZoomDelta(float delta);

protected:

    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    Drawable* drawableObject_;

private:

    //Przelicza biegunowe współrzędne punktu na radialne.
    void polarToCartesian();

    float zoomDelta_;
    QPoint lastMousePosition_;
    QVector3D cameraPosition_;
    QVector3D lookAtPosition_;
    QVector4D rotation_;
};

#endif // GLWIDGET_H
