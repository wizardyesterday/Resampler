A resampler increases a sequence sampled  at one sample rate to a sequence that is sampled at a diferent sample rate. This resampler is implemented using a
polyphase filter structure with the associated commutation functions for
interpolation and decimation such that the sample rate is increased by a
factor of L/M, L >= M. L is the integer interpolation factor and M is the
integer decimation factor. If M is equal to one, the result is interpolation
by L.

The source and included directories contain the implementation and definitions
of the resampler code.

The test directory contains two example applications: one that increases the
sample rate of a 16-bit PCM data from 16000S/s to 40000S/s and another that
decreases the sample rate from 40000D/s to 8000S/s. The first program tests
the resampler, and the second tests the decimator.

The rapid prototype directory contains a program to validate the commutator
function that is used for a polyphase resampler.

To build the test application, type 'sh buildResampler.sh'.

To build the interpolator commutator application, navigate to the
rapidPrototype directory, and type 'g++ -o commutator commutator.cc.

A decimator decreases a sequence at one sample rate to a lower sample rate.
This decimator is implemwnted as a polyphase decimator with its associated
input commutator function. The sample rate is reduced by a factor of M,
and if M is equal to one, no decimation will be performed.

To build the decimator application, type 'sh buildDecimator'.

The InterpolatorLM object and thetorM object are not the most optimal code,
But for learning purposes, I believe that there should be a one-to-one
mapping between what you see in a textbook presentation and the implementation.
These objects are more than adequate to use for SDR purposes.

In the future, I may decide to implement a resampler that decimates an input
sequence by L/M (L/M < 1). More than likely, I'll create a different C++
object and enhance the dest program to deal with both, the interpolation
case and the decimation case. I'll have to do more research on this matter.

I hope that others will learn from my code. If this occurs, I will have
achieved my mission: to successfully share knowledge.

Feel free to provide questions, suggesstions and criticisms.

Chris G.
IRC: irc.libera.chat, wizardyesterday, ##rtlsdr

