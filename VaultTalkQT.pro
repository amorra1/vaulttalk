QT       += core gui websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chatroom.cpp \
    encryption.cpp \
    main.cpp \
    mainwindow.cpp \
    message.cpp \
    networking.cpp \
    user.cpp

HEADERS += \
    chatroom.h \
    encryption.h \
    mainwindow.h \
    message.h \
    networking.h \
    user.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

LIBS += -L$$PWD/gmp_compiled/lib -lgmpxx -lgmp

INCLUDEPATH += $$PWD/gmp_compiled/inc

DEPENDPATH += $$PWD/gmp_compiled/inc
PRE_TARGETDEPS += $$PWD/gmp_compiled/lib/libgmpxx.a
PRE_TARGETDEPS += $$PWD/gmp_compiled/lib/libgmp.a

DISTFILES += \
    aes_keyfile

RESOURCES += \
    imageResources.qrc
