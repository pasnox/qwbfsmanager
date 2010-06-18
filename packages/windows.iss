; This script need to use a special Inno Setup version located here : http://jrsoftware.org/isdl.php#qsp

[CustomMessages]
QWBFS_NAME=QWBFS Manager
QWBFS_COPYRIGHTS=2010 Filipe AZEVEDO
QWBFS_URL=http://code.google.com/p/qwbfs/
QWBFS_ISSUES_URL=http://code.google.com/p/qwbfs/issues/list

#define QWBFS_VERSION "1.0.2"
#define QWBFS_REVISION GetEnv("SVN_REVISION")
#define QWBFS_SETUP_NAME "setup-qwbfsmanager-" +QWBFS_VERSION +"-svn" +QWBFS_REVISION +"-win32"
#define QT_PATH "Z:\" +GetEnv("CROSS_WIN32_QT_PATH")
#define DLLS_PATH "Z:\" +GetEnv("DLLS_PATH")

[Setup]
SourceDir=..\bin
OutputDir=..\packages\releases
OutputBaseFilename={#QWBFS_SETUP_NAME}
VersionInfoVersion={#QWBFS_VERSION}
VersionInfoCompany=Filipe AZEVEDO
VersionInfoDescription=Free, Fast and Powerfull cross platform Wii Backup File System Manager
VersionInfoTextVersion={#QWBFS_VERSION}
VersionInfoCopyright={cm:QWBFS_COPYRIGHTS}
AppCopyright={cm:QWBFS_COPYRIGHTS}
AppName={cm:QWBFS_NAME}
AppVerName={cm:QWBFS_NAME} {#QWBFS_VERSION}
InfoAfterFile=..\GPL-3
InfoBeforeFile=..\GPL-3
LicenseFile=..\GPL-3
ChangesAssociations=true
PrivilegesRequired=none
DefaultDirName={pf}\{cm:QWBFS_NAME}
EnableDirDoesntExistWarning=false
AllowNoIcons=true
DefaultGroupName={cm:QWBFS_NAME}
AlwaysUsePersonalGroup=true
;SetupIconFile=..\qwbfs\resources\qwbfs.ico
AppPublisher={cm:QWBFS_COPYRIGHTS}
AppPublisherURL={cm:QWBFS_URL}
AppSupportURL={cm:QWBFS_ISSUES_URL}
AppVersion={#QWBFS_VERSION}
AppComments=Thanks using {cm:QWBFS_NAME}
AppContact={cm:QWBFS_ISSUES_URL}
UninstallDisplayName={cm:QWBFS_NAME}
ShowLanguageDialog=yes
UsePreviousLanguage=no

[_ISTool]
UseAbsolutePaths=false

[Files]
; QWBFS Manager related files
Source: qwbfsmanager.exe; DestDir: {app}; Flags: promptifolder
Source: ..\packages\qt_windows.conf; DestDir: {app}; DestName: qt.conf; Flags: promptifolder
Source: ..\GPL-3; DestDir: {app}; Flags: promptifolder
;Source: ..\datas\translations\*.*; DestDir: {app}\translations; Flags: promptifolder
; Qt related files
Source: {#QT_PATH}\bin\QtCore4.dll; DestDir: {app}; Flags: promptifolder
Source: {#QT_PATH}\bin\QtGui4.dll; DestDir: {app}; Flags: promptifolder
Source: {#QT_PATH}\bin\QtXml4.dll; DestDir: {app}; Flags: promptifolder
Source: {#QT_PATH}\bin\QtNetwork4.dll; DestDir: {app}; Flags: promptifolder
Source: {#QT_PATH}\plugins\accessible\*.dll; DestDir: {app}\qt\plugins\accessible; Flags: promptifolder; Excludes: *d4.dll
Source: {#QT_PATH}\plugins\codecs\*.dll; DestDir: {app}\qt\plugins\codecs; Flags: promptifolder; Excludes: *d4.dll
Source: {#QT_PATH}\plugins\graphicssystems\*.dll; DestDir: {app}\qt\plugins\graphicssystems; Flags: promptifolder; Excludes: *d4.dll
Source: {#QT_PATH}\plugins\imageformats\*.dll; DestDir: {app}\qt\plugins\imageformats; Flags: promptifolder; Excludes: *d4.dll
Source: {#QT_PATH}\translations\qt*.qm; DestDir: {app}\qt\translations; Flags: promptifolder recursesubdirs; Excludes: qt_help*.qm,qtconfig*.qm
; MinGW related files
Source: {#DLLS_PATH}\mingwm10.dll; DestDir: {app}; Flags: promptifolder
Source: {#DLLS_PATH}\libgcc_s_dw2-1.dll; DestDir: {app}; Flags: promptifolder
Source: {#DLLS_PATH}\libeay32.dll; DestDir: {app}; Flags: promptifolder
Source: {#DLLS_PATH}\libssl32.dll; DestDir: {app}; Flags: promptifolder

[Icons]
Name: {group}\{cm:QWBFS_NAME}; Filename: {app}\qwbfsmanager.exe; WorkingDir: {app}; IconFilename: {app}\qwbfsmanager.exe; IconIndex: 0
Name: {userdesktop}\{cm:QWBFS_NAME}; Filename: {app}\qwbfsmanager.exe; WorkingDir: {app}; IconFilename: {app}\qwbfsmanager.exe; IconIndex: 0
Name: {group}\Home Page; Filename: {app}\Home Page.url; WorkingDir: {app}
Name: {group}\Tracker; Filename: {app}\Tracker.url; WorkingDir: {app}

[INI]
Filename: {app}\Home Page.url; Section: InternetShortcut; Key: URL; String: {cm:QWBFS_URL}; Flags: createkeyifdoesntexist uninsdeleteentry uninsdeletesectionifempty; Components:
Filename: {app}\Tracker.url; Section: InternetShortcut; Key: URL; String: {cm:QWBFS_ISSUES_URL}; Flags: createkeyifdoesntexist uninsdeleteentry uninsdeletesectionifempty

[UninstallDelete]
Name: {app}\Home Page.url; Type: files
Name: {app}\Tracker.url; Type: files
Name: {app}\*.ini; Type: files
Name: {app}; Type: dirifempty
