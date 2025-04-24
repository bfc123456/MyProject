QT       += core gui serialport sql virtualkeyboard serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

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
    calibrationdialog.cpp \
    cardiacoutputdialog.cpp \
    circularprogressbar.cpp \
    customdateedit.cpp \
    customkeyboard.cpp \
    custommessagebox.cpp \
    databasemanager.cpp \
    exitconfirmdialog.cpp \
    followupform.cpp \
#    historycheck.cpp \
    implantationsite.cpp \
    implantdatabase.cpp \
    implantinfowidget.cpp \
    implantmonitor.cpp \
    languagemanager.cpp \
    loginwindow.cpp \
    main.cpp \
    maintenancewidget.cpp \
    measurewidget.cpp \
    modernwaveplot.cpp \
    patientlistwidget.cpp \
    readoutrecorddialog.cpp \
    reviewwidget.cpp \
    rhcinputdialog.cpp \
    serialmanager.cpp \
    settingswidget.cpp \
    signalstrength.cpp \
    toucheventhandler.cpp \
    udpmanager.cpp

HEADERS += \
    calibrationdialog.h \
    cardiacoutputdialog.h \
    circularprogressbar.h \
    customdateedit.h \
    customkeyboard.h \
    custommessagebox.h \
    databasemanager.h \
    exitconfirmdialog.h \
    followupform.h \
#    historycheck.h \
    global.h \
    implantationsite.h \
    implantdatabase.h \
    implantinfowidget.h \
    implantmonitor.h \
    languagemanager.h \
    loginwindow.h \
    maintenancewidget.h \
    measurewidget.h \
    modernwaveplot.h \
    patientlistwidget.h \
    readoutrecorddialog.h \
    reviewwidget.h \
    rhcinputdialog.h \
    serialmanager.h \
    settingswidget.h \
    signalstrength.h \
    toucheventhandler.h \
    udpmanager.h

FORMS += \
#    historycheck.ui \

TRANSLATIONS += translations/zh_CN.ts \
               translations/en_US.ts


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    images.qrc


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
