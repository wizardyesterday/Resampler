//******************************************************************
// This program generates the required filter coefficients so that
// an input signal, that is sampled at 8000 S/s can be interpolated 
// to 16000 S/s.  We want to limit the bandwidth of the aliasing
// filter to 3400 Hz.  The filter coefficients are computed by
// the minimax function, and the result is an equiripple linear
// phase FIR filter.
// The filter specifications are listed below.
//
// Pass Band: 0 <= F <= 7000 Hz.
// Transition Band: 7000 < F <= 7900 Hz.
// Stop Band: 7900 < F < 8000 Hz.
// Passband Ripple: 0.01
// Stopband Ripple: 0.005
//
// Note that the filter length will be automatically  calculated
// from the filter parameters.
// Chris G. 05/29/2026
//******************************************************************

// Include the common code.
exec('utils.sci',-1);

//******************************************************************
// Set up parameters.
//******************************************************************
// Sample rate is 16000 S/s.
Fsample = 16000

// Passband edge.
Fp = 7000;

// Stopband edge.
Fs = 7900;

// The desired audio bandwidth.
F = [0 Fp; Fs Fsample/2];

// Bandwidth of transition band.
deltaF = (Fs - Fp) / Fsample;

// Passband ripple
deltaP = 0.01;

// Stopband ripple.
deltaS = 0.005;

// Number of taps for our filter.
n = computeFilterOrder(deltaP,deltaS,deltaF,Fs)

// Ensure that the filter order is a multiple of 5.
n = computeNextMultiple(n,5);

//******************************************************************
// Generate the FIR filter coefficients and magnitude of frequency
// response..  
//******************************************************************

//------------------------------------------------------------------
// This will be an image reject filter the preceeds the 1:5
// expander
//------------------------------------------------------------------
h = eqfir(n,F/Fsample,[1 0],[1/deltaP 1/deltaS]);

// Compensate for interpolation factor of 5.
h = h * 5;

// Compute magnitude and frequency points.
[hm,fr] = frmag(h,1024);

//******************************************************************
// Plot magnitudes.
//******************************************************************
set("figure_style","new");

title("Antimaging Filter - Sample Rate: 80000 S/s  (interpolated)");
a = gca();
a.margins = [0.225 0.1 0.125 0.2];
a.grid = [1 1];
a.x_label.text = "F, Hz";
a.y_label.text = "|H|";
plot2d(fr*Fsample,hm);

