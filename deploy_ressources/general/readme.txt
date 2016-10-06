What Acute Viewer Is

Acute Viewer is a slim image viewer that focuses on high quality image display a and 
minimalist interface. It does not include unnecessary features for things that you 
will use Photoshop for anyway. Instead it serves as an application to have a quick 
look at an image without having to launch a heavier application. It can also be used 
for presentation purposes and to evaluate how an image looks when it is downscaled to 
the actual screen resolution. Optionally, an additional unsharp mask can be applied,
after the downscaling to improve the viewing experience. The image resampling supports
GPU acceleration.

You can use the WinInstaller.exe to install Acute Viewer on your Windows system. The
Visual C++ 2015 Redistributable has to be installed before if not already present.
Refer to the How_to_install.txt for installation instructions on Linux.

Acute Viewer  is open source. The code repsitory can be found at:
https://bitbucket.org/bastian_weber/acuteviewer

===== Supported Filetypes =====

    These filetypes are fully supported:
        
        Jpeg, Tif, Png, Bitmap, Jpeg 2000, WebP, Portable Image Format, Sun Raster

    The following filetypes are partially supported, meaning AcuteViewer extracts
    the preview image from the EXIF if available:
        
        Arw, Dng, Psd, Nef, Cr2, Crw, Mrw, Pef, Rw2, Sr2, Srf, Srw, Orf, Pgf, Raf

===== Controls =====

 OPENING IMAGES

To open an image drag and drop it on the interface or use the open dialog (Ctrl + O).
When you've opened one single image, you can then scroll through the images from the same
folder by using the left and right arrow keys or the forward and backward button on
your mouse.

You can also open multiple images at a time. Then you will not be able to scroll
through the contents of the directory, but only through the images you opened. This can,
for example, be used to compare a few particular images. However, this only works with
images that are in the same directory.

In the file menu you can choose if preview-only images shall be included in the directory
listing. This means that also images that are only displayed by extracting the contained
preview image are included when scrolling through the directory content.

 REFRESHING

Hitting F5 will cause the program to reload the current image and also refresh the
current directory. You can do this to recognise images that have been added or removed
to the directory in the meantime. If you opened multiple images then hitting F5 will
bring you back to directory view.

 ZOOMING

The scroll whel can be used for zooming. Right clicking or the shortcut Ctrl + Alt + 0 
zooms to 100% magnification. Right clicking again or pressing Ctrl + 0 zooms out again.
Holding down the middle mouse button (scroll wheel) and moving the mouse forward or
backwards allows continuous zooming. The current zoom level can be displayed as an overlay.
This overlay is toggled with the Z key. 

 FULLSCREEN

To go to fullscreen mode, double click the canvas or hit F. To exit fullscreen mode,
double click again or hit ESC.

 ROTATING

The view can also be rotated. Please keep in mind that the image is never altered. The
rotation only changes the way it is viewed. To reset the rotation, hit Shift + ESC. If
EXIF-based rotation is enabled, the program rotates images automatically based on the
EXIF information.

 BACKGROUND COLOUR

The background colour can be set to any arbitrary colour. Hit Ctrl + B for blakc, Ctrl + W
for white, Ctrl + G for a dark grey and Ctrl + C for bringing up a colour picker to set a 
custom colour.

 EXIF

Some of the image's EXIF data can be displayed as an overlay. This overlay can be toggled
with the I key.

 MENU

Most controls have keyboard shortcuts assigned to them. However, there is also a menu
that can be brought up by pressing Alt. This menu will automatically hide again. If you
wish to, you can also let it be displayed permanently.

 SLIDESHOW

A slideshow can be started with Ctrl + Space. A dialog will pop up that lets you define
the slideshow options. You can also just hit Space to start a slideshow immediately with
the last used settings. Hit Space again to stop the slideshow.

 GPU ACCELLERATION AND RENDERING

