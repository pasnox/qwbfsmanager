###########################################################################################
##		Created using Monkey Studio v1.8.4.0 (1.8.4.0)
##
##	Author    : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
##	Project   : qwbfs
##	FileName  : shared.pri
##	Date      : 2010-04-04T11:04:05
##	License   : GPL2
##	Comment   : Creating using Monkey Studio RAD
##	Home Page   : http://code.google.com/p/qwbfs
##
###########################################################################################

CONFIG	-= debug_and_release release debug warn_off warn_on x86 ppc universal
CONFIG	*= release warn_on

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
mac:universal {
	QMAKE_MACOSX_DEPLOYMENT_TARGET	= 10.3
	QMAKE_MAC_SDK	= /Developer/SDKs/MacOSX10.4u.sdk
	CONFIG	*= x86 ppc
}