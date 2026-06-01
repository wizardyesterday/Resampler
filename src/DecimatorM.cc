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

  // Let's make that polyphase filter structure.
  createPolyphaseSubfilters(filterLength,coefficientsPtr,decimationFactor);

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
  initial values.

  Calling Sequence: resetFilterState()

  Inputs:

    None.

  Outputs:

    None.

*****************************************************************************/
void DecimatorM::resetFilterState(void)
{
  int i;

  // Clear the state of the subfilters.
  for (i = 0; i < polyphaseFilterLength; i++)
  {
    subfilterPtr[i]->resetFilterState();
  } // for

  // Start out with the first commutator position.
  decimatorCommutatorIndex = decimationFactor - 1;

  // Clear the output accumulator.
  outputSummer = 0;

  return;

} // resetFilterState

/*****************************************************************************

  Name: createPolyphaseSubfilters

  Purpose: The purpose of this function is to create the polyphase filter
  array given the decimation factor and an array of coefficients that
  represent the prototype filter.  Let's state some definitions so that we
  can describe how the data is formatted in the polyphase coefficient array.

  N - Prototype filter number of taps.
  L - Interpolation factor.
  q - Polyphase filter number of taps: N/L
  h - Prototype filter coefficient array.
  pi - Polyphase filter cofficient array for ith subfilter.

  Each pi has coefficients as illustrated below.

  p0  : h(0),h(M),h(2M)....
  p1  : h(1),h(ML+1),h(2M+2)...
  .
  .
  pM-1: h(M-1),h(2M-1+1),h(3M-1+2)....

  Let's provide an example.  Consider that the prototype filter has 8
  taps, and the decimation factor is 4.  Our pi polyphase filters
  will have the coefficients as illustrated below.

  p0: h(0),h(4)
  p1: h(1),h(5)
  p2: h(2),h(6)
  p3: h(3),h(7)  

  Calling Sequence: createPolyphaseSubfilters(coefficientPtr);

  Inputs:

    filterLength - The number of taps for the prototype filter.

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
 
  // Polyphase filter lengths needed for later.
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

      // Decimate the coefficients.
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
  decimator that decimates by the factor M.  Here's how things work.
  First, the a sample is presented to the subfilter that is selected by
  the current commutator index, and the output is added to the output
  summer (the summer is an accumulator). The commutator index is
  decremented so that the next lower-numbered subfilter will be selected
  when the next input sample arrives. If the commutator index becomes
  negative, it is understood that a commutator cycle has completed (all
  subfilters have been run). At this point, the output summer value is
  returned to the caller (stored to the location referenced by the caller
  via a supplied argument), the commutator index is set to use the highest-
  numbered subfilter, and the output summer is cleared to zero to be
  used during the next commutator cycle. Additionally a value of true is
  returned by this function to indicate to the caller that data is available.

  Calling Sequence:  outputSampleCount = decimate(inputSample,
                                                     outputBufferPtr)

  Inputs:

    inputSample - A pointer to a buffer to be decimated.

    outputBufferPtr - A pointer to storage that is to accept the
    decimated data.

  Outputs:

    dataAvailable - A flag that indicates whether or not data is available.
    A value of true indicates that a decimated sample value is available,
    and a value of false indicates that it is not.

*****************************************************************************/
bool DecimatorM::decimate(float inputSample,float *outputPtr)
{
  bool dataAvailable;

  // Indicate that no data is available.
  dataAvailable = false;

  // Send the sample to the currently selected subfilter.
  outputSummer += 
    subfilterPtr[decimatorCommutatorIndex]->filterData(inputSample);

  // Reference the next polyphase branch.
  decimatorCommutatorIndex--;

  if (decimatorCommutatorIndex < 0)
  {
    // We now have the decimated result.
    *outputPtr = outputSummer;

    // Clear for the next commutation cycle.
    outputSummer = 0;

    // Reset the commutator.
    decimatorCommutatorIndex = decimationFactor - 1;

    // Indicate that a decimated value is available.
    dataAvailable = true;
  } // if

  return (dataAvailable);

} // decimate

