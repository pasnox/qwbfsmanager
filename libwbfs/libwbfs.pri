###########################################################################################
##		Created using Monkey Studio v1.8.4.0 (1.8.4.0)
##
##	Author    : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
##	Project   : libwbfs
##	FileName  : libwbfs.pri
##	Date      : 2010-04-04T11:04:05
##	License   : GPL2
##	Comment   : Creating using Monkey Studio RAD
##	Home Page   : http://code.google.com/p/qwbfs
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
