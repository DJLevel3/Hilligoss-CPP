## Make sure ffmpeg is installed!

To compile on Linux or MacOS:

`gcc -O3 hilligoss-nnsts-512.cpp -o hilligoss-nnsts-512 -lm -lstdc++`


To compile on Windows (in a Visual Studio 2022 Developer PowerShell window):

`cl /EHsc hilligoss-nnsts-512.cpp`


To run on Linux or MacOS:

`./processall galleria.mp4`


To run on Windows (requires PowerShell 7 or greater!):

`.\process-video-win.ps1 galleria.mp4`


Enjoy!
