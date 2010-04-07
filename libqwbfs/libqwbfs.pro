###########################################################################################
##		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
##
##	Author    : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
##	Project   : libqwbfs
##	FileName  : libqwbfs.pro
##	Date      : 2010-04-04T11:04:05
##	License   : GPL3
##	Comment   : Creating using Monkey Studio RAD
##	Home Page   : http://code.google.com/p/qwbfs
##
##	This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
##	WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
##
###########################################################################################

TEMPLATE	= lib
LANGUAGE	= C++/Qt4
TARGET	= $$quote(qwbfs)
CONFIG	+= debug_and_release staticlib
DEFINES += LARGE_FILES _FILE_OFFSET_BITS=64
LIBS += -lcrypto
BUILD_PATH	= ../build
DESTDIR	= $${BUILD_PATH}

CONFIG(debug, debug|release) {
	#Debug
	CONFIG	+= console
	unix:TARGET	= $$join(TARGET,,,_debug)
	else:TARGET	= $$join(TARGET,,,d)
	unix:OBJECTS_DIR	= $${BUILD_PATH}/debug/.obj/unix
	win32:OBJECTS_DIR	= $${BUILD_PATH}/debug/.obj/win32
	mac:OBJECTS_DIR	= $${BUILD_PATH}/debug/.obj/mac
	UI_DIR	= $${BUILD_PATH}/debug/.ui
	MOC_DIR	= $${BUILD_PATH}/debug/.moc
	RCC_DIR	= $${BUILD_PATH}/debug/.rcc
} else {
	#Release
	unix:OBJECTS_DIR	= $${BUILD_PATH}/release/.obj/unix
	win32:OBJECTS_DIR	= $${BUILD_PATH}/release/.obj/win32
	mac:OBJECTS_DIR	= $${BUILD_PATH}/release/.obj/mac
	UI_DIR	= $${BUILD_PATH}/release/.ui
	MOC_DIR	= $${BUILD_PATH}/release/.moc
	RCC_DIR	= $${BUILD_PATH}/release/.rcc
}

INCLUDEPATH	+= libwbfs

HEADERS	+= libwbfs/libwbfs.h \
	libwbfs/wiidisc.h \
	libwbfs_os.h \
	tools.h

SOURCES	+= libwbfs/libwbfs_unix.c \
	libwbfs/libwbfs_win32.c \
	libwbfs/libwbfs.c \
	libwbfs/rijndael.c \
	libwbfs/wiidisc.c \
	bn.c \
	disc_usage_table.c \
	ec.c \
	tools.c

win32 {
	HEADERS	+= win32/xgetopt.h
	SOURCES	+= win32/xgetopt.cpp
}

#negentig.c scrub.c wbfs.c