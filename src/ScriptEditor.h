#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <cstdio>
#include "Parser.h"

class ScriptEditorMargin;

/**
 * @brief The ScriptEditor class in a simple L-System script
 *        text editor which have line numbers feature and auto
 *        indentation.
 */
class ScriptEditor : public QPlainTextEdit {

    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = 0);
    
    void marginPaintEvent(QPaintEvent *event);
    int marginWidth();

protected:

    void resizeEvent(QResizeEvent *e);
    void keyPressEvent(QKeyEvent * event);

public slots:

    void highlightBlocks(const std::vector<ParseError> &parse_errors);
    void cleanAllHighlights();
    void moveCursorTo(int row, int column);

private slots:

    void updateMarginWidth(int);
    void updateMargin(const QRect& rect, int dy);
    void updateIndentationInfo();

private:

    ScriptEditorMargin *margin_;

    int tabLevel_, lastTabLevel_;
    bool afterLeftBrace_, lastAfterLeftBrace_;
    bool afterSpaceOnly_, lastAfterSpaceOnly_;
};

/**
 * @brief The ScriptEditorMargin class is a widget responsible
 *        for drawing current line number on left margin.
 */
class ScriptEditorMargin : public QWidget {
public:
    ScriptEditorMargin(ScriptEditor *editor) : QWidget(editor) {
        this->editor_ = editor;
    }

    QSize sizeHint() const {
        return QSize(editor_->marginWidth(),0);
    }

protected:

    void paintEvent(QPaintEvent *event) {
        editor_->marginPaintEvent(event);
    }

private:
    ScriptEditor *editor_;
};

/**
 * @brief The LSystemScriptHighlighter class highlights
 *        L-System script syntax.
 */
class LSystemScriptHighlighter : public QSyntaxHighlighter {

    Q_OBJECT

public:
    LSystemScriptHighlighter(QTextDocument *document);

protected:
    void highlightBlock(const QString &text);

private:

    bool isIgnoredChar(QChar c);

    QTextCharFormat keywordFormat_;
    QTextCharFormat stringFormat_;
    QTextCharFormat charFormat_;
    QTextCharFormat numberFormat_;

    QRegExp numberRegExp_;
};

#endif // SCRIPTEDITOR_H
