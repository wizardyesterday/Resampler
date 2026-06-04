//************************************************************************
// file name: Resampler.cc
//************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "Resampler.h"

using namespace std;

/*****************************************************************************

  Name: Resampler

  Purpose: The purpose of this function is to serve as the constructor for
  an instance of an Resampler.  One thing should be mentioned.  The
  filter size needs to be an integer multiple of the interpolation factor
  so that the polyphase L polyphase filters have the same number of taps.
  The advantage of using polyphase filters is that, rather than stuffing
  zeros in the pipeline and filtering at interpolated sample rate, due to
  the commutation method, filtering occurs at the pre-interpolation (lower)
  sample rate. After interpolation the samples are decimated by the
  decimation factor. The result is an interpolation by L/M, where L is
  the interpolation factor and M is the decimation factor.
  If L < M, decimation will occur.

  Calling Sequence: Resampler(filterLength,
                                 coefficientsPtr,
                                 interpolationFactor,
                                 decimationFaxtor)

  Inputs:

    filterLength - The number of taps for the prototype filter.

    coefficientPtr - A pointer to the prototype filter coefficients.

    interpolationFactor - The interpolation factor of the interpolator.

    decimationFactor - The decimation factor of the decimator.

  Outputs:

    None.

*****************************************************************************/
Resampler::Resampler(int filterLength,
                           float *coefficientsPtr,
                           int interpolationFactor,
                           int decimationFactor)
{
  int i;

  if (decimationFactor == 0)
  {
    // Se, we'll just interpolate.
    decimationFactor = 1;
  } // if

  // Save for later use.
  this->interpolationFactor = interpolationFactor;
  this->decimationFactor = decimationFactor;

  // Let's make that polyphase filter.
  createPolyphaseCoefficients(filterLength,
                              coefficientsPtr,
                              interpolationFactor);

  // Set the filter state to an initial value.
  resetFilterState();

  return;

} // Resampler

/*****************************************************************************

  Name: ~Resampler

  Purpose: The purpose of this function is to serve as the destructor for
  an instance of an Resampler.

  Calling Sequence: ~Resampler()

  Inputs:

    None.

  Outputs:

    None.

*****************************************************************************/
Resampler::~Resampler(void)
{
  int i;

  // Release subfilter coefficients.
  for (i = 0; i < interpolationFactor; i++)
  {
    delete coefficientStoragePtr[i];
  } // for

  // Release the pointer array.
  delete[] coefficientStoragePtr;

  // Release the shared polyphase filter state.
  delete[] filterStatePtr;

  return;

} // ~Resampler

/*****************************************************************************

  Name: resetFilterState

  Purpose: The purpose of this function is to reset the filter state to its
  initial values.  This includes setting the ring buffer index to the
  beginning of the filter state memory and setting all entries of the
  filter state memory to a value of 0.

  Calling Sequence: resetFilterState()

  Inputs:

    None.

  Outputs:

    None.

*****************************************************************************/
void Resampler::resetFilterState(void)
{
  int i;

  // Set to the beginning of filter state memory.
  ringBufferIndex = 0;

  // Start out with commutator position 0.
  decimationCounter = 0;

  // Clear the filter state.
  for (i = 0; i < polyphaseFilterLength; i++)
  {
    filterStatePtr[i] = 0;
  } // for

  return;

} // resetFilterState

/*****************************************************************************

  Name: filterData

  Purpose: The purpose of this function is to filter one sample of data.
  It uses a circular buffer to avoid the copying of data when the filter
  state memory is updated.

  Calling Sequence: y = filterData(coefficientsPtr)

  Inputs:

    coefficientsPtr - A pointer to the filter coefficients.

  Outputs:

    y - The output value of the filter.

*****************************************************************************/
float Resampler::filterData(float *coefficientsPtr)
{
  float *h, y;
  int k, xIndex;

  // Reference the first filter coefficient.
  h = coefficientsPtr;

  // Set current position of index to deal with convolution sum.
  xIndex = ringBufferIndex;

  // Clear the accumulator.
  y = 0;
  
  for (k = 0; k < polyphaseFilterLength; k++)
  {
    // Perform multiply-accumulate operation.
    y = y + (h[k] * filterStatePtr[xIndex]);

    // Decrement the index in a modulo fashion.
    xIndex--;
    if (xIndex < 0)
    {
      // Wrap the index.
      xIndex = polyphaseFilterLength - 1;
    } // if
  } // for
 
  return (y);

} // filterData

