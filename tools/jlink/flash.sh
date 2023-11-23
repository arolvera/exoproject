#/bin/bash
FILE_NAME=$1
cat > /tmp/flash_loader.jlink << EOF
si 1
speed 2000
r
h
loadbin ${FILE_NAME},0x00000000
r
exit
EOF

JLinkExe -autoconnect 1 -device VA416XX -if SWD -speed 2000 -commandfile /tmp/flash_loader.jlink -jlinkscriptfile JLinkSettings.JLinkScript
