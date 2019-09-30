#!/bin/bash -ex

export CITRA_VERSION=vvanelslande-$(python3 .travis/common/get_version.py)

git config --local user.name travis-ci
git config --local user.email none

mkdir -p artifacts
