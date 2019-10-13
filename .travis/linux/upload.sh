#!/bin/bash -ex

. .travis/common/pre-upload.sh

export TRAVIS_TAG=$CITRA_VERSION
git tag $TRAVIS_TAG || true

REV_NAME="citra-valentin-linux-${CITRA_VERSION}"
ARCHIVE_NAME="${REV_NAME}.tar.xz"
COMPRESSION_FLAGS="-cJvf"

mkdir "$REV_NAME"

cp build/bin/citra-valentin "$REV_NAME"
cp build/bin/citra-valentin-qt "$REV_NAME"
cp build/bin/citra-valentin-room "$REV_NAME"

. .travis/common/post-upload.sh
