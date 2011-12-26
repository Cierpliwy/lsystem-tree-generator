#ifndef LSYSTEM_H
#define LSYSTEM_H
#include <iostream>
#include <vector>
#include <string>
#include <map>

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

    //void addDefinition(char letter, const command cmd) { definitions_[letter] = cmd;}
    //const std::map<char, command> getDefinitionMap() const { return definitions_; }

    //Zwraca ostatnią głębokość rekurencji.
    int getRecDepth() const { return recursions_.size();}

    //Generuje ciąg L-systemu na podanej generacji.
    const std::string& generate(int recursion_depth);

protected:
private:
    std::string name_;
    std::string axiom_;
    std::map<char, std::string> rules_;
    //map<char, command> definitions_;
    std::vector<std::string> recursions_;

};

#endif // LSYSTEM_H
