#!/usr/bin/env bash

# DEPENDENCIES

# ffmpeg to record mp4 video and convert to webm
# slop to get coordinates where to record, https://github.com/naelstrof/slop
# sox to play audio file

#HOW TO USE

#   make script executable, run the script
#   select area with the mouse
#   records the area for 10 seconds
#   find the done webm in your ~/Pictures/screenshots/

# PASSING ARGUMENTS

# "video_area 30" -records for 30 seconds
# "video_area 19 test" - records for 19 seconds, file will be named test.webm

# if flickering, try killing your compositor before starting recording

type slop ffmpeg play > /dev/null || exit

delay=3
path="$HOME/Pictures/screenshots/vid_$(date +"%s")"

beep() {
    play ~/media/shutter.ogg
}

# deal with passed arguments
if [ $# -gt 1 ]; then
    dur="$1"
    path=~/Pictures/screenshots/$2
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
# record mp4
ffmpeg -video_size $W\x$H -framerate 25 -f x11grab -i :0+$X,$Y -t $dur $path.mp4
beep
# convert to webm
ffmpeg -i $path.mp4 -an -qmax 40 -threads 2 -c:v libvpx $path.webm
# delete the original mp4 file
rm "$path.mp4"
