#!/usr/bin/env bash

#takes area screenshot, compares jpg/png
#keeps jpg only if its considerably smaller than png

#maim to take screenshots
#slop to allow maim take area screenshots
#xdotool to allow maim take active window screenshots
#sox to play audio file
#imagemagick to convert pictures

type maim slop convert play > /dev/null || exit

shot=$HOME/Pictures/screenshots/$(date +%s)
maim -s -b 1 "$shot.png" || exit
convert "$shot.png" "$shot.jpg" || exit

read -r pngsize _ < <(wc -c "$shot.png")
read -r jpgsize _ < <(wc -c "$shot.jpg")

if (( pngsize < 3 * jpgsize )); then
    rm "$shot.jpg"
else
    rm "$shot.png"
fi

play $HOME/media/shutter.ogg
