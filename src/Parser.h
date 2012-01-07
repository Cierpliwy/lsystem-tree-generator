#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <set>
#include <boost/shared_ptr.hpp>
#include "LSystem.h"

//Wstępna deklaracja LSystemu
class LSystemModelInterface;

//Opis błędu rzucanego przez Parser.
struct ParseError
{
    ParseError(unsigned int new_row, unsigned int new_column, const std::string& new_description) :
        row(new_row), column(new_column), description(new_description) {}
    unsigned int row, column;
    std::string description;
};

//Skrót do wektora LSystemów.
typedef std::vector<boost::shared_ptr<LSystem> > LSystemVector;

//Klasa odpowidzialna za tworzenie obiektów LSystemów.
class Parser
{
public:
    //Zwraca instację obiektu
    static Parser& getInstance();

    //Parsuje skrypt L-Systemu i zwraca wartość false, jeżeli wystąpiły błędy
    //lub wartość true, jeżeli plik został sparsowany poprawnie.
    bool parseLSystem( const std::string& script_string );

    //Zwraca listę Lsystemów, sparsowanych przez Parser.
    LSystemVector getLSystems() const;

    //Zwraca listę błędów, które wystąpiły podczas parsowania skryptu.
    const std::vector<ParseError>& getErrors() const;

    //Rejestruje definicje komend dla danego modelu LSystemu
    void registerCommands();

protected:

    //Prywatny konstruktor domyślny i kopiujący, ponieważ Parser jest singletonem.
    Parser() {}
    Parser(const Parser&) {}

    //Instancja obiektu parsera
    static Parser *parserInstance_;

    //Lista bledów, które wystąpiły podczas parsowania skryptu.
    std::vector<ParseError> parseErrors_;

    //Lista LSystemów, aktualnie skompilowanych przez Parser.
    std::vector<boost::shared_ptr<LSystem> > lsystems_;

    //Lista komend dostępna dla parsera.
    std::vector<Command> commands_;
  
private:

    //Sprawdza czy dane slowo jest poprawną nazwą LSystemu
    bool isLSystemName(const std::string& name) const;

    //Omija biale znaki w skrypcie. Zwraca fałsz gdy przekroczono ciąg znaków.
    bool ignoreWhiteChars();

    //Zwraca pozycje az do napotkania bialego znaku lub podanego znaku.
    std::string::size_type skipUntilWhiteCharOr(char stop_char, char stop_char2) const;

    //Skacze za koniec bloku LSystemu
    void jumpToEndOfLSystem();

    //Zwraca pozycje bledu
    void getErrorPosition();

    //Zgłasza blad podczas parsowania
    void reportError(const std::string &error_description);
  
    //Typ wyliczeniowy prezentujacy stany parsera.
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

    //Zmienne prywatne wykorzystywane przez funkcję parse();
    LSystem* lsystem_;                  // Aktualnie przerabiany LSystem
    const std::string* scriptString_;   // Aktualnie wczytany skrypt
    std::string::size_type position_;   // Pozycja w skrypce
    State state_;                       // Aktualny stan parsera
    bool errors_;                       // Czy wystapily bledy podczas kompilacji
    int errorColumn_;                   // Kolumna w ktorej wystapil ostatni blad
    int errorRow_;                      // Wiersz w ktorym wystapil ostatni blad
    std::set<char> alphabet_;           // Aktualny alfabet LSystemu
};

#endif //PARSER_H
