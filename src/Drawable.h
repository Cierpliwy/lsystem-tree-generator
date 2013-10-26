#ifndef DRAWABLE_H
#define DRAWABLE_H

/**
 * @brief A drawable interface which all objects who are displayed on
 *        OpenGL window should inherit from.
 */
class Drawable
{
public:
    Drawable();

    virtual void draw() = 0;
};

#endif // DRAWABLE_H
