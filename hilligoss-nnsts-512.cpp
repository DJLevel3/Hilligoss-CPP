//
// Hilligoss-NNSTS-512 v1.1b released 012925:0228 by Bus Error Collective
// 
// Hilligoss will convert any 8-bit 512x512 Binary/"Raw" PGM image to PCM stereo audio data
// suitable for layback on XY-mode oscilloscopes and similar visualizers. It is flexible 
// enough to accommodate a wide range of sample/frame rates, and smart enough about image
// quality to render excellent looking output. 
//
// Hilligoss uses an image reduction and conversion algorithm which favors bright pixels
// over dark pixels when rendering.
//
// Nearest Neighbor Simplified Travelling Salesman (NNSTS) version


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <string>
#include <fstream>
#include <vector>
#include <time.h>
//#include <iostream>

#define gridSize 512

std::ifstream inputFile;                // File handle..
std::ofstream outputFile;               // File handle..
int arg = 0;                      // For parsing command line arguments..
int debug = 0;                    // Turn debugging statements on or off.
int silent = 0;
int visited[512][512] = { 0 };      // Map of visited pixels..
int pixelBuffer[512][512] = { 0 };  // Image buffer for the PGM we've loaded..
int startBuffer[512][512] = { 0 };  // Image buffer for the PGM we've loaded..
int i = 0;                        // Throw-away temp variable, usually the brightness of the current pixel.
int x = 0;                        // Current X coordinate..
int y = 0;                        // Current Y coordinate..
int whiteThreshold = 255;         // Pixels with brightness values above this number will be considered white.
int blackThreshold = 0;           // Pixels with brightness values below this number will be considered black.
int originalPixelCount = 0;       // The original number of non-black pixels of the original image.
int pixelCount = 0;               // The number of pixels in the image buffer after counting.
int removalAttempts = 0;          // The number of attempted removals so far.
int pixelsRemoved = 0;            // The number of pixels removed to achieve the desired goal.
int jumpFrequency = 250;		// How often the travelling salesman randomly jumps to a new spot..
int targetCount = 2000;              // The desired number of pixels this image should be reduced to.
int thisPixel = 0;                // Brightness of the current pixel.
int xPreviewSize = 80;            // Width of the ASCII art preview in characters.
int yPreviewSize = 45;            // Height of the ASCII art preview in characters.
int startX = 0;                   // The X coordinate of the salesman's starting point.
int startY = 0;                   // The Y coordinate of the salesman's starting point.
int pixelsWritten = 0;            // Number of pixels committed to file.
int bytesWritten = 0;             // The total number of bytes written to outputfilename.pcm..
int fillerPixels = 0;             // The total amount of filler written to complete the frame..
int pixelList[262144][2];	// List of coordinates to help the travelling salesman..
int pixelListCount = 0;		// Counter for said list..
int16_t xShort = 0;		// Temporary X coordinate to be written out to file..
int16_t yShort = 0;		// Temporary Y coordinate to be written out to file..
std::string inputFileName;           // The name of the PGM file we're going to read from..
std::string outputFileName;           // The name of the PCM file we're going to write to..
char previewSize[8];		// Preview window size string for status line..
char c = 0;                       // Value of current pixel being read from file.
char gradient[] = " `.-'_,:;><+!rc*/z?LT)J(|F{CfIeZYja2SwqPhdVOKXH#$@Bg0NWQ%&M";
char header[] = "Hilligoss-NNSTS-512 v1.0:";

void readImage(std::string inputFileName)
{
    if (debug) printf("DEBUG: Inside readImage..\n");

    inputFile = std::ifstream(inputFileName.c_str(), std::ios_base::binary);
    std::vector<unsigned char> inputBuffer(std::istreambuf_iterator<char>(inputFile), {});

    int iter = 0;
    for (i = 0; i < 3; i++) {
        while ((c = inputBuffer[iter]) != '\n') iter++;    // We can safely ignore the header of the PGM file. 
        iter++;
    }

    for (y = 0; y < gridSize; y++)
    {
        for (x = 0; x < gridSize; x++)
        {
            thisPixel = inputBuffer[iter];
            iter++;
            if (thisPixel < blackThreshold) thisPixel = 0;
            if (thisPixel > whiteThreshold) thisPixel = 255;
            pixelBuffer[x][y] = thisPixel;
            startBuffer[x][y] = thisPixel;
            if (thisPixel > 0) originalPixelCount++;
        }
    }

    if (debug) printf("DEBUG: readImage: Found %d original pixels.\n", originalPixelCount);
    pixelCount = originalPixelCount;
    inputFile.close();
    inputBuffer.clear();
    if (debug) printf("DEBUG: Leaving readImage..\n");
}

