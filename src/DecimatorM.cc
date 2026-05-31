//************************************************************************
// file name: DecimatorM.cc
//************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "DecimatorM.h"

using namespace std;

/*****************************************************************************

  Name: DecimatorM

  Purpose: The purpose of this function is to serve as the constructor for
  an instance of a DecimatorM.  One thing should be mentioned.  The
  filter size needs to be an integer multiple of the decimation factor
  so that the polyphase M polyphase filters have the same number of taps.
  The advantage of using polyphase filters is that, rather than discarding 
  discarding values after filtering at decimated sample rate, due to
  the commutation method, filtering occurs at the pre-decimation (lower)
  sample rate. After decimation the samples are decimated by the
  decimation factor.

  Calling Sequence: DecimatorM(filterLength,
                               coefficientsPtr,
                               decimationFaxtor)

  Inputs:

    filterLength - The number of taps for the prototype filter.

    coefficientPtr - A pointer to the prototype filter coefficients.

    decimationFactor - The decimation factor of the decimator.

  Outputs:

    None.

*****************************************************************************/
DecimatorM::DecimatorM(int filterLength,
                           float *coefficientsPtr,
                           int decimationFactor)
{
  int i;


  // Save for later use.
  this->decimationFactor = decimationFactor;

  // Start out with the first commutator position.
  decimatorCommutatorIndex = decimationFactor - 1;

  // Let's make that polyphase filter.
  createPolyphaseSubfilters(filterLength,
                            coefficientsPtr,
                            decimationFactor);

  // Set the filter state to an initial value.
  resetFilterState();

  return;

} // DecimatorM

/*****************************************************************************

  Name: ~DecimatorM

  Purpose: The purpose of this function is to serve as the destructor for
  an instance of an DecimatorM.

  Calling Sequence: ~DecimatorM()

  Inputs:

    None.

  Outputs:

    None.

*****************************************************************************/
DecimatorM::~DecimatorM(void)
{
  int i;

  // Release resources.
  for (i = 0; i < decimationFactor; i++)
  {
    delete subfilterPtr[i];
  } // for

  return;

} // ~DecimatorM

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
void DecimatorM::resetFilterState(void)
{
  int i;

  // Clear the filter state.
  for (i = 0; i < polyphaseFilterLength; i++)
  {
    subfilterPtr[i]->resetFilterState();
  } // for

  return;

} // resetFilterState

/*****************************************************************************

  Name: createPolyphaseCoefficients

  Purpose: The purpose of this function is to create the polyphase filter
  array given the decimation factor and an array of coefficients that
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
  p1  : h(1),h(L+1),h(2L+2)...
  .
  .
  pL-1: h(L-1),h(2L-1),h(3L-1)....

  Let's provide an example.  Consider that the prototype filter has 8
  taps, and the decimation factor is 4.  Our pi polyphase filters
  will have the coefficients as illustrated below.

  p0: h(0),h(4)
  p1: h(1),h(5)
  p2: h(2),h(6)
  p3: h(3),h(7)  

  Calling Sequence: createPolyphaseCoefficients(filterLength,
                                                coefficientPtr,
                                                decimationFactor)

  Inputs:

    filterLength - The number of taps for the filter.

    coefficientPtr - A pointer to the prototype filter coefficients.

    decimationFactor - The decimation factor of the decimator.


  Outputs:

    None.

*****************************************************************************/
void DecimatorM::createPolyphaseSubfilters(int filterLength,
                                           float *coefficientsPtr,
                                           int decimationFactor)
{
  int i, j, index;
  int lookupIndex;
  float *coefficientStoragePtr;
  uint32_t polyphaseFilterLength;

  // Polyphase filter lengths.
  polyphaseFilterLength = filterLength / decimationFactor;
 
  // Allocate temporary storage for the filter state.
  coefficientStoragePtr = new float[polyphaseFilterLength];

 // Reference the beginning of the polyphase coefficients.
  index = 0;

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
  for (i = 0; i < decimationFactor; i++)
  {
    for (j = 0; j < polyphaseFilterLength; j++)
    {
      // Make this easier to follow.
      lookupIndex = i + (j * decimationFactor);

      // Permute the coefficients.
      coefficientStoragePtr[j] = coefficientsPtr[lookupIndex];

      // Reference the next location for storage of the permuted coefficients.
      index++;
    } // for

    // Allocate a subfilter.
    subfilterPtr[i] =
      new FirFilter(polyphaseFilterLength,coefficientStoragePtr);

  } // for

  // We do't need this anymore.
  delete coefficientStoragePtr;

  return;

} // createPolyphaseSubfilters

/*****************************************************************************

  Name:  decimate

  Purpose: The purpose of this function is to perform the function of a
  decimator that decimates by the factor L/M.  Here's how things
  work.  First, the sample is pushed into the pipeline.  Next, the selected
  FIR subfilter is invoked using the appropriate polyphase filter and the
  filtered output is stored in next location in the output buffer. This
  filtering operation is conditionally performed the number of times as
  dictated by the value of the decimation factor divided by the
  decimation factor.  For example, if the decimation factor were equal to
  4 and the decimation factor were equal to 3, there would exist 4 sets of
  polyphase coefficients, but only every third set would be presented to
  the FIR filter.
  The bottom line is that for every input sample, L/M output samples are
  generated, given that L is the decimation factor and M is the decimation
  factor..

  Calling Sequence:  outputSampleCount = decimate(inputSample,
                                                     outputBufferPtr)

  Inputs:

    inputSample - A pointer to a buffer to be decimated.

    outputBufferPtr - A pointer to storage that is to accept the
    decimated data.

  Outputs:

    outputSampleCount - The number of samples that were retained.

*****************************************************************************/
bool DecimatorM::decimate(float inputSample,float *outputBufferPtr)
{
  bool dataAvailable;
  float outputValue;

  // Indicate that no data is available.
  dataAvailable = false;

  // Accumulate the subfilter output.
  subfilterAccumulator +=
    subfilterPtr[decimatorCommutatorIndex]->filterData(inputSample);

  // Reference the next polyphase branch.
  decimatorCommutatorIndex--;

  if (decimatorCommutatorIndex < 0)
  {
    // Store decimated sample value.
    *outputBufferPtr = subfilterAccumulator;

    // Clear for next decimation cycle.
    subfilterAccumulator = 0;

    // Reset the commutator.
    decimatorCommutatorIndex = decimationFactor - 1;

    // Indicate that a decimated value is available.
    dataAvailable = true;
  } // if

  return (dataAvailable);

} // decimate

