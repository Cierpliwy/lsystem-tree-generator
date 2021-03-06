#ifndef ERRORLISTMODEL_H
#define ERRORLISTMODEL_H
#include <QAbstractListModel>
#include <QStringListModel>
#include <vector>
#include "Parser.h"

/**
 * @brief Class which defines model which stores list of
 *        errors generated by L-System script parser.
 */
class ErrorListModel : public QAbstractListModel
{
    Q_OBJECT

public:

    explicit ErrorListModel(QObject *parent = 0);

    int rowCount(const QModelIndex &) const;
    int columnCount(const QModelIndex &) const;

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void setParseErrors(const std::vector<ParseError>& parse_errors);
    void removeParseErrors();

    const ParseError& getParseError(QModelIndex index);

protected:

    std::vector<ParseError> parseErrors_;
};

#endif // ERRORLISTMODEL_H
