A resampler increases a sequence sampled  at one sample rate to a sequence that is sampled at a diferent sample rate. This resampler is implemented using a
polyphase filter structure with the associated commutation functions for
interpolation and decimation such that the sample rate is increased by a
factor of L/M, L >= M. L is the integer interpolation factor and M is the
decimation factor. If M is eual to one, the result is interpolation by L.

The source and inclucd directories contain the implementation and definitions
of the resampler code.

The test directory contains an example application that increases the sample
of 16-bit PCM data from 16000S/s to 40000S/s.

The rapid prototype directory contains a program to validate the commutator
function that is used for a polyphase resampler.

To build the test application, type 'sh buildResampler.sh'.

To build the commutator application, navigate to the rapidPrototype directory,
and type 'g++ -o commutator commutator.cc.


