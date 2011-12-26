#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QWidget>

class ScriptEditor;
class QPushButton;
class QPlainTextEdit;
class QLabel;
class Parser;

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

private:
    ScriptEditor *editor;
    QPushButton *parseButton;
    QPlainTextEdit *errorContainer;
    QLabel *editorStatusBar;
    Parser *parser;
};

#endif // MAINWINDOW_H
