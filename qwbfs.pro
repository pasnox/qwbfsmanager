XUPProjectSettings {
	EDITOR	= QMake
	QT_VERSION	= Qt System (4.6.3)
	OTHERS_PLATFORM_TARGET_RELEASE	= bin/qwbfs
	OTHERS_PLATFORM_TARGET_DEBUG	= bin/qwbfs_debug
	WINDOWS_PLATFORM_TARGET_DEBUG	= bin/qwbfsd.exe
	MAC_PLATFORM_TARGET_RELEASE	= bin/QWBFSManager.app
}

TEMPLATE	= subdirs
CONFIG	+= ordered

BUILD_PATH	= build
include( shared.pri )

SUBDIRS	+= libwbfs \
	qwbfs
