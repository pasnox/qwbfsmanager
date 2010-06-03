echo "This script MUST be started from the packagesfolder"
echo "1st parameter = host Qt version / 2nd parameter = windows Qt version"

PLATFORM=`uname -s`
CUR_PATH="$PWD"
QT_VERSION="4.6.2"
QT_WIN32_VERSION="4.6.0"
QT_WIN32_PATH=.
MKSPEC="x-win32-g++"
ISCC="ISCC.exe"
WINE="wine"
SVN_REVISION=`svn info .. | grep Revision: | awk '{ print $2 }'`
DLLS_PATH=.

if [ '!' -z "$1" ]; then
	QT_VERSION=$1
fi

if [ '!' -z "$2" ]; then
	QT_WIN32_VERSION=$2
fi

# mac os x
if [ $PLATFORM = "Darwin" ]; then
	QT_PATH="/usr/local/Trolltech/Qt-$QT_VERSION"
	MKSPEC="$HOME/mkspecs/4.6.x/win32-osx-g++"
	WINE="/Applications/Wine.app/Contents/Resources/bin/wine"
	ISCC="$HOME/Wine Files/drive_c/Program Files/Inno_Setup_5_gpl/ISCC.exe"
	DLLS_PATH="$HOME/Win32Libraries/bin"
	QT_WIN32_PATH="/usr/local/Trolltech/win32/$QT_WIN32_VERSION"
fi

# unix/linux
if [ $PLATFORM = "Linux" ]; then
	QT_PATH="/usr/local/Trolltech/Qt-$QT_VERSION"
	MKSPEC="$HOME/mkspecs/4.6.x/win32-osx-g++"
	QT_WIN32_PATH="/usr/local/Trolltech/win32/$QT_WIN32_VERSION"
fi

export SVN_REVISION
export QT_WIN32_PATH
export DLLS_PATH
export CROSS_WIN32_QT_PATH="$QT_WIN32_PATH"

echo "Using Qt Version: $QT_VERSION ($QT_WIN32_VERSION) on platform: $PLATFORM @revision: $SVN_REVISION"

cd "$CUR_PATH/.."
make distclean
"$QT_PATH/bin/qmake" -spec "$MKSPEC" -win32 -r
make distclean
"$QT_PATH/bin/qmake" -spec "$MKSPEC" -win32 -r
make -j4 release
cd "$CUR_PATH"
"$WINE" "$ISCC" ./windows.iss