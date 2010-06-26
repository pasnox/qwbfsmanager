#!/bin/sh

VERSION=$1
OS=`uname -s`
SOURCE_FOLDER=../../tags/$VERSION

if [ -z "$VERSION" ]; then
	SOURCE_FOLDER=..
fi

SVN_REVISION=`export LANG=C && [ -f /usr/bin/svnversion ] && svnversion $SOURCE_FOLDER`

if [ '!' -z "$VERSION" ]; then
	VERSION_STR=$VERSION
fi

if [ -z "$VERSION" ]; then
	#VERSION=`echo "$SVN_REVISION" | egrep -o '[0-9]+'`
	VERSION=`echo "$SVN_REVISION" | egrep -o '[0-9]+' | xargs -n 2 printf '%s.%s\n'`
	VERSION_STR="trunk-svn$VERSION"
fi

BASE_NAME="qwbfsmanager-$VERSION_STR"
FOLDER_NAME=$BASE_NAME-src
TAR_GZ_FILE=$FOLDER_NAME.tar.gz
ZIP_FILE=$FOLDER_NAME.zip
WIN_SETUP=setup-$BASE_NAME-win32.exe
WIN_FOLDER=$BASE_NAME-win32
WIN_PACKAGE=$WIN_FOLDER.zip
MAC_PACKAGE=$BASE_NAME.dmg
CUR_PATH=$PWD

if [ $OS = "Linux" ]; then
	WINE="wine"
	WINE_DRIVE="$HOME/.wine/drive_c"
	WINE_PROGRAM_FILES="$WINE_DRIVE/Program Files"
fi

if [ $OS = "Darwin" ]; then
	WINE="/Applications/Wine.app/Contents/Resources/bin/wine"
	WINE_DRIVE="$HOME/.wine/drive_c"
	WINE_PROGRAM_FILES="$WINE_DRIVE/Program Files"
fi

export OS
export VERSION
export VERSION_STR
export SVN_REVISION

# execute command and stop if fails
startCommand()
{
	error=0
	eval "$1" || error=1
	
	if [ $error = 1 -a -z "$2" ]; then
		echo "***************************************************"
		echo "***  Error when evaluing command: $1"
		echo "***************************************************"
		finish 1
	fi
}

# delete existing file/folder $1
deleteIfExists()
{
	if [ -d "$1" ]; then
		echo "*** Deleting folder: $1"
		startCommand "rm -fr \"$1\""
	elif [ -f "$1" ]; then
		echo "*** Deleting file: $1"
		startCommand "rm -f \"$1\""
	fi
}

# export a svn path from source $1 to target $2
svnExport()
{
	echo "Exporting repository: svn export $1 $2"
	startCommand "svn export \"$1\" \"$2\""
}

# create a tar.gz file $1 from path $2
createTarGz()
{
	echo "Creating tar.gz sources package: tar czf $1 $2"
	startCommand "tar czf \"$1\" \"$2\""
}

# create a zip file $1 from path $2 and extra parameters $3 and ending parameters $4
createZip()
{
	echo "Creating zip package: zip -q -r -9 $3 $1 $2 $4"
	
	params=
	
	if [ '!' -z "$3" ]; then
		params="$params \"$3\""
	fi
	
	if [ '!' -z "$1" ]; then
		params="$params \"$1\""
	fi
	
	if [ '!' -z "$2" ]; then
		params="$params \"$2\""
	fi
	
	if [ '!' -z "$4" ]; then
		params="$params $4"
	fi
	
	startCommand "zip -q -r -9 $params"
}

# crossbuild for windows
crossBuild()
{
	echo "Crossbuilding for windows"

	if [ $OS = "Linux" ]; then
		QT_VERSION="4.6.3"
		QT_WIN32_VERSION="4.6.1"
		QT_PATH="/usr"
		MKSPEC="$HOME/.qt/win32-x11-g++"
		QT_WIN32_PATH="$WINE_DRIVE/Development/Qt/$QT_WIN32_VERSION"
		ISCC="$WINE_PROGRAM_FILES/Inno Setup 5/ISCC.exe"
		DLLS_PATH="$WINE_DRIVE/Development/OpenSSL"
	fi

	if [ $OS = "Darwin" ]; then
		QT_VERSION="4.6.2-universal"
		QT_WIN32_VERSION="4.6.0"
		QT_PATH="/usr/local/Trolltech/Qt-$QT_VERSION"
		MKSPEC="$HOME/mkspecs/4.6.x/win32-osx-g++"
		QT_WIN32_PATH="/usr/local/Trolltech/win32/$QT_WIN32_VERSION"
		ISCC="$WINE_PROGRAM_FILES/Inno_Setup_5_gpl/ISCC.exe"
		DLLS_PATH="$HOME/Win32Libraries/bin"

	fi

	export QT_WIN32_PATH
	export QT_WIN32_VERSION
	export DLLS_PATH
	export CROSS_WIN32_QT_PATH="$QT_WIN32_PATH"

	startCommand "cd \"./$FOLDER_NAME\""
	startCommand "make distclean &> /dev/null" 0
	startCommand "\"$QT_PATH/bin/qmake\" -spec \"$MKSPEC\" -win32 -r"
	startCommand "make distclean &> /dev/null" 0
	startCommand "\"$QT_PATH/bin/qmake\" -spec \"$MKSPEC\" -win32 -r"
	startCommand "make -j4 release &> $CUR_PATH/winbuild.log"
	startCommand "\"$WINE\" \"$ISCC\" \"./packages/windows.iss\" &> $CUR_PATH/winpackage.log"
	startCommand "make distclean &> /dev/null" 0
	startCommand "cd \"$CUR_PATH\""

	if [ -f "./$FOLDER_NAME/packages/releases/$WIN_SETUP" ]; then
		startCommand "mv \"./$FOLDER_NAME/packages/releases/$WIN_SETUP\" \"./\""
	fi
}

