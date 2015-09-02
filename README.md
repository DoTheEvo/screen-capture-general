# maim_area, maim_active_window

Simple area screenshot, active window screenshot, making scripts that choses jpg or png depending on file size

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
* `xrectsel` for getting the recording area coordinates, dimensions
* `sox` for playing the shutter sound

The scripts uses dead project `byzanz` for recording the gif. It might be problem getting it, Arch users have currently working [AUR package](https://aur.archlinux.org/packages/byzanz-git/?comments=all). Byzanz does not have area selection by mouse on its own self, but it supports feeding it coordinates manually. But since no one wants to do it manually `xrectsel` is used.

`xrectsel` is a single binary file that you place in the same directory as `gif_area.sh` or in to `/usr/bin/`. You can run it on its own and see how it allows you to make rectangle selection with mouse and then spits out the dimmensions and coordinates of that rectangle.

gifs are saved in to `~/Pictures/screenshots` with the file name starting with "gif_" followed by epoch time(seconds since 1970), for example `byz_1440011339.gif`

##### run it

* `gif_area` - records gif for 10 seconds
* `gif_area 19` - records gif for 19 seconds
* `gif_area 120 testing` - recording for 2 minutes and the gif name is testing.gif

sox plays ~/media/shutter.ogg at the start and at the end of recording

___


# video_area

Records mp4 and webm of the selected area

### dependencies
* `ffmpeg` - for recording the video and coverting to webm
* `xrectsel` for getting the recording area coordinates, dimensions
* `sox` for playing the shutter sound

`xrectsel` is a single binary file that you place in the same directory as `video_area` or in to `/usr/bin/`. You can run it on its own and see how it allows you to make rectangle selection with mouse and then spits out the dimmensions and coordinates of that rectangle.

These coordinates are feeded to ffmpeg to record video of that area with default length of 10 seconds or whatever you pass as an argument

gifs are saved in to `~/Pictures/screenshots` with the file name starting with "vid_" followed by epoch time(seconds since 1970), for example `vid_1440011339.mp4` & `vid_1440011339.webm`

##### run it

* `video_area` - records for 10 seconds
* `video_area 19` - records for 19 seconds
* `video_area 120 testing` - recording for 2 minutes and the names are testing.mp4 and testing,webm

sox plays ~/media/shutter.ogg at the start and at the end of recording

___
