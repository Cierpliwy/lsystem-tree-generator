#include "Parser.h"
#include "LSystem.h"

#include <iostream>
#include <sstream>

using namespace std;
using namespace boost;

//Wskaźnik na instancję klasy Parser.
Parser* Parser::parserInstance_ = NULL;

//Zwraca instancję klasy Parser.
Parser& Parser::getInstance() {
  if( parserInstance_ == NULL ) parserInstance_ = new Parser();
  return *parserInstance_;
}

//Sprawdza czy string jest alfanumeryczny
bool Parser::isLSystemName(const string& name) const {
  for( string::size_type i = 0; i < name.length(); ++i )
    if( !isalnum(name[i]) && name[i] != '_' ) return false;
  return true;
}

//Ignoruje biale znaki
void Parser::ignoreWhiteChars() {
  for(; position_ < scriptString_->length(); ++position_)
    if(!isspace(scriptString_->at(position_))) break;
}

//Pomija znaki dopoki nie trafi na bialy znak lub znak podany.
std::size_t Parser::skipUntilWhiteCharOr(char stop_char, char stop_char2) const {
  string::size_type i = position_;
  for(;i < scriptString_->length(); ++i)
    if(isspace(scriptString_->at(i)) || scriptString_->at(i) == stop_char || scriptString_->at(i) == stop_char2) break;
  return i;
}

//Skacze za koniec aktualnego LSystemu
void Parser::jumpToEndOfLSystem() {
  for(;position_ < scriptString_->length(); ++position_)
    if( scriptString_->at(position_) == '}') break;
  position_++;
}

//Zwraca pozycję błędu.
void Parser::getErrorPosition() {
  errorRow_ = 1;
  errorColumn_ = 1;
  for(string::size_type i = 0; i < position_; ++i) {
    errorRow_++;
    if( scriptString_->at(i) == '\n' ) { errorColumn_++; errorRow_ = 1; }
  }
}

//Raportuje blad
void Parser::reportError(const std::string& error_description) {
  getErrorPosition();
  parseErrors_.push_back(ParseError(errorRow_,errorColumn_,error_description));
  jumpToEndOfLSystem();
  state_ = LSYSTEM_NAME;
  errors_ = true;
}

