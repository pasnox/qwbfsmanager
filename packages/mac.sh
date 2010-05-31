#!/bin/sh

QT_PATH=$1
MAC_DEPLOY_QT=macdeployqt
BUNDLE_PATH="$PWD/../bin/QWBFSManager.app"

if [ ! -z "$QT_PATH" ]; then
	MAC_DEPLOY_QT="$QT_PATH/bin/$MAC_DEPLOY_QT"
fi

echo "Deploy Qt frameworks and create package using $MAC_DEPLOY_QT..."
"$MAC_DEPLOY_QT" "$BUNDLE_PATH" -dmg
