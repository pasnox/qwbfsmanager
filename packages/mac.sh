#!/bin/sh

BUNDLE_VERSION="1.0.2"
SVN_REVISION=`svnversion ..'`
QT_VERSION="4.6.2-universal"
BUNDLE_NAME="QWBFSManager"
CUR_PATH="$PWD"
BUNDLE_PATH="$PWD/../bin"
BUNDLE_APP_PATH="$BUNDLE_PATH/$BUNDLE_NAME.app"

if [ '!' -z "$1" ]; then
	QT_VERSION=$1
fi

QT_PATH="/usr/local/Trolltech/Qt-$QT_VERSION"

echo "Deploy Qt frameworks and create package using $MAC_DEPLOY_QT..."

cd "$CUR_PATH/.."
make distclean
"$QT_PATH/bin/qmake" -r
make distclean
"$QT_PATH/bin/qmake" -r
make -j4 release
cd "$CUR_PATH"
"$QT_PATH/bin/macdeployqt" "$BUNDLE_APP_PATH" -dmg
mkdir -p "$CUR_PATH/releases"
mv "$BUNDLE_PATH/$BUNDLE_NAME.dmg" "$CUR_PATH/releases/qwbfs_manager_$BUNDLE_VERSION-svn$SVN_REVISION.dmg"