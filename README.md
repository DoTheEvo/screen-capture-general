# maim_area, maim_active_window

Simple screenshot scripts that chose jpg or png depending on file size

#### dependencies
* `maim` for making screenshots
* `slop` to allow maim area screnshots
* `xdotool` to allow active window screenshots
* `imagemagick` for converting png to jpg
* `sox` for playing the shutter sound

`maim_area` take screenshot of the area selected by mouse, `maim_active_window` to take screenshot of currently active window

Pictures are saved in to `~/Pictures/screenshots` with the file name being always unique based on the epoch time(seconds since 1970) - for example `144008968.png`

Imagemagick is then used to create a jpg version of the file. One of the files gets deleted based on the size `(( pngsize < 3 * jpgsize ))`
* jpg file is kept if it is considerably smaller than the png file, which is the case if screenshot contains a movie still or a photograph
* png file is kept in cases of bland text and uniform color use, like websites or desktop/filemanager screenshots

After screenshot is done, sox plays ~/media/shutter.ogg

##### run it

* recommended to bind this script to the PrtScr button

___


# gif_area

Records small size gif of the selected area

#### dependencies
* `byzanz` for recording the gif
* `slop` for getting the recording area coordinates, dimensions
* `sox` for playing the shutter sound

The scripts uses project `byzanz` for recording the gif. It might be problem getting it, Arch users have currently working [AUR package](https://aur.archlinux.org/packages/byzanz-git/?comments=all).
Byzanz does not have area selection by mouse on its own self, but it supports feeding it coordinates manually.
But since no one wants to do it manually `[slop](https://github.com/naelstrof/slop)` is used.

gifs are saved in to `~/Pictures/screenshots` with the file name starting with "gif_" followed by epoch time(seconds since 1970), for example `byz_1440011339.gif`

##### run it

* `gif_area` - records gif for 10 seconds
* `gif_area 19` - records gif for 19 seconds
* `gif_area 120 test` - recording for 2 minutes and the gif name is test.gif

sox plays ~/media/shutter.ogg at the start and at the end of recording

___


# video_area

Records mp4 and webm of the selected area

### dependencies
* `ffmpeg` - for recording the video and coverting to webm
* `slop` for getting the recording area coordinates, dimensions
* `sox` for playing the shutter sound

`slop` is used to make rectangle selection with mouse and then spits out the dimmensions and coordinates of that rectangle.

These coordinates are passed to ffmpeg to record video of that area with default length of 10 seconds or whatever you pass as an argument

For performance benefit, video is recorded as mp4, then converted to webm

Videos are saved in to `~/Pictures/screenshots` with the file name starting with "vid_" followed by epoch time(seconds since 1970), for example `vid_1440011339.webm`

##### run it

* `video_area` - records for 10 seconds
* `video_area 19` - records for 19 seconds
* `video_area 120 test` - recording for 2 minutes and the name would be test,webm

sox plays ~/media/shutter.ogg at the start and at the end of recording

___
