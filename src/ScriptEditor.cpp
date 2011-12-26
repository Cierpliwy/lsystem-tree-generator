#include "ScriptEditor.h"
#include <QPainter>
#include <QTextBlock>

using namespace std;

// W konstruktorze tworzymy obiekt lewego marginesu oraz
// łączymy sygnaly ze slotami.
ScriptEditor::ScriptEditor(QWidget *parent) :
    QPlainTextEdit(parent) {

    // Ustawienia podstawowe edytora.
    QFont editorFont("Monospace");
    editorFont.setStyleHint(QFont::TypeWriter);
    setFont(editorFont);
    setTabStopWidth(20);
    tabLevel = 0;
    tabUsed = false;

    // Tworzymy obiekt lewego marginesu.
    margin = new ScriptEditorMargin(this);

    // Łączymy wszystkie zdarzenia, ktore wplywają na zmianę wyglądu naszego marginesu,
    // bądz podświetleń.
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateMarginWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateMargin(QRect,int)));

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

    //Najpierw rysujemy cały margines na szaro.
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

//Dodajemy możliwość generowania automatycznych wcięć.
//Każde użycie { lub tab zwiększa poziom wcięcia, zaś
//} lub backspace zmienijsza.
void ScriptEditor::keyPressEvent(QKeyEvent * event) {

    //Gdy użyty zostaje backspace cofamy wcięcia jedynie wtedy gdy
    //ostatnim znakiem była tabulacja.
    if( event->key() == Qt::Key_Backspace && tabLevel > 0) {
        if( textCursor().positionInBlock() > 0 )
            if( textCursor().block().text().at(textCursor().positionInBlock()-1) == '\t' )
                tabLevel--;
    }

    //Wykonujemy funkcję klasy bazowej.
    QPlainTextEdit::keyPressEvent(event);

    //Gdy występuje zamknięcie klamry i wcześniejszym znakiem była tabulacja
    //zmniejszamy poziom wcięć.
    if( event->key() == Qt::Key_BraceRight && tabLevel > 0 ) {
        textCursor().deletePreviousChar();
        if( textCursor().positionInBlock() > 0 &&
            textCursor().block().text().at(textCursor().positionInBlock()-1) == '\t') {
            textCursor().deletePreviousChar();
            tabLevel--;
        }
        textCursor().insertText("}");
    }

    //Gdy używamy rozpoczęcia bloku lub tabulacji jest możliwość zwiększenia
    //wcięcia (jeśli zaraz po tym naciśniemy ENTER).
    if( event->key() == Qt::Key_BraceLeft || event->key() == Qt::Key_Tab )
        tabUsed = true;

    //Jeżeli został naciśnięty enter sprawdzamy poziom wcięć i odpowiednio je dodajemy.
    if( event->key() == Qt::Key_Return ) {
        if( tabUsed ) tabLevel++;
        for(int i=0; i<tabLevel; ++i)
            textCursor().insertText("\t");
    }

    //Jeżeli został obsłużony inny znak nie chcemy by nastąpiło wcięcie.
    if( event->key() != Qt::Key_BraceLeft && event->key() != Qt::Key_Tab )
        tabUsed = false;
}
