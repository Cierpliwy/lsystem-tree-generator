#include "ScriptEditor.h"
#include <QPainter>
#include <QTextBlock>

using namespace std;

ScriptEditor::ScriptEditor(QWidget *parent) :
    QPlainTextEdit(parent) {

    QFont editorFont("Consolas",10);
    editorFont.setStyleHint(QFont::TypeWriter);
    setFont(editorFont);
    setTabStopWidth(20);

    tabLevel_ = 0;
    lastTabLevel_= 0;
    afterLeftBrace_ = false;
    lastAfterLeftBrace_ = false;
    afterSpaceOnly_ = false;
    lastAfterSpaceOnly_ = false;

    margin_ = new ScriptEditorMargin(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateMarginWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateMargin(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updateIndentationInfo()));

    setWordWrapMode(QTextOption::NoWrap);

    updateMarginWidth(0);
}

int ScriptEditor::marginWidth() {

    int digits = 1;
    int max = qMax(1, blockCount());

    while(max >= 10) {
        max /= 10;
        digits ++;
    }

    int space = 10 + fontMetrics().width(QLatin1Char('9'))*digits;
    return space;
}

void ScriptEditor::updateMarginWidth(int) {
    setViewportMargins(marginWidth(),0,0,0);
}

void ScriptEditor::updateMargin(const QRect &rect, int dy) {
    if(dy)
        margin_->scroll(0,dy);
    else
        margin_->update(0, rect.y(), marginWidth(), rect.height());

    if( rect.contains(viewport()->rect()))
        updateMarginWidth(0);
}

void ScriptEditor::resizeEvent(QResizeEvent *e) {
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    margin_->setGeometry(QRect(cr.left(),cr.top(), marginWidth(), cr.height()));
}

void ScriptEditor::highlightBlocks(const std::vector<ParseError> &parse_errors) {
    QList<QTextEdit::ExtraSelection> extraSelections;

    if(!isReadOnly()) {

        vector<ParseError>::const_iterator it = parse_errors.begin();
        for(;it!=parse_errors.end();++it) {
            if( (int)it->row > 0 && (int)it->row <= blockCount() ) {
                QTextEdit::ExtraSelection selection;
                QColor lineColor = QColor(247, 153, 153);

                selection.format.setBackground(lineColor);
                selection.format.setProperty(QTextFormat::FullWidthSelection,true);
                selection.cursor = QTextCursor(document()->findBlockByLineNumber(it->row-1));
                selection.cursor.clearSelection();
                extraSelections.append(selection);
            }
        }
    }

    setExtraSelections(extraSelections);
}

void ScriptEditor::cleanAllHighlights() {
    QList<QTextEdit::ExtraSelection> extraSelections;
    setExtraSelections(extraSelections);
}

void ScriptEditor::marginPaintEvent(QPaintEvent *event) {

    QPainter painter(margin_);
    painter.fillRect(event->rect(), QColor(22, 104, 135));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
            if (block.isVisible() && bottom >= event->rect().top()) {
                QString number = QString::number(blockNumber + 1);
                painter.setPen(Qt::white);
                painter.drawText(0, top, margin_->width()-5, fontMetrics().height(),
                                 Qt::AlignRight, number);
            }

            block = block.next();
            top = bottom;
            bottom = top + (int) blockBoundingRect(block).height();
            ++blockNumber;
        }
}

void ScriptEditor::updateIndentationInfo() {

    lastTabLevel_ = tabLevel_;
    lastAfterLeftBrace_ = afterLeftBrace_;
    lastAfterSpaceOnly_ = afterSpaceOnly_;

    QString currentLine = textCursor().block().text();
    int currentPos = textCursor().positionInBlock();

    tabLevel_ = 0;
    int i = 0;
    for(; i < currentLine.length(); ++i) {
        if( currentLine.at(i) != '\t') break;
        else tabLevel_++;
    }

    if( i == currentLine.length() ) afterSpaceOnly_ = true;
    else afterSpaceOnly_ = false;

    afterLeftBrace_ = false;
    for(int i = currentPos-1; i >= 0; --i) {
        if( currentLine.at(i).isSpace()) continue;
        if( currentLine.at(i) == '{') afterLeftBrace_ = true;
        break;
    }
}

void ScriptEditor::keyPressEvent(QKeyEvent * event) {
    QPlainTextEdit::keyPressEvent(event);

    if( event->key() == Qt::Key_Return ) {
        if( lastAfterLeftBrace_ ) lastTabLevel_++;
        int tmpTabLevel = lastTabLevel_;
        for(int i=0; i<tmpTabLevel; ++i)
            textCursor().insertText("\t");
    }

    if( event->key() == Qt::Key_BraceRight && lastTabLevel_ > 0 ) {
        bool tmpAfterSpaceOnly = lastAfterSpaceOnly_;
        textCursor().deletePreviousChar();
        if( tmpAfterSpaceOnly )
            textCursor().deletePreviousChar();
        textCursor().insertText("}");
    }
}

void ScriptEditor::moveCursorTo(int row, int column) {

    QTextCursor cursor(textCursor());
    cursor.movePosition(QTextCursor::Start);
    for(int i = 1; i < row; ++i) {
        cursor.movePosition(QTextCursor::NextBlock);
    }
    cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::MoveAnchor,column);
    setTextCursor(cursor);
}

LSystemScriptHighlighter::LSystemScriptHighlighter(QTextDocument *document)
    : QSyntaxHighlighter(document), numberRegExp_("[-+]?[0-9]*\\.?[0-9]*([eE][-+]?[0-9]+)?") {

    keywordFormat_.setForeground(QColor(22, 104, 135));
    keywordFormat_.setFontUnderline(true);

    stringFormat_.setForeground(QColor(15, 70, 91));
    stringFormat_.setFontWeight(QFont::Bold);

    charFormat_.setForeground(QColor(12, 95, 38));

    numberFormat_.setForeground(QColor(163, 73, 164));
}

bool LSystemScriptHighlighter::isIgnoredChar(QChar c) {
    if( c.isSpace() || c == '{' || c == '}' || c == ';' ||
        c == ':' || c == ',' || c == '=') return true;
    return false;
}

void LSystemScriptHighlighter::highlightBlock(const QString &text) {

    enum State {
        NONE,
        CHAR,
        STRING
    } state = NONE;

    int startPos = 0;
    for(int i = 0; i <= text.length(); ++i)
    {
        switch(state) {
        case NONE:
            if( i == text.length() || isIgnoredChar(text.at(i)) ) break;
            startPos = i;
            state = CHAR;
            break;

        case CHAR:
            if( i == text.length() || isIgnoredChar(text.at(i)) ) {
                if(text.at(i-1).isDigit())
                    setFormat(startPos, i-startPos, numberFormat_);
                else setFormat(startPos, i-startPos, charFormat_);
                state = NONE;
                break;
            }
            state = STRING;
            break;

        case STRING:
            if( i == text.length() || isIgnoredChar(text.at(i)) ) {
                QString tmp = text.mid(startPos,i-startPos);
                if( tmp == "alphabet" || tmp == "axiom" ||
                    tmp == "rules" || tmp == "define" )
                    setFormat(startPos,i-startPos,keywordFormat_);
                else {
                    if( numberRegExp_.exactMatch(tmp) )
                        setFormat(startPos,i-startPos,numberFormat_);
                    else
                        setFormat(startPos,i-startPos,stringFormat_);
                }
                state = NONE;
            }
            break;
        }
    }
}

