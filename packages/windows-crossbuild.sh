echo "This script MUST be started from the packagesfolder"
echo "1st parameter = host Qt version / 2nd parameter = windows Qt version"

CUR_PATH="$PWD"
QT_VERSION="4.6.2-universal"
QT_WIN32_VERSION="4.6.0"
QT_WIN32_PATH=.
MKSPEC="x-win32-g++"
ISCC="ISCC.exe"
WINE="wine"
DLLS_PATH=.

if [ -z "$SVN_REVISION" ]; then
	SVN_REVISION=`export LANG=C && svnversion ..`
fi

if [ -z "$OS" ]; then
	OS=`uname -s`
fi

if [ '!' -z "$1" ]; then
	QT_VERSION=$1
fi

if [ '!' -z "$2" ]; then
	QT_WIN32_VERSION=$2
fi

# mac os x
if [ $OS = "Darwin" ]; then
	QT_PATH="/usr/local/Trolltech/Qt-$QT_VERSION"
	MKSPEC="$HOME/mkspecs/4.6.x/win32-osx-g++"
	WINE="/Applications/Wine.app/Contents/Resources/bin/wine"
	ISCC="$HOME/Wine Files/drive_c/Program Files/Inno_Setup_5_gpl/ISCC.exe"
	DLLS_PATH="$HOME/Win32Libraries/bin"
	QT_WIN32_PATH="/usr/local/Trolltech/win32/$QT_WIN32_VERSION"
fi

# unix/linux
if [ $OS = "Linux" ]; then
	#QT_PATH="/usr/local/Trolltech/Qt-$QT_VERSION"
	#MKSPEC="$HOME/mkspecs/4.6.x/win32-osx-g++"
	#QT_WIN32_PATH="/usr/local/Trolltech/win32/$QT_WIN32_VERSION"
	QT_VERSION="4.6.3"
	QT_WIN32_VERSION="4.6.1"
	QT_PATH=/usr
	MKSPEC="$HOME/.qt/win32-x11-g++"
	QT_WIN32_PATH="$HOME/Disk Wine/Development/Qt/$QT_WIN32_VERSION"
	ISCC="$HOME/Disk Wine/Program Files/Inno Setup 5/ISCC.exe"
	DLLS_PATH="$HOME/Disk Wine/Development/OpenSSL"
fi

export SVN_REVISION
export QT_WIN32_PATH
export QT_WIN32_VERSION
export DLLS_PATH
export CROSS_WIN32_QT_PATH="$QT_WIN32_PATH"

echo "Using Qt Version: $QT_VERSION ($QT_WIN32_VERSION) on platform: $OS @revision: $SVN_REVISION"

cd "$CUR_PATH/.."
make distclean
"$QT_PATH/bin/qmake" -spec "$MKSPEC" -win32 -r
make distclean
"$QT_PATH/bin/qmake" -spec "$MKSPEC" -win32 -r
make -j4 release
cd "$CUR_PATH"
"$WINE" "$ISCC" ./windows.iss