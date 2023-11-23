#/bin/bash
script_path=$(dirname $0)
JLinkExe -autoconnect 1 -device VA416XX -if SWD -speed 2000 -commandfile ${script_path}/erase.jlink -jlinkscriptfile ${script_path}/JLinkSettings.JLinkScript
