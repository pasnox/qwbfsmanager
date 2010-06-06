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
mac:TARGET	= $$quote(QWBFSManager)
CONFIG	*= qt resources warn_on thread x11 windows embed_manifest_exe app_bundle
QT	= core gui network xml
BUILD_PATH	= ../build
DESTDIR	= ../bin

include( ../shared.pri )
include( ../libwbfs/libwbfs.pri )

INCLUDEPATH	+= . \
	src
DEPENDPATH	+= . \
	src \
	src/qwbfsdriver \
	src/models \
	src/WiiTDB

RESOURCES	*= resources/resources.qrc
mac:ICON	= resources/qwbfs.icns

FORMS	*= src/UIMain.ui \
	src/PartitionWidget.ui \
	src/ProgressDialog.ui \
	src/UIAbout.ui

HEADERS	*= src/main.h \
	src/UIMain.h \
	src/Gauge.h \
	src/PartitionWidget.h \
	src/ProgressDialog.h \
	src/ExportThread.h \
	src/qwbfsdriver/PartitionDiscHandle.h \
	src/qwbfsdriver/PartitionHandle.h \
	src/qwbfsdriver/PartitionProperties.h \
	src/qwbfsdriver/PartitionStatus.h \
	src/models/Disc.h \
	src/models/DiscDelegate.h \
	src/models/DiscModel.h \
	src/qwbfsdriver/Driver.h \
	src/UIAbout.h \
	src/wiitdb/Covers.h

SOURCES	*= src/main.cpp \
	src/UIMain.cpp \
	src/Gauge.cpp \
	src/PartitionWidget.cpp \
	src/ProgressDialog.cpp \
	src/ExportThread.cpp \
	src/qwbfsdriver/PartitionDiscHandle.cpp \
	src/qwbfsdriver/PartitionHandle.cpp \
	src/qwbfsdriver/PartitionProperties.cpp \
	src/qwbfsdriver/PartitionStatus.cpp \
	src/models/Disc.cpp \
	src/models/DiscDelegate.cpp \
	src/models/DiscModel.cpp \
	src/qwbfsdriver/Driver.cpp \
	src/UIAbout.cpp \
	src/wiitdb/Covers.cpp