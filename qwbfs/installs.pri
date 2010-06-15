###########################################################################################
##		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
##
##	Author    : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
##	Project   : installs
##	FileName  : installs.pri
##	Date      : 2010-04-04T11:04:05
##	License   : GPL3
##	Comment   : The QWBFS Manager installs properties for unix like systems
##	Home Page   : http://code.google.com/p/qwbfs
##
##	This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
##	WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
##
###########################################################################################

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
