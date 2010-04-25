###########################################################################################
##		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
##
##	Author    : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
##	Project   : libwbfs
##	FileName  : libwbfs.pri
##	Date      : 2010-04-04T11:04:05
##	License   : GPL3
##	Comment   : Creating using Monkey Studio RAD
##	Home Page   : http://code.google.com/p/qwbfs
##
##	This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
##	WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
##
###########################################################################################

include($$PWD/shared.pri)
LIBS	*= -L$${LIB_WBFS_BUILD_PATH}

CONFIG(debug, debug|release) {
	#Debug
	unix:LIBS	*= -lwbfs_debug
	else:LIBS	*= -lwbfsd
} else {
	#Release
	LIBS	*= -lwbfs
}

win32:LIBS	*= -lws2_32 -leay32
else:LIBS	*=	-lcrypto