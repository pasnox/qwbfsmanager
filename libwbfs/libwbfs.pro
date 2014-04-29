###########################################################################################
##		Created using Monkey Studio v1.8.4.0 (1.8.4.0)
##
##	Author    : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
##	Project   : libwbfs
##	FileName  : libwbfs.pro
##	Date      : 2010-04-04T11:04:05
##	License   : GPL2
##	Comment   : Creating using Monkey Studio RAD
##	Home Page   : http://code.google.com/p/qwbfs
##
###########################################################################################
TEMPLATE = lib
LANGUAGE = C++/Qt4
CONFIG -= release debug debug_and_release warn_on warn_off qt
CONFIG *= static release warn_on
include( shared.pri )
TARGET = $${LIB_WBFS_TARGET}
BUILD_PATH = $${LIB_WBFS_BUILD_PATH}
DESTDIR = $${BUILD_PATH}
#win32:DEFINES -= UNICODE

QMAKE_CFLAGS_RELEASE *= -fomit-frame-pointer -fno-strict-aliasing -funroll-loops -Wall -Wno-parentheses -Wno-unused-function -O3
QMAKE_CFLAGS_DEBUG *= -fomit-frame-pointer -fno-strict-aliasing -funroll-loops -Wall -Wno-parentheses -Wno-unused-function

QMAKE_CXXFLAGS_RELEASE *= -fomit-frame-pointer -fno-strict-aliasing -funroll-loops -Wall -Wno-parentheses -Wno-unused-function -O3
QMAKE_CXXFLAGS_DEBUG *= -fomit-frame-pointer -fno-strict-aliasing -funroll-loops -Wall -Wno-parentheses -Wno-unused-function

include( ../shared.pri )

INCLUDEPATH *= \
    . \
    wit-src \
    wit-src/crypto \
    wit-src/libbz2 \
    wit-src/libwbfs \
    wit-src/lzma

include( wit-src.pri )