#include <QBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QLabel>
#include <sstream>

#include "MainWindow.h"
#include "ScriptEditor.h"
#include "Parser.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    //Ustawiamy tytuł okna.
    setWindowTitle(QString::fromUtf8("Generator drzew 3D według L-systemów"));

    //Tworzymy edytor
    editor = new ScriptEditor(this);
    //Tworzymy jego podświetlenie składni.
    editorHighlighter = new LSystemScriptHighlighter(editor->document());

    //Ustawiamy pasek stanu edytora
    editorStatusBar = new QLabel;
    editorCursorChanged();

    //Główny layout okna.
    QVBoxLayout *mainLayout = new QVBoxLayout;

    //Główny kontener edytora z tytułem.
    QGroupBox *editorBox = new QGroupBox("Edytor skryptu");

    //Layout dla kontrolki przetrzymującej edytor.
    QVBoxLayout *editorBoxLayout = new QVBoxLayout;
    editorBoxLayout->addWidget(editor);
    editorBoxLayout->addWidget(editorStatusBar);
    editorBox->setLayout(editorBoxLayout);

    //Dodajemy edytor
    mainLayout->addWidget(editorBox,4);

    //Dodajemy przycisk parsowania
    parseButton = new QPushButton("Parsuj...");
    mainLayout->addWidget(parseButton,1);

    //Dodajemy pole tekstowe pokazujące błędy.
    errorContainer = new QPlainTextEdit;
    errorContainer->setEnabled(false);
    mainLayout->addWidget(errorContainer,2);

    //Ustawiamy główny układ
    setLayout(mainLayout);

    //Tworzymy parsera.
    parser = &Parser::getInstance();

    //Łączymy sygnał klinicięcia przycisku Parsuj z rzeczywistym parsowaniem.
    connect(parseButton, SIGNAL(clicked()), this, SLOT(parse()));
    connect(editor, SIGNAL(cursorPositionChanged()), this, SLOT(editorCursorChanged()));
}

MainWindow::~MainWindow() {
    delete editorHighlighter;
}

//Parsujemy zawartość okna
void MainWindow::parse() {

    //Czyścimy okno błędów.
    errorContainer->clear();
    //Pobieramy tekst
    string scriptString = editor->document()->toPlainText().toStdString();
    //Wektor przechowywujący numery linii.
    vector<int> lineNumbers;
    //Parsujemy skrypt
    if(parser->parseLSystem(scriptString)) {
        errorContainer->insertPlainText("Skrypt sparsowano poprawnie.");
    } else {
        const vector<ParseError> &errors = parser->getErrors();
        vector<ParseError>::const_iterator it = errors.begin();
        for(;it!=errors.end();++it) {
            stringstream ss;
            ss << it->row << ":" << it->column << ": " << it->description;
            errorContainer->insertPlainText(QString::fromUtf8(ss.str().c_str()));
            errorContainer->insertPlainText("\n");
            lineNumbers.push_back(it->row);
        }
    }

    //Wyświetlamy błędne linie
    editor->highlightBlocks(lineNumbers);
}

//Funkcja uaktualniająca pozycję kursora w edytorze.
void MainWindow::editorCursorChanged() {
    stringstream ss;
    ss << "Wiersz: " << editor->textCursor().blockNumber()+1 << ", Kolumna: " << editor->textCursor().positionInBlock();
    editorStatusBar->setText(ss.str().c_str());
}
