#!/bin/sh

VERSION=$1
PASSWORD=$2

if [ -z "$VERSION" ]; then
  echo "Please give a version to upload as first parameter"
  exit 1
fi

if [ -z "PASSWORD" ]; then
  echo "Please give a password as second parameter"
  exit 1
fi

NAME="qwbfsmanager"

# $1 - package file path
# $2 - package summary
# $3 - package labels
googleCodeUpload() {
  echo "Uploading package '$1'..."
  googlecode_upload.pl \
    --progress \
    --user="pasnox" \
    --pass="$PASSWORD" \
    --project="qwbfs" \
    --summary="$2" \
    --label="$3" \
    --file="$1"
}

# source tar gz
if [ -f "$NAME-$VERSION-src.tar.gz" ]; then
  googleCodeUpload "$NAME-$VERSION-src.tar.gz" "Source Tgz Archive $VERSION" "OpSys-All,Type-Source,Featured"
fi

# source zip
if [ -f "$NAME-$VERSION-src.zip" ]; then
  googleCodeUpload "$NAME-$VERSION-src.zip" "Source Zip Archive $VERSION" "OpSys-All,Type-Source,Featured"
fi

# win32 zip
if [ -f "$NAME-$VERSION-win32.zip" ]; then
  googleCodeUpload "$NAME-$VERSION-win32.zip" "Windows Zip Archive $VERSION" "OpSys-Windows,Type-Archive,Featured"
fi

# win32 setup
if [ -f "setup-$NAME-$VERSION-win32.exe" ]; then
  googleCodeUpload "setup-$NAME-$VERSION-win32.exe" "Windows Installer $VERSION" "OpSys-Windows,Type-Installer,Featured"
fi
