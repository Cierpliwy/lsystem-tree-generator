#include <QBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QRadioButton>
#include <QPlainTextEdit>
#include <QLabel>
#include <QTreeView>
#include <QListView>
#include <QFileDialog>
#include <QMessageBox>
#include <QSpinBox>
#include <QColorDialog>
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

#define DEFAULT_RECURSION_DEPTH 4

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

    LSystemsListView_ = new QListView;
    LSystemsListView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    LSystemListModel_ = new QStringListModel;



    glWidget_ = new GLWidget;
    QGroupBox *glBox = new QGroupBox(QString::fromUtf8("Podgląd L-systemu"));
    QVBoxLayout *glLayout = new QVBoxLayout;
    glLayout->addWidget(glWidget_);
    glBox->setLayout(glLayout);
    glModel_ = new LSystemGLModel;

    QHBoxLayout *mainLayout = new QHBoxLayout;
    QVBoxLayout *leftPanelLayout = new QVBoxLayout;
    QVBoxLayout *rightPanelLayout = new QVBoxLayout;

    QGroupBox *editorBox = new QGroupBox("Edytor skryptu");
    QGroupBox *viewBox = new QGroupBox(QString::fromUtf8("Wyświetlanie:"));
    QGroupBox *LSystemListBox = new QGroupBox(QString::fromUtf8("Lista L-Systemów:"));

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

    QVBoxLayout *LSystemListBoxLayout = new QVBoxLayout;
    LSystemListBoxLayout->addWidget(LSystemsListView_);
    LSystemListBox->setLayout(LSystemListBoxLayout);

    rightPanelLayout->addWidget(LSystemListBox);
    drawButton_ = new QPushButton(QString::fromUtf8("Wyświetl"));
    colorSet1_ = new QRadioButton(QString::fromUtf8("Kolor według poziomu rekurencji"));
    colorSet2_ = new QRadioButton(QString::fromUtf8("Kolor według kolejnych iteracji"));
    colorSet2_->setChecked(true);

    QVBoxLayout *viewDisplayLayout = new QVBoxLayout;

    QHBoxLayout *recursionDepthSetLayout = new QHBoxLayout;
    QLabel *recursionDepthSetLabel = new QLabel("Poziom rekurencji: ");
    recursionDepthSet_ = new QSpinBox;
    recursionDepthSet_->setValue(DEFAULT_RECURSION_DEPTH);
    recursionDepthSet_->setMinimum(0);
    recursionDepthSet_->setMaximum(15);
    recursionDepthSetLayout->addWidget(recursionDepthSetLabel);
    recursionDepthSetLayout->addWidget(recursionDepthSet_);

    QHBoxLayout *colorLabelsLayout = new QHBoxLayout;
    QLabel *colorBeginSetLabel = new QLabel(QString::fromUtf8("Kolor początkowy:"));
    QLabel *colorEndSetLabel = new QLabel(QString::fromUtf8("Kolor końcowy:"));
    colorLabelsLayout->addWidget(colorBeginSetLabel);
    colorLabelsLayout->addWidget(colorEndSetLabel);

    QHBoxLayout *colorSetLayout = new QHBoxLayout;
    colorBeginSet_ = new QPushButton;
    colorBeginSet_->setAutoFillBackground(true);
    const QString styleSheet = QString("QPushButton {"
                                       "background-color: %1}").arg(QColor(102,255,51).name());
    //colorBeginSet_->setStyleSheet("background-color: rgb(102,255,51)");
    colorBeginSet_->setStyleSheet(styleSheet);
    colorEndSet_ = new QPushButton;
    colorEndSet_->setAutoFillBackground(true);
    colorEndSet_->setStyleSheet("background-color: rgb(127,255,255);");


    colorSetLayout->addWidget(colorBeginSet_);
    colorSetLayout->addWidget(colorEndSet_);

    viewDisplayLayout->addLayout(colorLabelsLayout);
    viewDisplayLayout->addLayout(colorSetLayout);
    viewDisplayLayout->addLayout(recursionDepthSetLayout);
    viewDisplayLayout->addWidget(colorSet1_);
    viewDisplayLayout->addWidget(colorSet2_);
    viewDisplayLayout->addWidget(drawButton_);
    viewBox->setLayout(viewDisplayLayout);
    rightPanelLayout->addWidget(viewBox);

    mainLayout->addLayout(leftPanelLayout,3);

    mainLayout->addWidget(glBox,10);

    mainLayout->addLayout(rightPanelLayout,2);

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
    connect(drawButton_, SIGNAL(clicked()), this, SLOT(loadView()));
    connect(colorBeginSet_, SIGNAL(clicked()), this, SLOT(chooseColorBegin()));
    connect(colorEndSet_, SIGNAL(clicked()), this, SLOT(chooseColorEnd()));
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

    } else {
        errorListModel_->setParseErrors(parser_->getErrors());
        editor_->highlightBlocks(parser_->getErrors());
    }
    loadLSystemNameList();
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

