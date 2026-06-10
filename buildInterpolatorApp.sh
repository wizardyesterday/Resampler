#!/bin/sh

g++ -o interpolate -O2 -I include test/interpolate.cc src/FirFilter.cc src/InterpolatorPoly.cc

