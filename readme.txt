Simple Viewer is an open source image viewer application.

With cmake you can automatically create a Visual Studio project on Windows or a makefile on Linux.

============================================ Libraries ============================================

The required libraries are:

	1. OpenCV (currently 3.0)
			http://opencv.org/downloads.html
	2. Qt (currently 5.5.1)
	3. Exiv2 (currently 0.25)
			Precompiled for Windows (VS2015, x64): https://bitbucket.org/bastian_weber/simpleviewer/downloads
			Or source: http://www.exiv2.org/download.html

============================================= Windows =============================================

	-	Qt 
		-	Can be downloaded precompiled as installer. QtCreator is _not_ required.
	-	OpenCV 
		-	Can also be downloaded precompiled
	-	Exiv2 
		-	Can be downloaded precompiled (VS2015, x64) from the repository
				https://bitbucket.org/bastian_weber/simpleviewer/downloads
		-	Can also be built from source:
			-	Extract to C:/gnu/exiv2
			-	Download dependencies from
					http://clanmills.com/files/exiv2libs.zip
			-	Extract dependencies to C:/
			-	Rename the folders of the dependencies to match the folder names in 
				C:/gnu/exiv2/msvc2005/copylibs.bat (add the version numbers to the folders)
			-	Run C:/gnu/exiv2/msvc2005/copylibs.bat, this copies the dependencies to the
				correct folders
			-	Open the Visual Studio project inside the folder C:/gnu/exiv2/msvc2005 and
				upgrade to the current compiler version
			-	If you're using VS2015 the compiler will complain that 'tzname' is not defined.
				Replace it by '_tzname'.
			-	Compile
			-	Note: the drive letter does not matter
		-	The directory structure of the compiled Exiv2 library should look like this:

			<root>
				include
					<header files here>
				bin
					vc12 (or the vc version you built for)
						x64
							Debug
								<debug dlls and libs here>
							Release
								<release dlls and libs here>

In cmake just set the necessary paths to the libraries and click configure and generate. 
The dlls should automatically be copied to the Release and Debug directory.

============================================== Linux ==============================================

	-	OpenCV
		-	Can be compiled using cmake and make
	-	Qt
		-	Can be downloaded precompiled as an installer version
	-	Exiv2
		-	If libexpat is missing run 
				sudo apt-get install expat libexpat1-dev
		-	If zlib is missing run
				sudo apt-get install zlibc zlib1g zlib1g-dev
		-	Go to the directory where Exiv2 was extracted
		-	Run ./configure
		-	Run make
		-	Run sudo make install

Then just use cmake as on Windows.