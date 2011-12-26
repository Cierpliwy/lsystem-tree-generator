#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QWidget>
#include <QPlainTextEdit>
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

private:

    //Lewy margines dla naszego edytora.
    ScriptEditorMargin *margin;
};

// Klasa, której zadaniem jest rysowanie lewego marginesu dla
// edytora skryptów. Wyświetla aktualny numer wiersza i
// oznaczenie o błędzie.
class ScriptEditorMargin : public QWidget {
public:
    ScriptEditorMargin(ScriptEditor *editor = 0) : QWidget(editor) {
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

#endif // SCRIPTEDITOR_H
