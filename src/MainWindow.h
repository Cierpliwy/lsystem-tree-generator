#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QWidget>
#include <QModelIndex>

class ScriptEditor;
class QPushButton;
class QPlainTextEdit;
class QLabel;
class Parser;
class LSystemScriptHighlighter;
class QTreeView;
class ErrorListModel;

class MainWindow : public QWidget
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:

    //Slot parsujący zawartość edytora.
    void parse();
    //Uaktualniamy informację o aktualnej pozycji kursora
    void editorCursorChanged();
    //Klikniecie na błąd z listy błędów powoduje przesunięcie kursora
    //w odpowiednie miejsce.
    void moveCursorToError(QModelIndex index);
    //kliknięcie prawym przyciskiem na błędzie wyświetli okno z dokładnym opisem błędu.
    void showDetailedError(QModelIndex index);

    //Wczytanie skryptu z pliku
    void loadScript();
    //Zapisanie skryptu do pliku.
    void saveScript();

private:

    //Kontrolki lewego panelu.
    ScriptEditor *editor;
    QPushButton *parseButton;
    QPushButton *loadButton;
    QPushButton *saveButton;
    QLabel *editorStatusBar;

    //Singleton parsera.
    Parser *parser;

    //Widok wyświetlający listę błędów wraz z jego modelem.
    QTreeView* errorTreeView;
    ErrorListModel* errorListModel;

    //Klasa podświetlająca składnię dla edytora
    LSystemScriptHighlighter* editorHighlighter;
};

#endif // MAINWINDOW_H
