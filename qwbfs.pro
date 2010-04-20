XUPProjectSettings {
	EDITOR	= QMake
	OTHERS_PLATFORM_TARGET_DEBUG	= bin/qwbfs_debug
	QT_VERSION	= Qt System (4.6.0)
	OTHERS_PLATFORM_TARGET_RELEASE	= bin/qwbfs
	WINDOWS_PLATFORM_TARGET_DEBUG	= bin/qwbfsd.exe
}

TEMPLATE	= subdirs
CONFIG	+= ordered
SUBDIRS	+= libwbfs \
	qwbfs
BUILD_PATH	= build

include( shared.pri )