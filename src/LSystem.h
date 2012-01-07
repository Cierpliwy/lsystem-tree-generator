#ifndef LSYSTEM_H
#define LSYSTEM_H
#include <iostream>
#include <vector>
#include <string>
#include <map>

//Struktura opisująca deklarację komendy.
struct Command{
    //Nazwa komendy
    std::string name;
    //Liczba argumentów komendy
    unsigned int args;
    //Wektor argumentów (już konkretnego wywołania komendy)
    std::vector<float> argv;


    //Konstruktor domyślny.
    Command(){}

    //Konstruktor ustawiający nazwę komendy oraz jej liczbę argumentów
    //zmiennoprzecinkowych.
    Command(const std::string& new_name, unsigned int new_args) {
        name = new_name;
        args = new_args;
    }
};

//Klasa opisująca L-System.
class LSystem
{
public:
    LSystem();
    virtual ~LSystem();

    //Ustawia/Zwraca nazwę L-systemu.
    void setName(const std::string& new_name){ name_ = new_name;}
    const std::string& getName() const { return name_; }

    //Ustawia/Zwraca ciąg początkowy L-systemu.
    void setAxiom(const std::string& new_axiom){ axiom_ = new_axiom;}
    const std::string& getAxiom() const { return axiom_; }

    //Dodaje regułę do L-Systemu. W czasie generowania znak reguły podmienia się
    //z ciągiem reguły.
    void addRule(char letter, const std::string& rule) { rules_[letter] = rule;}
    //Zwraca mapę reguł.
    const std::map<char, std::string>& getRuleMap() const { return rules_; }

    //Dodaje powiązanie między komendą a znakiem.
    void addDefinition(char letter, const Command cmd) { definitions_[letter] = cmd;}

    //Zwraca mapę definicji.
    const std::map<char, Command>& getDefinitionMap() const { return definitions_; }

    //Zwraca ostatnią głębokość rekurencji.
    int getRecDepth() const { return recursions_.size();}

    //Generuje ciąg L-systemu na podanej generacji.
    const std::string& generate(int recursion_depth);

protected:
    std::string name_;
    std::string axiom_;
    std::map<char, std::string> rules_;
    std::map<char, Command> definitions_;
    std::vector<std::string> recursions_;

private:


};

#endif // LSYSTEM_H
