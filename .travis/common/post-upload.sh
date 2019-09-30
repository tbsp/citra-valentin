#!/bin/bash -ex

# Copy documentation
cp license.txt "$REV_NAME"
cp readme.md "$REV_NAME"

# Copy cross-platform scripting support
cp -r dist/scripting "$REV_NAME"

tar $COMPRESSION_FLAGS "$ARCHIVE_NAME" "$REV_NAME"

mv "$REV_NAME" vvanelslande
7z a "$REV_NAME.7z" vvanelslande

# move the compiled archive into the artifacts directory to be uploaded by Travis releases
mv "$ARCHIVE_NAME" artifacts/
mv "$REV_NAME.7z" artifacts/
