#include "Parser.h"
#include "LSystemModelInterface.h"

#include <iostream>
#include <sstream>
#include <boost/lexical_cast.hpp>

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

//Uwaga: position_ moze zostać ustawione poza zakresem
//ciągu znaków. W takim wypadku funkcja zwraca false.
bool Parser::ignoreWhiteChars() {
    for(; position_ < scriptString_->length(); ++position_)
        if(!isspace(scriptString_->at(position_))) break;

    if( position_ >= scriptString_->length()) return false;
    return true;
}

//Uwaga: Jeżeli na niczym się nie zatrzyma ustawi się na
//pozycji za ostatnim znakiem!
std::size_t Parser::skipUntilWhiteCharOr(char stop_char, char stop_char2) const {
    string::size_type i = position_;
    for(;i < scriptString_->length(); ++i)
        if(isspace(scriptString_->at(i)) || scriptString_->at(i) == stop_char || scriptString_->at(i) == stop_char2) break;
    return i;
}

//Uwaga: pozycja znaku może się znaleźć za końcem aktualnego
//ciągu.
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

    //Dopóki pozycja nie przekroczy pliku parsujemy LSystemy
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

                //Sprawdzamy czy nazwa nie wystapila w zadnym z L-systemów.
                vector<boost::shared_ptr<LSystem> >::const_iterator it = lsystems_.begin();
                for(; it!=lsystems_.end(); ++it)
                    if( (*it)->getName().compare(tmpString) == 0 ) break;

                if( it == lsystems_.end()) {
                    position_ = tmpPosition;
                    state_ = LSYSTEM_START;
                    lsystem_->setName(tmpString);
                } else {
                    stringstream ss;
                    ss << "W skrypcie występuje już L-system o nazwie: '" << tmpString << "'.";
                    reportError(ss.str());
                }
            } else {
                stringstream ss;
                ss << "Nazwa '" << tmpString << "' LSystemu musi być alfanumeryczna i opcjonalnie zawierać znak '_'.";
                reportError(ss.str());
            }
            break;

        case LSYSTEM_START:
            if( scriptString_->at(position_) == '{' ) {
                position_++;
                state_ = ALPHABET_KEYWORD;
            } else reportError("Po nazwie L-systemu musi się rozpocząć jego blok: '{'.");
            break;

        case ALPHABET_KEYWORD:
            tmpPosition = skipUntilWhiteCharOr(':','\n');
            tmpString = scriptString_->substr(position_, tmpPosition-position_);
            if( tmpString.compare("alphabet") == 0 ) {
                position_ = tmpPosition;
                state_ = ALPHABET_COLON;
            } else reportError("Po otwarciu bloku musi wystąpić słowo kluczowe 'alphabet' i następujący po nim dwukropek.");
            break;

        case ALPHABET_COLON:
            if( scriptString_->at(position_) == ':' ) {
                position_++;
                state_ = ALPHABET_CHAR;
            } else reportError("Po słowie kluczowym 'alphabet' musi wystąpić dwukropek.");
            break;

        case ALPHABET_CHAR:
            tmpPosition = skipUntilWhiteCharOr(';','\n');
            tmpString = scriptString_->substr(position_, tmpPosition-position_);

            if( tmpString.length() == 1 && isprint(tmpString.at(0)) && tmpString.at(0) != ':' &&
                tmpString.at(0) != '{' && tmpString.at(0) != '}' && tmpString.at(0) != ',' && tmpString.at(0) != '=' ) {
                if( alphabet_.insert(tmpString.at(0)).second == false ) {
                    getErrorPosition();
                    stringstream ss;
                    ss << "Ostrzeżenie: znak '" << tmpString.at(0) << "' występuje już w alfabecie.";
                    parseErrors_.push_back(ParseError(errorRow_,errorColumn_,ss.str()));
                }
                position_ = tmpPosition;
            } else if ( tmpString.length() == 0 ) {
                if( alphabet_.empty() ) reportError("Alfabet nie może być pusty");
                else {
                    position_++;
                    state_ = AXIOM_KEYWORD;
                }
            } else {
                stringstream ss;
                ss << "Ciąg '" << tmpString << "' nie jest znakiem alfabetu.";
                reportError(ss.str());
            }
            break;

        case AXIOM_KEYWORD:
            tmpPosition = skipUntilWhiteCharOr(':','\n');
            tmpString = scriptString_->substr(position_, tmpPosition-position_);
            if( tmpString.compare("axiom") == 0 ) {
                position_ = tmpPosition;
                state_ = AXIOM_COLON;
            } else reportError("Po zdefiniowaniu alfabetu musi wystąpić słowo kluczowe 'axiom' i następujący po nim dwukropek.");
            break;

        case AXIOM_COLON:
            if( scriptString_->at(position_) == ':' ) {
                position_++;
                state_ = AXIOM_STRING;
            } else reportError("Po słowie kluczowym 'axiom' musi wystąpić dwukropek.");
            break;

        case AXIOM_STRING:
            tmpPosition = skipUntilWhiteCharOr(';','\n');
            tmpString = scriptString_->substr(position_, tmpPosition-position_);
            if( tmpString.length() == 0 ) {
                if( lsystem_->getAxiom().empty() ) reportError("LSystem nie może zawierać pustego ciągu początkowego.");
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
                        ss << "Znak '" << axiom.at(j) << "' występujący w ciągu początkowym '" << axiom << "' nie należy do alfabetu.";
                        reportError(ss.str());
                    }
                }
            } else {
                if( lsystem_->getAxiom().empty() ) {
                    position_ = tmpPosition;
                    lsystem_->setAxiom(tmpString);
                } else reportError("Istnieć może tylko jeden ciąg początkowy. Spacja nie może wliczać się do ciągu początkowego.");
            }
            break;

        case RULES_KEYWORD:
            tmpPosition = skipUntilWhiteCharOr(':','\n');
            tmpString = scriptString_->substr(position_, tmpPosition-position_);
            if( tmpString.compare("rules") == 0 ) {
                position_ = tmpPosition;
                state_ = RULES_COLON;
            } else reportError("Po zdefinowaniu ciągu początkowego musi wystąpić słowo kluczowe 'rules' i następujący po nim dwukropek.");
            break;

        case RULES_COLON:
            if( scriptString_->at(position_) == ':' ) {
                position_++;
                state_ = RULES_CHAR;
            } else reportError("Po słowie kluczowym 'rules' musi wystąpić dwukropek.");
            break;

        case RULES_CHAR:
            ruleSet = false;
            tmpPosition = skipUntilWhiteCharOr('=','\n');
            tmpString = scriptString_->substr(position_, tmpPosition-position_);
            if( tmpString.length() == 1 ) {
                if( lsystem_->getRuleMap().find(tmpString.at(0)) == lsystem_->getRuleMap().end() ) {
                    //Sprawdzamy czy znak należy do alfabetu.
                    if( alphabet_.find(tmpString.at(0)) == alphabet_.end() ) {
                        stringstream ss;
                        ss << "Znak reguły '" << tmpString.at(0) << "' nie występuje w alfabecie.";
                        reportError(ss.str());
                    } else {
                        //Nie znaleziono takiego znaku w regułach wiec dodajemy
                        ruleChar = tmpString.at(0);
                        position_ = tmpPosition;
                        state_ = RULES_EQUAL;
                    }
                } else {
                    stringstream ss;
                    ss << "Reguła '" << tmpString.at(0) << "' została już zdefiniowana. Redefinicja reguł jest zabroniona.";
                    reportError(ss.str());
                }
            }
            else {
                if( tmpString.length() == 0 ) reportError("Przed znakiem równości musi wystąpić znak reguły.");
                else {
                    stringstream ss;
                    ss << "Ciąg '" << tmpString << "' jest niepoprawnym znakiem reguły. Oczekiwano pojedyńczego znaku zdefiniowanego alfabetu.";
                    reportError(ss.str());
                }
            }
            break;

        case RULES_EQUAL:
            if( scriptString_->at(position_) == '=' ) {
                position_++;
                state_ = RULES_STRING;
            } else reportError("Po znaku reguły musi wystąpić znak równości.");
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
                        ss << "W ciągu reguły '" << tmpString << "' występuje znak '" << tmpString.at(j) << "' który nie należy do alfabetu.";
                        reportError(ss.str());
                    }
                } else reportError("Spodziewano się podania nowej reguły po znaku ',' bądź zakończenia ';'.");
            } else {
                //Musimy sprawdzic na jakim znaku zatrzymal sie parser. Mógł zatrzymać się na za plikiem
                //wtedy sprawdzenia może zawiesić program.
                if( tmpPosition >= scriptString_->length()) {
                    errors_ = true;
                }
                else {
                    if( ruleSet ) {
                        if( scriptString_->at(tmpPosition) == ';' ) state_ = DEFINE_KEYWORD;
                        else state_ = RULES_CHAR;
                        position_++;
                    } else {
                        reportError("Ciąg reguły nie został ustawiony");
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
            } else reportError("Po zdefinowaniu zasad musi wystąpić słowo kluczowe 'define' i następujący po nim dwukropek.");
            break;

        case DEFINE_COLON:
            if( scriptString_->at(position_) == ':' ) {
                position_++;
                state_ = DEFINE_CHAR;
            } else reportError("Po słowie kluczowym 'define' musi wystąpić dwukropek.");
            break;

        case DEFINE_CHAR:
            defineParams.clear();
            defineCmd = NULL;
            tmpPosition = skipUntilWhiteCharOr('=','\n');
            tmpString = scriptString_->substr(position_, tmpPosition-position_);
            if( tmpString.length() == 1) {
                if( lsystem_->getDefinitionMap().find(tmpString.at(0)) == lsystem_->getDefinitionMap().end()) {
                    //Sprawdzamy czy znak należy do alfabetu.
                    if( alphabet_.find(tmpString.at(0)) == alphabet_.end() ) {
                        stringstream ss;
                        ss << "Znak definicji '" << tmpString.at(0) << "' nie występuje w alfabecie.";
                        reportError(ss.str());
                    } else {
                        //Nie znaleziono takiego znaku w definicjach wiec dodajemy
                        defineChar = tmpString.at(0);
                        position_ = tmpPosition;
                        state_ = DEFINE_EQUAL;
                    }
                }
                else {
                    stringstream ss;
                    ss << "Definicja '" << tmpString.at(0) << "' została już zdefiniowana. Redefinicja definicji jest zabroniona.";
                    reportError(ss.str());
                }
            }
            else {
                if( tmpString.length() == 0 ) reportError("Przed znakiem równości musi wystąpić znak definicji.");
                else {
                    stringstream ss;
                    ss << "Ciąg '" << tmpString << "' jest niepoprawnym znakiem definicji. Oczekiwano pojedyńczego znaku zdefiniowanego alfabetu.";
                    reportError(ss.str());
                }
            }
            break;

        case DEFINE_EQUAL:
            if( scriptString_->at(position_) == '=' ) {
                position_++;
                state_ = DEFINE_COMMAND;
            } else reportError("Po znaku definicji musi wystąpić znak równości.");
            break;

        case DEFINE_COMMAND:
            tmpPosition = skipUntilWhiteCharOr(',',';');
            tmpString = scriptString_->substr(position_, tmpPosition-position_);
            if( tmpString.length() > 0 ){
                //Sprawdzamy czy komenda istnieje.
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
                    ss << "Komenda '" << tmpString << "' nie istnieje na liście dozwolonych komend.";
                    reportError(ss.str());
                }
            } else {
                //Sprawdzamy czy możemy zakończyć od razu komendę (dla komend bezparametrowych).
                if( defineCmd != NULL ) {
                    state_ = DEFINE_PARAM;
                } else reportError("Spodziewano się nazwy komendy.");
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
                        ss << "'" << tmpString << " nie jest poprawną liczbą zmiennoprzecinkową.";
                        reportError(ss.str());
                    }
                }
                else {
                    stringstream ss;
                    ss << "Nie spodziewano się kolejnego argumentu w komendzie '" << defineCmd->name << "'. "
                       << "Zakończ definicję ';' bądź podaj nową definicję po ','.";
                    reportError(ss.str());
                }
            } else {
                //Musimy sprawdzic na jakim znaku zatrzymal sie parser. Mógł zatrzymać się na za plikiem
                //wtedy sprawdzenia może zawiesić program.
                if( tmpPosition >= scriptString_->length()) {
                    errors_ = true;
                }
                else {
                    //Sprawdzamy czy definicja została utworzona.
                    if( defineCmd->args == defineParams.size()) {
                        if( scriptString_->at(tmpPosition) == ';' ) state_ = LSYSTEM_END;
                        else state_ = DEFINE_CHAR;
                        position_++;

                        Command newCommand = *defineCmd;
                        newCommand.argv = defineParams;
                        lsystem_->addDefinition(defineChar, newCommand);

                    } else {
                        stringstream ss;
                        ss << "Podano złą liczbę argumentów dla komendy '" << defineCmd->name << "'. "
                           << "Spodziewano się " << defineCmd->args;
                        if( defineCmd->args > 1)
                            ss << " komend.";
                        else
                            ss << " komendy.";
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
            } else reportError("Po zdefiniowaniu wszystkich elementów L-systemu musi wystąpić zakończenie bloku: '}'.");
            break;
        }
    }

    //Usuwamy tymczasowy LSystem.
    if( !lsystemFinished ) {

        if(!errors_) {

            if( position_ >= scriptString_->length() ) {
                position_ = scriptString_->length();
            }

            getErrorPosition();

            switch(state_) {
            case LSYSTEM_NAME:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Spodziewano się nazwy L-systemu."));
                break;
            case LSYSTEM_START:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Spodziewano się rozpoczęcia bloku L-Systemu '{'."));
                break;
            case LSYSTEM_END:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Spodziewano się zakończenia bloku L-Systemu '}'."));
                break;
            case ALPHABET_KEYWORD:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Spodziewano się słowa kluczowego 'alphabet'."));
                break;
            case ALPHABET_COLON:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Spodziewano się ':' po słowie kluczowym 'alphabet'."));
                break;
            case ALPHABET_CHAR:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Spodziewano się znaku alfabetu lub ';'."));
                break;
            case AXIOM_KEYWORD:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Spodziewano się słowa kluczowego 'axiom'."));
                break;
            case AXIOM_COLON:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Spodziewano się ':' po słowie kluczowym 'axiom'."));
                break;
            case AXIOM_STRING:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Spodziewano się ciągu początkowego lub ';'."));
                break;
            case RULES_KEYWORD:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Spodziewano się słowa kluczowego 'rules'."));
                break;
            case RULES_COLON:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Spodziewano się ':' po słowie kluczowym 'rules'."));
                break;
            case RULES_CHAR:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Spodziewano się znaku reguły."));
                break;
            case RULES_EQUAL:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Spodziewano się równości '=' po znaku reguły."));
                break;
            case RULES_STRING:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Spodziewano się ciągu reguły, następnej reguły ',', bądź zakończenia definicji reguł ';'."));
                break;
            case DEFINE_KEYWORD:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Spodziewano się słowa kluczowego 'define'."));
                break;
            case DEFINE_COLON:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Spodziewano się ':' po słowie kluczowym 'define'."));
                break;
            case DEFINE_CHAR:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Spodziewano się znaku definicji."));
                break;
            case DEFINE_EQUAL:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Spodziewano się równości '=' po znaku definicji."));
                break;
            case DEFINE_COMMAND:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Spodziewano się nazwy komendy po znaku równości."));
                break;
            case DEFINE_PARAM:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Spodziewano się parametru komendy, następnej definicji ',', bądź zakończenia definicji ';'."));
                break;
            default:
                parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Nieznany błąd"));
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