/*****************************************************************************

  Name: createPolyphaseCoefficients

  Purpose: The purpose of this function is to create the polyphase filter
  array given the interpolation factor and an array of coefficients that
  represent the prototype filter.  Let's state some definitions so that we
  can describe how the data is formatted in the polyphase coefficient array.

  N - Prototype filter number of taps.
  L - Interpolation factor.
  q - Polyphase filter number of taps: N/L
  h - Prototype filter coefficient array.
  pi - Polyphase filter cofficient array for ith subfilter.

  The aggregate polyphase filter array is arranged as illustrated below.

  p0,p1,...pL-1.

  Each pi is arranged as illustrated below.

  p0  : h(0),h(L),h(2L)....
  p1  : h(1),h(L+1),h(2L+1)...
  .
  .
  pL-1: h(L-1),h(2L-1),h(3L-1)....

  Let's provide an example.  Consider that the prototype filter has 8
  taps, and the interpolation factor is 4.  Our pi polyphase filters
  will have the coefficients as illustrated below.

  p0: h(0),h(4)
  p1: h(1),h(5)
  p2: h(2),h(6)
  p3: h(3),h(7)  

  Calling Sequence: createPolyphaseCoefficients(filterLength,
                                                coefficientPtr,
                                                interpolationFactor)

  Inputs:

    filterLength - The number of taps for the prototype filter.

    coefficientPtr - A pointer to the prototype filter coefficients.

    interpolationFactor - The interpolation factor of the interpolator.


  Outputs:

    None.

*****************************************************************************/
void Resampler::createPolyphaseCoefficients(int filterLength,
                                               float *coefficientsPtr,
                                               int interpolationFactor)
{
  int i, j;
  int lookupIndex;

  // Polyphase filter lengths.
  polyphaseFilterLength = filterLength / interpolationFactor;
 
  // Allocate storage for polyphase filter coefficients.
  coefficientStoragePtr = new float * [interpolationFactor];

  // Allocate storage for the filter state.
  filterStatePtr = new float[polyphaseFilterLength];

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // The outer loop increments through the polyphase
  // filter number, and the inner loop fills in the
  // coefficients of a particular polyphase filter.  This
  // is performed so that the control structure that
  // invokes the filter method need not be concerned with
  // dealing with indexing through the prototype filter
  // array.  Thus, everything is permuted over here, so
  // that the appropriate polyphase filter array is
  // passed to the filter method.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  for (i = 0; i < interpolationFactor; i++)
  {
    // Allocate a subbfilter coefficient array.
    coefficientStoragePtr[i] = new float[polyphaseFilterLength];

    for (j = 0; j < polyphaseFilterLength; j++)
    {
      // Make this easier to follow.
      lookupIndex = i + (j * interpolationFactor);

      // Decimate the coefficients.
      coefficientStoragePtr[i][j] = coefficientsPtr[lookupIndex];

    } // for
  } // for

  return;

} // createPolyphaseCoefficients

/*****************************************************************************

  Name: advancePipeline

  Purpose: The purpose of this function is to shift a sample into the
  pipeline.  This function is used for multirate processing.

  Calling Sequence: advancePipeline()

  Inputs:

    None.

  Outputs:

    None.

*****************************************************************************/
void Resampler::advancePipeline(void)
{

  // Increment the index in a modulo fashion.
  ringBufferIndex++;
  if (ringBufferIndex == polyphaseFilterLength)
  {
    // Wrap the index.
    ringBufferIndex = 0;
  } // if

  return;

} // advancePipeline

/*****************************************************************************

  Name: resample

  Purpose: The purpose of this function is to perform the function of an
  interpolator that interpolates by the factor L/M.  Here's how things
  work.  First, the sample is pushed into the pipeline.  Next, the selected
  FIR subfilter is invoked using the appropriate polyphase filter and the
  filtered output is stored in next location in the output buffer. This
  filtering operation is conditionally performed the number of times as
  dictated by the value of the interpolation factor divided by the
  decimation factor.  For example, if the interpolation factor were equal to
  4 and the decimation factor were equal to 3, there would exist 4 sets of
  polyphase coefficients, but only every third set would be presented to
  the FIR filter.
  The bottom line is that for every input sample, L/M output samples are
  generated, given that L is the interpolation factor and M is the decimation
  factor..

  Calling Sequence:  outputSampleCount = resam`ple(inputSample,
                                                   outputBufferPtr)

  Inputs:

    inputSample - A pointer to a buffer to be decimated.

    outputBufferPtr - A pointer to storage that is to accept the
    interpolated (or decimated if L < M) data.

  Outputs:

    outputSampleCount - The number of samples that were retained.

*****************************************************************************/
uint32_t Resampler::resample(float inputSample,float *outputBufferPtr)
{
  int i;
  uint32_t outputSampleCount;

  // Store the sample into the pipeline.
  filterStatePtr[ringBufferIndex] = inputSample;

  // Indicate that no interpolated samples were retained.
  outputSampleCount = 0;

  // Perform the interpolation operation.
  for (i = 0; i < interpolationFactor; i++)
  {
    // This actually performs the decimation by M.
    if (decimationCounter == 0)
    {
      // Run the selected subfilter.
      outputBufferPtr[outputSampleCount] =
          filterData(coefficientStoragePtr[i]);

      // Another interpolated output has been saved.
      outputSampleCount++;
    } // if

    // Increment modulo M, where M is the decimation factor.
    decimationCounter =
      (decimationCounter + 1) % decimationFactor;

  } // for

  // We're done, so advance the pipeline.
  advancePipeline();

  return (outputSampleCount);

} // resample

