#!/bin/bash -ex

. .travis/common/pre-upload.sh

REV_NAME="citra-linux-${CITRA_VERSION}"
ARCHIVE_NAME="${REV_NAME}.tar.xz"
COMPRESSION_FLAGS="-cJvf"

mkdir "$REV_NAME"

cp build/bin/citra "$REV_NAME"
cp build/bin/citra-room "$REV_NAME"
cp build/bin/citra-qt "$REV_NAME"

# We need icons on Linux for .desktop entries
mkdir "$REV_NAME/dist"
cp dist/icon.png "$REV_NAME/dist/citra.png"

. .travis/common/post-upload.sh
