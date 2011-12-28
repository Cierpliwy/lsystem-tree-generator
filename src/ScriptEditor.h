#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <cstdio>

//Deklaracja z wyprzedzeniem.
class ScriptEditorMargin;

// Klasa umożliwiająca prostą edycję skryptu L-systemów wraz
// z numerowaniem linii oraz podświetlaniem linii zawierających
// błąd parsowania. W przyszłości można dodać także kolorowanie
// składni.
class ScriptEditor : public QPlainTextEdit {

    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = 0);
    
    //Rysuje lewy margines edytora.
    void marginPaintEvent(QPaintEvent *event);

    //Zwraca szerokość marginesu edytora.
    int marginWidth();

protected:

    //Funkcja wywoływana podczas zmiany rozmiaru edytora.
    void resizeEvent(QResizeEvent *e);

    //Przeciążamy funkcję wciśnięcia klawisza by móc umożliwić
    //automatyczne dodawanie wcięć.
    void keyPressEvent(QKeyEvent * event);

public slots:

    //Podświetlamy linie, które podano w liście
    void highlightBlocks(const std::vector<int> &block_list);

private slots:

    //Aktualizujemy szerokość marginesu.
    void updateMarginWidth(int newBlockCount);
    //Uaktualniamy obszar marginesu
    void updateMargin(const QRect& rect, int dy);
    //Uaktualniamy informację potrzebne do generowania automatycznych
    //wcięć.
    void updateIndentationInfo();

private:

    //Lewy margines dla naszego edytora.
    ScriptEditorMargin *margin;
    //Informacja o aktualnym i poprzednim poziomie wcięcia.
    int tabLevel, lastTabLevel;
    //Informacja czy znakiem przed kursorem jest { i czy był {
    bool afterLeftBrace, lastAfterLeftBrace;
    //Czy kursor znajduje się tylko przed białymi znakami.
    bool afterSpaceOnly, lastAfterSpaceOnly;
};

// Klasa, której zadaniem jest rysowanie lewego marginesu dla
// edytora skryptów. Wyświetla aktualny numer wiersza i
// oznaczenie o błędzie.
class ScriptEditorMargin : public QWidget {
public:
    ScriptEditorMargin(ScriptEditor *editor) : QWidget(editor) {
        this->editor = editor;
    }

    //Zwraca dopuszczalny rozmiar marginesu pobrany z klasy edytora.
    QSize sizeHint() const {
        return QSize(editor->marginWidth(),0);
    }

protected:

    void paintEvent(QPaintEvent *event) {
        editor->marginPaintEvent(event);
    }

private:
    ScriptEditor *editor;
};

//Klasa kolorująca składnię skryptu L-systemów
class LSystemScriptHighlighter : public QSyntaxHighlighter {

    Q_OBJECT

public:

    //Konstruktor
    LSystemScriptHighlighter(QTextDocument *document);

protected:

    //Funkcja definiująca podświetlanie składni
    void highlightBlock(const QString &text);

private:

    //Funkcja sprawdza czy znak jest białym znakiem lub '{','}',',',';',':','='
    bool isIgnoredChar(QChar c);

    //Formaty dla typów danych w składni Lsystemu
    QTextCharFormat keywordFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat charFormat;
};

#endif // SCRIPTEDITOR_H
