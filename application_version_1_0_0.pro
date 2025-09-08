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
    CalibrationDialog.cpp \
    CardiaCoutputDialog.cpp \
    CircularProgressBar.cpp \
    CustomComboBox.cpp \
    CustomDateEdit.cpp \
    CustomKeyBoard.cpp \
    CustomMessageBox.cpp \
    DatabaseManager.cpp \
    DebugModeselector.cpp \
    DeviceAcquisitionWorker.cpp \
    ExitconfirmDialog.cpp \
    Global.cpp \
    ImplantAtionSite.cpp \
    ImplantMonitor.cpp \
    ImplantRegistrationWidget.cpp \
    LanguageManager.cpp \
    MeasurementDataProcessor.cpp \
    MeasurementDialog.cpp \
    MeasurementTrendWidget.cpp \
    MedicalLogger.cpp \
    ModernWavePlot.cpp \
    PatientSignalStrengthWidget.cpp \
    ReadoutrecordDialog.cpp \
    ReviewWidget.cpp \
    RhcinputDialog.cpp \
    SerialDebugWidget.cpp \
    SerialManager.cpp \
    SettingsWidget.cpp \
    TouchEventHandler.cpp \
    UdpDebugWidget.cpp \
    MultiUserLoginWindow.cpp \
    UdpManager.cpp \
    UpdateManager.cpp \
    main.cpp

HEADERS += \
    CalibrationDialog.h \
    CardiacoutputDialog.h \
    CircularProgressbar.h \
    CloseOnlyWindow.h \
    CustomCombobox.h \
    CustomDateedit.h \
    CustomMessagebox.h \
    Customkeyboard.h \
    DatabaseManager.h \
    DebugModeSelector.h \
    DeviceAcquisitionWorker.h \
    ExitconfirmDialog.h \
    FramelessWindow.h \
    Global.h \
    ImplantAtionSite.h \
    ImplantMonitor.h \
    ImplantRegistrationWidget.h \
    LanguageManager.h \
    MeasurementConfig.h \
    MeasurementData.h \
    MeasurementDataProcessor.h \
    MeasurementDialog.h \
    MeasurementTrendWidget.h \
    MedicalLogger.h \
    ModernWaveplot.h \
    PatientSignalStrengthWidget.h \
    ReadoutrecordDialog.h \
    ReviewWidget.h \
    RhcinputDialog.h \
    SerialDebugWidget.h \
    SerialManager.h \
    SettingsWidget.h \
    TouchEventHandler.h \
    UdpDebugWidget.h \
    UdpManager.h \
    MultiUserLoginWindow.h \
    UpdateManager.h

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
