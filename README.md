# Summary

I named this window system as "KiWin GUI System" because I like eating kiwi fruits.

I have some screenshots for the KiWin GUI System. If you are interested, you can see those pictures at the bottom of this web page.

* As to the graphic files: I finished the reading, writing and the display for the png file format. I used libpng to implement the low level of this approach. Furthermore, the alpha channel of the png file can be correctly displayed.
* Displaying the truetype font: I used the FreeType2 library to be the low level of this approach. That is to say, the KiWin GUI system can correctly display Chinese fonts, which you can see in the following screenshots. If you want to use Chinese truetype fonts, you can get bkai00mp.ttf or bsmi00lp.ttf files, and put them into the KWDIR/Font directory.
* As to the window system: it includes parent, child window, and main window. It has some functions. For instance, the window can be enlarged as well as reduced. You can also create a new one and close a window which has existed. I choose the microwindow architecture to be the clipping in KiWin module.
* I imitated the internal register file of the gnome2 and the Microsoft OSes, and implemented an internal register file to store the relative information.
* I also implemented a Chinese input program, which only supports the zoyin input method now. You can see the display of the program in the following screenshots. The users can choose the Chinese characters they want in the input method window.
* The underlying architecture of the video layer only supports the frame buffer device now. The display mechanism of the 24-bit and 32-bit mode is correct, and that of the 8-bit and 16-bit might be OK (for it is not tested). The screen resolution has no limit because the KiWin will detect it automatically. (1024 x 768, 800 x 600, 640 x 480, etc).
* KiWin uses xpm3 graphic format as its Cursor file format now. When the cursor is moved to the edge of a window, it's shape changes from single-angled to double-angled.
* The KiWin supports 2 user GUI components - pressed button and textbox. KiWin itself can memorize which button is pressed last, and change the shape of the button. In the textbox, users can input English characters or run the Chinese input program to input Chinese characters. (see the following screenshots)
* Like the linux console mode, press the Alt-F1, Alt-F2 etc, to change to another virtual terminal.
* Press the 'Print Screen' key to scratch a screenshot to the KWDIR/SrcBin/kiwin-screenshot.png.
* KiWin is released under the GPL, and the copyright of all the libraries KiWin has used is under GPL.

# How to compile

It's easy to compile KiWin. You need to get the source code, extract it into some directory, and set the KWDIR environment variables. (ex: If you extract the source code to the /home/mango, you have to 'export KWDIR=/home/mango')

Finally, type 'make' and 'make program' to finish the compilation and installation. The 'make' is to build KiWin GUI server and the client library, and the 'make program' is to build the Chinese input program, register file renderer, and the demo program.

# How to run

Enter the KWDIR/SrvBin directory, and execute the kiwin executable to start the whole KiWin GUI system. To run the Chinese input program is trivial. Press ctrl-space key to start it. (Attention: Because of the copy-right problem, there is no pre-installed Chinese font in the KiWin. You have to find some Chinese font and put them into the KWDIR/Font directory to run the KiWin correctly.)

If you want to run the demo program, press Alt-F2 change to another virtual terminal, and enter KWDIR/Program_files/demo1/bin. Then, run the demo1 program. After that, you will see the display of the demo1 program on the screen.

# Current Released Version

The current version of KiWin is 0.1pre1, and the next version (0.2 by default) is developing. In the next version, KiWin GUI system will have a whole new internal architecture and more functions.

# A small bootable-cdrom

I released a kernel patch before. Using that kernel patch, you can hide the linux os under the Microsoft OSes. Therefore, you can run the linux in a no pre-installation environment. (For example, if all the hard disk space is used by the Microsoft OSes, no other free space is left) In addition, I also released a series of shell scripts in the slat-tech mailing list on 6/3/2002. Using those shell scripts, you can build a simple bootable cdrom to install a linux where its kernel is patched by my kernel patch. Now, with the creation of KiWin GUI system, I hope I can build a small linux system which uses less then 5MB disk space.

If you are interested in this kernel patch, you can click on the following web link to get more information.

[My Linux kernel patch](http://wei-hu-tw.blogspot.com/2007/11/linux-ms-windows-kernel-patch.html)

# Screenshots

demo1 program

![](http://lh5.google.com/wei.hu.tw/RzxWlbZHyLI/AAAAAAAAAGQ/eKp_7rrCdOk/Screenshot-demo1.png)

Chinese input method: screenshot 1

![](http://lh3.google.com/wei.hu.tw/RzxWk7ZHyII/AAAAAAAAAF4/poLH0BMVSCU/kiwin-screenshot-ime.png)

Chinese input method: screenshot 2

![](http://lh6.google.com/wei.hu.tw/RzxWrrZHyNI/AAAAAAAAAGg/N0UdJ5tH3V4/Screenshot-ime.png)

Press the left key of the mouse to move a window

![](http://lh3.google.com/wei.hu.tw/RzxWr7ZHyOI/AAAAAAAAAGo/433Brfc5uZM/Screenshot-movewindow.png)

Press the the cross icon on the right top to close a window

![](http://lh5.google.com/wei.hu.tw/RzxWlbZHyMI/AAAAAAAAAGY/s0-c2--yy5g/Screenshot-destroywindow.png)

Press the new window button to create a new window, and this newly created window will treat window 7 as its parent window.

![](http://lh4.google.com/wei.hu.tw/RzxWlLZHyKI/AAAAAAAAAGI/ZWqQBABAmXU/Screenshot-createwindow.png)

Close window 1 (i.e. the main window of this demo1 program) to close the whole demo1 program.

![](http://lh4.google.com/wei.hu.tw/RzxWlLZHyJI/AAAAAAAAAGA/Wn310bgXEP8/Screenshot-closeall.png)
