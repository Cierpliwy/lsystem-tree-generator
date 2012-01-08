# -------------------------------------------------
# Project created by QtCreator 2011-12-25T17:39:29
# -------------------------------------------------
QT += core \
    gui \
    opengl

TARGET = LSystemTreeGenerator

TEMPLATE = app

SOURCES += src/Parser.cpp \
    src/MainWindow.cpp \
    src/Main.cpp \
    src/LSystem.cpp \
    src/ScriptEditor.cpp \
    src/LSystemModelInterface.cpp \
    src/LSystemTextModel.cpp \
    src/ErrorListModel.cpp \
    src/GLWidget.cpp

HEADERS += src/Parser.h \
    src/MainWindow.h \
    src/LSystem.h \
    src/ScriptEditor.h \
    src/LSystemModelInterface.h \
    src/LSystemTextModel.h \
    src/ErrorListModel.h \
    src/GLWidget.h

OTHER_FILES += README

INCLUDEPATH += "D:/studia/ZPR/boost/boost_1_48_0"
