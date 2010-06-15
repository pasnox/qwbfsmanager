# QWBFS Manager install script

unix:!mac {
	PACKAGE_PREFIX = $${prefix}
	isEmpty( PACKAGE_PREFIX ):PACKAGE_PREFIX = /usr/local
	
	!build_pass:message( "The application will be installed to $${PACKAGE_PREFIX}" )

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
