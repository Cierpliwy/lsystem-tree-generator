#ifndef ERRORLISTMODEL_H
#define ERRORLISTMODEL_H
#include <QAbstractListModel>
#include <QStringListModel>
#include <vector>
#include "Parser.h"

//Klasa definiująca model danych, który będzie wyświetlać tabela
//z błędami.
class ErrorListModel : public QAbstractListModel
{
    Q_OBJECT

public:

    //Konstruktor tabeli
    explicit ErrorListModel(QObject *parent = 0);

    //Funkcja zwracająca liczbę wierszy
    int rowCount(const QModelIndex &parent) const;

    //Zwraca liczbę kolumn w modelu
    int columnCount(const QModelIndex &parent) const;

    //Funkcja odczytująca dane potrzebne do wypełnienia tabeli.
    QVariant data(const QModelIndex &index, int role) const;

    //Zwraca dane do wyświetlenia w nagłówku.
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    //Funkcja, która umożliwia ustawienie aktualnej tablicy błędów.
    void setParseErrors(const std::vector<ParseError>& parse_errors);
    //Usuwamy wszystkie błędu z listy.
    void removeParseErrors();

    //Zwraca aktualny błąd parsera na podstawie jego indeksu wewnątrz modelu.
    const ParseError& getParseError(QModelIndex index);

protected:

    //Obiekt przetrzymuje listę błędów.
    std::vector<ParseError> parseErrors_;
};

#endif // ERRORLISTMODEL_H
