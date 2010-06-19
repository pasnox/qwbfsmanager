###########################################################################################
##		Created using Monkey Studio v1.8.4.0 (1.8.4.0)
##
##	Author    : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
##	Project   : libwbfs
##	FileName  : shared.pri
##	Date      : 2010-04-04T11:04:05
##	License   : GPL3
##	Comment   : Creating using Monkey Studio RAD
##	Home Page   : http://code.google.com/p/qwbfs
##
###########################################################################################

DEFINES *= LARGE_FILES _FILE_OFFSET_BITS=64
LIB_WBFS_PWD	= $$PWD
LIB_WBFS_BUILD_PATH	= $${LIB_WBFS_PWD}/../build

INCLUDEPATH	*= $${LIB_WBFS_PWD}
DEPENDPATH	*= $${LIB_WBFS_PWD}

win32 {
	OS=$$system( "uname -s" )
	
	OPENSSL_INSTALL_DIR = D:/Developpement/OpenSSL
	isEqual( OS, "Darwin" ):OPENSSL_INSTALL_DIR	= $(HOME)/Win32Libraries
	else:isEqual( OS, "Linux" ):OPENSSL_INSTALL_DIR	= $(HOME)/.wine/drive_c/Development/OpenSSL
	
	*-g++*:LIBS	*= -L$${OPENSSL_INSTALL_DIR}/lib -L$${OPENSSL_INSTALL_DIR}/lib/MinGW
	*-msvc*:LIBS	*= -L$${OPENSSL_INSTALL_DIR}/lib -L$${OPENSSL_INSTALL_DIR}/lib/VC
	INCLUDEPATH	*= $${OPENSSL_INSTALL_DIR}/include
}