#include "LSystemGLModel.h"
#include <QGLWidget>
#include <iostream>
#include <map>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

using std::map;


LSystemGLModel::LSystemGLModel() {
    startColor_ = glm::vec3(0.4f,1.0f,0.2f);
    endColor_ = glm::vec3(0.5f,2.0f,2.1f);
    colorMode_ = SEQUENCE_COLOR;
}

void LSystemGLModel::draw() {
    if( glIsList(displayList_))
        glCallList(displayList_);
}

void LSystemGLModel::process(LSystem &l, int recursion) {

    if( glIsList(displayList_)) glDeleteLists(displayList_,1);
    displayList_ = glGenLists(1);

    const LSystem::RecursionElement& elements = l.generate(recursion);
    const std::string& commands_list = elements.first;
    const std::vector<int>& recursion_depth_list = elements.second;

    currentMatrix_ = glm::mat4(1.0f);
    numberOfVertices_ = 0;
    xLimits_ = glm::vec2(0.0f);
    yLimits_ = glm::vec2(0.0f);
    zLimits_ = glm::vec2(0.0f);

    glNewList(displayList_, GL_COMPILE);

    glBegin(GL_LINES);
    for(unsigned int i = 0; i < commands_list.size(); ++i) {

         map<char, Command>::const_iterator it = l.getDefinitionMap().find(commands_list[i]);

         if(it != l.getDefinitionMap().end()) {

             const Command& c = it->second;

             if(c.name.compare("move") == 0) {
                 move(c.argv[0]);

             } else if(c.name.compare("draw") == 0) {
                 if( colorMode_ == SEQUENCE_COLOR)
                     color_ = startColor_ + (endColor_ - startColor_)*((float)i/commands_list.size());
                 else
                     color_ = startColor_ + (endColor_ - startColor_)*((float)recursion_depth_list[i]/recursion);
                 numberOfVertices_+=2;
                 draw(c.argv[0]);

             } else if(c.name.compare("rotate") == 0) {
                 rotate(c.argv[0], c.argv[1], c.argv[2]);

             } else if(c.name.compare("push") == 0) {
                 push();

             } else if(c.name.compare("pop") == 0) {
                 pop();
             }
         }
    }
    glEnd();

    glEndList();

    std::cout << "Vertices: " << numberOfVertices_ << std::endl
            << "X - max:" << xLimits_.x << " min:" << xLimits_.y << std::endl
            << "Y - max:" << yLimits_.x << " min:" << yLimits_.y << std::endl
            << "Z - max:" << zLimits_.x << " min:" << zLimits_.y << std::endl
            << "Max distance: " << getDefaultDistanceFromModel() << std::endl;
}

void LSystemGLModel::move(float length) {
    currentMatrix_ = glm::translate(currentMatrix_, glm::vec3(0.0f,length,0.0f));
}

void LSystemGLModel::draw(float length) {
    point_.x = 0;
    point_.y = 0;
    point_.z = 0;
    point_.w = 1.0f;
    finalPoint_ = currentMatrix_ * point_ ;

    glColor3f(color_.x,color_.y,color_.z);
    glVertex3f(finalPoint_.x, finalPoint_.y, finalPoint_.z);

    xLimits_.x = glm::max(finalPoint_.x,xLimits_.x);
    xLimits_.y = glm::min(finalPoint_.x,xLimits_.y);

    yLimits_.x = glm::max(finalPoint_.y,yLimits_.x);
    yLimits_.y = glm::min(finalPoint_.y,yLimits_.y);

    zLimits_.x = glm::max(finalPoint_.z,zLimits_.x);
    zLimits_.y = glm::min(finalPoint_.z,zLimits_.y);

    point_.y = length;
    finalPoint_ = currentMatrix_ * point_ ;

    glVertex3f(finalPoint_.x, finalPoint_.y, finalPoint_.z);

    currentMatrix_ = glm::translate(currentMatrix_, glm::vec3(0.0f,length,0.0f));
}

void LSystemGLModel::rotate(float x_axis_angle, float y_axis_angle, float z_axis_angle) {
   currentMatrix_ = glm::rotate(currentMatrix_, x_axis_angle, glm::vec3(1.0f,0.0f,0.0f));
   currentMatrix_ = glm::rotate(currentMatrix_, y_axis_angle, glm::vec3(0.0f,1.0f,0.0f));
   currentMatrix_ = glm::rotate(currentMatrix_, z_axis_angle, glm::vec3(0.0f,0.0f,1.0f));
}

void LSystemGLModel::push() {
    matrixStack_.push(currentMatrix_);
}

void LSystemGLModel::pop() {
    currentMatrix_ = matrixStack_.top();
    matrixStack_.pop();
}

void LSystemGLModel::setColorMode(ColorMode mode, glm::vec3 start_color, glm::vec3 end_color) {
    startColor_ = start_color;
    endColor_ = end_color;
    colorMode_ = mode;
}

glm::vec3 LSystemGLModel::getCenterOfModel() {
    return glm::vec3((xLimits_.x+xLimits_.y)/2.0f,(yLimits_.x+yLimits_.y)/2.0f,(zLimits_.x+zLimits_.y)/2.0f);
}

float LSystemGLModel::getDefaultDistanceFromModel() {

    float distance, maxDistance = 0;
    glm::vec3 center = getCenterOfModel();

    distance = glm::distance(glm::vec3(xLimits_.x,yLimits_.x,zLimits_.x), center);
    maxDistance = glm::max(distance,maxDistance);
    distance = glm::distance(glm::vec3(xLimits_.y,yLimits_.x,zLimits_.x), center);
    maxDistance = glm::max(distance,maxDistance);
    distance = glm::distance(glm::vec3(xLimits_.x,yLimits_.x,zLimits_.y), center);
    maxDistance = glm::max(distance,maxDistance);
    distance = glm::distance(glm::vec3(xLimits_.y,yLimits_.x,zLimits_.y), center);
    maxDistance = glm::max(distance,maxDistance);
    distance = glm::distance(glm::vec3(xLimits_.x,yLimits_.y,zLimits_.x), center);
    maxDistance = glm::max(distance,maxDistance);
    distance = glm::distance(glm::vec3(xLimits_.y,yLimits_.y,zLimits_.x), center);
    maxDistance = glm::max(distance,maxDistance);
    distance = glm::distance(glm::vec3(xLimits_.x,yLimits_.y,zLimits_.y), center);
    maxDistance = glm::max(distance,maxDistance);
    distance = glm::distance(glm::vec3(xLimits_.y,yLimits_.y,zLimits_.y), center);
    maxDistance = glm::max(distance,maxDistance);

    return 3*maxDistance;
}
