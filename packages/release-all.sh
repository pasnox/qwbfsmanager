#!/bin/sh

VERSION=$1
OS=`uname -s`

SVN_REVISION=`export LANG=C && [ -f /usr/bin/git ] && git --git-dir=../.git describe --long $VERSION`

if [ '!' -z "$VERSION" ]; then
    VERSION_STR=$VERSION
else
    VERSION_STR=$SVN_REVISION
fi

if [ -z "$VERSION" ]; then
    VERSION=`echo "$SVN_REVISION" | cut -d'-' -f1`
fi

VERSION=`echo "$VERSION" | cut -d'v' -f2`

BASE_NAME=qwbfsmanager-$VERSION_STR
FOLDER_NAME=$BASE_NAME-src
TAR_GZ_FILE=$FOLDER_NAME.tar.gz
ZIP_FILE=$FOLDER_NAME.zip
WIN_SETUP=$BASE_NAME-win32.exe
WIN_FOLDER=$BASE_NAME-win32
WIN_PACKAGE=$WIN_FOLDER.zip
MAC_PACKAGE=$BASE_NAME.dmg
CUR_PATH=$PWD
LOG_FOLDER=$CUR_PATH/log

if [ $OS = "Linux" ]; then
    WINE="wine"
    WINE_DRIVE="$HOME/.wine/drive_c"
    WINE_PROGRAM_FILES="$WINE_DRIVE/Program Files"
fi

if [ $OS = "Darwin" ]; then
    WINE="/Applications/Wine.app/Contents/Resources/bin/wine"
    WINE_DRIVE="$HOME/Wine Files/drive_c"
    WINE_PROGRAM_FILES="$WINE_DRIVE/Program Files"
fi

export OS
export VERSION
export VERSION_STR
export SVN_REVISION

