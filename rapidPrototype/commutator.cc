//*************************************************************************
// File name: commutator.cc
//*************************************************************************

//*************************************************************************
// This program was written to explore concepts related to polyphase
// resampler concepts such that the sample rate can be scaled by L/M
// where L, M assume positive integer values.
//
// Presently, L has to be >= M. If L = M, interpolation is controlled.
// If L > M, both, interpolation and decimation are controlled.
//
// To run this program type,
// 
//     ./commutator -l interpolationFactor -m decimationFactor
//
// where,
//
//    interpolationFactor - A positive integer that  specifies the sample
//    rate increase.
//
//     decimationFactor - A positive integer that  specifies the sample
//    rate decrease.
///*************************************************************************

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

// This structure is used to consolidate user parameters.
struct MyParameters
{
  uint32_t *interpolationFactorPtr;
  uint32_t *decimationFactorPtr;
};

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// Globals.
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
uint32_t interpolationFactor;
uint32_t decimationFactor;
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

/*****************************************************************************

  Name: getUserArguments

  Purpose: The purpose of this function is to retrieve the user arguments
  that were passed to the program.  Any arguments that are specified are
  set to reasonable default values.

  Calling Sequence: exitProgram = getUserArguments(parameters)

  Inputs:

    parameters - A structure that contains pointers to the user parameters.

  Outputs:

    exitProgram - A flag that indicates whether or not the program should
    be exited.  A value of true indicates to exit the program, and a value
    of false indicates that the program should not be exited..

*****************************************************************************/
bool getUserArguments(int argc,char **argv,struct MyParameters parameters)
{
  bool exitProgram;
  bool done;
  int opt;

  // Default not to exit program.
  exitProgram = false;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Default parameters.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Default to no interpolation.
  *parameters.interpolationFactorPtr = 1;

  // Default to no decimation.
  *parameters.decimationFactorPtr = 1;
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // Set up for loop entry.
  done = false;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Retrieve the command line arguments.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  while (!done)
  {
    // Retrieve the next option.
    opt = getopt(argc,argv,"l:m:h");

    switch (opt)
    {
      case 'l':
      {
        *parameters.interpolationFactorPtr = atol(optarg);
        break;
      } // case

      case 'm':
      {
        *parameters.decimationFactorPtr = atol(optarg);
        break;
      } // case

      case 'h':
      {
        // Display usage.
        fprintf(stderr,"./noisyPcm -l interpolationFactor "
                "-m decimationFactor\n");

        // Indicate that program must be exited.
        exitProgram = true;
        break;
      } // case

      case -1:
      {
        // All options consumed, so bail out.
        done = true;
      } // case
    } // switch

  } // while
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  return (exitProgram);

} // getUserArguments

//*************************************************************************
// Mainline code.
//*************************************************************************
int main(int argc,char **argv)
{
bool exitProgram;
  uint32_t n;
  uint32_t count;
  uint32_t l;
  uint32_t m;
  uint32_t my;
  struct MyParameters parameters;

  // Set up for parameter transmission.
  parameters.interpolationFactorPtr = &interpolationFactor;
  parameters.decimationFactorPtr = &decimationFactor;

  // Retrieve the system parameters.
  exitProgram = getUserArguments(argc,argv,parameters);

  if (exitProgram)
  {
    // Bail out.
    return (0);
  } // if

  // Initialize commutator indices.
  l = 0;
  m = 0;

  // Initialize output sample number.
  my = 0;

  count = 10;

  for (n = 0; n < count; n++)
  {
    //******************************************************* 
    // This block of code performs the commutation function
    // For both interpolation and decimation.  The outer
    // loop performs the indexing for polyphase subfilter
    // access, and the modulo counter, m, is used to select
    // The approiate polyphase subfilter to operate during
    // the interpolation operation. The selection occurs
    // when m wraps back to zero (overflows) due to the
    // relation: m = (m + 1) modulo M, where M is the
    // decimation factor.
    //******************************************************* 
    for (l = 0; l < interpolationFactor; l++)
    {
      if ((m % decimationFactor) == 0)
      {
        printf("n: %d  Selected Filter Branch: %d  OutputIndex: %d\n",n,l,my);

        // Increment output sample number.
        my++;
      } 

      // Increment the decimator commutator index at the same rate
      // as the interpolation commutator index   
      m = (m + 1) % decimationFactor;
      //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    } // for

  } // for

  return (0);

} // main
