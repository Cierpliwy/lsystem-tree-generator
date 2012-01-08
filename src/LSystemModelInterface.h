#ifndef LSYSTEMMODELINTERFACE_H
#define LSYSTEMMODELINTERFACE_H
#include <vector>
#include "LSystem.h"

class LSystemModelInterface
{
public:
    LSystemModelInterface();
    //Dodanie nowej komendy
    static void addCommand(const Command& c){ commands_.push_back(c); }
    //Zwraca wektor komend
    static const std::vector<Command>& getCommands() {return commands_;}
    //Funkcja generująca drzewo 3D
    virtual void process(LSystem& l, int recursion) = 0;

protected:
    static std::vector<Command> commands_;
};

#endif // LSYSTEMMODELINTERFACE_H