void showPreview(void)
{
    if (debug) printf("DEBUG: Inside showPreview..\n");
    const int max = 524200 + 1024;
    char output[max] = { 0 };  // Just enough but not too far..
    int pos = 0;


    if (silent == 0) {
        snprintf(previewSize, 8, "%dx%d", xPreviewSize, yPreviewSize);

        pos += snprintf(output + pos, max - pos, "%s Input:  [%15s]     Source: [%6dpx] White Threshold: [%3d/255] Filler:  [%7dpx]\n                          Output: [%15s] Target: [%6dpx] Black Threshold: [%3d/255] Preview: [%7spx]\n", header, inputFileName.c_str(), originalPixelCount, whiteThreshold, fillerPixels, outputFileName.c_str(), targetCount, blackThreshold, previewSize);

        pos += snprintf(output + pos, max - pos, "p");

        for (x = 0; x <= (xPreviewSize * 2); x++)
        {
            pos += snprintf(output + pos, max - pos, "-");
        }

        pos += snprintf(output + pos, max - pos, "q\n");

        for (y = 0; y < yPreviewSize; y++)
        {
            pos += snprintf(output + pos, max - pos, "|");

            for (x = 0; x < xPreviewSize; x++)
            {
                pos += snprintf(output + pos, max - pos, "%c", gradient[(startBuffer[x * 512 / xPreviewSize][y * 512 / yPreviewSize] * (strlen(gradient))) / 256]);
            }

            pos += snprintf(output + pos, max - pos, "|");

            for (x = 0; x < xPreviewSize; x++)
            {
                pos += snprintf(output + pos, max - pos, "%c", gradient[(pixelBuffer[x * 512 / xPreviewSize][y * 512 / yPreviewSize] * (strlen(gradient))) / 256]);
            }

            pos += snprintf(output + pos, max - pos, "|");

            pos += snprintf(output + pos, max - pos, "\n");
        }

        pos += snprintf(output + pos, max - pos, "b");

        for (x = 0; x <= (xPreviewSize * 2); x++)
        {
            pos += snprintf(output + pos, max - pos, "-");
        }

        pos += snprintf(output + pos, max - pos, "d\n");
        printf("%s\n", output);
    }
    else {
        printf("%s\n", inputFileName.c_str());
    }

    if (debug) printf("DEBUG: Leaving showPreview..\n");
}

void reducePixels(void)
{
    struct timespec tv;
    int probabilityCurve = 0;
    int diceRoll = 0;
    if (!timespec_get(&tv, TIME_UTC)) return;
    srand(tv.tv_sec ^ tv.tv_nsec);

    if (pixelCount <= targetCount) return;

    while (pixelCount > targetCount && removalAttempts < 1000000)
    {
        x = rand() % gridSize;
        y = rand() % gridSize;

        /*if (rand()%whiteThreshold>=pixelBuffer[x][y]) Hilligoss 1.0*/
//probabilityCurve=(int)whiteThreshold*pow(cos(0.005*pixelBuffer[x][y]),9); // Secret sauce
//printf("WT: %4d    Value: %5d   Curve: %5d   DiceRoll: %5d\n",whiteThreshold, pixelBuffer[x][y],probabilityCurve,diceRoll);


//if (pixelBuffer[x][y] <= rand()%255)
        if (rand() % whiteThreshold < sin(3.1415926535 / 2 * (whiteThreshold - pixelBuffer[x][y]) / whiteThreshold) * whiteThreshold * 0.9) // Extremely secret sauce
        {
            if (pixelBuffer[x][y] != 0)
            {
                pixelBuffer[x][y] = 0;
                pixelCount--;
                pixelsRemoved++;
                removalAttempts = 0;
            }
        }

        removalAttempts++;
    }

    if (debug) printf("DEBUG: Leaving reducePixels. %d pixels removed.\n", pixelsRemoved);
}


