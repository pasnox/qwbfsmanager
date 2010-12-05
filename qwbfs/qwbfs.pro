###########################################################################################
##		Created using Monkey Studio v1.8.4.0 (1.8.4.0)
##
##	Author    : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
##	Project   : qwbfs
##	FileName  : qwbfs.pro
##	Date      : 2010-04-04T11:04:05
##	License   : GPL2
##	Comment   : Creating using Monkey Studio RAD
##	Home Page   : http://code.google.com/p/qwbfs
##
###########################################################################################

TEMPLATE	= app
LANGUAGE	= C++/Qt4
TARGET	= $$quote(qwbfsmanager)
mac:TARGET	= $$quote(QWBFSManager)
CONFIG	*= qt resources warn_on thread x11 windows embed_manifest_exe app_bundle
QT	= core gui network xml
BUILD_PATH	= ../build
DESTDIR	= ../bin

include( ../shared.pri )
include( ../libwbfs/libwbfs.pri )

exists( ../fresh/fresh.pro ) {
	FRESH_LIBRARY_PATH = ../fresh
	!build_pass:message( "Using bundled fresh library." )
} else:exists( ../../../fresh/fresh.pro ) {
	FRESH_LIBRARY_PATH = ../../../fresh
	!build_pass:message( "Using external fresh library." )
} else {
	!build_pass:error( "Fresh library not found - download from http://bettercodes.org/projects/fresh and uncompress in ROOT/fresh folder." )
}

include( $${FRESH_LIBRARY_PATH}/fresh.pri )

# define some usefull values
QMAKE_TARGET_COMPANY	= "QWBFS Team"
QMAKE_TARGET_PRODUCT	= "QWBFS Manager"
QMAKE_TARGET_DESCRIPTION	= "The Free, Fast and Powerful cross platform Wii Backup File System manager"
QMAKE_TARGET_COPYRIGHT	= "(C) 2010 Filipe Azevedo"
PACKAGE_DOMAIN	= "code.google.com/p/qwbfs"
PACKAGE_DOWNLOADS_FEED	= "http://code.google.com/feeds/p/qwbfs/downloads/basic"
PACKAGE_REPORT_BUG_URL	= "http://code.google.com/p/qwbfs/issues/list"
PACKAGE_DISCUSS_URL	= "http://groups.google.com/group/qwbfs-discuss"
PACKAGE_VERSION	= 1.2.0
SVN_REVISION	= $$system( export LANG=C && [ -f /usr/bin/svnversion ] && svnversion $$PWD/.. )

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
	"_APPLICATION_ORGANIZATION=\"\\\"$${QMAKE_TARGET_COMPANY}\\\"\"" \
	"_APPLICATION_DOWNLOADS_FEED=\"\\\"$${PACKAGE_DOWNLOADS_FEED}\\\"\"" \
	"_APPLICATION_REPORT_BUG_URL=\"\\\"$${PACKAGE_REPORT_BUG_URL}\\\"\"" \
	"_APPLICATION_DISCUSS_URL=\"\\\"$${PACKAGE_DISCUSS_URL}\\\"\""

INCLUDEPATH	*= . \
	src \
	src/fresh

DEPENDPATH	*= src \
	src/models \
	src/qwbfsdriver \
	src/wiitdb

mac:ICON	= resources/qwbfsmanager.icns
win32:RC_FILE	= resources/qwbfsmanager.rc

RESOURCES	= resources/resources.qrc

TRANSLATIONS	*= ../translations/qwbfsmanager-ru_RU.ts \
	../translations/qwbfsmanager-sl_SI.ts \
	../translations/qwbfsmanager-pl_PL.ts \
	../translations/qwbfsmanager-zh_CN.ts \
	../translations/qwbfsmanager-he_IL.ts \
	../translations/qwbfsmanager-da_DK.ts \
	../translations/qwbfsmanager-sk_SK.ts \
	../translations/qwbfsmanager-ja_JP.ts \
	../translations/qwbfsmanager-uk_UA.ts \
	../translations/qwbfsmanager-cs_CZ.ts \
	../translations/qwbfsmanager-ca_ES.ts \
	../translations/qwbfsmanager-ar_SA.ts \
	../translations/qwbfsmanager-fr_FR.ts \
	../translations/qwbfsmanager-zh_TW.ts \
	../translations/qwbfsmanager-es_ES.ts \
	../translations/qwbfsmanager-de_DE.ts \
	../translations/qwbfsmanager-pt_PT.ts \
	../translations/qwbfsmanager-sv_SE.ts

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
	src/PropertiesDialog.cpp \
	src/Properties.cpp

include( installs.pri )