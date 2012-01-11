#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QWidget>
#include <QModelIndex>

class QPushButton;
class QPlainTextEdit;
class QLabel;
class QTreeView;
class ScriptEditor;
class Parser;
class LSystemScriptHighlighter;
class ErrorListModel;
class GLWidget;
class LSystemGLModel;

// Główne okno programu.
class MainWindow : public QWidget
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:

    void parseScript();
    void editorCursorChanged();

    void moveCursorToError(QModelIndex index);
    void showDetailedError(QModelIndex index);

    void loadScript();
    void saveScript();

private:

    //Kontrolki lewego panelu.
    ScriptEditor *editor_;
    QPushButton *parseButton_;
    QPushButton *loadButton_;
    QPushButton *saveButton_;
    QLabel *editorStatusBar_;

    GLWidget* glWidget_;
    LSystemGLModel *glModel_;
    Parser *parser_;

    QTreeView* errorTreeView_;
    ErrorListModel* errorListModel_;

    LSystemScriptHighlighter* editorHighlighter_;
};

#endif // MAINWINDOW_H
