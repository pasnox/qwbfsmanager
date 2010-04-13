###########################################################################################
##		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
##
##	Author    : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
##	Project   : libwbfs
##	FileName  : libwbfs.pro
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
TARGET	= $$quote(wbfs)
CONFIG	+= staticlib
include(shared.pri)
BUILD_PATH	= $${LIB_WBFS_BUILD_PATH}
DESTDIR	= $${BUILD_PATH}

include(../shared.pri)

INCLUDEPATH	+= libwbfs

HEADERS	+= tools.h \
	wiidisc.h \
	libwbfs_os.h \
	libwbfs.h

SOURCES	+= tools.c \
	ec.c \
	bn.c \
	rijndael.c \
	wiidisc.c \
	libwbfs.c \
	libwbfs_unix.c \
	libwbfs_win32.c