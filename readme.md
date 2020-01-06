# Acute Viewer

## Fast High-Quality Open-Source Image Viewer

![Screenshot][screenshot]

### What Acute Viewer Is

Acute Viewer is a slim image viewer that focuses on high quality image display and a minimalist interface. It does not include unnecessary features for things that you will use Photoshop for anyway. Instead it serves as an application to have a quick look at an image without having to launch a heavier application. It can also be used for presentation purposes and to evaluate how an image looks when it is downscaled to the actual screen resolution.

### Latest Release: 1.6

Binaries are available for download from the Github [releases section](https://github.com/bastian-weber/AcuteViewer/releases). Note: The Visual C++ 2015 Redistributable is required on Windows (included).

New in latest Release:

* User-defined hotkeys for deleting, moving or copying a file
* Version information dialog and automatic update reminders
* Executables are now digitally signed

### Features

* High quality downscaling algorithm - best quality at any zoom level
* Optional and customisable post-resize sharpening
* GPU acceleration
* Allows skipping through images in the current folder
* EXIF display (currently just overlay, more comprehensive EXIF panel planned for future releases)
* Automatic slideshow support with user defined slide duration
* Fullscreen mode
* Minimalist interface focused on hotkey controls
Supported filetypes: Jpeg, Tif, Png, Bitmap, Jpeg 2000, WebP, Portable Image Format, Sun Raster
* Partially supported filetypes (contained preview image is displayed): Arw, Dng, Psd, Nef, Cr2, Crw, Mrw, Pef, Rw2, Sr2, Srf, Srw, Orf, Pgf, Raf

### Controls

##### Opening Images

To open an image drag and drop it on the interface or use the open dialog (Ctrl + O). When you've opened one single image, you can then scroll through the images from the same folder by using the left and right arrow keys or the forward and backward button on your mouse.

You can also open multiple images at a time. Then you will not be able to scroll through the contents of the directory, but only through the images you opened. This can, for example, be used to compare a few particular images. However, this only works with images that are in the same directory.

In the file menu you can choose if preview-only images shall be included in the directory listing. This means that also images that are only displayed by extracting the contained preview image are included when scrolling through the directory content.

##### Refreshing

Hitting F5 will cause the program to reload the current image and also refresh the current directory. You can do this to recognise images that have been added or removed to the directory in the meantime. If you opened multiple images then hitting F5 will bring you back to directory view.

##### Zooming

The scroll whel can be used for zooming. Right clicking or the shortcut Ctrl + Alt + 0 zooms to 100% magnification. Right clicking again or pressing Ctrl + 0 zooms out again. Holding down the middle mouse button (scroll wheel) and moving the mouse forward or backwards allows continuous zooming. The current zoom level can be displayed as an overlay. This overlay is toggled with the Z key.

##### Fullscreen

To go to fullscreen mode, double click the canvas or hit F. To exit fullscreen mode, double click again or hit ESC.

##### Rotating

The view can also be rotated. Please keep in mind that the image is never altered. The rotation only changes the way it is viewed. To reset the rotation, hit Shift + ESC. If EXIF-based rotation is enabled, the program rotates images automatically based on the EXIF information.

##### Background Colour

The background colour can be set to any arbitrary colour. Hit Ctrl + B for blakc, Ctrl + W for white, Ctrl + G for a dark grey and Ctrl + C for bringing up a colour picker to set a custom colour.

##### EXIF

Some of the image's EXIF data can be displayed as an overlay. This overlay can be toggled with the I key.

##### Menu

Most controls have keyboard shortcuts assigned to them. However, there is also a menu that can be brought up by pressing Alt. This menu will automatically hide again. If you wish to, you can also let it be displayed permanently.

##### Slideshow

A slideshow can be started with Ctrl + Space. A dialog will pop up that lets you define the slideshow options. You can also just hit Space to start a slideshow immediately with the last used settings. Hit Space again to stop the slideshow.

##### GPU Acceleration and Rendering

GPU acceleration can be turned on and of. On some graphics cards, performance might be better if you leave it turned off. There are also some options regarding how the images are displayed. It can be selected whether images that are smaller than the window shall be scaled up (Ctrl + U) and whether pixel values shall be smoothly interpolated when magnification is above 100% (Ctrl + S).

##### Post-Resize Sharpening

There is also a post-resize sharpening filter available. This filter sharpens the image after it has been downscaled to fit the window's resolution and can be activated with Ctrl + E. The options for the filter can be set in a dialog that is brought up with O. The filter is optimal for presentations, where you want to have the best possible viewing experience. This way the images do not have to be resized to screen resolution and sharpened beforehand, because Acute Viewer can do this on the fly.

##### Deleting, Moving and Copying Images

There are two shortcuts that can be set up by the user to perform an action such as deleting a file or copying or moving it to a user-defined folder. These shortcuts can be set up under "File > Hotkey Options..." (Ctrl + Shift + O). Optionally, sidecar files can be included in the action. This means that, for example, if an NEF file has an XMP sidecar file there will be the same action performed on the XMP file as to the NEF file. There are two options. The first one is to include all sidecar files in the action. Be careful with this feature! You might unintentionally delete files. For example, a TIF file in the same folder with the same name as that NEF file would be included in the action as well. For the comparison only the base name of the file is used, i.e. an action on the file abc.xy would include the file abc.z.xy. The second option only indcludes XMP sidecar files in the action, and only if the main file is a raw image. These custom hotkeys can then be toggled on and off by using the shortcut Ctrl + H.

#### Interface

* __Alt Key__: Show menu bar; will auto-hide again upon next action
* __M Key__: Toggle option to always show menu bar on and off

#### View Navigation

* __Any Mouse Button__: Hold and drag to pan
* __Right Mouse Button__: Click to toggle 100% view and back
* __Mouse Wheel__: Zoom in and out; hold Ctrl to zoom in smaller steps
* __Middle Mouse Button (Mouse Wheel)__: Hold and drag mouse forth or back to zoom
* __+ and - Keys__: Same as scroll wheel
* __Ctrl + 0__: Zoom to fit
* __Ctrl + Alt + 0__: Zoom to 100%
* __Shift + Mouse Wheel__: Rotate view in steps of 10°
* __Ctrl + Left Arrow Key__: Rotate view 90° to the left
* __Ctrl + Right Arrow Key__: Rotate view 90° to the right
* __Ctrl + R Key__: Toggle EXIF-based image rotation
* __Shift + Escape Key__: Reset view rotation
* __Left Mouse button__: Double click to toggle full-screen mode
* __F Key__: Toggle full-screen mode

#### Browsing Images & Slideshow

* __Left and Right Arrow Keys__: Go to previous or next image
* __Up and Down Arrow Keys__: Same as left and right arrow keys
* __Backward and Forward Mouse Buttons__: Same as left and right arrow keys
* __F5 Key__: Reload the current image and refresh the list of files in the current directory
* __Ctrl + P Key__: Toggle include preview images
* __Ctrl + Space Key__: Toggle slideshow play/pause; dialog will appear that lets you choose the slide duration.
* __Space Key__: Toggle slideshow play/pause.

#### Image Rendering and Display Options

* __I Key__: Toggle info and EXIF overlay on and off
* __Z Key__: Toggle zoom level overlay on and off
* __Ctrl + B Key__: Set black background colour
* __Ctrl + W Key__: Set white background colour
* __Ctrl + G Key__: Set dark grey background colour
* __Ctrl + C Key__: Set custom background colour
* __Ctrl + Shift + Left or Right Arrow Keys__: Change the font size of the text overlay
* __Ctrl + Shift + Up or Down Arrow Keys__: Change the line spacing of the text overlay
* __Ctrl + U Key__: Toggle option to upscale smaller images to fit the window on and off
* __Ctrl + S Key__: Toggle the use of a smooth interpolation method instead of nearest neighbour when enlarging above 100%
* __Ctrl + E Key__: Toggle post-resize sharpening (effect)
* __O Key__: Show sharpening options dialog

#### Application Shortcuts

* __Ctrl + O__: Open a file (shows open dialog)
* __Ctrl + Q__: Quit the application

[screenshot]: readme_images/screenshot.png