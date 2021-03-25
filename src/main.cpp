/*
 * QPitch 1.0.1 - Simple chromatic tuner
 * Copyright (C) 1999-2009 William Spinelli <wylliam@tiscali.it>
 *                         Florian Berger <harpin_floh@yahoo.de>
 *                         Reinier Lamers <tux_rocker@planet.nl>
 *                         Pierre Dumuid
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/**
\mainpage
\htmlonly
<div style="text-align:justify">
<h3>About QPitch 1.0.1</h3>
<p style="font-size: 13px;">
<a style="float:right; padding:0px 0px 15px 25px" href="QPitch-1.0.1-big.png"><img src="QPitch-1.0.1.png" alt="QPitch 1.0.1"/></a>
QPitch is a program to tune a musical instrument using your computer and its
microphone or line-in input. It's accurate and easy to use. It is designed to
be completely cross-platform and works on Linux, Mac and Windows platforms.</p>

<p style="font-size: 13px;">The pitch detection algorithm is based on finding
the first peak in the
<a href="http://en.wikipedia.org/wiki/Autocorrelation">autocorrelation</a> of
the input signal, which is computed as the inverse Fourier transform of the
power spectral density. The pitch detection method is quite reliable (even if a
peak may also occur at sub-harmonics or harmonics leading to an erroneous pitch
detection).</p>

<p style="font-size: 13px;">The estimated pitch is displayed on a note scale
graphic, while the input signal and the autocorrelation function may be
displayed using an oscilloscope-like plot. The note will be found
automatically, since the program doesn't require to select the note to tune
manually.</p>

<p style="font-size: 13px;">QPitch is an offspring of
<a href="http://home.planet.nl/~lamer024/k3guitune.html">K3Guitune</a>. The
layout of the GUI is quite similar to the original ones, however the
application has been rewritten almost from scratch to take advantage of the new
features of <a href="http://trolltech.com/">Qt4</a> and to use the
cross-platform <a href="http://www.portaudio.com">PortAudio</a> library. The
FFT is performed using the <a href="http://www.fftw.org">FFTW</a> library.</p>

<p style="font-size: 13px;">The documentation (and even this site) is generated
with <a href="http://www.doxygen.org">Doxygen</a>.</p>

<h3>Compilation from source</h3>
<p style="font-size: 13px;">The compilation of QPitch from source requires
the following libraries and tools</p>
<ul style="padding-left: 30px; font-size: 13px;">
<li><a href="http://trolltech.com">Qt4</a> (version &gt;= 4.2.0)</li>
<li><a href="http://www.portaudio.com">PortAudio</a> (version &gt;= pa_stable_v19_061121)</li>
<li><a href="http://www.fftw.org">FFTW</a> (version &gt;= 3.1.0)</li>
<li><a href="http://www.cmake.org">CMake</a> (version &gt;= 2.4.5)</li>

<pre style="font-size: 13px; background: #f7f7f7; border: 1px solid #d7d7d7;
margin: 1em 1.75em; padding: .25em; overflow: auto;">
$ tar -xzvf qpitch-1.*.tar.gz
$ cd qpitch-1.*
$ mkdir build
$ cd build
$ cmake ..
$ make</pre>
</div>
\endhtmlonly
*/

#include <QApplication>

#include "qpitch.h"

int main( int argc, char *argv[] )
{
	// ** CREATE QT APPLICATION ** //
	QApplication app( argc, argv );

	// ** OPEN MAIN WINDOW ** //
	QPitch* qpitch = new QPitch( );
	qpitch->show( );

	// ** GIVE CONTROL TO QT ** //
	return app.exec( );
}

