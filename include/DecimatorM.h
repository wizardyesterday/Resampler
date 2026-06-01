//**************************************************************************
// file name: DecimatorM.h
//**************************************************************************
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// This class implements a signal processing block known as a
// polyphase decimatorpolyphase decimator.

// A decimator consists of an antialiasing filter followd by a sampling
// rate compressor.  In its most naive implementation, the input signal
// is presented to the filter, and samples are discarded as dictated by
// the decimation factor.  Unfortunately, the filter is operating at the
// pre-decimation sample rate (the higher sample rate).
// To make things more efficient,
// a polyphase filter structure is used.  Essentially, one starts out
// with a prototype filter, designed with the pre-decimation sample rate
// used as the sampling frequency.  Call this h(n).  The coefficients of
// h(n) and then permuted in a certain way, and the results are written
// to a polyphase coefficient array, such that when filtering, the
// coefficients are accessed in a sequential manner.  The polyphase
// coefficient array is arranged as follows:
//
// p0,p1,p2,p3.....,
//
// Now, pi is the ith polyphase filter, and each has the same number of
// coefficients.  Now consider that we have a coefficient array called p.
// Then to access p0, the address of p[0] is rereferenced.  To access p1,
// the address of p[q] is referenced.  The symbol, q, is the number of
// coefficients in each polyphase filter.
// This causes a constraint on the number of taps in the prototype filter.
// They must be an integer mutiple of the decimation factor.  That is,
// given a prototype filter length of N, and a decimation factor of L,
// q = N/L must be an integer since that is the number of taps in each
// polyphase filter.
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#ifndef __DECIMATORM__
#define __DECIMATORM__

#include <stdint.h>
#include "FirFilter.h"

class DecimatorM
{
  //***************************** operations **************************

  public:

  DecimatorM(int filterLength,
               float *coefficientsPtr,
               int decimationFactor);

  ~DecimatorM(void);

  void resetFilterState(void);

  bool decimate(float inputSample,float *outputPtr);

  private:

  void createPolyphaseSubfilters(int filterLength,
                                 float *coefficientsPtr);

  //***************************** attributes **************************
  private:

  // The number of taps each polyphase filter.
  int polyphaseFilterLength;

  int decimationFactor;

  // Modulo M counter, where, M is the decimation ratio.
  int decimatorCommutatorIndex;

  // This accumulates samples during a commutation cycle.
  float outputSummer;

  // This references an array of FIR filter objects.
  FirFilter **subfilterPtr;
};

#endif // __DECIMATORM__
