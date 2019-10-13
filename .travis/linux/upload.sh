#!/bin/bash -ex

. .travis/common/pre-upload.sh

export TRAVIS_TAG=$CITRA_VERSION
git tag $TRAVIS_TAG || true

REV_NAME="citra-valentin-linux-${CITRA_VERSION}"
ARCHIVE_NAME="${REV_NAME}.tar.xz"
COMPRESSION_FLAGS="-cJvf"

mkdir "$REV_NAME"

cp build/bin/citra "$REV_NAME"
cp build/bin/citra-room "$REV_NAME"
cp build/bin/citra-qt "$REV_NAME"

. .travis/common/post-upload.sh
