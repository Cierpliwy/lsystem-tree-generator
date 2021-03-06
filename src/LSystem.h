#ifndef LSYSTEM_H
#define LSYSTEM_H
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <utility>

/**
 * @brief The Command structure describes a command which
 *        can be bound to any character of L-system
 *        alphabet.
 */
struct Command{

    std::string name;
    unsigned int args;
    std::vector<float> argv;

    Command(){}
    Command(const std::string& new_name, unsigned int new_args) {
        name = new_name;
        args = new_args;
    }
};

/**
 * @brief The LSystem class
 */
class LSystem
{

public:

    LSystem();
    virtual ~LSystem();

    typedef std::pair<std::string, std::vector<int> > RecursionElement;

    void setName(const std::string& new_name){ name_ = new_name;}
    const std::string& getName() const { return name_; }

    void setAxiom(const std::string& new_axiom){ axiom_ = new_axiom;}
    const std::string& getAxiom() const { return axiom_; }

    void addRule(char letter, const std::string& rule) { rules_[letter] = rule;}
    const std::map<char, std::string>& getRuleMap() const { return rules_; }

    void addDefinition(char letter, const Command cmd) { definitions_[letter] = cmd;}
    const std::map<char, Command>& getDefinitionMap() const { return definitions_; }

    int getRecDepth() const { return recursions_.size();}
    const RecursionElement& generate(int recursion_depth);

protected:

    std::string name_;
    std::string axiom_;
    std::map<char, std::string> rules_;
    std::map<char, Command> definitions_;
    std::vector<RecursionElement> recursions_;

};

#endif // LSYSTEM_H
