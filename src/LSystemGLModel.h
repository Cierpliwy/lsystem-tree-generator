#ifndef LSYSTEMGLMODEL_H
#define LSYSTEMGLMODEL_H
#include "Drawable.h"
#include "LSystemModelInterface.h"

struct Matrix
{
    float data[16];
    Matrix(float* ptr) {
        for(int i=0;i<16;++i)
            data[i] = ptr[i];
    }
};

class LSystemGLModel : public Drawable, public LSystemModelInterface
{
public:
    LSystemGLModel();
    void process(LSystem& l, int recursion);
    void draw();

private:
    void move(int length);
    void draw(int length);
    void rotate(int x_axis_angle, int y_axis_angle, int z_axis_angle);
    void push();
    void pop();

    const LSystem::RecursionElement* recurtionToDraw_;
    const LSystem* lsystemToDraw_;
    float recurtions_;
    std::vector<Matrix> matrixesStack_;
    bool processed_;
    float color_[3];
};

#endif // LSYSTEMGLMODEL_H
