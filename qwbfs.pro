###########################################################################################
##      Created using Monkey Studio v1.8.4.0 (1.8.4.0)
##
##  Author    : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
##  Project   : qwbfs
##  FileName  : qwbfs.pro
##  Date      : 2010-04-04T11:04:05
##  License   : GPL2
##  Comment   : Creating using Monkey Studio RAD
##  Home Page   : https://github.com/pasnox/qwbfsmanager
##
###########################################################################################

TEMPLATE = subdirs
CONFIG *= ordered

BUILD_PATH = build

include( shared.pri )

win32_crossbuild {
    exists( "$(QT_WIN32_PATH)/lib/*fresh*" ):CONFIG *= fresh
} else {
    exists( "$$[QT_INSTALL_LIBS]/*fresh*" ):CONFIG *= fresh
}

fresh {
    !build_pass:message( "Using system fresh library." )
} else {
    exists( fresh.git/fresh.pro ) {
        SUBDIRS *= fresh.git/fresh.pro
        !build_pass:message( "Using bundled fresh library." )
    } else {
        !build_pass:error( "Fresh library not found - initialize the submodules using the init-repositories script." )
    }
}

SUBDIRS *= libwbfs \
    qwbfs
