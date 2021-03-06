TODO
====

Port away from portaudio, it is a shitshow.



QPitch 1.0.1 - Simple chromatic tuner
=====================================

![screenshot](/screenshot.png)

QPitch is a program to tune a musical instrument using your
computer and its microphone or line-in input. It is designed
to be completely cross-platform and works on Linux, Mac and
Windows platforms.

QPitch 1.0 is an offspring of K3Guitune 1.0. The layout of
the GUI and the pitch detection algorithm are quite similar
to the original ones, however the application has been
rewritten almost from scratch to take advantage of the new
features of Qt4 and to use the cross-platform PortAudio
library. The FFT is performed using the FFTW library.

The pitch detection algorithm is based on finding the first
peak in the autocorrelation of the input signal. The method
is quite reliable (even if a peak may also occur at
sub-harmonics or harmonics leading to an erroneous pitch
detection). The fundamental frequency used as a reference
for pitch detection can be configured according to the user
preferences.

The estimated pitch is displayed on a note scale graphic,
while the input signal and the autocorrelation function are
displayed using an oscilloscope-like plot.


Requirements
============
 - Portaudio	<www.portaudio.com>
 - Qt toolkit			<www.trolltech.com>
 - FFTW3			<www.fftw.org>

As usual the latest the version, the best =)


Compilation and installation
============================
There are basically two ways to compile QPitch (with the
first one preferred)

Compilation using cmake )contributed by Nico Schlömer)
------------------------------------------------------
The first option is mainly based on the cmake utility
to create a suitable Makefile. Any version of cmake grater
than 2.4 should be enough

$ tar -xjvf qpitch-1.0.1.tar.bz2
$ cd qpitch-1.0.1
$ mkdir build
$ cd build
$ cmake ..
$ make


Authors and contributors
========================

```
Main author of QPitch
  William Spinelli <wylliam@tiscali.it>

Authors of the original K3Guitune
  Florian Berger <harpin_floh@yahoo.de>
  Reinier Lamers <tux_rocker@planet.nl>
  Pierre Dumuid

Cmake build script provided by
  Nico Schlömer <nico.schloemer@gmx.net>
```
