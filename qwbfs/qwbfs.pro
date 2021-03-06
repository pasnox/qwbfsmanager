###########################################################################################
##      Created using Monkey Studio v1.8.4.0 (1.8.4.0)
##
##  Author     : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
##  Project    : qwbfs
##  FileName   : qwbfs.pro
##  Date       : 2010-04-04T11:04:05
##  License    : GPL2
##  Comment    : Creating using Monkey Studio RAD
##  Home Page  : https://github.com/pasnox/qwbfsmanager
##
###########################################################################################

TEMPLATE = app
LANGUAGE = C++/Qt4
TARGET = $$quote( qwbfsmanager )
macx:TARGET = $$quote( QWBFSManager )
BUILD_PATH = ../build
DESTDIR = ../bin

include( ../shared.pri )

CONFIG *= warn_on thread x11 windows qt resources embed_manifest_exe app_bundle
QT *= core gui network xml
greaterThan( QT_MAJOR_VERSION, 4 ):QT *= widgets

include( ../libwbfs/libwbfs.pri )

win32_crossbuild {
    exists( "$(QT_WIN32_PATH)/lib/*fresh*" ):CONFIG *= fresh
} else {
    exists( "$$[QT_INSTALL_LIBS]/*fresh*" ):CONFIG *= fresh
}

fresh {
    !build_pass:message( "Using system fresh library." )
} else {
    exists( ../fresh.git/fresh.pro ) {
        !build_pass:message( "Using bundled fresh library." )
        FRESH_PATH = ../fresh.git

        include( $$FRESH_PATH/qmake-extensions.git/qmake-extensions.pri )

        FRESH_SOURCES_PATHS = $$getFolders( $$PWD/$$FRESH_PATH/src )

        DEFINES *= FRESH_CORE_BUILD

        DEPENDPATH *= $${FRESH_SOURCES_PATHS}
        DEPENDPATH *= $$FRESH_PATH/include/FreshCore \
            $$FRESH_PATH/include/FreshGui

        INCLUDEPATH *= $$FRESH_PATH/include
        INCLUDEPATH *= $${FRESH_SOURCES_PATHS}

        PRE_TARGETDEPS *= $${FRESH_PATH}

        QMAKE_RPATHDIR *= $$OUT_PWD/$$FRESH_PATH/build
        macx:LIBS *= -F$$OUT_PWD/$$FRESH_PATH/build
        LIBS *= -L$$OUT_PWD/$$FRESH_PATH/build

        QT *= xml network
        CONFIG( debug, debug|release ) {
            # Debug
            unix:LIBS *= -lfresh_debug
            else:LIBS *= -lfreshd
        } else {
            # Release
            LIBS *= -lfresh
        }
    } else {
        !build_pass:error( "Fresh library not found - initialize the submodules using the init-repositories script." )
    }
}

# define some usefull values
OS = $$lower( $$QMAKE_HOST.os )
QMAKE_TARGET_COMPANY = "QWBFS Team"
QMAKE_TARGET_PRODUCT = "QWBFS Manager"
QMAKE_TARGET_DESCRIPTION = "The Free, Fast and Powerful cross platform Wii Backup File System manager"
greaterThan( QT_MAJOR_VERSION, 4 ) {
    QMAKE_TARGET_COPYRIGHT = "© 2010 - 2016 Filipe Azevedo"
} else {
    QMAKE_TARGET_COPYRIGHT = "\\251 2010 - 2016 Filipe Azevedo"
}
PACKAGE_DOMAIN = "github.com/pasnox/qwbfsmanager"
PACKAGE_DOWNLOADS_FEED = "http://to-be-changed"
PACKAGE_REPORT_BUG_URL = "https://github.com/pasnox/qwbfsmanager/issues"
PACKAGE_DISCUSS_URL = "http://groups.google.com/group/qwbfs-discuss"
PACKAGE_VERSION = 1.2.6
!isEqual( OS, "windows" ):exists( "$$PWD/../.git" ):SVN_REVISION = $$system( export LANG=C && [ -f /usr/bin/git ] && git --git-dir="$$PWD/../.git" describe )
isEmpty( SVN_REVISION ):SVN_REVISION = $(SVN_REVISION)
isEmpty( SVN_REVISION ):SVN_REVISION = "N/C"

!isEqual( OS, "windows" ):system( touch $$PWD/src/main.h )

