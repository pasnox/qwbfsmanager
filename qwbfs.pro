XUPProjectSettings {
	EDITOR	= QMake
	OTHERS_PLATFORM_TARGET_DEBUG	= bin/qwbfs_debug
	QT_VERSION	= Qt System (4.6.2)
	OTHERS_PLATFORM_TARGET_RELEASE	= bin/qwbfs
}

TEMPLATE	= subdirs
CONFIG	+= ordered debug_and_release
SUBDIRS	= libqwbfs qwbfs