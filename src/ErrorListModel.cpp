#include "ErrorListModel.h"
#include <sstream>

//Domyślny konstruktor
ErrorListModel::ErrorListModel(QObject *parent) :
    QAbstractListModel(parent) {

}

//Zwracamy liczbę wierszy, czyli liczbę błędów aktualnie przechowywanych
//w modelu.
int ErrorListModel::rowCount(const QModelIndex &parent) const {
    return parseErrors_.size();
}

//Zwracamy liczbę kolumn modelu.
int ErrorListModel::columnCount(const QModelIndex &parent) const {
    return 2;
}

//Zwracamy kolumny.
QVariant ErrorListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    //Interesują nas tylko kolumny horyzontalne
    if( orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        //Pierwsza kolumna to pozycja, druga to opis błędu.
        if( section == 0 )
            return QString::fromUtf8("Nr Linii");
        else
            return QString::fromUtf8("Opis błędu");
    }
    return QVariant();
}

//Zwracamy błędy.
QVariant ErrorListModel::data(const QModelIndex &index, int role) const {
    //Sprawdzamy czy index jest poprawny.
    if(!index.isValid())
        return QVariant();

    //Póki co zwracamy losowe dane.
    if( role == Qt::DisplayRole ) {
        if( index.column() == 0 ) {
            std::stringstream ss;
            ss << parseErrors_.at(index.row()).row << ":" << parseErrors_.at(index.row()).column;
            return ss.str().c_str();
        }
        else
            return QString::fromUtf8(parseErrors_.at(index.row()).description.c_str());
    }
    return QVariant();
}

//Ustawiamy aktualną listę błędów.
void ErrorListModel::setParseErrors(const std::vector<ParseError> &parse_errors) {

    //Usuwamy wszystkie linie.
    if( parseErrors_.size() > 0) {
        beginRemoveRows(QModelIndex(),0,parseErrors_.size()-1);
        endRemoveRows();
    }

    parseErrors_ = parse_errors;

    //Aktualizujemy listę by odświeżyła widok.
    if( parseErrors_.size() > 0) {
        beginInsertRows(QModelIndex(),0,parseErrors_.size()-1);
        endInsertRows();
    }
}

void ErrorListModel::removeParseErrors() {

    //Usuwamy wszystkie linie.
    if( parseErrors_.size() > 0) {
        beginRemoveRows(QModelIndex(),0,parseErrors_.size()-1);
        endRemoveRows();
    }

    parseErrors_ = std::vector<ParseError>();
}

//Zwracamy błąd parsera pod danym indeksem.
const ParseError& ErrorListModel::getParseError(QModelIndex index) {
    return parseErrors_.at(index.row());
}