GPU acceleration can be turned on and of. On some graphics cards, performance might be 
better if you leave it turned off. There are also some options regarding how the images
are displayed. It can be selected whether images that are smaller than the window shall
be scaled up (Ctrl + U) and whether pixel values shall be smoothly interpolated when magnification
is above 100% (Ctrl + S).

 POST-RESIZE SHARPENING

There is also a post-resize sharpening filter available. This filter sharpens the image
after it has been downscaled to fit the window's resolution and can be activated with Ctrl + E. 
The options for the filter can be set in a dialog that is brought up with O. The filter
is optimal for presentations, where you want to have the best possible viewing experience.
This way the images do not have to be resized to screen resolution and sharpened
beforehand, because Acute Viewer can do this on the fly.

 DELETING, MOVING AND COPYING IMAGES

There are two shortcuts that can be set up by the user to perform an action such as deleting
a file or copying or moving it to a user-defined folder. These shortcuts can be set up in
"File > Hotkey Options..." (Ctrl + Shift + O). Optionally, sidecar files can be included in
the action. This means that, for example, if an NEF file has an XMP sidecar file there will
be performed the same action to the XMP file as to the NEF file. Be careful with this feature!
You might unintentionally delete files. For example, also a TIF file with the same name of
that NEF file in the same folder would be included in the action.

This feature can then be toggled on and off
by using the shortcut Ctrl + H.

It follows a full list of the available controls

Interface

    Alt Key:                                    Show menu bar; will auto-hide again 
                                                upon next action
    M Key:                                      Toggle option to always show menu bar 
                                                on and off

View Navigation

    Any Mouse Button                            Hold and drag to pan
    Right Mouse Button                          Click to toggle 100% view and back
    Mouse Wheel                                 Zoom in and out; hold Ctrl to zoom in 
                                                smaller steps
    Middle Mouse Button (Mouse Wheel)           Hold and drag mouse forth or back to zoom
    + and - Keys                                Same as scroll wheel
    Ctrl + 0                                    Zoom to fit
    Ctrl + Alt + 0                              Zoom to 100%
    Shift + Mouse Wheel                         Rotate view in steps of 10°  
    Ctrl + Left Arrow Key                       Rotate view 90° to the left
    Ctrl + Right Arrow Key                      Rotate view 90° to the right
    Ctrl + R Key                                Toggle EXIF-based image rotation
    Shift + Escape Key                          Reset view rotation
    Left Mouse button                           Double click to toggle full-screen mode
    F Key                                       Toggle full-screen mode

Browsing Images & Slideshow

    Left and Right Arrow Keys                   Go to previous or next image
    Up and Down Arrow keys                      Same as left and right arrow keys
    F5 Key                                      Reload the current image and refresh
                                                the list of files in the current directory
    Ctrl + P Key                                Toggle include preview images
    Space Key                                   Toggle slideshow play/pause
    Ctrl + Space Key                            Toggle slideshow play/pause; dialog will 
                                                appear that lets you choose the slide 
                                                duration

Image Rendering and Display Options

    I Key                                       Toggle info and EXIF overlay on and off
    Z Key                                       Toggle zoom level overlay on and off
    Ctrl + B Key                                Set black background colour
    Ctrl + W Key                                Set white background colour
    Ctrl + G Key                                Set dark grey background colour
    Ctrl + C Key                                Set custom background colour
    Ctrl + Shift + Left or Right Arrow Keys     Change the font size of the text overlay
    Ctrl + Shift + Up or Down Arrow Keys        Change the line spacing of the text overlay
    Ctrl + U Key                                Toggle option to upscale smaller images to 
                                                fit the window on and off
    Ctrl + S Key                                Toggle the use of a smooth interpolation 
                                                method instead of nearest neighbour when 
                                                enlarging above 100%
    Ctrl + E Key                                Toggle post-resize sharpening (effect)
    O Key                                       Show sharpening options dialog

Actions

    Ctrl + H                                    Toggle custom hotkeys on or off
    Ctrl + Shift + O                            Show custom hotkey options

Application Shortcuts

    Ctrl + O                                    Open a file (shows open dialog)
    Ctrl + Q                                    Quit the application