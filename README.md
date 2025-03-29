## Make sure ffmpeg is installed!

To compile on Windows (via MinGW x64), Linux or MacOS:

`gcc -O3 hilligoss-nnsts-512.cpp -o hilligoss-nnsts-512 -lm -lstdc++`


To run on Linux or MacOS (make sure process.sh and processall.sh have the executable bit set with `chmod +x ./*.sh`):

`./processall galleria.mp4`


To run on Windows (requires PowerShell 7 or greater!):

`.\process-video-win.ps1 galleria.mp4`


Enjoy!
