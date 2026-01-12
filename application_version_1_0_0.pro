QT       += core gui quick qml serialport  sql virtualkeyboard serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
QT_VIRTUALKEYBOARD_STYLE=test virtualkeyboard

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    bluroverlayguard.cpp \
    cardiacoutputdialog.cpp \
    circularprogressbar.cpp \
    customcombobox.cpp \
    customkeyboard.cpp \
    custommessagebox.cpp \
    customyscaledraw.cpp \
    customzoomer.cpp \
    databasemanager.cpp \
    debugmodeselector.cpp \
    deviceacquisitionworker.cpp \
    exitconfirmdialog.cpp \
    global.cpp \
    implantmonitor.cpp \
    languagemanager.cpp \
    measurementdataprocessor.cpp \
    medicallogger.cpp \
    modernwaveplot.cpp \
    readoutrecorddialog.cpp \
    rhcinputdialog.cpp \
    serialdebugwidget.cpp \
    serialmanager.cpp \
    settingswidget.cpp \
    splashscreen.cpp \
    touchdateedit.cpp \
    toucheventhandler.cpp \
    udpdebugwidget.cpp \
    udpmanager.cpp \
    updatemanager.cpp \
    main.cpp

HEADERS += \
    bluroverlayguard.h \
    cardiacoutputdialog.h \
    circularprogressbar.h \
    closeonlywindow.h \
    customcombobox.h \
    customkeyboard.h \
    custommessagebox.h \
    customyscaledraw.h \
    customzoomer.h \
    databasemanager.h \
    databuffer.h \
    debugmodeselector.h \
    deviceacquisitionworker.h \
    exitconfirmdialog.h \
    framelesswindow.h \
    global.h \
    implantmonitor.h \
    languagemanager.h \
    measurementconfig.h \
#    measuremeFntdata.h \
    measurementdataprocessor.h \
    medicallogger.h \
    modernwaveplot.h \
    readoutrecorddialog.h \
    rhcinputdialog.h \
    serialdebugwidget.h \
    serialmanager.h \
    settingswidget.h \
    splashscreen.h \
    thread_work_state.h \
    touchdateedit.h \
    toucheventhandler.h \
    udpdebugwidget.h \
    udpmanager.h \
    updatemanager.h

FORMS += \
#    historycheck.ui \

TRANSLATIONS += translations/zh_CN.ts \
               translations/en_US.ts


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images.qrc \
    qml.qrc


DISTFILES += \
    Lang/zh_CN.ts \
    translations/en_US.ts \
    translations/zh_CN.ts


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../qt_5_12_9/5.12.9/mingw73_64/lib/ -lqwt
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../qt_5_12_9/5.12.9/mingw73_64/lib/ -lqwtd
else:unix: LIBS += -L$$PWD/../../qt_5_12_9/5.12.9/mingw73_64/lib/ -lqwt

INCLUDEPATH += $$PWD/../../qt_5_12_9/5.12.9/mingw73_64/include/Qwt
DEPENDPATH += $$PWD/../../qt_5_12_9/5.12.9/mingw73_64/include/Qwt

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../qt_5_12_9/5.12.9/mingw73_64/lib/ -lqwt
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../qt_5_12_9/5.12.9/mingw73_64/lib/ -lqwtd
else:unix: LIBS += -L$$PWD/../../qt_5_12_9/5.12.9/mingw73_64/lib/ -lqwt

INCLUDEPATH += $$PWD/../../qt_5_12_9/5.12.9/mingw73_64/include/Qwt
DEPENDPATH += $$PWD/../../qt_5_12_9/5.12.9/mingw73_64/include/Qwt
