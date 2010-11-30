PUPDATECHECKER_PWD	= $$PWD

RESOURCES	*= $${PUPDATECHECKER_PWD}/resources/pUpdateChecker.qrc

FORMS	*= $${PUPDATECHECKER_PWD}/pUpdateCheckerDialog.ui

HEADERS	*= $${PUPDATECHECKER_PWD}/pUpdateChecker.h \
	$${PUPDATECHECKER_PWD}/pUpdateCheckerDialog.h

SOURCES	*= $${PUPDATECHECKER_PWD}/pUpdateChecker.cpp \
	$${PUPDATECHECKER_PWD}/pUpdateCheckerDialog.cpp
