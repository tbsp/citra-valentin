#!/bin/bash -ex

. .travis/common/pre-upload.sh

export TRAVIS_TAG=$CITRA_VERSION
git tag $TRAVIS_TAG || true

REV_NAME="citra-windows-mingw-${CITRA_VERSION}"
ARCHIVE_NAME="${REV_NAME}.tar.gz"
COMPRESSION_FLAGS="-czvf"

mkdir "$REV_NAME"

# get around the permission issues
cp -r package/* "$REV_NAME"

. .travis/common/post-upload.sh