# execute command and stop if fails
startCommand()
{
    echo "    -Starting command: $1"
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
exportRepository()
{
    echo "*** Exporting repository: $1 to $2"

    startCommand "cd \"..\""
    startCommand "git submodule update --init --recursive > /dev/null 2>&1"
    startCommand "cd \"$CUR_PATH\""

    startCommand "rsync -a .. \"$2\" --exclude \"$2\" > /dev/null 2>&1"

    startCommand "cd \"$2\""
    if [ '!' -z "$1" ]; then
        startCommand "git clean -f -q -x -d && git reset -q --hard && git checkout -q \"$1\" > /dev/null 2>&1"
    else
        startCommand "git clean -f -X -q -d > /dev/null 2>&1"
    fi
    startCommand "cd \"$CUR_PATH\""

    startCommand "rm -fr \"$2/.git\""
}

# create a tar.gz file $1 from path $2
createTarGz()
{
    echo "*** Creating tar.gz package: $1"
    startCommand "tar czf \"$1\" \"$2\""
}

# create a zip file $1 from path $2 and extra parameters $3 and ending parameters $4
createZip()
{
    echo "*** Creating zip package: $1"

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
    QMAKE="qmake"

    if [ $OS = "Linux" ]; then
        QMAKE="qmake-qt4"
        QT_VERSION="4.8.6"
        QT_WIN32_VERSION="4.8.3"
        QT_PATH="/usr"
        MKSPEC="$HOME/.qt/mkspecs/win32-x11-g++"
        QT_WIN32_PATH="$WINE_DRIVE/Development/Qt/$QT_WIN32_VERSION"
        ISCC="$WINE_PROGRAM_FILES/Inno Setup 5/ISCC.exe"
        DLLS_PATH="$WINE_DRIVE/Development/OpenSSL"
    fi

    if [ $OS = "Darwin" ]; then
        QT_VERSION="4.7.0-lgpl"
        QT_WIN32_VERSION="4.7.1"
        QT_PATH="/usr/local/Trolltech/$QT_VERSION"
        MKSPEC="$HOME/mkspecs/4.6.x/win32-osx-g++"
        QT_WIN32_PATH="/usr/local/Trolltech/win32/$QT_WIN32_VERSION"
        ISCC="$WINE_PROGRAM_FILES/Inno_Setup_5/ISCC.exe"
        DLLS_PATH="$HOME/Win32Libraries/bin"
    fi

    if [ -d "$QT_PATH" ] && [ -d "$MKSPEC" ] && [ -d "$QT_WIN32_PATH" ] && [ -f "$ISCC" ] && [ -d "$DLLS_PATH" ]; then
        echo "*** Crossbuilding for windows"

        export QT_WIN32_PATH
        export QT_WIN32_VERSION
        export DLLS_PATH
        export CROSS_WIN32_QT_PATH="$QT_WIN32_PATH"
        export QT_WINDOWS_PATH="$QT_WIN32_PATH"

        startCommand "cd \"$FOLDER_NAME\""
        startCommand "make distclean > /dev/null 2>&1" 0
        startCommand "\"$QT_PATH/bin/$QMAKE\" -spec \"$MKSPEC\" -win32 -r > /dev/null 2>&1"
        startCommand "make distclean > /dev/null 2>&1" 0
        startCommand "\"$QT_PATH/bin/$QMAKE\" -spec \"$MKSPEC\" -win32 -r > /dev/null 2>&1"
        startCommand "make -j4 > \"$CUR_PATH/log/winbuild.log\" 2>&1"
        startCommand "\"$WINE\" \"$ISCC\" \"packages/windows.iss\" > \"$CUR_PATH/log/winpackage.log\" 2>&1"
        startCommand "make distclean > /dev/null 2>&1" 0
        startCommand "cd \"$CUR_PATH\""

        if [ -f "$FOLDER_NAME/packages/$WIN_SETUP" ]; then
            startCommand "mv \"$FOLDER_NAME/packages/$WIN_SETUP\" \".\""
        fi
    fi
}

# create windows zip package
windowsZipPackage()
{
    if [ -f "$WIN_SETUP" ]; then
        echo "*** Creating windows zip package"

        # uninstall previous package
        startCommand "find \"$WINE_PROGRAM_FILES/QWBFS Manager\" -name \"unins*.exe\" -print0 | xargs -0 -I {} \"$WINE\" {} /silent > /dev/null 2>&1"

        # install the current one
        startCommand "\"$WINE\" \"$WIN_SETUP\" /silent > /dev/null 2>&1"

        # create zip
        startCommand "cp -fr \"$WINE_PROGRAM_FILES/QWBFS Manager\" \"$WIN_FOLDER\""
        startCommand "createZip \"$WIN_PACKAGE\" \"$WIN_FOLDER\" \"\" \"-x *unins*.exe -x *unins*.dat\""
        startCommand "deleteIfExists \"$WIN_FOLDER\""

        # uninstall installed package
        startCommand "find \"$WINE_PROGRAM_FILES/QWBFS Manager\" -name \"unins*.exe\" -print0 | xargs -0 -I {} \"$WINE\" {} /silent > /dev/null 2>&1"
    fi
}

# create mac os x package
macPackage()
{
    echo "*** Create Mac OS X package"

    QT_VERSION="4.7.0-lgpl"
    BUNDLE_NAME="QWBFSManager"
    BUNDLE_PATH="bin"
    BUNDLE_APP_PATH="$BUNDLE_PATH/$BUNDLE_NAME.app"
    QT_PATH="/usr/local/Trolltech/$QT_VERSION"
    QMAKE_FLAGS="\"CONFIG *= universal no_fresh_install\""

    # qt4-mac package mac port qt4 support
    if [ ! -d "$QT_PATH" ]; then
        QT_VERSION=""
        QT_PATH="/opt/local"
        QMAKE_FLAGS="\"CONFIG *= no_fresh_install\""
    fi

    startCommand "cd \"$FOLDER_NAME\""
    startCommand "make distclean > /dev/null 2>&1" 0
    startCommand "\"$QT_PATH/bin/qmake\" $QMAKE_FLAGS -r > /dev/null 2>&1"
    startCommand "make distclean > /dev/null 2>&1" 0
    startCommand "\"$QT_PATH/bin/qmake\" $QMAKE_FLAGS -r > /dev/null 2>&1"
    startCommand "make -j4 > \"$CUR_PATH/log/macbuild.log\" 2>&1"
    startCommand "make install > /dev/null 2>&1"
    startCommand "\"$QT_PATH/bin/macdeployqt\" \"$BUNDLE_APP_PATH\" -dmg > /dev/null 2>&1"
    startCommand "make distclean > /dev/null 2>&1" 0
    startCommand "cd \"$CUR_PATH\""

    if [ -f "$FOLDER_NAME/$BUNDLE_PATH/$BUNDLE_NAME.dmg" ]; then
        startCommand "mv \"$FOLDER_NAME/$BUNDLE_PATH/$BUNDLE_NAME.dmg\" \"$MAC_PACKAGE\""
    fi
}

# startup function
startup()
{
    if [ '!' -d "$LOG_FOLDER" ]; then
        echo "*** Create log folder"
        startCommand "mkdir -p \"$LOG_FOLDER\""
    fi
}

# finish function, must not use startCommand or function calling it to avoid possible unfinite loop on errors.
finish()
{
    # close wine, WineBottler & X11
    if [ $OS = "Darwin" ]; then
        startCommand "killall wine > /dev/null 2>&1" 0
        startCommand "killall WineBottler > /dev/null 2>&1" 0
        startCommand "killall X11.bin > /dev/null 2>&1" 0
    fi

    # come back to start folder
    startCommand "cd \"$CUR_PATH\"" 0

    # delete exported repository
    startCommand "rm -fr \"$FOLDER_NAME\"" 0

    echo "********** Processing release finished - Exit code: $1 **********"

    exit $1
}

# startup call
startup
# delete source folder
deleteIfExists "$FOLDER_NAME"
# delete tar.gz source
deleteIfExists "$TAR_GZ_FILE"
# delete zip source
deleteIfExists "$ZIP_FILE"
# delete win setup
deleteIfExists "$WIN_SETUP"
# delete win package
deleteIfExists "$WIN_PACKAGE"
# delete mac package
deleteIfExists "$MAC_PACKAGE"
# export the taggued version to release
exportRepository "$1" "$FOLDER_NAME"
# create tar.gz source
createTarGz "$TAR_GZ_FILE" "$FOLDER_NAME"
# create zip source
createZip "$ZIP_FILE" "$FOLDER_NAME"
# create win setup
crossBuild
# create windows zip package
windowsZipPackage
# darwin specific
if [ $OS = "Darwin" ]; then
    echo "*** Mac"
    macPackage
fi
# linux specific
if [ $OS = "Linux" ]; then
    echo "*** Linux"
fi
# finish call
finish 0