# create windows zip package
windowsZipPackage()
{
	echo "Creating windows zip package"

	# uninstall previous package
	startCommand "find \"$WINE_PROGRAM_FILES/QWBFS Manager\" -name \"unins*.exe\" -print0 | xargs -0 -I {} \"$WINE\" {} /silent"

	# install the current one
	startCommand "\"$WINE\" \"./$WIN_SETUP\" /silent"

	# create zip
	startCommand "cp -fr \"$WINE_PROGRAM_FILES/QWBFS Manager\" \"./$WIN_FOLDER\""
	startCommand "createZip \"./$WIN_PACKAGE\" \"./$WIN_FOLDER\" \"\" \"-x *unins*.exe -x *unins*.dat\""
	startCommand "deleteIfExists \"./$WIN_FOLDER\""

	# uninstall installed package
	startCommand "find \"$WINE_PROGRAM_FILES/QWBFS Manager\" -name \"unins*.exe\" -print0 | xargs -0 -I {} \"$WINE\" {} /silent"
}

# create mac os x package
macPackage()
{
	echo "Create Mac OS X package"

	QT_VERSION="4.6.2-universal"
	BUNDLE_NAME="QWBFSManager"
	BUNDLE_PATH="./bin"
	BUNDLE_APP_PATH="$BUNDLE_PATH/$BUNDLE_NAME.app"
	QT_PATH="/usr/local/Trolltech/Qt-$QT_VERSION"

	startCommand "cd \"./$FOLDER_NAME\""
	startCommand "make distclean &> /dev/null" 0
	startCommand "\"$QT_PATH/bin/qmake\" -r"
	startCommand "make distclean &> /dev/null" 0
	startCommand "\"$QT_PATH/bin/qmake\" -r"
	startCommand "make -j4 release &> $CUR_PATH/macbuild.log"
	startCommand "make install"
	startCommand "\"$QT_PATH/bin/macdeployqt\" \"$BUNDLE_APP_PATH\" -dmg"
	startCommand "make distclean &> /dev/null" 0
	startCommand "cd \"$CUR_PATH\""

	if [ -f "./$FOLDER_NAME/$BUNDLE_PATH/$BUNDLE_NAME.dmg" ]; then
		startCommand "mv \"./$FOLDER_NAME/$BUNDLE_PATH/$BUNDLE_NAME.dmg\" \"./$MAC_PACKAGE\""
	fi
}

# finish function, muse not use startCommand or function calling it to avoid possible unfinite loop on errors.
finish()
{
	# close wine, WineBottler & X11
	if [ $OS = "Darwin" ]; then
		killall wine
		killall WineBottler
		killall X11.bin
	fi
	
	# come back to start folder
	cd "$CUR_PATH"
	
	# delete exported repository
	rm -fr "./$FOLDER_NAME"

	echo "********** Processing release finished - Exit code: $1 **********"
	
	exit $1
}

# delete source folder
deleteIfExists "./$FOLDER_NAME"
# delete tar.gz source
deleteIfExists "./$TAR_GZ_FILE"
# delete zip source
deleteIfExists "./$ZIP_FILE"
# delete win setup
deleteIfExists "./$WIN_SETUP"
# delete win package
deleteIfExists "./$WIN_PACKAGE"
# delete mac package
deleteIfExists "./$MAC_PACKAGE"
# export the taggued version to release
svnExport "./$SOURCE_FOLDER" "./$FOLDER_NAME"
# create tar.gz source
createTarGz "./$TAR_GZ_FILE" "./$FOLDER_NAME"
# create zip source
createZip "./$ZIP_FILE" "./$FOLDER_NAME"
# create win setup
crossBuild
# create windows zip package
windowsZipPackage

if [ $OS = "Darwin" ]; then
	echo "*** Mac"
	macPackage
fi

if [ $OS = "Linux" ]; then
	echo "*** Linux"
fi

# exit with success
finish 0