#ifndef LSYSTEMMODELINTERFACE_H
#define LSYSTEMMODELINTERFACE_H
#include <vector>
#include "LSystem.h"

// Interfejs dla modelu, który potrafi przetrzymywać obiekt LSystemu we
// wsłasnej formie np. Graficznej. Wszystkie interfejsy powinny obsługiwać
// ten sam zbiór komend dla L-Systemów.
class LSystemModelInterface
{

public:

    LSystemModelInterface();


    static void addCommand(const Command& c){ commands_.push_back(c); }
    static const std::vector<Command>& getCommands() {return commands_;}

    // Funkcja odpowiedzialna za przetworzenie L-Systemu do własnej postaci.
    virtual void process(LSystem& l, int recursion) = 0;

protected:

    static std::vector<Command> commands_;
};

#endif // LSYSTEMMODELINTERFACE_H
