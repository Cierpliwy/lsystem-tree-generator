#ifndef LSYSTEMTEXTMODEL_H
#define LSYSTEMTEXTMODEL_H
#include "LSystemModelInterface.h"

class LSystemTextModel : public LSystemModelInterface
{
public:
    LSystemTextModel();
    void process(const LSystem& l, int recursion);
};

#endif // LSYSTEMTEXTMODEL_H