void MainWindow::loadLSystemNameList(){
    QStringList LSystemNameList;
    for(unsigned int i = 0; i < parser_->getLSystems().size(); ++i){
        LSystemNameList << (*(parser_->getLSystems()[i])).getName().c_str();
    }
    LSystemListModel_->setStringList(LSystemNameList);
    LSystemsListView_->setModel(LSystemListModel_);
    LSystemsListView_->setCurrentIndex(LSystemListModel_->index(0,0));
}

void MainWindow::loadView(){
    if(LSystemsListView_->selectionModel() != NULL)
        if(!LSystemsListView_->selectionModel()->selectedIndexes().isEmpty())
        {
            QModelIndexList selectedLSystems = LSystemsListView_->selectionModel()->selectedIndexes();
            QString selectedLSystemName = selectedLSystems.first().data().toString();
            try{
                QPalette palette = colorBeginSet_->palette();
                QColor colorBegin = palette.color(QPalette::Background);
                palette = colorEndSet_->palette();
                QColor colorEnd = palette.color(QPalette::Background);
                if(colorSet1_->isChecked()){
                    glModel_->setColorMode(LSystemGLModel::RECURCION_LEVEL_COLOR,
                                           glm::vec3(colorBegin.redF(),
                                                     colorBegin.greenF(),
                                                     colorBegin.blueF()),
                                           glm::vec3(colorEnd.redF(),
                                                     colorEnd.greenF(),
                                                     colorEnd.blueF()) );
                }else if(colorSet2_->isChecked()){
                    glModel_->setColorMode(LSystemGLModel::SEQUENCE_COLOR,
                                           glm::vec3(colorBegin.redF(),
                                                     colorBegin.greenF(),
                                                     colorBegin.blueF()),
                                           glm::vec3(colorEnd.redF(),
                                                     colorEnd.greenF(),
                                                     colorEnd.blueF())  );
                }
                for( unsigned int i = 0; i < parser_->getLSystems().size(); ++i){

                    if(parser_->getLSystems()[i]->getName().c_str() == selectedLSystemName ){
                        int current_recursion_depth = recursionDepthSet_->value();
                        glModel_->process(*(parser_->getLSystems()[i]),current_recursion_depth);
                        glm::vec3 center = glModel_->getCenterOfModel();
                        float distance = glModel_->getDefaultDistanceFromModel();
                        glWidget_->setLookAtPosition(center.x,center.y,center.z);
                        glWidget_->setLookAtDistance(distance);
                        glWidget_->setZoomDelta(distance/7.0f);
                        glWidget_->setDrawable(glModel_);
                        glWidget_->repaint();
                    }
                }



            }
            catch(std::bad_alloc& e){
                QMessageBox *bad_alloc_message = new QMessageBox(QMessageBox::Critical,
                                                                 QString::fromUtf8("Ostrzeżenie!"),
                                                                 QString::fromUtf8("Zużyto za dużo pamięci.\nNależy zmniejszyć poziom rekurencji.")
                                                                 );
                bad_alloc_message->exec();
            }
        }
}
void MainWindow::chooseColorBegin(){
    QColorDialog *colorSetDialog = new QColorDialog;
    QPalette palette = colorBeginSet_->palette();
    QColor current_color = palette.color(QPalette::Background);
    colorSetDialog->setCurrentColor(current_color);
    connect(colorSetDialog, SIGNAL(colorSelected(QColor)),this, SLOT(changeColorBegin(QColor)));
    colorSetDialog->open();

}
void MainWindow::changeColorBegin(QColor c){
    const QString styleSheet = QString("QPushButton {"
                                       "background-color: %1}").arg(c.name());
    colorBeginSet_->setStyleSheet(styleSheet);
}


void MainWindow::chooseColorEnd(){
    QColorDialog *colorSetDialog = new QColorDialog;
    QPalette palette = colorEndSet_->palette();
    QColor current_color = palette.color(QPalette::Background);
    colorSetDialog->setCurrentColor(current_color);
    connect(colorSetDialog, SIGNAL(colorSelected(QColor)),this, SLOT(changeColorEnd(QColor)));
    colorSetDialog->open();

}

void MainWindow::changeColorEnd(QColor c){
    const QString styleSheet = QString("QPushButton {"
                                       "background-color: %1}").arg(c.name());
    colorEndSet_->setStyleSheet(styleSheet);
}

void MainWindow::keyPressEvent(QKeyEvent *e){
    QWidget::keyPressEvent(e);

    if(e->key() == Qt::Key_F11){
        if(this->isFullScreen())
            this->showMaximized();
        else this->showFullScreen();
    }

}
