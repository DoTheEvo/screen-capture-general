#!/usr/bin/env bash

# DEPENDENCIES

# byzanz to record gif, https://github.com/GNOME/byzanz
# slop to get coordinates where to record, https://github.com/naelstrof/slop
# sox to play audio

# HOW TO USE

#   make script executable, run the script
#   select area with the mouse
#   records gif of the area for 10 seconds
#   find the done gif in your ~/Pictures/screenshots/

# PASSING ARGUMENTS

# "gif_area 30" - records for 30 seconds
# "gif_area 19 test" - records for 19 secs, file will be called test.gif

type slop byzanz-record play > /dev/null || exit

delay=3
path="$HOME/Pictures/screenshots/gif_$(date +"%s")"

beep() {
    play ~/media/shutter.ogg
}

# deal with passed arguments
if [ $# -gt 1 ]; then
    dur="=$1"
    path="$HOME/Pictures/screenshots/"$2
elif [ $# -gt 0 ]; then
    dur="$1"
else
    dur="10"
fi

# get $X $Y $W $H variables from slop
read -r X Y W H G ID < <(slop -f "%x %y %w %h %g %i")

# show countdown
for (( i=$delay; i>0; --i )) ; do
    echo $i
    sleep 1
done

beep
# record the gif
byzanz-record --verbose --delay=0 --x=$X --y=$Y --width=$W --height=$H --duration=$dur $path.gif
beep
