#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <set>
#include <boost/shared_ptr.hpp>
#include "LSystem.h"

class LSystemModelInterface;

/**
 * @brief The ParseError struct decribes error thrown by a parser
 */
struct ParseError
{
    ParseError(unsigned int new_row, unsigned int new_column, const std::string& new_description) :
        row(new_row), column(new_column), description(new_description) {}
    unsigned int row, column;
    std::string description;
};

typedef std::vector<boost::shared_ptr<LSystem> > LSystemVector;

/**
 * @brief The Parser class creates L-System objects from L-System
 *        script language.
 */
class Parser
{
public:

    static Parser& getInstance();
    bool parseLSystem( const std::string& script_string );
    LSystemVector getLSystems() const;
    const std::vector<ParseError>& getErrors() const;

    void registerCommands();

protected:

    Parser() {}
    Parser(const Parser&) {}

    static Parser *parserInstance_;
    std::vector<ParseError> parseErrors_;
    std::vector<boost::shared_ptr<LSystem> > lsystems_;
    std::vector<Command> commands_;
  
private:

    bool isLSystemName(const std::string& name) const;

    bool ignoreWhiteChars();
    std::string::size_type skipUntilWhiteCharOr(char stop_char, char stop_char2) const;

    void jumpToEndOfLSystem();

    void getErrorPosition();
    void reportError(const std::string &error_description);
  
    enum State {
        LSYSTEM_NAME,
        LSYSTEM_START,
        ALPHABET_KEYWORD,
        ALPHABET_COLON,
        ALPHABET_CHAR,
        AXIOM_KEYWORD,
        AXIOM_COLON,
        AXIOM_STRING,
        RULES_KEYWORD,
        RULES_COLON,
        RULES_CHAR,
        RULES_EQUAL,
        RULES_STRING,
        DEFINE_KEYWORD,
        DEFINE_COLON,
        DEFINE_CHAR,
        DEFINE_EQUAL,
        DEFINE_COMMAND,
        DEFINE_PARAM,
        LSYSTEM_END
    };

    LSystem* lsystem_;
    const std::string* scriptString_;
    std::string::size_type position_;
    State state_;
    bool errors_;
    int errorColumn_;
    int errorRow_;
    std::set<char> alphabet_;
};

#endif //PARSER_H
