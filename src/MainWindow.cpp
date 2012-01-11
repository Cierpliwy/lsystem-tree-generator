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

#define MAX_REC 6

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    //Ustawiamy tytuł okna.
    setWindowTitle(QString::fromUtf8("Generator drzew 3D według L-systemów"));

    //Tworzymy edytor
    editor = new ScriptEditor(this);
    editorHighlighter = new LSystemScriptHighlighter(editor->document());
    editorStatusBar = new QLabel;
    editorCursorChanged();

    //Tworzymy listę błędów.
    errorTreeView = new QTreeView;
    errorListModel = new ErrorListModel;
    errorTreeView->setModel(errorListModel);
    errorTreeView->setRootIsDecorated(false);

    //Tworzymy okno do renderowania w OpenGL, jego kontener i layout dla kontenera.
    glWidget = new GLWidget;
    QGroupBox *glBox = new QGroupBox(QString::fromUtf8("Podgląd L-systemu"));
    QVBoxLayout *glLayout = new QVBoxLayout;
    glLayout->addWidget(glWidget);
    glBox->setLayout(glLayout);
    glModel_ = new LSystemGLModel;

    //Główny layout okna.
    QHBoxLayout *mainLayout = new QHBoxLayout;
    //Lewy panel okna
    QVBoxLayout *leftPanelLayout = new QVBoxLayout;

    //Główny kontener edytora z tytułem.
    QGroupBox *editorBox = new QGroupBox("Edytor skryptu");

    //Layout dla kontrolki przetrzymującej edytor.
    QVBoxLayout *editorBoxLayout = new QVBoxLayout;
    editorBoxLayout->addWidget(editor);
    editorBoxLayout->addWidget(editorStatusBar);
    editorBox->setLayout(editorBoxLayout);

    //Dodajemy edytor
    leftPanelLayout->addWidget(editorBox,4);

    //Dodajemy przycisk parsowania
    parseButton = new QPushButton("Parsuj...");
    leftPanelLayout->addWidget(parseButton,1);

    //Dodajemy przyciski wczytywania i zapisywania
    loadButton = new QPushButton("Wczytaj skrypt");
    saveButton = new QPushButton("Zapisz skrypt");
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(loadButton);
    buttonsLayout->addWidget(saveButton);
    leftPanelLayout->addLayout(buttonsLayout);

    //Dodajemy listę błędów.
    leftPanelLayout->addWidget(errorTreeView);

    //Dodajemy lewy panel do głównego layoutu.
    mainLayout->addLayout(leftPanelLayout,0);
    //Dodajemy okno.
    mainLayout->addWidget(glBox,1);

    //Ustawiamy główny układ
    setLayout(mainLayout);

    //Tworzymy parsera.
    parser = &Parser::getInstance();
    //Ustawiamy domyślne komendy i rejestrujemy je w parserze.
    LSystemModelInterface::addCommand(Command("move",1));
    LSystemModelInterface::addCommand(Command("draw", 1));
    LSystemModelInterface::addCommand(Command("rotate",3));
    LSystemModelInterface::addCommand(Command("push",0));
    LSystemModelInterface::addCommand(Command("pop",0));
    parser->registerCommands();

    //Łączymy sygnał klinicięcia przycisku Parsuj z rzeczywistym parsowaniem.
    connect(parseButton, SIGNAL(clicked()), this, SLOT(parse()));
    //Zmiana pozycji kursora powoduje zmianę wartości etykiety.
    connect(editor, SIGNAL(cursorPositionChanged()), this, SLOT(editorCursorChanged()));
    //Kliknięcie na jeden z błędów na liście błędów powinienen przenieść kursor w miejsce błędu.
    connect(errorTreeView, SIGNAL(clicked(QModelIndex)), this, SLOT(moveCursorToError(QModelIndex)));
    //Kliknięcie prawym przyciskiem myszy pokazuje dokładny opis błędu.
    connect(errorTreeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showDetailedError(QModelIndex)));
    //Wczytywanie skryptu.
    connect(loadButton, SIGNAL(clicked()), this, SLOT(loadScript()));
    //Zapisanie skryptu.
    connect(saveButton, SIGNAL(clicked()), this, SLOT(saveScript()));
}

MainWindow::~MainWindow() {
    delete editorHighlighter;
    delete glModel_;
}

//Parsujemy zawartość okna
void MainWindow::parse() {

    //Pobieramy tekst
    string scriptString = editor->document()->toPlainText().toStdString();

    //Parsujemy skrypt
    if(parser->parseLSystem(scriptString)) {
        errorListModel->removeParseErrors();
        editor->cleanAllHighlights();
        glModel_->process(*(parser->getLSystems().front()),MAX_REC);
        glWidget->setDrawable(glModel_);
        glWidget->repaint();
    } else {
        errorListModel->setParseErrors(parser->getErrors());
        editor->highlightBlocks(parser->getErrors());
    }
}

//Funkcja uaktualniająca pozycję kursora w edytorze.
void MainWindow::editorCursorChanged() {
    stringstream ss;
    ss << "Wiersz: " << editor->textCursor().blockNumber()+1 << ", Kolumna: " << editor->textCursor().positionInBlock();
    editorStatusBar->setText(ss.str().c_str());
}

//Przenosimy kursor w miejsce błędu.
void MainWindow::moveCursorToError(QModelIndex index)
{
    if(!index.isValid()) return;
    const ParseError& error = errorListModel->getParseError(index);
    editor->moveCursorTo(error.row,error.column);
    editor->setFocus();
}

//Pokazujemy cały błąd w oknie dialogowym.
void MainWindow::showDetailedError(QModelIndex index)
{
    if(!index.isValid()) return;
    const ParseError& error = errorListModel->getParseError(index);
    QMessageBox msgBox;
    msgBox.setText(QString::fromUtf8(error.description.c_str()));
    msgBox.exec();
}

//Wczytujemy skrypt z pliku.
void MainWindow::loadScript() {

    //Czyścimy wszystkie błędy.
    errorListModel->removeParseErrors();
    editor->cleanAllHighlights();

    //Wyrzucamy okno z wyborem pliku.
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
            editor->document()->setPlainText(QString::fromUtf8(script.toAscii()));
        }
        file.close();
    }
}

//Zapisujemy aktualny skrypt do pliku
void MainWindow::saveScript() {

    //Okno z wyborem pliku do zapisania
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    QString::fromUtf8("Zapisz plik skryptu LSystemów"),
                                                    QString(),
                                                    QString::fromUtf8("Pliki skryptu LSystemów (*.lsys);;Wszystkie pliki (*)"));
    if( !fileName.isEmpty()) {
        ofstream file(fileName.toAscii());
        QString script = editor->document()->toPlainText();
        QByteArray array = script.toUtf8();
        file.write(array.constData(),array.size());
        file.close();
    }
}
