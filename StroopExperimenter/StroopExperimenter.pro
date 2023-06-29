# Common basic configurations
QT += core gui widgets

TARGET = StroopExperimenter
TEMPLATE = app

# Require C++17 support
CONFIG += c++17

# Precompiled header support and definition
CONFIG += precompile_header
PRECOMPILED_HEADER = stdafx.h

# The following define makes the compiler emit warnings if any Qt feature is used 
# that has been marked deprecated (the exact warnings depend on the compiler).
#DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060200    # disables all the APIs deprecated before Qt 6.2.0

# Sources
#INCLUDEPATH += ./inc/

SOURCES +=  main.cpp \
            MainWindow.cpp \
            DataReaderWriter.cpp \
            Experiment.cpp \
            Experimenter.cpp \
            StroopExperiment.cpp \
            ExperimentDialog.cpp \
            StroopExperimentDialog.cpp \
            
HEADERS +=  MainWindow.h \
            DataReaderWriter.h \
            Experiment.h \
            Experimenter.h \
            StroopExperiment.h \
            ExperimentDialog.h \
            StroopExperimentDialog.h \
            
FORMS +=    MainWindow.ui
