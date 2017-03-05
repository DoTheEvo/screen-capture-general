# make_screenshot.sh

Simple screenshot scripts that choses jpg or png depending on file size

#### dependencies
* `maim` for making screenshots
* `slop` to allow maim area screnshots
* `xdotool` to allow active window screenshots
* `imagemagick` for converting png to jpg
* `sox` for playing the shutter sound

Clicking on an active window of an application or a dekstop will select it whole, dragging rectangle will pick that selection

Pictures are saved in to `~/Pictures/screenshots` with the file name being always unique based on the epoch time(seconds since 1970) - for example `144008968.png`

Imagemagick is then used to create a jpg version of the file. One of the files gets deleted based on the size `(( pngsize < 3 * jpgsize ))`
* jpg file is kept if it is considerably smaller than the png file, which is the case if screenshot contains a movie still or a photograph
* png file is kept in cases of bland text and uniform color use, like websites or desktop/filemanager screenshots

After screenshot is done, sox plays ~/media/shutter.ogg

##### run it

* recommended to bind this script to the PrtScr button

___


# make_gif.sh

Records small size gif of the selected area

#### dependencies
* `byzanz` for recording the gif
* `slop` for getting the recording area coordinates, dimensions
* `sox` for playing the shutter sound

The scripts uses project `byzanz` for recording the gif. It might be problem getting it, Arch users have currently working [AUR package](https://aur.archlinux.org/packages/byzanz-git/?comments=all).

Clicking on an active window of an application or a dekstop will select it whole, dragging rectangle will pick that selection

gifs are saved in to `~/Pictures/screenshots` with the file name starting with "gif_" followed by epoch time(seconds since 1970), for example `gif_1440011339.gif`

##### run it

* `make_gif.sh` - records gif for 10 seconds
* `make_gif.sh 19` - records gif for 19 seconds
* `make_gif.sh 120 test` - recording for 2 minutes and the gif name is test.gif

sox plays ~/media/shutter.ogg at the start and at the end of recording

___


# make_video.sh

Makes webm recording of selected area

### dependencies
* `ffmpeg` - for recording the video and coverting to webm
* `slop` for getting the recording area coordinates, dimensions
* `sox` for playing the shutter sound

Clicking on an active window of an application or a dekstop will select it whole, dragging rectangle will pick that selection

For performance benefit, video is recorded as mp4, then converted to webm

Videos are saved in to `~/Pictures/screenshots` with the file name starting with "vid_" followed by epoch time(seconds since 1970), for example `vid_1440011339.webm`

##### run it

* `make_video.sh` - records for 10 seconds
* `make_video.sh 19` - records for 19 seconds
* `make_video.sh 120 test` - recording for 2 minutes and the name would be test,webm

sox plays ~/media/shutter.ogg at the start and at the end of recording

___
