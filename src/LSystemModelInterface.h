#ifndef LSYSTEMMODELINTERFACE_H
#define LSYSTEMMODELINTERFACE_H
#include <vector>
#include "LSystem.h"

/**
 * @brief The LSystemModelInterface class in as interface for a model which
 *        can store it's own representation of an L-System (for example
 *        graphical).
 */
class LSystemModelInterface
{

public:

    LSystemModelInterface();


    static void addCommand(const Command& c){ commands_.push_back(c); }
    static const std::vector<Command>& getCommands() {return commands_;}

    virtual void process(LSystem& l, int recursion) = 0;

protected:

    static std::vector<Command> commands_;
};

#endif // LSYSTEMMODELINTERFACE_H
