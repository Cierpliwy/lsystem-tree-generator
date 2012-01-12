#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QWidget>
#include <QModelIndex>

class QPushButton;
class QRadioButton;
class QPlainTextEdit;
class QLabel;
class QTreeView;
class QListView;
class ScriptEditor;
class Parser;
class LSystemScriptHighlighter;
class ErrorListModel;
class GLWidget;
class LSystemGLModel;
class QStringListModel;
class QSpinBox;

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

    void loadView();
    void chooseColorBegin();
    void changeColorBegin(QColor c);
    void chooseColorEnd();
    void changeColorEnd(QColor c);

protected:
    void keyPressEvent(QKeyEvent *);

private:

    //Kontrolki lewego panelu.

    QPushButton *parseButton_;
    QPushButton *loadButton_;
    QPushButton *saveButton_;
    QLabel *editorStatusBar_;
    ScriptEditor *editor_;
    LSystemScriptHighlighter* editorHighlighter_;
    ErrorListModel* errorListModel_;
    QTreeView* errorTreeView_;

    //Okno OpenGL
    GLWidget* glWidget_;
    LSystemGLModel *glModel_;

    Parser *parser_;

    //Prawy panel
    QPushButton *drawButton_;
    QRadioButton *colorSet1_;
    QRadioButton *colorSet2_;

    QSpinBox *recursionDepthSet_;
    QPushButton *colorBeginSet_;
    QPushButton *colorEndSet_;

    QListView *LSystemsListView_;
    QStringListModel *LSystemListModel_;

    void loadLSystemNameList();


};

#endif // MAINWINDOW_H
