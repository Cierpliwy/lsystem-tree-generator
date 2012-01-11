#ifndef LSYSTEMGLMODEL_H
#define LSYSTEMGLMODEL_H
#include <glm/glm.hpp>
#include <stack>
#include "Drawable.h"
#include "LSystemModelInterface.h"

class LSystemGLModel : public Drawable, public LSystemModelInterface
{

public:

    enum ColorMode {
        RECURCION_LEVEL_COLOR,
        SEQUENCE_COLOR
    };

    LSystemGLModel();

    void setColorMode(ColorMode mode, glm::vec3 start_color, glm::vec3 end_color);
    void process(LSystem& l, int recursion);
    void draw();

private:

    void move(float length);
    void draw(float length);
    void rotate(float x_axis_angle, float y_axis_angle, float z_axis_angle);
    void push();
    void pop();

    glm::mat4 rotationMatrix_;
    glm::mat4 translationMatrix_;
    glm::mat4 currentMatrix_;

    glm::vec4 point_;
    glm::vec4 finalPoint_;

    glm::vec3 color_;
    glm::vec3 startColor_;
    glm::vec3 endColor_;

    ColorMode colorMode_;

    std::stack<glm::mat4> matrixStack_;

    unsigned int displayList_;
};

#endif // LSYSTEMGLMODEL_H
