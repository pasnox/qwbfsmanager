###########################################################################################
##		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
##
##	Author    : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
##	Project   : shared
##	FileName  : shared.pri
##	Date      : 2010-04-04T11:04:05
##	License   : GPL3
##	Comment   : Creating using Monkey Studio RAD
##	Home Page   : http://code.google.com/p/qwbfs
##
##	This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
##	WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
##
###########################################################################################

CONFIG	*= debug_and_release

CONFIG(debug, debug|release) {
	#Debug
	CONFIG	*= console
	unix:TARGET	= $$join(TARGET,,,_debug)
	else:TARGET	= $$join(TARGET,,,d)
	unix:OBJECTS_DIR	= $${BUILD_PATH}/debug/obj/unix
	win32:OBJECTS_DIR	= $${BUILD_PATH}/debug/obj/win32
	mac:OBJECTS_DIR	= $${BUILD_PATH}/debug/obj/mac
	UI_DIR	= $${BUILD_PATH}/debug/ui
	MOC_DIR	= $${BUILD_PATH}/debug/moc
	RCC_DIR	= $${BUILD_PATH}/debug/rcc
} else {
	#Release
	unix:OBJECTS_DIR	= $${BUILD_PATH}/release/obj/unix
	win32:OBJECTS_DIR	= $${BUILD_PATH}/release/obj/win32
	mac:OBJECTS_DIR	= $${BUILD_PATH}/release/obj/mac
	UI_DIR	= $${BUILD_PATH}/release/ui
	MOC_DIR	= $${BUILD_PATH}/release/moc
	RCC_DIR	= $${BUILD_PATH}/release/rcc
}

# Mac universal build from 10.3 to up to 10.5
mac {
	QMAKE_MACOSX_DEPLOYMENT_TARGET	= 10.3
	QMAKE_MAC_SDK	= /Developer/SDKs/MacOSX10.4u.sdk
	CONFIG	+= x86 ppc
}