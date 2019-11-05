mkdir build
Set-Location build

cmake -DCITRA_ENABLE_DISCORD_RP=ON -DENABLE_FFMPEG_VIDEO_DUMPER=ON ..
cmake --build . --config Release
