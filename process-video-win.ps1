del .\frames\*.pgm
del .\frames\*.pgm.pcm
ffmpeg -i "$args" -r 24 -an -vf "scale=(iw*sar)*min(512/(iw*sar)\,512/ih):ih*min(512/(iw*sar)\,512/ih), pad=512:512:(512-iw*min(512/iw\,512/ih))/2:(512-ih*min(512/iw\,512/ih))/2" frames/out-%06d.pgm 
Get-ChildItem ".\frames\*.pgm" | ForEach-Object -Parallel { .\hilligoss-nnsts-512.exe -f "$($_)" -j 1600 -w 200 -s -c 8000 } -ThrottleLimit $env:NUMBER_OF_PROCESSORS 
Get-Content .\frames\*.pcm -AsByteStream -Read 512 | Set-Content result.pcm -AsByteStream
ffmpeg -y -f s16le -sample_fmt s16le -ar 192000 -ac 2 -i result.pcm -c:a pcm_s16le out.wav