void determinePath()
{
    if (debug) printf("DEBUG: Inside determinePath..\n");

    outputFile = std::ofstream(outputFileName.c_str(), std::ios_base::binary);

    std::vector<int> pixelList[2];
    pixelList[0].resize(gridSize * gridSize);
    pixelList[1].resize(gridSize * gridSize);
    int pixelListCount = 0;

    for (x = 0; x < gridSize; x++)
    {
        for (y = 0; y < gridSize; y++)
        {
            if (pixelBuffer[x][y] > 0)
            {
                pixelList[0][pixelListCount] = x;
                pixelList[1][pixelListCount] = y;
                pixelListCount++;
            }
        }
    }

    std::vector<int> path[2];
    path[0].resize(targetCount);
    path[1].resize(targetCount);
    std::vector<int> visited;
    visited.resize(pixelListCount);
    for (int i = 0; i < pixelListCount; i++) visited[i] = 0;
    int pathLength = 0;

    while (pathLength < targetCount && pathLength < pixelListCount)
    {
        int startPoint = rand() % pixelListCount;

        while (visited[startPoint]) startPoint = rand() % pixelListCount;

        path[0][pathLength] = pixelList[0][startPoint];
        path[1][pathLength] = pixelList[1][startPoint];
        visited[startPoint] = 1;
        pathLength++;

        for (int jumpCount = 1; jumpCount < jumpFrequency && pathLength < targetCount; jumpCount++)
        {
            int closestIndex = -1;
            double minDistance = DBL_MAX;

            for (int i = 0; i < pixelListCount; i++)
            {
                if (!visited[i])
                {
                    double dist = sqrt(pow(path[0][pathLength - 1] - pixelList[0][i], 2) + pow(path[1][pathLength - 1] - pixelList[1][i], 2));
                    if (dist < minDistance)
                    {
                        minDistance = dist;
                        closestIndex = i;
                    }
                }
            }

            if (closestIndex != -1) // Add that fucker to the path
            {
                path[0][pathLength] = pixelList[0][closestIndex];
                path[1][pathLength] = pixelList[1][closestIndex];
                visited[closestIndex] = 1;
                pathLength++;
            }
            else
            {
                break; // Break the loop if no unvisited pixels are found
            }
        }

        if (pathLength == pixelListCount)
        {
            break;
        }
    }

    if (pixelListCount < 0.01 * targetCount)
    {
        while (pixelsWritten < targetCount)
        {
            xShort = (rand() % gridSize) * 128 - 32767;
            yShort = (rand() % gridSize) * 128 - 32767;

            outputFile.write((char*)(&xShort), sizeof(int16_t));
            outputFile.write((char*)(&yShort), sizeof(int16_t));

            bytesWritten += 4;
            pixelsWritten++;
            fillerPixels++;
        }
    }
    else
    {
        for (i = 0; i < pathLength; i++)
        {
            xShort = (int16_t) (path[0][i] & 0x1FF) * 128 - 32767;
            yShort = -(int16_t)(path[1][i] & 0x1FF) * 128 - 32767;

            outputFile.write((char*)(&xShort), sizeof(int16_t));
            outputFile.write((char*)(&yShort), sizeof(int16_t));

            bytesWritten += 4;
            pixelsWritten++;
        }

        i = 0;

        while (pixelsWritten < targetCount)
        {
            xShort = (int16_t) (path[0][i] & 0x1FF) * 128 - 32767;
            yShort = -(int16_t)(path[1][i] & 0x1FF) * 128 - 32767;

            outputFile.write((char*)(&xShort), sizeof(int16_t));
            outputFile.write((char*)(&yShort), sizeof(int16_t));

            bytesWritten += 4;
            fillerPixels++;
            pixelsWritten++;

            i++;

            if (i >= pathLength)
            {
                i = 0; // Loop back to start if we reach the end of the path
            }
        }
    }

    outputFile.flush();
    outputFile.close();

    if (debug) printf("DEBUG: Leaving determinePath..\n");
    if (debug) printf("\n%s Frame rendering completed. %d stereo samples written as %d bytes to %s. Filler points: %d. Exiting..\n\n", header, targetCount, bytesWritten, outputFileName.c_str(), fillerPixels);
}

int hilligoss(std::string infname, int targetCount, int blackThreshold, int whiteThreshold, int jumpFrequency, bool debug)
{
    inputFileName = infname;
    outputFileName = infname + ".pcm";

    readImage(inputFileName);
    reducePixels();
    determinePath();
    showPreview();
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2)
    {
        printf("\nUsage: %s -f <filename> -c <desired vectors per frame, usually 2000> -b <black threshold 0-255> -w <white threshold 1-255> -x <preview width in chars> -y <preview height in chars>\nExample: %s -f image.pgm -c2000 -b20 -w234 -x160 -y100 -j200 \nNotes: Images must be 8-bit ASCII PGM, 512x512 only.\n\n", argv[0], argv[0]);
        exit(0);
    }

    std::vector<std::string> args(argv + 1, argv + argc);

    int argn = 0;
    char arg;
    std::string optarg;
    while (argn < args.size())
    {
        if (args[argn].length() < 2) return -1;
        if (args[argn].at(0) != '-') return -1;
        arg = args[argn].c_str()[1];
        if (arg != 's' && arg != 'd' && argn + 1 < args.size()) {
            optarg = args[argn + 1];
            switch (arg)
            {
            case 'f': inputFileName = optarg; break;
            case 'c': targetCount = atoi(optarg.c_str()); break;
            case 'b': blackThreshold = atoi(optarg.c_str()); break;
            case 'w': whiteThreshold = atoi(optarg.c_str()); break;
            case 'x': xPreviewSize = atoi(optarg.c_str()); break;
            case 'y': yPreviewSize = atoi(optarg.c_str()); break;
            case 'j': jumpFrequency = atoi(optarg.c_str()); break;
            default: printf("Invalid option %s", args[argn].c_str()); return -1;
            }
            argn += 2;
        } else if (arg == 'd') {
            debug = 1;
            argn++;
        }
        else if (arg == 's') {
            silent = 1;
            argn++;
        }
    }
    if (inputFileName.length() < 5) return -1;
    if (inputFileName.find(".pgm", inputFileName.length() - 4) == std::string::npos) return -1;

    return hilligoss(inputFileName, targetCount, blackThreshold, whiteThreshold, jumpFrequency, debug);
}
