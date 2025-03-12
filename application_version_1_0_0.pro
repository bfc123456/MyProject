QT       += core gui serialport sql virtualkeyboard serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    customkeyboard.cpp \
    custommessagebox.cpp \
    databasemanager.cpp \
    historycheck.cpp \
    main.cpp \
    mainwindow.cpp \
    serialportmanager.cpp

HEADERS += \
    customkeyboard.h \
    custommessagebox.h \
    databasemanager.h \
    historycheck.h \
    mainwindow.h \
    serialportmanager.h

FORMS += \
    historycheck.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../qt_5_15/qt_5_12/5.12.9/mingw73_64/lib/ -lqwt
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../qt_5_15/qt_5_12/5.12.9/mingw73_64/lib/ -lqwtd
else:unix: LIBS += -L$$PWD/../../qt_5_15/qt_5_12/5.12.9/mingw73_64/lib/ -lqwt

INCLUDEPATH += $$PWD/../../qt_5_15/qt_5_12/5.12.9/mingw73_64/include/Qwt
DEPENDPATH += $$PWD/../../qt_5_15/qt_5_12/5.12.9/mingw73_64/include/Qwt

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../qt_5_15/qt_5_12/5.12.9/mingw73_64/lib/ -lqwt
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../qt_5_15/qt_5_12/5.12.9/mingw73_64/lib/ -lqwtd
else:unix: LIBS += -L$$PWD/../../qt_5_15/qt_5_12/5.12.9/mingw73_64/lib/ -lqwt

INCLUDEPATH += $$PWD/../../qt_5_15/qt_5_12/5.12.9/mingw73_64/include/Qwt
DEPENDPATH += $$PWD/../../qt_5_15/qt_5_12/5.12.9/mingw73_64/include/Qwt
