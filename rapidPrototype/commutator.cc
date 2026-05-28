//*************************************************************************
// File name: noisyPcm.cc
//*************************************************************************

//*************************************************************************
// This program tests anadaptive noise cnceller when driven by PCM data
// that has noise injected into the samples.  Two  selectable output file
// types . Thll be written to sdtout.e first file will contain PCM samples
// that are perturbed by additive white Gaussian noise. The second file
// will contain noise-reduced PCM samples. The user selects the output
// file type via a command line argument.
//
// To run this program type,
// 
//     ./noisyPcm -t filetype -v noiseVariance -l filterLength
//      -d delay -b beta < inputFileName > outputFileName,
//
// where,
//
//    filetype - Either noisy or noise-reduced.
//
//    noiseVariance - The variance of the noise source.
//
//    filterLength  The number of taps for the nonrecursive filter in
//    the adaptive noise canceller.
//
//    delay - The number of unit delays used by the adaptive noise
//    canceller.
//
//    beta - The convergence factor used by the adaptive noise canceller.
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

// Globals.
uint32_t interpolationFactor;
uint32_t decimatonFactor;

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
  uint32_t interpolationFactor;
  uint32_t decimationFactor;
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
    for (l = 0; l < interpolationFactor; l++)
    {
      if ((m % decimationFactor) == 0)
      {
        printf("n: %d  Selected Filter Branch: %d  OutputIndex: %d\n",n,l,my);

        // Increment output sample number.
        my++;
      } 

      m = (m + 1) % decimationFactor;

    } // for

  } // for

  return (0);

} // main
