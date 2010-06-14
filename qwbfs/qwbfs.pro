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

# define some usefull values
QMAKE_TARGET_COMPANY	= "QWBFS Team"
QMAKE_TARGET_PRODUCT	= "QWBFS Manager"
QMAKE_TARGET_DESCRIPTION	= "The Free, Fast and Powerfull cross platform Wii Backup File System manager"
QMAKE_TARGET_COPYRIGHT	= "(C) 2010 Filipe AZEVEDO"
PACKAGE_DOMAIN	= "code.google.com/p/qwbfs"
PACKAGE_VERSION	= 1.0.0
SVN_REVISION	= $$system( export LANG="" && svnversion $$PWD/.. )

system( touch $$PWD/src/main.h )

CONFIG( debug, debug|release ) {
	PACKAGE_VERSION_STR	= $${PACKAGE_VERSION} (svn$$SVN_REVISION debug)
} else {
	PACKAGE_VERSION_STR	= $${PACKAGE_VERSION} (svn$$SVN_REVISION release)
}

# define variable for source code
DEFINES	*= "_APPLICATION_NAME=\"\\\"$${QMAKE_TARGET_PRODUCT}\\\"\"" \
	"_APPLICATION_VERSION=\"\\\"$${PACKAGE_VERSION}\\\"\"" \
	"_APPLICATION_VERSION_STR=\"\\\"$${PACKAGE_VERSION_STR}\\\"\"" \
	"_APPLICATION_DOMAIN=\"\\\"$${PACKAGE_DOMAIN}\\\"\"" \
	"_APPLICATION_COPYRIGHTS=\"\\\"$${QMAKE_TARGET_COPYRIGHT}\\\"\"" \
	"_APPLICATION_DESCRIPTION=\"\\\"$${QMAKE_TARGET_DESCRIPTION}\\\"\"" \
	"_APPLICATION_ORGANIZATION=\"\\\"$${QMAKE_TARGET_COMPANY}\\\"\""

include( ../shared.pri )
include( ../libwbfs/libwbfs.pri )

INCLUDEPATH	*= . \
	src
DEPENDPATH	*= . \
	src \
	src/qwbfsdriver \
	src/models \
	src/WiiTDB

RESOURCES	*= resources/resources.qrc
mac:ICON	= resources/qwbfs.icns
win32:RC_FILE	*= resources/qwbfs.rc

FORMS	*= src/UIMain.ui \
	src/PartitionWidget.ui \
	src/ProgressDialog.ui \
	src/UIAbout.ui \
	src/PropertiesDialog.ui

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
	src/wiitdb/Covers.h \
	src/datacache/DataNetworkCache.h \
	src/PropertiesDialog.h \
	src/Properties.h

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
	src/wiitdb/Covers.cpp \
	src/datacache/DataNetworkCache.cpp \
	src/PropertiesDialog.cpp \
	src/Properties.cpp