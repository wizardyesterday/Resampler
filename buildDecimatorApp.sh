#!/bin/sh

g++ -o decimate -O2 -I include test/decimate.cc src/FirFilter.cc src/DecimatorPoly.cc

