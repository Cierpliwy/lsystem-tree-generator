#include <QBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QLabel>
#include <QTreeView>
#include <QFileDialog>
#include <QMessageBox>
#include <sstream>
#include <fstream>

#include "MainWindow.h"
#include "ScriptEditor.h"
#include "Parser.h"
#include "LSystemModelInterface.h"
#include "ErrorListModel.h"
#include "GLWidget.h"
#include "LSystemGLModel.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(QString::fromUtf8("Generator drzew 3D według L-systemów"));

    editor_ = new ScriptEditor(this);
    editorHighlighter_ = new LSystemScriptHighlighter(editor_->document());
    editorStatusBar_ = new QLabel;
    editorCursorChanged();

    errorTreeView_ = new QTreeView;
    errorListModel_ = new ErrorListModel;
    errorTreeView_->setModel(errorListModel_);
    errorTreeView_->setRootIsDecorated(false);

    glWidget_ = new GLWidget;
    QGroupBox *glBox = new QGroupBox(QString::fromUtf8("Podgląd L-systemu"));
    QVBoxLayout *glLayout = new QVBoxLayout;
    glLayout->addWidget(glWidget_);
    glBox->setLayout(glLayout);
    glModel_ = new LSystemGLModel;

    QHBoxLayout *mainLayout = new QHBoxLayout;
    QVBoxLayout *leftPanelLayout = new QVBoxLayout;

    QGroupBox *editorBox = new QGroupBox("Edytor skryptu");

    QVBoxLayout *editorBoxLayout = new QVBoxLayout;
    editorBoxLayout->addWidget(editor_);
    editorBoxLayout->addWidget(editorStatusBar_);
    editorBox->setLayout(editorBoxLayout);

    leftPanelLayout->addWidget(editorBox,4);

    parseButton_ = new QPushButton("Parsuj...");
    leftPanelLayout->addWidget(parseButton_,1);

    loadButton_ = new QPushButton("Wczytaj skrypt");
    saveButton_ = new QPushButton("Zapisz skrypt");
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(loadButton_);
    buttonsLayout->addWidget(saveButton_);
    leftPanelLayout->addLayout(buttonsLayout);

    leftPanelLayout->addWidget(errorTreeView_);

    mainLayout->addLayout(leftPanelLayout,0);

    mainLayout->addWidget(glBox,1);

    setLayout(mainLayout);

    parser_ = &Parser::getInstance();
    LSystemModelInterface::addCommand(Command("move",1));
    LSystemModelInterface::addCommand(Command("draw", 1));
    LSystemModelInterface::addCommand(Command("rotate",3));
    LSystemModelInterface::addCommand(Command("push",0));
    LSystemModelInterface::addCommand(Command("pop",0));
    parser_->registerCommands();

    connect(parseButton_, SIGNAL(clicked()), this, SLOT(parseScript()));
    connect(editor_, SIGNAL(cursorPositionChanged()), this, SLOT(editorCursorChanged()));
    connect(errorTreeView_, SIGNAL(clicked(QModelIndex)), this, SLOT(moveCursorToError(QModelIndex)));
    connect(errorTreeView_, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showDetailedError(QModelIndex)));
    connect(loadButton_, SIGNAL(clicked()), this, SLOT(loadScript()));
    connect(saveButton_, SIGNAL(clicked()), this, SLOT(saveScript()));
}

MainWindow::~MainWindow() {
    delete editorHighlighter_;
    delete glModel_;
}

void MainWindow::parseScript() {

    string scriptString = editor_->document()->toPlainText().toStdString();

    if(parser_->parseLSystem(scriptString)) {
        errorListModel_->removeParseErrors();
        editor_->cleanAllHighlights();
        glModel_->process(*(parser_->getLSystems().front()),10);
        glWidget_->setDrawable(glModel_);
        glWidget_->repaint();

    } else {
        errorListModel_->setParseErrors(parser_->getErrors());
        editor_->highlightBlocks(parser_->getErrors());
    }
}

void MainWindow::editorCursorChanged() {
    stringstream ss;
    ss << "Wiersz: " << editor_->textCursor().blockNumber()+1 << ", Kolumna: " << editor_->textCursor().positionInBlock();
    editorStatusBar_->setText(ss.str().c_str());
}

void MainWindow::moveCursorToError(QModelIndex index) {
    if(!index.isValid()) return;
    const ParseError& error = errorListModel_->getParseError(index);
    editor_->moveCursorTo(error.row,error.column);
    editor_->setFocus();
}

void MainWindow::showDetailedError(QModelIndex index) {
    if(!index.isValid()) return;
    const ParseError& error = errorListModel_->getParseError(index);
    QMessageBox msgBox;
    msgBox.setText(QString::fromUtf8(error.description.c_str()));
    msgBox.exec();
}

void MainWindow::loadScript() {

    errorListModel_->removeParseErrors();
    editor_->cleanAllHighlights();

    QString fileName = QFileDialog::getOpenFileName(
                        this,
                        QString::fromUtf8("Otwórz plik skryptu L-Systemów"),
                        QString(),
                        QString::fromUtf8("Pliki skryptu LSystemów (*.lsys);;Wszystkie pliki (*)"));

    if (!fileName.isEmpty()) {
        ifstream file(fileName.toAscii());
        if( file.good()) {
            QString script;
            char c;
            while(file.get(c)) {
                script.append(c);
            }
            editor_->document()->setPlainText(QString::fromUtf8(script.toAscii()));
        }
        file.close();
    }
}

void MainWindow::saveScript() {

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    QString::fromUtf8("Zapisz plik skryptu LSystemów"),
                                                    QString(),
                                                    QString::fromUtf8("Pliki skryptu LSystemów (*.lsys);;Wszystkie pliki (*)"));
    if( !fileName.isEmpty()) {
        ofstream file(fileName.toAscii());
        QString script = editor_->document()->toPlainText();
        QByteArray array = script.toUtf8();
        file.write(array.constData(),array.size());
        file.close();
    }
}