isEqual( SVN_REVISION, "N/C" ) {
    CONFIG( debug, debug|release ) {
        PACKAGE_VERSION_STR = $${PACKAGE_VERSION} (debug)
    } else {
        PACKAGE_VERSION_STR = $${PACKAGE_VERSION} (release)
    }
} else {
    CONFIG( debug, debug|release ) {
        PACKAGE_VERSION_STR = $${PACKAGE_VERSION} ($$SVN_REVISION debug)
    } else {
        PACKAGE_VERSION_STR = $${PACKAGE_VERSION} ($$SVN_REVISION release)
    }
}

# define variable for source code
DEFINES *= "_APPLICATION_NAME=\"\\\"$${QMAKE_TARGET_PRODUCT}\\\"\"" \
    "_APPLICATION_VERSION=\"\\\"$${PACKAGE_VERSION}\\\"\"" \
    "_APPLICATION_VERSION_STR=\"\\\"$${PACKAGE_VERSION_STR}\\\"\"" \
    "_APPLICATION_DOMAIN=\"\\\"$${PACKAGE_DOMAIN}\\\"\"" \
    "_APPLICATION_COPYRIGHTS=\"\\\"$${QMAKE_TARGET_COPYRIGHT}\\\"\"" \
    "_APPLICATION_DESCRIPTION=\"\\\"$${QMAKE_TARGET_DESCRIPTION}\\\"\"" \
    "_APPLICATION_ORGANIZATION=\"\\\"$${QMAKE_TARGET_COMPANY}\\\"\"" \
    "_APPLICATION_DOWNLOADS_FEED=\"\\\"$${PACKAGE_DOWNLOADS_FEED}\\\"\"" \
    "_APPLICATION_REPORT_BUG_URL=\"\\\"$${PACKAGE_REPORT_BUG_URL}\\\"\"" \
    "_APPLICATION_DISCUSS_URL=\"\\\"$${PACKAGE_DISCUSS_URL}\\\"\""

INCLUDEPATH *= src

DEPENDPATH *= src \
    src/models \
    src/qwbfsdriver \
    src/wiitdb

macx:ICON = resources/qwbfsmanager.icns
win32:RC_FILE = resources/qwbfsmanager.rc

RESOURCES = resources/resources.qrc

XUP.TRANSLATIONS_BASENAME = qwbfsmanager
XUP.TRANSLATIONS_DIRECTORY = ../translations

TRANSLATIONS *= ../translations/qwbfsmanager-ru_RU.ts \
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
    ../translations/qwbfsmanager-sv_SE.ts \
    ../translations/qwbfsmanager-it_IT.ts

FORMS *= src/UIMain.ui \
    src/PartitionWidget.ui \
    src/ProgressDialog.ui \
    src/UIAbout.ui \
    src/PropertiesDialog.ui

HEADERS *= src/main.h \
    src/Utils.h \
    src/UIMain.h \
    src/Gauge.h \
    src/PartitionWidget.h \
    src/ProgressDialog.h \
    src/WorkerThread.h \
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
    src/Properties.h \
    src/ListView.h \
    src/ofi-labs-pictureflow/pictureflow-qt/pictureflow.h \
    src/CoverFlowView.h \
    src/models/pPartitionModel.h \
    src/models/PartitionDelegate.h \
    src/PartitionComboBox.h \
    src/models/pPartition.h

SOURCES *= src/main.cpp \
    src/Utils.cpp \
    src/UIMain.cpp \
    src/Gauge.cpp \
    src/PartitionWidget.cpp \
    src/ProgressDialog.cpp \
    src/WorkerThread.cpp \
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
    src/Properties.cpp \
    src/ListView.cpp \
    src/ofi-labs-pictureflow/pictureflow-qt/pictureflow.cpp \
    src/CoverFlowView.cpp \
    src/models/pPartitionModel.cpp \
    src/models/PartitionDelegate.cpp \
    src/PartitionComboBox.cpp \
    src/models/pPartition.cpp

linux* {
    QT *= dbus
    LIBS *= -ludev
    SOURCES *= src/models/pPartitionModel_linux.cpp
}

macx {
    LIBS *= "-framework DiskArbitration" "-framework CoreServices"
    OBJECTIVE_SOURCES *= src/models/pPartitionModel_mac.mm
}

win32 {
    SOURCES *= src/models/pPartitionModel_win.cpp
}

include( installs.pri )
