#include "Parser.h"
#include "LSystemModelInterface.h"

#include <iostream>
#include <sstream>
#include <boost/lexical_cast.hpp>

/*
 * Author note: PLEASE DON'T WRITE LEXERS AND PARSERS LIKE THAT!
 *              It's wrong. It's ugly. Learn the good way!
 */

using namespace std;
using namespace boost;

Parser* Parser::parserInstance_ = NULL;

Parser& Parser::getInstance() {
    if( parserInstance_ == NULL ) parserInstance_ = new Parser();
    return *parserInstance_;
}

bool Parser::isLSystemName(const string& name) const {
    for( string::size_type i = 0; i < name.length(); ++i )
        if( !isalnum(name[i]) && name[i] != '_' ) return false;
    return true;
}

bool Parser::ignoreWhiteChars() {
    for(; position_ < scriptString_->length(); ++position_)
        if(!isspace(scriptString_->at(position_))) break;

    if( position_ >= scriptString_->length()) return false;
    return true;
}

std::size_t Parser::skipUntilWhiteCharOr(char stop_char, char stop_char2) const {
    string::size_type i = position_;
    for(;i < scriptString_->length(); ++i)
        if(isspace(scriptString_->at(i)) || scriptString_->at(i) == stop_char || scriptString_->at(i) == stop_char2) break;
    return i;
}

void Parser::jumpToEndOfLSystem() {
    for(;position_ < scriptString_->length(); ++position_)
        if( scriptString_->at(position_) == '}') break;

    if( position_ < scriptString_->length() ) position_++;
}

void Parser::getErrorPosition() {
    errorRow_ = 1;
    errorColumn_ = 0;
    for(string::size_type i = 0; i < position_; ++i) {
        errorColumn_++;
        if( scriptString_->at(i) == '\n' ) { errorRow_++; errorColumn_ = 0; }
    }
}

void Parser::reportError(const std::string& error_description) {
    getErrorPosition();
    parseErrors_.push_back(ParseError(errorRow_,errorColumn_,error_description));
    jumpToEndOfLSystem();
    state_ = LSYSTEM_NAME;
    errors_ = true;
}

