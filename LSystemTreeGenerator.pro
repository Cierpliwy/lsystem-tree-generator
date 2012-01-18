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
    src/ErrorListModel.cpp \
    src/GLWidget.cpp \
    src/Drawable.cpp \
    src/LSystemGLModel.cpp
HEADERS += src/Parser.h \
    src/MainWindow.h \
    src/LSystem.h \
    src/ScriptEditor.h \
    src/LSystemModelInterface.h \
    src/ErrorListModel.h \
    src/GLWidget.h \
    src/Drawable.h \
    src/LSystemGLModel.h
OTHER_FILES += README \
    test/demos.lsys
INCLUDEPATH += "D:\Libs\boost_1_48_0" \
               "D:\Libs\glm-0.9.3.0"
