QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    add_friend.cpp \
    add_group.cpp \
    chatlist.cpp \
    create_group.cpp \
    group_chat.cpp \
    main.cpp \
    private_chat.cpp \
    recvfile.cpp \
    recvfile_thread.cpp \
    sendfile_thread.cpp \
    widget.cpp

HEADERS += \
    add_friend.h \
    add_group.h \
    chatlist.h \
    create_group.h \
    group_chat.h \
    private_chat.h \
    recvfile.h \
    recvfile_thread.h \
    sendfile_thread.h \
    widget.h

FORMS += \
    add_friend.ui \
    add_group.ui \
    chatlist.ui \
    create_group.ui \
    group_chat.ui \
    private_chat.ui \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
