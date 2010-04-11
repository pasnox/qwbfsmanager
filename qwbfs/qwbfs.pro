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
CONFIG	+= qt resources warn_on thread x11 windows embed_manifest_exe x86 ppc app_bundle
QT	= core gui xml
BUILD_PATH	= ../build
DESTDIR	= ../bin

include(../shared.pri)
include(../libwbfs/libwbfs.pri)

INCLUDEPATH	+= src

FORMS	+= src/UIMain.ui \
	src/PartitionWidget.ui \
	src/ProgressDialog.ui

HEADERS	+= src/UIMain.h \
	src/qWBFS.h \
	src/DiscModel.h \
	src/Gauge.h \
	src/DiscDelegate.h \
	src/PartitionWidget.h \
	src/Disc.h \
	src/ProgressDialog.h \
	src/ExportThread.h

SOURCES	+= src/main.cpp \
	src/UIMain.cpp \
	src/qWBFS.cpp \
	src/DiscModel.cpp \
	src/Gauge.cpp \
	src/DiscDelegate.cpp \
	src/PartitionWidget.cpp \
	src/Disc.cpp \
	src/ProgressDialog.cpp \
	src/ExportThread.cpp