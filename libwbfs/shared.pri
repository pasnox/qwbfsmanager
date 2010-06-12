###########################################################################################
##		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
##
##	Author    : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
##	Project   : shared
##	FileName  : shared.pri
##	Date      : 2010-04-04T11:04:05
##	License   : GPL3
##	Comment   : Creating using Monkey Studio RAD
##	Home Page   : http://code.google.com/p/qwbfs
##
##	This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
##	WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
##
###########################################################################################

DEFINES *= LARGE_FILES _FILE_OFFSET_BITS=64
LIB_WBFS_PWD	= $$PWD
LIB_WBFS_BUILD_PATH	= $${LIB_WBFS_PWD}/../build

INCLUDEPATH	*= $${LIB_WBFS_PWD}
DEPENDPATH	*= $${LIB_WBFS_PWD}

win32 {
	isEqual( QT_ARCH, "x86_64" ):OPENSSL_INSTALL_DIR = $(HOME)/.wine/drive_c/Development/OpenSSL
	else:OPENSSL_INSTALL_DIR = D:/Developpement/OpenSSL
	#OPENSSL_INSTALL_DIR = $(HOME)/Win32Libraries
	*-g++*:LIBS	*= -L$${OPENSSL_INSTALL_DIR}/lib -L$${OPENSSL_INSTALL_DIR}/lib/MinGW
	*-msvc*:LIBS	*= -L$${OPENSSL_INSTALL_DIR}/lib -L$${OPENSSL_INSTALL_DIR}/lib/VC
	INCLUDEPATH	*= $${OPENSSL_INSTALL_DIR}/include
}