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

XUPProjectSettings {
	EDITOR	= QMake
	QT_VERSION	= 4.6.2-universal
	OTHERS_PLATFORM_TARGET_RELEASE	= bin/qwbfsmanager
	OTHERS_PLATFORM_TARGET_DEBUG	= bin/qwbfs_debug
	WINDOWS_PLATFORM_TARGET_DEBUG	= bin/qwbfsd.exe
	MAC_PLATFORM_TARGET_RELEASE	= bin/QWBFSManager.app
}

TEMPLATE	= subdirs
CONFIG	*= ordered

BUILD_PATH	= build
include( shared.pri )

fresh_static|fresh_shared {
	!build_pass:message( "Using system fresh library." )
} else {
	exists( fresh/fresh.pro ) {
		SUBDIRS	*= fresh
		!build_pass:message( "Using bundled fresh library." )
	} else:exists( ../../fresh/fresh.pro ) {
		!build_pass:message( "Using external fresh library." )
		SUBDIRS	*= ../../fresh
	} else {
		!build_pass:error( "Fresh library not found - download from http://bettercodes.org/projects/fresh and uncompress in ROOT/fresh folder." )
	}
}

SUBDIRS	*= libwbfs \
	qwbfs