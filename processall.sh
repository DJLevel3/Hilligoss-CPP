#!/bin/sh
set -e
src=$1
nproc=7

## Set nproc to however many cores your system has available.

echo
echo Hilligoss: Starting up!
echo Hilligoss: Number of concurrent threads is now set to $nproc.
echo Hilligoss: Creating work subdirectory for extracted frames..
mkdir -p frames 
touch frames/out.pcm
echo Hilligoss: Determing current time..
timestamp=`date +%m%d%y-%H%M%S`
echo Hilligoss: Removing old frames if any..
find frames -type f | xargs rm 
echo Hilligoss: Extracting PGM frames from $src..


## CROP A 512x512 FROM THE CENTER OF THE VIDEO
#ffmpeg -loglevel quiet -i "$src" -r 30 -an -filter:v "minterpolate='mi_mode=mci:mc_mode=aobmc:vsbmc=1:fps=24'" -vf "scale=-1:512,crop=512:512" frames/out-%09d.pgm 
#ffmpeg -loglevel quiet -i "$src" -r 30 -an -vf "scale=-1:512,crop=512:512" frames/out-%09d.pgm 

### LETTERBOX THE VIDEO WITH VERTICAL RESOLUTION OF 512P
ffmpeg -loglevel quiet -i "$src" -r 24 -an -vf "scale=(iw*sar)*min(512/(iw*sar)\,512/ih):ih*min(512/(iw*sar)\,512/ih), pad=512:512:(512-iw*min(512/iw\,512/ih))/2:(512-ih*min(512/iw\,512/ih))/2" frames/out-%06d.pgm 


echo Hilligoss: Post-processing frames into singular .wav files..
find frames -type f -name '*.pgm' | sort -n | sed 's/\.pgm$//' | xargs -I'{}' -P "$nproc" ./process.sh '{}' 
echo Hilligoss: Concatenating PCM files...
printf '' > out.pcm 
find frames -type f -name '*.pcm' | sort -n | xargs -I'{}' cat '{}' >> out.pcm 
echo Hilligoss: Prefixing AIFF WAV header onto reassembled PCM data..


ffmpeg -loglevel quiet -y -f s16le -sample_fmt s16le -ar 48000 -ac 2 -i out.pcm -c:a pcm_s16le out.wav

### To overample by 4X, multiply your desired count (-c) in Hilligoss by four, and encode using this line:
### ffmpeg -loglevel quiet -y -f s16le -sample_fmt s16le -ar 192000 -ac 2 -i out.pcm -c:a pcm_s16le out.wav



echo Hilligoss: Renaming workfile to hilligoss-$timestamp.wav..
mv out.wav hilligoss-$timestamp.wav 
echo Hilligoss: $src encoding run commpleted.
echo
ls -l $PWD/hilligoss-$timestamp.wav 
echo

