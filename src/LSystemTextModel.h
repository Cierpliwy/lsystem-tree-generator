#ifndef LSYSTEMTEXTMODEL_H
#define LSYSTEMTEXTMODEL_H
#include "LSystemModelInterface.h"
#include <QString>

class LSystemTextModel : public LSystemModelInterface
{
public:
    LSystemTextModel();
    QString getBuffer(){ return buffer_; }
    void process(LSystem& l, int recursion);
protected:
    QString buffer_;
};

#endif // LSYSTEMTEXTMODEL_H
