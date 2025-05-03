# Bar.pro
QT += core gui qml quick
RC_ICONS = app.ico

greaterThan(QT_MAJOR_VERSION, 6): QT += widgets

CONFIG += c++17

SOURCES += \
    src/main.cpp \
    src/menuitemmodel.cpp \
    src/menucontroller.cpp \
    src/topbarcontroller.cpp \
    src/windowsstructures.cpp \
    src/windowsstructures.cpp

HEADERS += \
    src/menuitemmodel.hpp \
    src/menucontroller.hpp \
    src/topbarcontroller.hpp \
    src/windowsapi.hpp \
    src/windowsstructures.hpp

RESOURCES += \
    res/shared.qrc

# Windows specific


win32 {
    LIBS += \
        -luser32 \
        -lshell32 \
        -ladvapi32 \
        -lpsapi \
        -lwbemuuid \
        -ldwmapi \
        -lversion  # Add this line for version info functions

    DEFINES += WIN32_LEAN_AND_MEAN
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
