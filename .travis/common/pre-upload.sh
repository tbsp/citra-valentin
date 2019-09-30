#!/bin/bash -ex

export CITRA_VERSION=vvanelslande-$(python3 .travis/common/get-version.py)

git config --local user.name travis-ci
git config --local user.email none

export TRAVIS_TAG=$CITRA_VERSION

mkdir -p artifacts
