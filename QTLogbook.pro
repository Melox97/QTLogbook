QT += core gui widgets sql network

CONFIG += c++17

TARGET = QTLogbook
TEMPLATE = app

# Definizioni del preprocessore
DEFINES += QT_DEPRECATED_WARNINGS QT_DISABLE_DEPRECATED_BEFORE=0x060000

# File sorgenti
SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/contact.cpp \
    src/database.cpp \
    src/apiservice.cpp \
    src/logbookmodel.cpp \
    src/setupdialog.cpp \
    src/settingsdialog.cpp \
    src/adifhandler.cpp

# File header
HEADERS += \
    src/mainwindow.h \
    src/contact.h \
    src/database.h \
    src/apiservice.h \
    src/logbookmodel.h \
    src/setupdialog.h \
    src/settingsdialog.h \
    src/adifhandler.h

# Risorse
RESOURCES += resources.qrc

# Directory di output
DESTDIR = $$PWD/bin
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR = $$PWD/build/moc
RCC_DIR = $$PWD/build/rcc
UI_DIR = $$PWD/build/ui

# Configurazioni specifiche per piattaforma
win32 {
    CONFIG += windows
    RC_ICONS = icon.ico
}

macx {
    ICON = icon.icns
    QMAKE_INFO_PLIST = Info.plist
}

unix:!macx {
    target.path = /usr/local/bin
    INSTALLS += target
}

# Ottimizzazioni per il rilascio
RELEASE:QMAKE_CXXFLAGS += -O2
DEBUG:QMAKE_CXXFLAGS += -g -O0