//Parsuje skrypt L-Systemu i zwraca wartość false, jeżeli wystąpiły błędy
//lub wartość true, jeżeli plik został sparsowany poprawnie.
bool Parser::parseLSystem ( const std::string& script_string ) {

  //Tworzymy LSystem gotowy do uzupelniania.
  lsystem_ = NULL;
  //Zapisujemy wskaźnik do aktualnego pliku
  scriptString_ = &script_string;
  //Ustawiamy pozycję w pliku na 0
  position_ = 0;
  //Stan rozpoczynamy od podania nazwy LSystemu
  state_ = LSYSTEM_NAME;
  //Czyscimy flage bledow
  errors_ = false;
  //Tymczasowa pozycja do wyznaczenia
  string::size_type tmpPosition;
  //Tymczasowy string do przeanalizowania
  string tmpString;
  //Aktualnie wybrany znak zasady
  char ruleChar;
  //Czy zasada zostala ustawiona
  bool ruleSet;
  //Czy zakonczono parsowanie LSystemu
  bool lsystemFinished = false;

  //Dopóki pozycja nie przekroczy pliku parsujemy LSystemy
  while( position_ < scriptString_->length() ) {
    //Ignorujemy jakiekolwiek biale znaki
    ignoreWhiteChars();

    switch(state_){
      case LSYSTEM_NAME:
        //Tutaj czyscimy. Mozemy usuwac bezpiecznie NULL'a.
        if( !lsystemFinished ) delete lsystem_;
        lsystem_ = new LSystem;
        lsystemFinished = false;

        //Sprawdzamy nazwe LSystemu
        tmpPosition = skipUntilWhiteCharOr('{','\n');
        tmpString = scriptString_->substr(position_, tmpPosition-position_);
        if( isLSystemName(tmpString) ) {
          position_ = tmpPosition;
          state_ = LSYSTEM_START;
          lsystem_->setName(tmpString);
        } else {
          stringstream ss;
          ss << "Nazwa '" << tmpString << "' LSystemu musi byc alfanumeryczna i opcjonalnie zawierac znak '_'.";
          reportError(ss.str());
        }
        break;

      case LSYSTEM_START:
        //Sprawdzamy czy jest znak '{'
        if( scriptString_->at(position_) == '{' ) {
          position_++;
          state_ = ALPHABET_KEYWORD;
        } else reportError("Spodziewano sie rozpoczecia bloku LSystemu '{'.");
        break;

      case ALPHABET_KEYWORD:
        //Sprawdzamy czy aktualnie jest wprowadzany alfabet
        tmpPosition = skipUntilWhiteCharOr(':','\n');
        tmpString = scriptString_->substr(position_, tmpPosition-position_);
        if( tmpString.compare("alphabet") == 0 ) {
          position_ = tmpPosition;
          state_ = ALPHABET_COLON;
        } else reportError("Spodziewano się słowa kluczowego 'alphabet' i dwukropka.");
        break;

      case ALPHABET_COLON:
        //Upewniamy sie ze jest dwukropek
        if( scriptString_->at(position_) == ':' ) {
          position_++;
          state_ = ALPHABET_CHAR;
        } else reportError("Po slowie kluczowym 'alphabet' musi wystapic dwukropek.");
        break;

      case ALPHABET_CHAR:
        //Pobieramy znak do alfabetu
        tmpPosition = skipUntilWhiteCharOr(';','\n');
        tmpString = scriptString_->substr(position_, tmpPosition-position_);
        if( tmpString.length() == 1 && isprint(tmpString.at(0)) && tmpString.at(0) != ':' &&
            tmpString.at(0) != '{' && tmpString.at(0) != '}' && tmpString.at(0) != ',' ) {
          if( alphabet_.insert(tmpString.at(0)).second == false ) {
            getErrorPosition();
            stringstream ss;
            ss << "Ostrzezenie: znak '" << tmpString.at(0) << "' wystepuje juz w alfabecie.";
            parseErrors_.push_back(ParseError(errorRow_,errorColumn_,ss.str()));
            errors_ = true;
          }
          position_ = tmpPosition;
        } else if ( tmpString.length() == 0 ) {
          if( alphabet_.empty() ) reportError("Alfabet nie moze byc pusty");
          else {
            position_++;
            state_ = AXIOM_KEYWORD;
          }
        } else {
          stringstream ss;
          ss << "Ciag '" << tmpString << "' nie jest znakiem alfabetu.";
          reportError(ss.str());
        }
        break;

      case AXIOM_KEYWORD:
        //Sprawdzamy czy aktualnie jest wprowadzany axiom
        tmpPosition = skipUntilWhiteCharOr(':','\n');
        tmpString = scriptString_->substr(position_, tmpPosition-position_);
        if( tmpString.compare("axiom") == 0 ) {
          position_ = tmpPosition;
          state_ = AXIOM_COLON;
        } else reportError("Spodziewano się słowa kluczowego 'axiom' i dwukropka.");
        break;

      case AXIOM_COLON:
        //Upewniamy sie ze jest dwukropek
        if( scriptString_->at(position_) == ':' ) {
          position_++;
          state_ = AXIOM_STRING;
        } else reportError("Po slowie kluczowym 'axiom' musi wystapic dwukropek.");
        break;

      case AXIOM_STRING:
        //Pobieramy ciag poczatkowy LSystemu
        tmpPosition = skipUntilWhiteCharOr(';','\n');
        tmpString = scriptString_->substr(position_, tmpPosition-position_);
        if( tmpString.length() == 0 ) {
          if( lsystem_->getAxiom().empty() ) reportError("LSystem nie moze zawierac pustego ciagu poczatkowego.");
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
              ss << "Znak '" << axiom.at(j) << "' wystepujacy w ciagu poczatkowym '" << axiom << "' nie nalezy do alfabetu.";
              reportError(ss.str());
            }
          }
        } else {
          if( lsystem_->getAxiom().empty() ) {
            position_ = tmpPosition;
            lsystem_->setAxiom(tmpString);
          } else reportError("Istniec moze tylko jeden ciag poczatkowy. Spacja nie moze wliczac sie do ciagu poczatkowego.");
        }
        break;

      case RULES_KEYWORD:
        //Sprawdzamy czy sa aktualnie wprowadzane zasady
        tmpPosition = skipUntilWhiteCharOr(':','\n');
        tmpString = scriptString_->substr(position_, tmpPosition-position_);
        if( tmpString.compare("rules") == 0 ) {
          position_ = tmpPosition;
          state_ = RULES_COLON;
        } else reportError("Spodziewano się słowa kluczowego 'rules' i dwukropka.");
        break;

      case RULES_COLON:
        //Upewniamy sie ze jest dwukropek
        if( scriptString_->at(position_) == ':' ) {
          position_++;
          state_ = RULES_CHAR;
        } else reportError("Po slowie kluczowym 'rules' musi wystapic dwukropek.");
        break;

      case RULES_CHAR:
        ruleSet = false;
        tmpPosition = skipUntilWhiteCharOr('=','\n');
        tmpString = scriptString_->substr(position_, tmpPosition-position_);
        if( tmpString.length() == 1 && tmpString.at(0) != ':' && tmpString.at(0) != ';' &&
            tmpString.at(0) != '{' && tmpString.at(0) != '}' && tmpString.at(0) != ',' ) {
          if( lsystem_->getRuleMap().find(tmpString.at(0)) == lsystem_->getRuleMap().end() ) {
            //Nie znaleziono takiego znaku w zasadach wiec dodajemy
            ruleChar = tmpString.at(0);
            position_ = tmpPosition;
            state_ = RULES_EQUAL;
          } else {
            stringstream ss;
            ss << "Zasada '" << tmpString.at(0) << "' zostala juz zdefiniowana. Redefinicja zasad jest zabroniona.";
            reportError(ss.str());
          }
        }
        else {
          //Nie podano poprawnego znaku
          if( tmpString.length() == 0 ) reportError("Spodziewano sie podania znaku zasady przed znakiem rownosci.");
          else {
            stringstream ss;
            ss << "Ciag '" << tmpString << "' jest niepoprawnym znakiem zasady. Oczekiwano pojedynczego znaku zdefiniowanego alfabetu.";
            reportError(ss.str());
          }
        }
        break;

      case RULES_EQUAL:
        //Upewniamy sie ze jest rownosc
        if( scriptString_->at(position_) == '=' ) {
          position_++;
          state_ = RULES_STRING;
        } else reportError("Po znaku zasady musi wystapic znak rownosci.");
        break;

      case RULES_STRING:
        //Pobieramy ciag ktory przyporzadkujemy znakowi zasady.
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
              ss << "W ciagu zasady '" << tmpString << "' wystepuje znak '" << tmpString.at(j) << "' ktory nie nalezy do alfabetu.";
              reportError(ss.str());
            }
          } else reportError("Zasada zostala juz zdefiniowana. Spodziewano sie podania nowej zasady ',' badz zakonczenia ';'.");
        } else {
          //Musimy sprawdzic na jakim znaku zatrzymal sie parser
          if( scriptString_->at(tmpPosition) == ';' ) state_ = LSYSTEM_END;
          else state_ = RULES_CHAR;
          position_++;
        }
        break;

      case LSYSTEM_END:
        //Sprawdzamy czy jest znak '}'
        if( scriptString_->at(position_) == '}' ) {
          position_++;
          state_ = LSYSTEM_NAME;
          shared_ptr<LSystem> pLSystem(lsystem_);
          lsystems_.push_back(pLSystem);
        } else reportError("Spodziewano sie zakonczenia bloku LSystemu '}'.");
        break;
    }
  }

  //Usuwamy tymczasowy LSystem
  if( !lsystemFinished ) {
    delete lsystem_;
    getErrorPosition();
    parseErrors_.push_back(ParseError(errorRow_,errorColumn_,"Nie zakonczono definicji LSystemu."));
    errors_ = true;
  }

  //Zwracamy czy byly jakies bledy
  return errors_;
}

vector<shared_ptr<LSystem> > Parser::getLSystems() const { return lsystems_; }

const vector< ParseError >& Parser::getErrors() const { return parseErrors_; }


