###########################################################################################
##		Created using Monkey Studio v1.8.4.0 (1.8.4.0)
##
##	Author    : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
##	Project   : libwbfs
##	FileName  : shared.pri
##	Date      : 2010-04-04T11:04:05
##	License   : GPL2
##	Comment   : Creating using Monkey Studio RAD
##	Home Page   : http://code.google.com/p/qwbfs
##
###########################################################################################

# wit defines
DEFINES *= LARGE_FILES _FILE_OFFSET_BITS=64
DEFINES *= WIT
DEFINES *= DEBUG_ASSERT
DEFINES *= EXTENDED_ERRORS=1
DEFINES *= _7ZIP_ST=1
DEFINES *= _LZMA_PROB32=1
DEFINES *= HAVE_POSIX_FALLOCATE=1 HAVE_FIEMAP=1

CONFIG(debug, debug|release) {
	#Debug
	DEFINES *= DEBUG TEST
} else {
	#Release
	DEFINES *= RELEASE IGNORE_DEBUG
	#DEFINES *= DEBUG TEST
}

LIB_WBFS_TARGET = $$quote(wbfs)
LIB_WBFS_PWD	= $$PWD
LIB_WBFS_BUILD_PATH	= $${LIB_WBFS_PWD}/../build

unix {
    UNIX_RAM_DISK   = /media/ramdisk
    exists( $${UNIX_RAM_DISK} ) {
        LIB_WBFS_BUILD_PATH    = $${UNIX_RAM_DISK}/$${LIB_WBFS_TARGET}
    }
}

INCLUDEPATH	*= \
	$${LIB_WBFS_PWD} \
    $${LIB_WBFS_PWD}/wit-src \
    $${LIB_WBFS_PWD}/wit-src/crypto \
    $${LIB_WBFS_PWD}/wit-src/libbz2 \
    $${LIB_WBFS_PWD}/wit-src/libwbfs \
    $${LIB_WBFS_PWD}/wit-src/lzma
	
DEPENDPATH	*= \
	$${LIB_WBFS_PWD} \
    $${LIB_WBFS_PWD}/wit-src \
    $${LIB_WBFS_PWD}/wit-src/crypto \
    $${LIB_WBFS_PWD}/wit-src/libbz2 \
    $${LIB_WBFS_PWD}/wit-src/libwbfs \
    $${LIB_WBFS_PWD}/wit-src/lzma

LIBS *= -lbz2
