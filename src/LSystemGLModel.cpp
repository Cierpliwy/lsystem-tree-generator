#include "LSystemGLModel.h"
#include <map>
#include <QGLWidget>
#include <iostream>
#include <cmath>
using std::map;


LSystemGLModel::LSystemGLModel() {
    processed_ = false;
}

void LSystemGLModel::draw() {
    if(processed_){
        const std::string& commands_list = recurtionToDraw_->first;
        const std::vector<int> recursion_depth_list = recurtionToDraw_->second;
        for(unsigned int i = 0; i < commands_list.size(); ++i){
            map<char, Command>::const_iterator it = (*lsystemToDraw_).getDefinitionMap().find(commands_list[i]);

            if(it != (*lsystemToDraw_).getDefinitionMap().end()){
                const Command& c = it->second;

                if(c.name.compare("move") == 0){
                    move(c.argv[0]);
                }else if(c.name.compare("draw") == 0){
                    color_[0] = std::max(i/(float)commands_list.size() - 0.2f,0.6f);
                    color_[1] = std::max(i/(float)commands_list.size() + 0.1f,0.4f);
                    color_[2] = std::max(i/(float)commands_list.size(),0.4f);
                    std::cout <<  recursion_depth_list[i] << " " << recurtions_ << std::endl;
                    draw(c.argv[0]);
                }else if(c.name.compare("rotate") == 0){
                    rotate(c.argv[0], c.argv[1], c.argv[2]);
                }else if(c.name.compare("push") == 0){
                    push();
                }else if(c.name.compare("pop") == 0){
                    pop();
                }
            }
        }
    }
}

void LSystemGLModel::process(LSystem &l, int recursion) {
    recurtionToDraw_ = &(l.generate(recursion));
    lsystemToDraw_ = &l;
    processed_ = true;
    recurtions_ = recursion;
}

void LSystemGLModel::move(int length){
    glTranslatef(0,length,0);
}

void LSystemGLModel::draw(int length){
    glBegin(GL_LINES);
    glColor3f(color_[0], color_[1], color_[2]);
    glVertex3f(0,0,0);
    glVertex3f(0,length,0);
    glEnd();
    move(length);
}

void LSystemGLModel::rotate(int x_axis_angle, int y_axis_angle, int z_axis_angle){
    glRotatef(x_axis_angle,1,0,0);
    glRotatef(y_axis_angle,0,1,0);
    glRotatef(z_axis_angle,0,0,1);
}

void LSystemGLModel::push(){
    float tmp_matrix[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, tmp_matrix );
    matrixesStack_.push_back(tmp_matrix);
}

void LSystemGLModel::pop(){
    Matrix m = matrixesStack_.back();
    glLoadMatrixf(m.data);
    matrixesStack_.pop_back();
}
