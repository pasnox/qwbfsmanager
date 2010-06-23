###########################################################################################
##		Created using Monkey Studio v1.8.4.0 (1.8.4.0)
##
##	Author    : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
##	Project   : qwbfs
##	FileName  : installs.pri
##	Date      : 2010-04-04T11:04:05
##	License   : GPL3
##	Comment   : Creating using Monkey Studio RAD
##	Home Page   : http://code.google.com/p/qwbfs
##
###########################################################################################

unix {
	PACKAGE_PREFIX = $${PREFIX}
	isEmpty( PACKAGE_PREFIX ) {
		mac:PACKAGE_PREFIX	= /Applications
		else:PACKAGE_PREFIX = /usr/local
	}
	
	!build_pass {
		message( "The application will be installed to $${PACKAGE_PREFIX}" )
		message( "You can overwrite the prefix calling qmake with parameter: qmake PREFIX=/usr" )
	}
	
	mac:qwbfs_translations.path	= $${DESTDIR}/$${TARGET}.app/Contents/Resources/Translations
	else:qwbfs_translations.path	=	$$[QT_INSTALL_TRANSLATIONS]
	qwbfs_translations.files	= $$PWD/../translations/*.qm
	
	INSTALLS	+= qwbfs_translations
	
	mac {
		qwbfs_qt_translations.path	= $${DESTDIR}/$${TARGET}.app/Contents/Resources/Translations
		qwbfs_qt_translations.files	= $$[QT_INSTALL_TRANSLATIONS]/*.qm
		
		INSTALLS	+= qwbfs_qt_translations
	}
}

unix:!mac {
	# binary
	qwbfs_target.path	= $${PACKAGE_PREFIX}/bin
	qwbfs_target.files	= $${DESTDIR}/$${TARGET}
	qwbfs_target.CONFIG += no_check_exist

	# desktop file
	qwbfs_desktop.path	= $${PACKAGE_PREFIX}/share/applications
	qwbfs_desktop.files	= ../packages/QWBFSManager.desktop

	# desktop icon file
	qwbfs_desktopicon.path	= $${PACKAGE_PREFIX}/share/icons/hicolor/256x256/apps
	qwbfs_desktopicon.files	= $$PWD/resources/qwbfsmanager.png

	INSTALLS	+= qwbfs_target qwbfs_desktop qwbfs_desktopicon
}

mac {
	# binary
	qwbfs_target.path	= $${PACKAGE_PREFIX}
	qwbfs_target.files	= $${DESTDIR}/$${TARGET}
	qwbfs_target.CONFIG += no_check_exist
	
	INSTALLS	+= qwbfs_target
}
