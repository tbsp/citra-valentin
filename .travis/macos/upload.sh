#!/bin/bash -ex

. .travis/common/pre-upload.sh

REV_NAME="citra-macos-${CITRA_VERSION}"
ARCHIVE_NAME="${REV_NAME}.tar.gz"
COMPRESSION_FLAGS="-czvf"

mkdir "$REV_NAME"

cp build/bin/citra "$REV_NAME"
cp -r build/bin/citra-qt.app "$REV_NAME"
cp build/bin/citra-room "$REV_NAME"

macpack "${REV_NAME}/citra-qt.app/Contents/MacOS/citra-qt" -d "../Frameworks"

# Move Qt frameworks into app bundle for deployment
$(brew --prefix)/opt/qt5/bin/macdeployqt "${REV_NAME}/citra-qt.app" -executable="${REV_NAME}/citra-qt.app/Contents/MacOS/citra-qt"

# Move libs into folder for deployment
macpack "${REV_NAME}/citra" -d "libs"

# Make the launching script executable
chmod +x ${REV_NAME}/citra-qt.app/Contents/MacOS/citra-qt

# Verify loader instructions
find "$REV_NAME" -exec otool -L {} \;

. .travis/common/post-upload.sh
