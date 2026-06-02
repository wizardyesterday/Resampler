//*************************************************************************
// File name: resample.cc
//*************************************************************************

//*************************************************************************
// This program provides a means to decimates a PCM file by a factor of
// M = 5.
//
// To run this program type,
// 
//     ./decimate < inputFileName > outputFileName,
//
//*************************************************************************

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "DecimatorM.h"

// Globals.
int16_t inputBuffer[32768];
int16_t outputBuffer[32768];

// Coefficients for 5:1 decimation anti-aliasing filter.
float coef[] =
{
    0.0117540,
  -0.0248343,
  -0.0249858,
  -0.0286259,
  -0.0302043,
  -0.0273786,
  -0.0198376,
  -0.0089212,
    0.0028435,
    0.0124558,
    0.0173299,
    0.0161359,
    0.0093068,
  -0.0009836,
  -0.0113981,
  -0.0184283,
  -0.0195177,
  -0.0139535,
  -0.0032128,
    0.0093894,
    0.0196868,
    0.0240050,
    0.0204348,
    0.0096040,
  -0.0053485,
  -0.0196463,
  -0.0283675,
  -0.0280998,
  -0.0182148,
  -0.0013033,
    0.0174461,
    0.0317964,
    0.0364906,
    0.0291136,
    0.0111488,
  -0.0121977,
  -0.0334427,
  -0.0451552,
  -0.0424704,
  -0.0249720,
    0.0027596,
    0.0322457,
    0.0535983,
    0.0586729,
    0.0440179,
    0.0126468,
  -0.0266523,
  -0.0612718,
  -0.0787641,
  -0.0710078,
  -0.0374302,
    0.0139034,
    0.0678324,
    0.1061471,
    0.1131235,
    0.0809792,
    0.0135946,
  -0.0727974,
  -0.1524916,
  -0.1962858,
  -0.1791288,
  -0.0874429,
    0.0759224,
    0.2906208,
    0.5221614,
    0.7290422,
    0.8719892,
    0.9230144,
    0.8719892,
    0.7290422,
    0.5221614,
    0.2906208,
    0.0759224,
  -0.0874429,
  -0.1791288,
  -0.1962858,
  -0.1524916,
  -0.0727974,
    0.0135946,
    0.0809792,
    0.1131235,
    0.1061471,
    0.0678324,
    0.0139034,
  -0.0374302,
  -0.0710078,
  -0.0787641,
  -0.0612718,
  -0.0266523,
    0.0126468,
    0.0440179,
    0.0586729,
    0.0535983,
    0.0322457,
    0.0027596,
  -0.0249720,
  -0.0424704,
  -0.0451552,
  -0.0334427,
  -0.0121977,
    0.0111488,
    0.0291136,
    0.0364906,
    0.0317964,
    0.0174461,
  -0.0013033,
  -0.0182148,
  -0.0280998,
  -0.0283675,
  -0.0196463,
  -0.0053485,
    0.0096040,
    0.0204348,
    0.0240050,
    0.0196868,
    0.0093894,
  -0.0032128,
  -0.0139535,
  -0.0195177,
  -0.0184283,
  -0.0113981,
  -0.0009836,
    0.0093068,
    0.0161359,
    0.0173299,
    0.0124558,
    0.0028435,
  -0.0089212,
  -0.0198376,
  -0.0273786,
  -0.0302043,
  -0.0286259,
  -0.0249858,
  -0.0248343,
    0.0117540
};

//*************************************************************************
// Mainline code.
//*************************************************************************
int main(int argc,char **argv)
{
  int i;
  int m;
  int count;
  int outputBufferIndex;
  int sampleCount;
  bool dataAvailable;
  float outputSample;
  bool done;
  DecimatorM *myDecimatorPtr;

  // Instantiate a resampler with ratio L/M = 5/2).
  myDecimatorPtr = new DecimatorM(sizeof(coef)/sizeof(float),coef,5);

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
        // Decimate the samples.
        dataAvailable =
          myDecimatorPtr->decimate((float)inputBuffer[i],&outputSample);

        if (dataAvailable)
        {
          // Store the decimated value.
          outputBuffer[outputBufferIndex] = (int16_t)outputSample;

          // Reference the next storage location.
          outputBufferIndex++; 
        } // if

      } // for

      // Write the next block of decimatedsamples.
      fwrite(outputBuffer,sizeof(int16_t),outputBufferIndex, stdout);

    } // else

  } // while


  // Release resources.
  if (myDecimatorPtr != NULL)
  {
    delete myDecimatorPtr;
  } // if

  return (0);

} // main

