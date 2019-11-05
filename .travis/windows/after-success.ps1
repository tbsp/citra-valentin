$Version = python ".\.travis\common\get_version.py"

Set-Location ".\build\bin"
Rename-Item "Release" "citra-valentin-windows-$Version"
7z a "citra-valentin-windows-$Version.7z" "citra-valentin-windows-$Version"

Get-ChildItem
