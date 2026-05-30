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
   0.0176984,
  -0.0184172,
   0.0266117,
  -0.0358500,
   0.0455820,
  -0.0550524,
   0.0632278,
  -0.0689562,
   0.0708500,
  -0.0673952,
   0.0569105,
  -0.0375229,
   0.0070475,
   0.0373845,
  -0.1000912,
   0.1887877,
  -0.3198881,
   0.5366169,
  -0.9995099,
   3.1623325,
   3.1623325,
  -0.9995099,
   0.5366169,
  -0.3198881,
   0.1887877,
  -0.1000912,
   0.0373845,
   0.0070475,
  -0.0375229,
   0.0569105,
  -0.0673952,
   0.0708500,
  -0.0689562,
   0.0632278,
  -0.0550524,
   0.0455820,
  -0.0358500,
   0.0266117,
  -0.0184172,
   0.0176984
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
  myResamplerPtr = new InterpolatorLM(sizeof(coef)/sizeof(float),coef,5,2);

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
      // Reference the beginning of the output buffer.
      outputBufferIndex = 0;

      for (i = 0; i < count; i++)
      {
        // Interpolate the samples.
        sampleCount = 
          myResamplerPtr->interpolate((float)inputBuffer[i],
                                       floatBuffer);

        // Convert to 16-bit little endian PCM samples.
        for (m = 0; m < sampleCount; m++)
        {
          outputBuffer[outputBufferIndex + m] = (int16_t)floatBuffer[m];
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
