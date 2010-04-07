###########################################################################################
##		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
##
##	Author    : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
##	Project   : qwbfs
##	FileName  : qwbfs.pro
##	Date      : 2010-04-05T04:45:27
##	License   : GPL3
##	Comment   : Creating using Monkey Studio RAD
##	Home Page   : http://code.google.com/p/qwbfs
##
##	This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
##	WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
##
###########################################################################################

TEMPLATE	= app
LANGUAGE	= C++/Qt4
TARGET	= $$quote(qwbfs)
CONFIG	+= qt resources debug_and_release warn_on thread x11 windows console embed_manifest_exe x86 ppc app_bundle
QT	= core gui xml
BUILD_PATH	= ../build
DESTDIR	= ../bin

INCLUDEPATH	+= ../libqwbfs/libwbfs \
	../libqwbfs \
	src

LIBS	+= -L$${BUILD_PATH} -lcrypto

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
	unix:LIBS	*= -lqwbfs_debug
	else:LIBS	*= -lqwbfsd
} else {
	#Release
	unix:OBJECTS_DIR	= $${BUILD_PATH}/release/.obj/unix
	win32:OBJECTS_DIR	= $${BUILD_PATH}/release/.obj/win32
	mac:OBJECTS_DIR	= $${BUILD_PATH}/release/.obj/mac
	UI_DIR	= $${BUILD_PATH}/release/.ui
	MOC_DIR	= $${BUILD_PATH}/release/.moc
	RCC_DIR	= $${BUILD_PATH}/release/.rcc
	LIBS	+= -lqwbfs
}

FORMS	+= src/UIMain.ui \
	src/PartitionWidget.ui

HEADERS	+= src/UIMain.h \
	src/qWBFS.h \
	src/DiscModel.h \
	src/Gauge.h \
	src/DiscDelegate.h \
	src/PartitionWidget.h \
	src/Disc.h

SOURCES	+= src/main.cpp \
	src/UIMain.cpp \
	src/qWBFS.cpp \
	src/DiscModel.cpp \
	src/Gauge.cpp \
	src/DiscDelegate.cpp \
	src/PartitionWidget.cpp \
	src/Disc.cpp