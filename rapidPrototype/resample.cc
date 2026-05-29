//*************************************************************************
// File name: resample.cc
//*************************************************************************

//*************************************************************************
// This program provides a means to resample a PCM file by a factor of
//  L/M = 5/2.
//
// To run this program type,
// 
//     ./resample < inputFileName > outputFileName,
//
//*************************************************************************

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "InterpolatorLM.h"

// Globals.
int16_t inputBuffer[32768];
int16_t outputBuffer[32768];
float floatBuffer[32768];

// Coefficients for 1:5 interpolation anti-imaging filter.
float coef[] =
{
  -0.0403405,
   0.0493479,
  -0.0786330,
   0.1191287,
  -0.1751283,
   0.2544455,
  -0.3737319,
   0.5766416,
  -1.0241667,
   3.1706652,
   3.1706652,
  -1.0241667,
   0.5766416,
  -0.3737319,
   0.2544455,
  -0.1751283,
   0.1191287,
  -0.0786330,
   0.0493479,
  -0.0403405
};

//*************************************************************************
// Mainline code.
//*************************************************************************
int main(int argc,char **argv)
{
  int i;
  int m;
  int count;
  uint32_t sampleCount;
  int outputBufferIndex;
  bool done;
  InterpolatorLM *myResamplerPtr;

  // Instantiate a resampler with ratio L/M = 5/2).
  myResamplerPtr = new InterpolatorLM(sizeof(coef)/5,coef,5,2);

  // Reference the beginning of the output buffer.
  outputBufferIndex = 0;

  // Set up for loop entry.
  done = false;

  while (!done)
  {
    // Read a block of PCM samples.
    count = fread(inputBuffer,sizeof(int16_t),1024,stdin);

    if (count == 0)
    {
      // We're done.
      done = true;
    } // if
    else
    {
      for (i = 0; i < count; i++)
      {
        // Interpolate the samples.
        sampleCount = 
          myResamplerPtr->interpolate((float)inputBuffer[i],
                                       floatBuffer);

        // Convert to 16-bit little endian PCM samples.
        for (m = 0; m < sampleCount; m++)
        {
          outputBuffer[m] = (int16_t)floatBuffer[m];
        } // for

        // Write the next block of interpolated samples.
        fwrite(&outputBuffer[outputBufferIndex],
               sizeof(int16_t),
               sampleCount,
               stdout);

        // Reference the next location for y(m).
        outputBufferIndex += sampleCount;

      } // for

    } // else

  } // while

  // Release resources.
  if (myResamplerPtr != NULL)
  {
    delete myResamplerPtr;
  } // if

  return (0);

} // main
