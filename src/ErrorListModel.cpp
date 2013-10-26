#include "ErrorListModel.h"
#include <sstream>

ErrorListModel::ErrorListModel(QObject *parent) :
    QAbstractListModel(parent) {

}

int ErrorListModel::rowCount(const QModelIndex &) const {
    return parseErrors_.size();
}

int ErrorListModel::columnCount(const QModelIndex &) const {
    return 2;
}

QVariant ErrorListModel::headerData(int section, Qt::Orientation orientation, int role) const {

    if( orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if( section == 0 )
            return trUtf8("Line number");
        else
            return trUtf8("Error description");
    }
    return QVariant();
}

QVariant ErrorListModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid())
        return QVariant();

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

void ErrorListModel::setParseErrors(const std::vector<ParseError> &parse_errors) {

    if( parseErrors_.size() > 0) {
        beginRemoveRows(QModelIndex(),0,parseErrors_.size()-1);
        endRemoveRows();
    }

    parseErrors_ = parse_errors;

    if( parseErrors_.size() > 0) {
        beginInsertRows(QModelIndex(),0,parseErrors_.size()-1);
        endInsertRows();
    }
}

void ErrorListModel::removeParseErrors() {

    if( parseErrors_.size() > 0) {
        beginRemoveRows(QModelIndex(),0,parseErrors_.size()-1);
        endRemoveRows();
    }

    parseErrors_ = std::vector<ParseError>();
}

const ParseError& ErrorListModel::getParseError(QModelIndex index) {
    return parseErrors_.at(index.row());
}
