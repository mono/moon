mono --debug audit.exe /opt/mono/lib/moonlight/plugin audit $1
grep unaudited audit/*.audit | wc -l