bool Parser::parseLSystem ( const std::string& script_string ) {

    lsystem_ = NULL;
    scriptString_ = &script_string;
    position_ = 0;
    state_ = LSYSTEM_NAME;
    errors_ = false;
    string::size_type tmpPosition;
    string tmpString;
    char ruleChar = '\0';
    bool ruleSet = false;
    char defineChar = '\0';
    const Command *defineCmd = NULL;
    vector<float> defineParams;
    bool lsystemFinished = false;
    parseErrors_.clear();
    lsystems_.clear();

    while( ignoreWhiteChars() ) {

        switch(state_) {

        case LSYSTEM_NAME:

            if( !lsystemFinished ) delete lsystem_;
            lsystem_ = new LSystem;
            lsystemFinished = false;

            alphabet_.clear();

            tmpPosition = skipUntilWhiteCharOr('{','\n');
            tmpString = scriptString_->substr(position_, tmpPosition-position_);

            if( isLSystemName(tmpString) && tmpString.length() > 0) {

                vector<boost::shared_ptr<LSystem> >::const_iterator it = lsystems_.begin();
                for(; it!=lsystems_.end(); ++it)
                    if( (*it)->getName().compare(tmpString) == 0 ) break;

                if( it == lsystems_.end()) {
                    position_ = tmpPosition;
                    state_ = LSYSTEM_START;
                    lsystem_->setName(tmpString);
                } else {
                    stringstream ss;
                    ss << "L-System called '" << tmpString << "' already exists in script file.";
                    reportError(ss.str());
                }
            } else {
                stringstream ss;
                ss << "L-System name '" << tmpString << "' must be alphanumeric and can optionally contain '_' character.";
                reportError(ss.str());
            }
            break;

        case LSYSTEM_START:
            if( scriptString_->at(position_) == '{' ) {
                position_++;
                state_ = ALPHABET_KEYWORD;
            } else reportError("After L-System name you have to start a block: '{' is missing.");
            break;

        case ALPHABET_KEYWORD:
            tmpPosition = skipUntilWhiteCharOr(':','\n');
            tmpString = scriptString_->substr(position_, tmpPosition-position_);
            if( tmpString.compare("alphabet") == 0 ) {
                position_ = tmpPosition;
                state_ = ALPHABET_COLON;
            } else reportError("After a start of block 'alphabet' keyword and colon should be present.");
            break;

        case ALPHABET_COLON:
            if( scriptString_->at(position_) == ':' ) {
                position_++;
                state_ = ALPHABET_CHAR;
            } else reportError("After 'alphabet keyword colon character should be present.");
            break;

        case ALPHABET_CHAR:
            tmpPosition = skipUntilWhiteCharOr(';','\n');
            tmpString = scriptString_->substr(position_, tmpPosition-position_);

            if( tmpString.length() == 1 && isprint(tmpString.at(0)) && tmpString.at(0) != ':' &&
                tmpString.at(0) != '{' && tmpString.at(0) != '}' && tmpString.at(0) != ',' && tmpString.at(0) != '=' ) {
                if( alphabet_.insert(tmpString.at(0)).second == false ) {
                    getErrorPosition();
                    stringstream ss;
                    ss << "Warning: '" << tmpString.at(0) << "' character already exists in an alphabet.";
                    parseErrors_.push_back(ParseError(errorRow_,errorColumn_,ss.str()));
                }
                position_ = tmpPosition;
            } else if ( tmpString.length() == 0 ) {
                if( alphabet_.empty() ) reportError("Alphabet cannot be empty.");
                else {
                    position_++;
                    state_ = AXIOM_KEYWORD;
                }
            } else {
                stringstream ss;
                ss << "String '" << tmpString << "' is not an alphabet character.";
                reportError(ss.str());
            }
            break;

        case AXIOM_KEYWORD:
            tmpPosition = skipUntilWhiteCharOr(':','\n');
            tmpString = scriptString_->substr(position_, tmpPosition-position_);
            if( tmpString.compare("axiom") == 0 ) {
                position_ = tmpPosition;
                state_ = AXIOM_COLON;
            } else reportError("After alphabet definition 'axiom' keyword and colon are expected.");
            break;

        case AXIOM_COLON:
            if( scriptString_->at(position_) == ':' ) {
                position_++;
                state_ = AXIOM_STRING;
            } else reportError("After 'axiom' keyword colon should be present.");
            break;

        case AXIOM_STRING:
            tmpPosition = skipUntilWhiteCharOr(';','\n');
            tmpString = scriptString_->substr(position_, tmpPosition-position_);
            if( tmpString.length() == 0 ) {
                if( lsystem_->getAxiom().empty() ) reportError("LSystem cannot conatin empty start string.");
                else {
                    const string& axiom = lsystem_->getAxiom();
                    string::size_type j = 0;

                    for(; j < axiom.length(); ++j)
                        if(alphabet_.find(axiom.at(j)) == alphabet_.end()) break;
                    if( j == axiom.length() ) {
                        position_++;
                        state_ = RULES_KEYWORD;
                    } else {
                        stringstream ss;
                        ss << "'" << axiom.at(j) << "' character present in start string '" << axiom << "' is not included in an alphabet.";
                        reportError(ss.str());
                    }
                }
            } else {
                if( lsystem_->getAxiom().empty() ) {
                    position_ = tmpPosition;
                    lsystem_->setAxiom(tmpString);
                } else reportError("Only one start string can exist. Space character cannot be part of it.");
            }
            break;

        case RULES_KEYWORD:
            tmpPosition = skipUntilWhiteCharOr(':','\n');
            tmpString = scriptString_->substr(position_, tmpPosition-position_);
            if( tmpString.compare("rules") == 0 ) {
                position_ = tmpPosition;
                state_ = RULES_COLON;
            } else reportError("After start string definition 'rules' keyword and colon are expected.");
            break;

        case RULES_COLON:
            if( scriptString_->at(position_) == ':' ) {
                position_++;
                state_ = RULES_CHAR;
            } else reportError("After 'rules' keyword colon is expected.");
            break;

        case RULES_CHAR:
            ruleSet = false;
            tmpPosition = skipUntilWhiteCharOr('=','\n');
            tmpString = scriptString_->substr(position_, tmpPosition-position_);
            if( tmpString.length() == 1 ) {
                if( lsystem_->getRuleMap().find(tmpString.at(0)) == lsystem_->getRuleMap().end() ) {
                    if( alphabet_.find(tmpString.at(0)) == alphabet_.end() ) {
                        stringstream ss;
                        ss << "Rule character '" << tmpString.at(0) << "' is not included in an alphabet.";
                        reportError(ss.str());
                    } else {
                        ruleChar = tmpString.at(0);
                        position_ = tmpPosition;
                        state_ = RULES_EQUAL;
                    }
                } else {
                    stringstream ss;
                    ss << "Rule '" << tmpString.at(0) << "' has already been defined. Redefinition of rules is not allowed.";
                    reportError(ss.str());
                }
            }
            else {
                if( tmpString.length() == 0 ) reportError("Before '=' character rule sign has to be present.");
                else {
                    stringstream ss;
                    ss << "Following string '" << tmpString << "' is not correct rule character. Single character defined in alphabet was expected.";
                    reportError(ss.str());
                }
            }
            break;

        case RULES_EQUAL:
            if( scriptString_->at(position_) == '=' ) {
                position_++;
                state_ = RULES_STRING;
            } else reportError("After rule character '=' sign is expected.");
            break;

        case RULES_STRING:
            tmpPosition = skipUntilWhiteCharOr(',',';');
            tmpString = scriptString_->substr(position_, tmpPosition-position_);
            if( tmpString.length() > 0 ){
                if( !ruleSet ) {
                    string::size_type j;
                    for( j = 0; j < tmpString.length(); ++j )
                        if(alphabet_.find(tmpString.at(j)) == alphabet_.end()) break;

                    if( j == tmpString.length() ) {
                        lsystem_->addRule(ruleChar,tmpString);
                        position_ = tmpPosition;
                        ruleSet = true;
                    } else {
                        stringstream ss;
                        ss << "Rule string '" << tmpString << "' contains character '" << tmpString.at(j) << "' which is not included in an alphabet.";
                        reportError(ss.str());
                    }
                } else reportError("New rule was expected after ',' character or end of it marked by ';' character.");
            } else {
                if( tmpPosition >= scriptString_->length()) {
                    errors_ = true;
                }
                else {
                    if( ruleSet ) {
                        if( scriptString_->at(tmpPosition) == ';' ) state_ = DEFINE_KEYWORD;
                        else state_ = RULES_CHAR;
                        position_++;
                    } else {
                        reportError("Rule string was not defined.");
                    }
                }
            }
            break;

        case DEFINE_KEYWORD:
            tmpPosition = skipUntilWhiteCharOr(':','\n');
            tmpString = scriptString_->substr(position_, tmpPosition-position_);
            if( tmpString.compare("define") == 0 ) {
                position_ = tmpPosition;
                state_ = DEFINE_COLON;
            } else reportError("After rules definition 'define' keyword and colon are expected.");
            break;

        case DEFINE_COLON:
            if( scriptString_->at(position_) == ':' ) {
                position_++;
                state_ = DEFINE_CHAR;
            } else reportError("After 'define' keyword colon should be present.");
            break;

        case DEFINE_CHAR:
            defineParams.clear();
            defineCmd = NULL;
            tmpPosition = skipUntilWhiteCharOr('=','\n');
            tmpString = scriptString_->substr(position_, tmpPosition-position_);
            if( tmpString.length() == 1) {
                if( lsystem_->getDefinitionMap().find(tmpString.at(0)) == lsystem_->getDefinitionMap().end()) {
                    if( alphabet_.find(tmpString.at(0)) == alphabet_.end() ) {
                        stringstream ss;
                        ss << "Definition character '" << tmpString.at(0) << "' is not included in an alphabet.";
                        reportError(ss.str());
                    } else {
                        defineChar = tmpString.at(0);
                        position_ = tmpPosition;
                        state_ = DEFINE_EQUAL;
                    }
                }
                else {
                    stringstream ss;
                    ss << "Defition '" << tmpString.at(0) << "' is already defined. Redefinition of definitions is not allowed.";
                    reportError(ss.str());
                }
            }
            else {
                if( tmpString.length() == 0 ) reportError("Before '=' definition character should be present.");
                else {
                    stringstream ss;
                    ss << "String '" << tmpString << "' is not correct definition sign. Single character defined in alphabet was expected.";
                    reportError(ss.str());
                }
            }
            break;

        case DEFINE_EQUAL:
            if( scriptString_->at(position_) == '=' ) {
                position_++;
                state_ = DEFINE_COMMAND;
            } else reportError("After definition character '=' has to be present.");
            break;

        case DEFINE_COMMAND:
            tmpPosition = skipUntilWhiteCharOr(',',';');
            tmpString = scriptString_->substr(position_, tmpPosition-position_);
            if( tmpString.length() > 0 ){
                vector<Command>::const_iterator it = commands_.begin();
                for(; it != commands_.end(); ++it) {
                    if( it->name.compare(tmpString) == 0 ) break;
                }
                if( it != commands_.end()) {
                    defineCmd = &*it;
                    position_ = tmpPosition;
                    state_ = DEFINE_PARAM;
                } else {
                    stringstream ss;
                    ss << "Command '" << tmpString << "' was not specified.";
                    reportError(ss.str());
                }
            } else {
                if( defineCmd != NULL ) {
                    state_ = DEFINE_PARAM;
                } else reportError("Command name was expected.");
            }
            break;

        case DEFINE_PARAM:
            tmpPosition = skipUntilWhiteCharOr(';',',');
            tmpString = scriptString_->substr(position_, tmpPosition-position_);
            if( tmpString.length() > 0 ) {
                if( defineParams.size() < defineCmd->args) {
                    bool isFloat = true;
                    float arg;
                    try {
                        arg = boost::lexical_cast<float>(tmpString);
                    } catch( bad_lexical_cast &e) {
                        isFloat = false;
                    }
                    if( isFloat ) {
                        defineParams.push_back(arg);
                        position_ = tmpPosition;
                    }
                    else {
                        stringstream ss;
                        ss << "'" << tmpString << " is not correct floating point number.";
                        reportError(ss.str());
                    }
                }
                else {
                    stringstream ss;
                    ss << "Next parameter in '" << defineCmd->name << "' command was not expected. "
                       << "Finish definition by ';' or define new definition after ','.";
                    reportError(ss.str());
                }
            } else {
                if( tmpPosition >= scriptString_->length()) {
                    errors_ = true;
                }
                else {
                    if( defineCmd->args == defineParams.size()) {
                        if( scriptString_->at(tmpPosition) == ';' ) state_ = LSYSTEM_END;
                        else state_ = DEFINE_CHAR;
                        position_++;

                        Command newCommand = *defineCmd;
                        newCommand.argv = defineParams;
                        lsystem_->addDefinition(defineChar, newCommand);

                    } else {
                        stringstream ss;
                        ss << "Wrong number of parameters was passed for '" << defineCmd->name << "' command. "
                           << "Expected " << defineCmd->args;
                        if( defineCmd->args > 1)
                            ss << " parameters.";
                        else
                            ss << " parameter.";
                        reportError(ss.str());
                    }
                }
            }
            break;

        case LSYSTEM_END:
            if( scriptString_->at(position_) == '}' ) {
                position_++;
                state_ = LSYSTEM_NAME;
                shared_ptr<LSystem> pLSystem(lsystem_);
                lsystems_.push_back(pLSystem);
                lsystemFinished = true;
            } else reportError("After definition of all L-System elements end of block '}' is expected.");
            break;
        }
    }

    if( !lsystemFinished ) {

        if(!errors_) {

            if( position_ >= scriptString_->length() ) {
                position_ = scriptString_->length();
            }

            getErrorPosition();

            switch(state_) {
            case LSYSTEM_NAME:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Expected L-system name."));
                break;
            case LSYSTEM_START:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Expected start of L-System block '{'."));
                break;
            case LSYSTEM_END:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Expected end of L-System block '}'."));
                break;
            case ALPHABET_KEYWORD:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Expected 'alphabet' keyword."));
                break;
            case ALPHABET_COLON:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Expected ':' after 'alphabet' keyword."));
                break;
            case ALPHABET_CHAR:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Expected alphabet character or ';'."));
                break;
            case AXIOM_KEYWORD:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Expected 'axiom' keyword."));
                break;
            case AXIOM_COLON:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Expected ':' after 'axiom' keyword."));
                break;
            case AXIOM_STRING:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Expected start string or ';'."));
                break;
            case RULES_KEYWORD:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Expected 'rules' keyword."));
                break;
            case RULES_COLON:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Expected ':' after 'rules' keyword."));
                break;
            case RULES_CHAR:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Expected rule character."));
                break;
            case RULES_EQUAL:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Expected '=' after rule character."));
                break;
            case RULES_STRING:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Expected rule string, next rule ',', or end of rule definition ';'."));
                break;
            case DEFINE_KEYWORD:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Expected 'define' keyword."));
                break;
            case DEFINE_COLON:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Expected ':' after 'define' keyword."));
                break;
            case DEFINE_CHAR:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Expected definition character."));
                break;
            case DEFINE_EQUAL:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Expected '=' after definition character."));
                break;
            case DEFINE_COMMAND:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Expected command name after '=' character."));
                break;
            case DEFINE_PARAM:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Expected command parameters, next definition after ',', or end of definition ';'."));
                break;
            default:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Unknown error"));
            }

            errors_ = true;
        }
        delete lsystem_;
    }

    return !errors_;
}

vector<shared_ptr<LSystem> > Parser::getLSystems() const { return lsystems_; }

const vector< ParseError >& Parser::getErrors() const { return parseErrors_; }

void Parser::registerCommands() {
    commands_ = LSystemModelInterface::getCommands();
}
