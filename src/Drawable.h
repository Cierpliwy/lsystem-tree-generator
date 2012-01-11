#ifndef DRAWABLE_H
#define DRAWABLE_H

// Interfejs po którym muszą dziedziczyć wszystkie obekty, które
// chcą być wyświetlanie w oknie OpenGL.
class Drawable
{
public:
    Drawable();

    virtual void draw() = 0;
};

#endif // DRAWABLE_H
