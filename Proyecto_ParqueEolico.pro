QT += core gui widgets 3dcore 3drender 3dinput 3dextras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Para M_PI en Windows
DEFINES += _USE_MATH_DEFINES

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    ParqueEolico.cpp \
    TurbinaEolica.cpp \
    Vista3D.cpp

HEADERS += \
    MainWindow.h \
    ParqueEolico.h \
    TurbinaEolica.h \
    Vista3D.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

TRANSLATIONS += \
    Proyecto_ParqueEolico_en_US.ts