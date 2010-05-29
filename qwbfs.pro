XUPProjectSettings {
	EDITOR	= QMake
	QT_VERSION	= 4.7.0-beta1
	OTHERS_PLATFORM_TARGET_RELEASE	= bin/qwbfs
	OTHERS_PLATFORM_TARGET_DEBUG	= bin/qwbfs_debug
	WINDOWS_PLATFORM_TARGET_DEBUG	= bin/qwbfsd.exe
	MAC_PLATFORM_TARGET_RELEASE	= bin/QWBFS Manager.app
}

TEMPLATE	= subdirs
CONFIG	+= ordered

BUILD_PATH	= build
include( shared.pri )

SUBDIRS	+= libwbfs \
	qwbfs
