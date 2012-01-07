#include "ScriptEditor.h"
#include <QPainter>
#include <QTextBlock>

using namespace std;

// W konstruktorze tworzymy obiekt lewego marginesu oraz
// łączymy sygnaly ze slotami.
ScriptEditor::ScriptEditor(QWidget *parent) :
    QPlainTextEdit(parent) {

    // Ustawienia podstawowe edytora.
    QFont editorFont("Consolas",10);
    editorFont.setStyleHint(QFont::TypeWriter);
    setFont(editorFont);
    setTabStopWidth(20);

    // Zmienne pomocnicze do wyznaczania automatycznych wcięć.
    tabLevel = 0;
    lastTabLevel = 0;
    afterLeftBrace = false;
    lastAfterLeftBrace = false;
    afterSpaceOnly = false;
    lastAfterSpaceOnly = false;

    // Tworzymy obiekt lewego marginesu.
    margin = new ScriptEditorMargin(this);

    // Łączymy wszystkie zdarzenia, ktore wplywają na zmianę wyglądu naszego marginesu,
    // bądz podświetleń.
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateMarginWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateMargin(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updateIndentationInfo()));

    //Wyłączamy załamywanie linii.
    setWordWrapMode(QTextOption::NoWrap);

    // Kalkulujemy dla pierwszej linii.
    updateMarginWidth(0);
}

// Zwracamy wymagany rozmiar marginesu potrzebny do narysowania linii.
int ScriptEditor::marginWidth() {
    //Liczba cyfr numeru linii ostatniego bloku.
    int digits = 1;
    //Ile mamy linii.
    int max = qMax(1, blockCount());
    //Obliczamy liczbe cyfr w ostatniej linii.
    while(max >= 10) {
        max /= 10;
        digits ++;
    }
    //Podajemy szerokość marginesu w pikselach.
    int space = 10 + fontMetrics().width(QLatin1Char('9'))*digits;
    return space;
}

// Funkcja ustawia "prawdziwy" margines dla edytora skryptów
// byśmy mogli na nim narysować nasz margines.
void ScriptEditor::updateMarginWidth(int newBlockCount) {
    setViewportMargins(marginWidth(),0,0,0);
}

// Uaktualniamy margines gdy nastąpiła zmiana rozmiaru lub
// przewinięcie suwakiem aktualnego tekstu.
void ScriptEditor::updateMargin(const QRect &rect, int dy) {
    if(dy)
        margin->scroll(0,dy);
    else
        margin->update(0, rect.y(), marginWidth(), rect.height());

    if( rect.contains(viewport()->rect()))
        updateMarginWidth(0);
}

// Wywolujemy zdarzenie z klasy bazowej.
void ScriptEditor::resizeEvent(QResizeEvent *e) {
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    margin->setGeometry(QRect(cr.left(),cr.top(), marginWidth(), cr.height()));
}

// Podświtlamy aktualną linię.
void ScriptEditor::highlightBlocks(const std::vector<int> &block_list) {
    QList<QTextEdit::ExtraSelection> extraSelections;

    if(!isReadOnly()) {

        vector<int>::const_iterator it = block_list.begin();
        for(;it!=block_list.end();++it) {
            if( *it > 0 && *it <= blockCount() ) {
                QTextEdit::ExtraSelection selection;
                QColor lineColor = QColor(247, 153, 153);

                selection.format.setBackground(lineColor);
                selection.format.setProperty(QTextFormat::FullWidthSelection,true);
                selection.cursor = QTextCursor(document()->findBlockByLineNumber(*it-1));
                selection.cursor.clearSelection();
                extraSelections.append(selection);
            }
        }
    }

    setExtraSelections(extraSelections);
}

// Funkcja rysująca margines.
void ScriptEditor::marginPaintEvent(QPaintEvent *event) {

    //Najpierw rysujemy cały margines na niebiesko.
    QPainter painter(margin);
    painter.fillRect(event->rect(), QColor(22, 104, 135));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
            if (block.isVisible() && bottom >= event->rect().top()) {
                QString number = QString::number(blockNumber + 1);
                painter.setPen(Qt::white);
                painter.drawText(0, top, margin->width()-5, fontMetrics().height(),
                                 Qt::AlignRight, number);
            }

            block = block.next();
            top = bottom;
            bottom = top + (int) blockBoundingRect(block).height();
            ++blockNumber;
        }
}

