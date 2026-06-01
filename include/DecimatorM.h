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
// To make things more efficient, a polyphase filter structure is used.
//  Essentially, one starts out with a prototype filter, designed with
// the pre-decimation sample rate used as the sampling frequency. Call
// this h(n).  The coefficients of h(n) and then decimated in a certain
// way, and the results are written to each polyphase coefficient array,
// of a particular subfilter such that when filtering, a particular
// subfilter accepts a sample. The next sample is presented to the next
// subfilter.  These subfilters are accessed in a modulo M manner where
// M is a the decimaion factor.
// The polyphase coefficient arrays are allocated to each subfilter as,
// subfilter 0 has p0, subfilter 1 has p1, ..., subfilter pM-1 has
// coefficients pM-1.
// Now the coefficient array pi contains the folloeing coefficients from
// the prototype filter: pi(k) = h(i*M + k), i = 0, 1, ,,,, M-1, k = 0,
// 1, ..., M-1 (for positive k).  Each, pi has the same number of
// coefficients, N/M where N is the number of coefficients in the prototype
// filter, and M is the decimation factor. Thus N must be an integer multiple
// of the coefficients for each polyphase subfilter.
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
                                 float *coefficientsPtr,
                                 int decimationFactor);

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
