//*************************************************************************
// File name: decimate.cc
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
    0.0117540 / 5,
  -0.0248343 / 5,
  -0.0249858 / 5,
  -0.0286259 / 5,
  -0.0302043 / 5,
  -0.0273786 / 5,
  -0.0198376 / 5,
  -0.0089212 / 5,
    0.0028435 / 5,
    0.0124558 / 5,
    0.0173299 / 5,
    0.0161359 / 5,
    0.0093068 / 5,
  -0.0009836 / 5,
  -0.0113981 / 5,
  -0.0184283 / 5,
  -0.0195177 / 5,
  -0.0139535 / 5,
  -0.0032128 / 5,
    0.0093894 / 5,
    0.0196868 / 5,
    0.0240050 / 5,
    0.0204348 / 5,
    0.0096040 / 5,
  -0.0053485 / 5,
  -0.0196463 / 5,
  -0.0283675 / 5,
  -0.0280998 / 5,
  -0.0182148 / 5,
  -0.0013033 / 5,
    0.0174461 / 5,
    0.0317964 / 5,
    0.0364906 / 5,
    0.0291136 / 5,
    0.0111488 / 5,
  -0.0121977 / 5,
  -0.0334427 / 5,
  -0.0451552 / 5,
  -0.0424704 / 5,
  -0.0249720 / 5,
    0.0027596 / 5,
    0.0322457 / 5,
    0.0535983 / 5,
    0.0586729 / 5,
    0.0440179 / 5,
    0.0126468 / 5,
  -0.0266523 / 5,
  -0.0612718 / 5,
  -0.0787641 / 5,
  -0.0710078 / 5,
  -0.0374302 / 5,
    0.0139034 / 5,
    0.0678324 / 5,
    0.1061471 / 5,
    0.1131235 / 5,
    0.0809792 / 5,
    0.0135946 / 5,
  -0.0727974 / 5,
  -0.1524916 / 5,
  -0.1962858 / 5,
  -0.1791288 / 5,
  -0.0874429 / 5,
    0.0759224 / 5,
    0.2906208 / 5,
    0.5221614 / 5,
    0.7290422 / 5,
    0.8719892 / 5,
    0.9230144 / 5,
    0.8719892 / 5,
    0.7290422 / 5,
    0.5221614 / 5,
    0.2906208 / 5,
    0.0759224 / 5,
  -0.0874429 / 5,
  -0.1791288 / 5,
  -0.1962858 / 5,
  -0.1524916 / 5,
  -0.0727974 / 5,
    0.0135946 / 5,
    0.0809792 / 5,
    0.1131235 / 5,
    0.1061471 / 5,
    0.0678324 / 5,
    0.0139034 / 5,
  -0.0374302 / 5,
  -0.0710078 / 5,
  -0.0787641 / 5,
  -0.0612718 / 5,
  -0.0266523 / 5,
    0.0126468 / 5,
    0.0440179 / 5,
    0.0586729 / 5,
    0.0535983 / 5,
    0.0322457 / 5,
    0.0027596 / 5,
  -0.0249720 / 5,
  -0.0424704 / 5,
  -0.0451552 / 5,
  -0.0334427 / 5,
  -0.0121977 / 5,
    0.0111488 / 5,
    0.0291136 / 5,
    0.0364906 / 5,
    0.0317964 / 5,
    0.0174461 / 5,
  -0.0013033 / 5,
  -0.0182148 / 5,
  -0.0280998 / 5,
  -0.0283675 / 5,
  -0.0196463 / 5,
  -0.0053485 / 5,
    0.0096040 / 5,
    0.0204348 / 5,
    0.0240050 / 5,
    0.0196868 / 5,
    0.0093894 / 5,
  -0.0032128 / 5,
  -0.0139535 / 5,
  -0.0195177 / 5,
  -0.0184283 / 5,
  -0.0113981 / 5,
  -0.0009836 / 5,
    0.0093068 / 5,
    0.0161359 / 5,
    0.0173299 / 5,
    0.0124558 / 5,
    0.0028435 / 5,
  -0.0089212 / 5,
  -0.0198376 / 5,
  -0.0273786 / 5,
  -0.0302043 / 5,
  -0.0286259 / 5,
  -0.0249858 / 5,
  -0.0248343 / 5,
    0.0117540 / 5
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

  // Instantiate a Decimator with decimation of 5.
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