//Zwracamy informację o aktualnych parametrach dotyczących linii:
//Jaki jest poziom wcięć linii, czy przed kursorem jest znak {.
void ScriptEditor::updateIndentationInfo() {

    //Zapisujemy poprzedni stan.
    lastTabLevel = tabLevel;
    lastAfterLeftBrace = afterLeftBrace;
    lastAfterSpaceOnly = afterSpaceOnly;

    //Pobieramy akualną linię w której znajduje się kursor.
    QString currentLine = textCursor().block().text();
    //Zwracamy aktualną pozycję kursora.
    int currentPos = textCursor().positionInBlock();

    //Srawdzamy poziom wcięć
    tabLevel = 0;
    int i = 0;
    for(; i < currentLine.length(); ++i) {
        if( currentLine.at(i) != '\t') break;
        else tabLevel++;
    }

    //Dodatkowe sprawdzanie czy przed kursorem są tylko białe znaki.
    if( i == currentLine.length() ) afterSpaceOnly = true;
    else afterSpaceOnly = false;

    //Sprawdzamy czy jesteśmy przed '{'
    afterLeftBrace = false;
    for(int i = currentPos-1; i >= 0; --i) {
        if( currentLine.at(i).isSpace()) continue;
        if( currentLine.at(i) == '{') afterLeftBrace = true;
        break;
    }
}

//Dodajemy możliwość generowania automatycznych wcięć.
//Każde użycie { lub tab zwiększa poziom wcięcia, zaś
//} lub backspace zmienijsza.
void ScriptEditor::keyPressEvent(QKeyEvent * event) {

    //Wykonujemy funkcję klasy bazowej.
    QPlainTextEdit::keyPressEvent(event);

    //Jeżeli został naciśnięty klawisz Enter ustawiamy poziom wcięć taki
    //jak w poprzedniej linii + o jeden poziom większy gdy wcześniej był znak
    //'{'
    if( event->key() == Qt::Key_Return ) {
        if( lastAfterLeftBrace ) lastTabLevel++;
        int tmpTabLevel = lastTabLevel;
        for(int i=0; i<tmpTabLevel; ++i)
            textCursor().insertText("\t");
    }

    //Jeżeli użytkownik uzył '}' i poziom wcięć jest większy od zera i
    //kursor znajdował się tylko przed białymi znakami. Cofnij wcięcie.
    if( event->key() == Qt::Key_BraceRight && lastTabLevel > 0 ) {
        bool tmpAfterSpaceOnly = lastAfterSpaceOnly;
        textCursor().deletePreviousChar();
        if( tmpAfterSpaceOnly )
            textCursor().deletePreviousChar();
        textCursor().insertText("}");
    }
}

//Konstruktor klasy kolorującej składnie skryptu do L-systemów. Przekazujemy
//dokument do rodzica.
LSystemScriptHighlighter::LSystemScriptHighlighter(QTextDocument *document)
    : QSyntaxHighlighter(document) {

    //Jasny niebieski i pogrubiony dla słów kluczowych.
    keywordFormat.setForeground(QColor(22, 104, 135));
    keywordFormat.setFontUnderline(true);

    //Ciemny niebieski dla napisów.
    stringFormat.setForeground(QColor(15, 70, 91));
    stringFormat.setFontWeight(QFont::Bold);

    //Ciemny zielony dla znaków.
    charFormat.setForeground(QColor(12, 95, 38));

}

//Sprawdzamy czy aktualnie jest znak który ignorujemy
bool LSystemScriptHighlighter::isIgnoredChar(QChar c) {
    if( c.isSpace() || c == '{' || c == '}' || c == ';' ||
        c == ':' || c == ',' || c == '=') return true;
    return false;
}

//Właściwa funkcja kolorująca składnie skryptu L-systemów.
void LSystemScriptHighlighter::highlightBlock(const QString &text) {

    //Co aktualnie przeglądamy
    enum State {
        NONE,
        CHAR,
        STRING
    } state = NONE;

    //Pozycja początkowa dla formatowania.
    int startPos = 0;
    for(int i = 0; i <= text.length(); ++i)
    {
        switch(state) {
        //Narazie nic nie mamy.
        case NONE:
            //Jeżeli natrafiliśmy na biały znak lub znaki specjalne nic nie rób.
            if( i == text.length() || isIgnoredChar(text.at(i)) ) break;
            startPos = i;
            state = CHAR;
            break;
        //Mamy jeden znak
        case CHAR:
            if( i == text.length() || isIgnoredChar(text.at(i)) ) {
                setFormat(startPos, i-startPos, charFormat);
                state = NONE;
                break;
            }
            state = STRING;
            break;
        //Mamy ciąg znaków
        case STRING:
            if( i == text.length() || isIgnoredChar(text.at(i)) ) {
                QString tmp = text.mid(startPos,i-startPos);
                if( tmp == "alphabet" || tmp == "axiom" ||
                    tmp == "rules" || tmp == "define" )
                    setFormat(startPos,i-startPos,keywordFormat);
                else
                    setFormat(startPos,i-startPos,stringFormat);
                state = NONE;
            }
            break;
        }
    }
}

