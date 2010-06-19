#!/bin/sh

VERSION=$1
OS=`uname -s`
SOURCE_FOLDER=../../tags/$VERSION

if [ -z "$VERSION" ]; then
	SOURCE_FOLDER=..
fi

SVN_REVISION=`export LANG=C && svnversion $SOURCE_FOLDER`

if [ '!' -z "$VERSION" ]; then
	VERSION_STR=$VERSION
fi

if [ -z "$VERSION" ]; then
	VERSION=`echo "$SVN_REVISION" | egrep -o '[0-9]+' | xargs -n 2 printf '%s.%s\n'`
	VERSION_STR="trunk-svn$VERSION"
fi

BASE_NAME="qwbfsmanager-$VERSION_STR"
FOLDER_NAME=$BASE_NAME-src
TAR_GZ_FILE=$FOLDER_NAME.tar.gz
ZIP_FILE=$FOLDER_NAME.zip
WIN_SETUP=setup-$BASE_NAME-win32.exe
WIN_PACKAGE=$BASE_NAME-win32.zip
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

# delete existing file/folder $1
deleteIfExists()
{
	if [ -d $1 ]; then
		rm -fr $1
	elif [ -f $1 ]; then
		rm -f $1
	fi
}

# export a svn path from source $1 to target $2
svnExport()
{
	echo "Exporting repository: svn export $1 $2"
	svn export $1 $2
}

# create a tar.gz file $1 from path $2
createTarGz()
{
	echo "Creating tar.gz sources package: tar czf $1 $2"
	tar czf $1 $2
}

# create a zip file $1 from path $2 and extra parameters $3 and ending parameters $4
createZip()
{
	echo "Creating zip sources package: zip -q -r -9 $3 $1 $2 $4"
	zip -q -r -9 $3 $1 $2 $4
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

	cd "./$FOLDER_NAME"
	make distclean
	"$QT_PATH/bin/qmake" -spec "$MKSPEC" -win32 -r
	make distclean
	"$QT_PATH/bin/qmake" -spec "$MKSPEC" -win32 -r
	make -j4 release
	"$WINE" "$ISCC" "./packages/windows.iss"
	make distclean
	cd "$CUR_PATH"

	if [ -f "./$FOLDER_NAME/packages/releases/$WIN_SETUP" ]; then
		mv "./$FOLDER_NAME/packages/releases/$WIN_SETUP" "./"
	fi
}

# create windows zip package
windowsZipPackage()
{
	echo "Creating windows zip package"

	# uninstall previous package
	find "$WINE_PROGRAM_FILES/QWBFS Manager" -name "unins*.exe" -print0 | xargs -0 -I {} "$WINE" {} /silent

	# install the current one
	"$WINE" "./$WIN_SETUP" /silent

	# create zip
	ZIP_FOLDER="./$BASE_NAME-win32"
	cp -fr "$WINE_PROGRAM_FILES/QWBFS Manager" "$ZIP_FOLDER"
	createZip "./$WIN_PACKAGE" "$ZIP_FOLDER" "" "-x *unins*.exe -x *unins*.dat"
	rm -fr "$ZIP_FOLDER"

	# uninstall installed package
	find "$WINE_PROGRAM_FILES/QWBFS Manager" -name "unins*.exe" -print0 | xargs -0 -I {} "$WINE" {} /silent
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

	cd "./$FOLDER_NAME"
	make distclean
	"$QT_PATH/bin/qmake" -r
	make distclean
	"$QT_PATH/bin/qmake" -r
	make -j4 release
	"$QT_PATH/bin/macdeployqt" "$BUNDLE_APP_PATH" -dmg
	make distclean
	cd "$CUR_PATH"

	if [ -f "./$FOLDER_NAME/$BUNDLE_PATH/$BUNDLE_NAME.dmg" ]; then
		mv "./$FOLDER_NAME/$BUNDLE_PATH/$BUNDLE_NAME.dmg" "./$MAC_PACKAGE"
	fi
}

# delete source folder
deleteIfExists ./$FOLDER_NAME
# delete tar.gz source
deleteIfExists ./$TAR_GZ_FILE
# delete zip source
deleteIfExists ./$ZIP_FILE
# delete win setup
deleteIfExists ./$WIN_SETUP
# delete win package
deleteIfExists ./$WIN_PACKAGE
# export the taggued version to release
svnExport ./$SOURCE_FOLDER ./$FOLDER_NAME
# create tar.gz source
createTarGz ./$TAR_GZ_FILE ./$FOLDER_NAME
# create zip source
createZip ./$ZIP_FILE ./$FOLDER_NAME
# create win setup
crossBuild
# create windows zip package
windowsZipPackage

if [ $OS = "Darwin" ]; then
	macPackage
	killall wine
	killall WineBottler
	killall X11.bin
fi

if [ $OS = "Linux" ]; then
	echo "Linux"
fi

# delete exported repository
deleteIfExists ./$FOLDER_NAME

echo "********** Processing release finished **********"
