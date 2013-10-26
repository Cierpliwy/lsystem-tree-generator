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
#include <QSplitter>
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
#define MAX_RECURSION_DEPTH 15

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(trUtf8("L-System Based 3D Tree Generator"));

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
    QGroupBox *glBox = new QGroupBox(trUtf8("L-System preview"));
    QVBoxLayout *glLayout = new QVBoxLayout;
    glLayout->addWidget(glWidget_);
    glBox->setLayout(glLayout);
    glModel_ = new LSystemGLModel;

    QSplitter *mainLayout = new QSplitter;
    QVBoxLayout *leftPanelLayout = new QVBoxLayout;
    QVBoxLayout *rightPanelLayout = new QVBoxLayout;

    QGroupBox *editorBox = new QGroupBox(trUtf8("Script editor"));
    QGroupBox *viewBox = new QGroupBox(trUtf8("Displayed"));
    QGroupBox *LSystemListBox = new QGroupBox(trUtf8("L-System list:"));

    QVBoxLayout *editorBoxLayout = new QVBoxLayout;
    editorBoxLayout->addWidget(editor_);
    editorBoxLayout->addWidget(editorStatusBar_);
    editorBox->setLayout(editorBoxLayout);

    leftPanelLayout->addWidget(editorBox,4);

    parseButton_ = new QPushButton(trUtf8("Parse..."));
    leftPanelLayout->addWidget(parseButton_,1);

    loadButton_ = new QPushButton(trUtf8("Load script"));
    saveButton_ = new QPushButton(trUtf8("Save script"));
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(loadButton_);
    buttonsLayout->addWidget(saveButton_);
    leftPanelLayout->addLayout(buttonsLayout);

    leftPanelLayout->addWidget(errorTreeView_);

    QVBoxLayout *LSystemListBoxLayout = new QVBoxLayout;
    LSystemListBoxLayout->addWidget(LSystemsListView_);
    LSystemListBox->setLayout(LSystemListBoxLayout);

    rightPanelLayout->addWidget(LSystemListBox);
    drawButton_ = new QPushButton(trUtf8("Display"));
    colorSet1_ = new QRadioButton(trUtf8("Color based on recursion level"));
    colorSet2_ = new QRadioButton(trUtf8("Color based on iteration level"));
    colorSet2_->setChecked(true);

    QVBoxLayout *viewDisplayLayout = new QVBoxLayout;

    QHBoxLayout *recursionDepthSetLayout = new QHBoxLayout;
    QLabel *recursionDepthSetLabel = new QLabel(trUtf8("Recusion level:"));
    recursionDepthSet_ = new QSpinBox;
    recursionDepthSet_->setValue(DEFAULT_RECURSION_DEPTH);
    recursionDepthSet_->setMinimum(0);
    recursionDepthSet_->setMaximum(MAX_RECURSION_DEPTH);
    recursionDepthSetLayout->addWidget(recursionDepthSetLabel);
    recursionDepthSetLayout->addWidget(recursionDepthSet_);

    QHBoxLayout *colorLabelsLayout = new QHBoxLayout;
    QLabel *colorBeginSetLabel = new QLabel(trUtf8("Start color:"));
    QLabel *colorEndSetLabel = new QLabel(trUtf8("End color:"));
    colorLabelsLayout->addWidget(colorBeginSetLabel);
    colorLabelsLayout->addWidget(colorEndSetLabel);

    QHBoxLayout *colorSetLayout = new QHBoxLayout;
    colorBeginSet_ = new QPushButton;
    colorBeginSet_->setAutoFillBackground(true);
    const QString styleSheet = QString("QPushButton {"
                                       "background-color: %1}").arg(QColor(102,255,51).name());
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

    QWidget *leftWidget = new QWidget;
    leftWidget->setLayout(leftPanelLayout);
    mainLayout->addWidget(leftWidget);

    mainLayout->addWidget(glBox);

    QWidget *rightLayout = new QWidget;
    rightLayout->setLayout(rightPanelLayout);
    mainLayout->addWidget(rightLayout);

    QHBoxLayout *mainHLayout = new QHBoxLayout;
    mainHLayout->addWidget(mainLayout);
    setLayout(mainHLayout);

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
    ss << trUtf8("Line: ").toStdString()  << editor_->textCursor().blockNumber()+1
       << trUtf8(", column: ").toStdString() << editor_->textCursor().positionInBlock();
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
                        trUtf8("Open L-System script file"),
                        QString(),
                        trUtf8("L-System script files (*.lsys);;All files (*)"));

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
                                                    trUtf8("Save L-System script file"),
                                                    QString(),
                                                    trUtf8("L-System script files (*.lsys);;All files (*)"));
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
                                                                 trUtf8("Warning!"),
                                                                 trUtf8("Too much memory has been used.\nTry to lower recursion level.")
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
