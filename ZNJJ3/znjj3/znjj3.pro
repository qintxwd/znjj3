QT += core network serialport xml multimedia
QT -= gui

TARGET = znjj3
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    arm.cpp \
    chest.cpp \
    configure.cpp \
    eyes.cpp \
    gochargethread.cpp \
    gopositionthread.cpp \
    motor.cpp \
    qyhtts.cpp \
    qyhttsthread.cpp \
    robot.cpp \
    serial.cpp \
    stargazer.cpp \
    stargazermap.cpp \
    stm32.cpp \
    tcpthread.cpp \
    usercommandtcpserver.cpp

HEADERS += \
    arm.h \
    chest.h \
    configure.h \
    eyes.h \
    gochargethread.h \
    gopositionthread.h \
    motor.h \
    qyhtts.h \
    qyhttsthread.h \
    robot.h \
    serial.h \
    stargazer.h \
    stargazermap.h \
    stm32.h \
    tcpthread.h \
    usercommandtcpserver.h

