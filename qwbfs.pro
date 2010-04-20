XUPProjectSettings {
	EDITOR	= QMake
	OTHERS_PLATFORM_TARGET_DEBUG	= bin/qwbfs_debug
	QT_VERSION	= Windows Crossbuild Qt 4.6.x
	OTHERS_PLATFORM_TARGET_RELEASE	= bin/qwbfs
}

TEMPLATE	= subdirs
CONFIG	+= ordered
SUBDIRS	+= libwbfs \
	qwbfs
BUILD_PATH	= build

include( shared.pri )