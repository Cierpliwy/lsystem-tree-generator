#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QVector3D>
#include <QVector4D>
#include <QGLWidget>

class Drawable;

//Klasa opakowująca okno OpenGL
class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    explicit GLWidget(QWidget *parent = 0);

    //Ustawia obiekt rysujacy.
    void setDrawable(Drawable* d) { drawableObject_ = d;}
    //Wskazówki dotyczące rozmiaru okna.
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    //Inicjalizacjia stanu maszyny OpenGL
    void initializeGL();
    //Funkcja rysująca scenę OpenGL
    void paintGL();
    //Funkcja wywoływana gdy nastąpiła zmiana rozmiaru okna OpenGL.
    void resizeGL(int w, int h);

    //Obsługa kliknięcia myszy.
    void mousePressEvent(QMouseEvent *event);
    //Obsługa ruchów myszy.
    void mouseMoveEvent(QMouseEvent *event);
    //Obsługa scrolla.
    void wheelEvent(QWheelEvent *event);

    Drawable* drawableObject_;

private:
    //Przelicza biegunowe współrzędne punktu na radialne.
    void polarToCartesian();
    //Ostatnia pozycja myszy.
    QPoint lastMousePosition_;
    //Aktualna pozycja kamery.
    QVector3D camera;
    //Aktualna rotacja wokół obiektu + odległość.
    QVector4D rotation;

    GLuint listIndex_;
};

#endif // GLWIDGET_H
