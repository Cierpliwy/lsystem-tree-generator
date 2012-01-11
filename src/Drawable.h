#ifndef DRAWABLE_H
#define DRAWABLE_H

//Interfejs dla klas, które mogą zostać narysowane w oknie OpenGL.
class Drawable
{
public:
    Drawable();

    //Funkcja wywołująca funkcje OpenGL.
    virtual void draw() = 0;
};

#endif // DRAWABLE_H
