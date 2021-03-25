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

#include "qpitchcore.h"

#include <QMessageBox>
#include <QMutex>
#include <QtDebug>
#include <QWaitCondition>

#ifdef _REFERENCE_SQUAREWAVE_INPUT
	#include <cmath>
#endif


// ** INITIALIZATION OF STATIC VARIABLES ** //
const int QPitchCore::ZERO_PADDING_FACTOR	= 8;
const int QPitchCore::SIGNAL_THRESHOLD_ON	= 100;
const int QPitchCore::SIGNAL_THRESHOLD_OFF	= 20;


QPitchCore::QPitchCore( const unsigned int plotPlot_size, QObject* parent ) : QThread( parent )
{
	// ** INITIALIZE PRIVATE VARIABLES ** //
	_mutex			= new QMutex( );
	_waitCond		= new QWaitCondition( );
	_stream			= NULL;
	_buffer			= NULL;
	_fftw_plan_FFT	= NULL;
	_fftw_plan_IFFT	= NULL;
	_fftw_in_time	= NULL;
	_fftw_out_freq 	= NULL;

	// ** INITIALIZE TEMPORARY BUFFERS ** //
	_plotData_size	= plotPlot_size;
	_plotSample		= new double[plotPlot_size];
	_plotAutoCorr	= new double[plotPlot_size];

	// ** INITIALIZE PORTAUDIO ** //
	PaError err = Pa_Initialize( );
	if ( err != paNoError ) {
		throw QPaSoundInputException( Pa_GetErrorText( err ) );
	}
}


QPitchCore::~QPitchCore( )
{
	// ** ENSURE THAT THE STREAM IS STOPPED AND THE THREAD NOT RUNNING ** //
	Q_ASSERT( _stream		== NULL );
	Q_ASSERT( _mutex		!= NULL );
	Q_ASSERT( _waitCond		!= NULL );
	Q_ASSERT( _plotSample	!= NULL );
	Q_ASSERT( _plotAutoCorr	!= NULL );
	Q_ASSERT( _running		== false );
	Q_ASSERT( ! this->isRunning( ) );

	// ** TERMINATE PORTAUDIO ** //
	Pa_Terminate( );

	// ** RELEASE RESOURCES ** //
	delete		_mutex;
	delete		_waitCond;
	delete[]	_plotSample;
	delete[]	_plotAutoCorr;
}


void QPitchCore::startStream( const unsigned int sampleFrequency, const unsigned int fftFrameSize )
{
	// ** ENSURE THAT THE STREAM IS STOPPED AND THE THREAD IS NOT RUNNING ** //
	Q_ASSERT( _stream == NULL );
	Q_ASSERT( _buffer == NULL );
	Q_ASSERT( ! this->isRunning( ) );

#ifdef _REFERENCE_SQUAREWAVE_INPUT
	// create the artificial square wave with some harmonics :: 110.0 Hz
	for ( unsigned int k = 0 ; k < 4410 ; ++k ) {
		_referenceSineWave[k] = ( (sin( 2 * M_PI * 110.0 * (double) k / sampleFrequency ) >= 0.0) ? 1000 : -1000 );
	}
	_referenceSineWave_index = 0;
#endif

	// ** CONFIGURE THE INPUT AUDIO STREAM ** //
	_sampleFrequency							=	sampleFrequency;
	_inputParameters.device						=	Pa_GetDefaultInputDevice( );				// default input device
	_inputParameters.channelCount				=	1;											// mono input
	_inputParameters.sampleFormat				=	paInt16;									// 16 bit integer
	_inputParameters.suggestedLatency			=	Pa_GetDeviceInfo( _inputParameters.device )->defaultHighInputLatency;
																								// set the latency for a robust non-interactive application
	_inputParameters.hostApiSpecificStreamInfo	=	NULL;

	// ** OPEN AN AUDIO INPUT STREAM ** //
	_buffer_size = (unsigned int)((double) _inputParameters.suggestedLatency * sampleFrequency);
	PaError err = Pa_OpenStream(
		&_stream,
		&_inputParameters,
		NULL,									// no output
		_sampleFrequency,						// sample rate (default 44100 Hz)
		(long) _buffer_size,					// frames per buffer
		paClipOff,								// disable clipping
        paCallback,                             // callback
		this									// pointer to user data
	);

	if ( err != paNoError ) {
		throw QPaSoundInputException( Pa_GetErrorText( err ) );
	}

	// ** INITIALIZE BUFFERS ** //
	_buffer 			= new short int[_buffer_size];
	_fftw_in_time_size 	= fftFrameSize;			// size of the external buffer (default 2048)
	_fftw_in_time_index	= 0;

	// ** INITIALIZE FFT STRUCTURES ** //
	_fftw_in_time	= (double*) fftw_malloc( ZERO_PADDING_FACTOR * sizeof(double) * _fftw_in_time_size );
	_fftw_out_freq	= (fftw_complex*) fftw_malloc( ZERO_PADDING_FACTOR * sizeof(fftw_complex) * _fftw_in_time_size );
	_fftw_plan_FFT	= fftw_plan_dft_r2c_1d( _fftw_in_time_size, _fftw_in_time, _fftw_out_freq, FFTW_ESTIMATE );							// FFT
	_fftw_plan_IFFT	= fftw_plan_dft_c2r_1d( ZERO_PADDING_FACTOR * _fftw_in_time_size, _fftw_out_freq, _fftw_in_time, FFTW_ESTIMATE );	// IFFT zero-padded

	// ** START PORTAUDIO STREAM ** //
	err = Pa_StartStream( _stream );
	if( err != paNoError ) {
		throw QPaSoundInputException( Pa_GetErrorText( err ) );
	}

	// ** START WORKING THREAD ** //
	_running = true;
	this->start( );

	// ** ENSURE THAT THE STREAM IS STARTED AND THE THREAD IS RUNNING ** //
	Q_ASSERT( _stream != NULL );
	Q_ASSERT( _buffer != NULL );
	Q_ASSERT( this->isRunning( ) );

	qDebug( ) << "QPitchCore::startStream";
	qDebug( ) << " - sampleFrequency         = " << _sampleFrequency;
	qDebug( ) << " - defaultHighInputLatency = " << _inputParameters.suggestedLatency;
	qDebug( ) << " - framesPerBuffer         = " << _buffer_size;
	qDebug( ) << " - fftFrameSize            = " << _fftw_in_time_size << "\n";
}


void QPitchCore::stopStream( )
{
	// ** ENSURE THAT THE STREAM IS STARTED ** //
	Q_ASSERT( _stream			!= NULL );
	Q_ASSERT( _buffer			!= NULL );
	Q_ASSERT( _fftw_plan_FFT	!= NULL );
	Q_ASSERT( _fftw_plan_IFFT 	!= NULL );
	Q_ASSERT( _fftw_in_time		!= NULL );
	Q_ASSERT( _fftw_out_freq	!= NULL );

	// ** STOP THE THREAD ** //
	_mutex->lock( );
	_running = false;
	_mutex->unlock( );

	// ** STOP PORTAUDIO STREAM ** //
	PaError err = Pa_StopStream( _stream );
	if( err != paNoError ) {
		throw QPaSoundInputException( Pa_GetErrorText( err ) );
	}

	// ** CLOSE PORTAUDIO STREAM ** //
	err = Pa_CloseStream( _stream );
	if( err != paNoError ) {
		throw QPaSoundInputException( Pa_GetErrorText( err ) );
	}

	// ** DESTROY FFTW STRUCTURES ** //
	fftw_destroy_plan( _fftw_plan_FFT );
	fftw_destroy_plan( _fftw_plan_IFFT );
	fftw_free( _fftw_in_time );
	fftw_free( _fftw_out_freq );

	// ** RELEASE RESOURCES ** //
	delete[] _buffer;
	_buffer			= NULL;
	_stream			= NULL;
	_fftw_plan_FFT	= NULL;
	_fftw_plan_FFT	= NULL;
	_fftw_in_time 	= NULL;
	_fftw_out_freq 	= NULL;
}


void QPitchCore::getStreamParameters( unsigned int& sampleFrequency, unsigned int& fftBufferSize ) const
{
	// ** ENSURE THAT THE STREAM IS STARTED ** //
	Q_ASSERT( _stream != NULL );

	// ** GET STREAM PROPERTIES ** //
	sampleFrequency	= (unsigned int) _sampleFrequency;
	fftBufferSize	= _fftw_in_time_size;
}


void QPitchCore::getPortAudioInfo( QString& device ) const
{
	// ** ENSURE THAT THE STREAM IS STARTED ** //
	Q_ASSERT( _stream != NULL );

	// ** RETRIEVE STREAM PROPERTIES ** //
	device = QString( "Device: " + QString(Pa_GetDeviceInfo( _inputParameters.device )->name) + " [" +
		QString(Pa_GetHostApiInfo( Pa_GetDeviceInfo( _inputParameters.device )->hostApi )->name)  + "]");
}


int QPitchCore::paCallback( const void* input, void* /*output*/, unsigned long frameCount,
		const PaStreamCallbackTimeInfo* /*timeInfo*/, PaStreamCallbackFlags /*statusFlags*/, void* userData )
{
	Q_ASSERT( input		!= NULL );
	Q_ASSERT( userData	!= NULL );

    return( static_cast<QPitchCore*>( userData )->paStoreInputBufferCallback( (const short int*)input, frameCount ) );
}

int QPitchCore::paStoreInputBufferCallback( const short int* input, unsigned long frameCount )
{
    // ** TRY TO ACQUIRE THE SEMAPHORE ** //
    if ( _mutex->tryLock( ) ) {
        // buffer has been locked
        _mutex->unlock( );

        // ** COPY BUFFER ** //
#ifdef _REFERENCE_SQUAREWAVE_INPUT
        // ** USE THE REFERENCE SINE WAVE INPUT SIGNAL ** //
        for ( unsigned int k = 0 ; k < frameCount ; ++k ) {
            _buffer[k] = _referenceSineWave[_referenceSineWave_index++];
            if ( _referenceSineWave_index >= 4410 ) {
                _referenceSineWave_index = 0;
            }
        }
#else
        // ** READ THE REAL AUDIO SIGNAL ** //
        memcpy( _buffer, input, frameCount * sizeof( short int ) );
#endif

        // ** REQUEST BUFFER RELEASE ** //
        _waitCond->wakeOne( );
    } else {
        // buffer cannot be locked
        std::cerr << "QPitch: buffer full, dropping " << frameCount << " samples!\n";
        // drop all the samples in the external buffer
        _fftw_in_time_index = 0;
    }

	return paContinue;
}


void QPitchCore::run( )
{
	// ** ENSURE THAT FFTW STRUCTURES ARE VALID ** //
	Q_ASSERT( _fftw_plan_FFT	!= NULL );
	Q_ASSERT( _fftw_plan_IFFT 	!= NULL );
	Q_ASSERT( _fftw_in_time		!= NULL );
	Q_ASSERT( _fftw_out_freq	!= NULL );
	Q_ASSERT( _fftw_in_time		!= NULL );
	Q_ASSERT( _fftw_out_freq	!= NULL );
	Q_ASSERT( _plotSample		!= NULL );
	Q_ASSERT( _plotAutoCorr		!= NULL );

	// initialize the visualization status
	_visualizationStatus = STOPPED;

	forever {
		// lock the buffer
		_mutex->lock( );
		if ( _running == false ) {
			_mutex->unlock( );
				_visualizationStatus = STOPPED;
			return;
		}

		// ** PROCESS THE BUFFER AND SLEEP TILL THE NEXT FRAME ** //
		// transfer the internal buffer to the external buffer and
		// drop all the samples that exceed its length
		// check if the whole signal is below a given threshold to
		// stop visualization

		unsigned int k = 0;

		// trigger the signal to have the first sample on a rising edge accross zero
		if ( _fftw_in_time_index == 0 ) {
            for (  ; (k < (_buffer_size - 1)) && ((_buffer[k] >= 0) || (_buffer[k+1] < 0)) ; ++k ) {};
		}

		// check if the audio stream is below a given threshold to stop visualization
		if ( _visualizationStatus == STOPPED ) {
			for (  ; ( (k < _buffer_size) && (_fftw_in_time_index < _fftw_in_time_size) && ( (_buffer[k] < SIGNAL_THRESHOLD_ON) && (_buffer[k] > -SIGNAL_THRESHOLD_ON) ) ) ; ++k ) {
				_fftw_in_time[_fftw_in_time_index++] = _buffer[k];
			}
		} else if ( _visualizationStatus == RUNNING ) {
			for (  ; ( (k < _buffer_size) && (_fftw_in_time_index < _fftw_in_time_size) && ( (_buffer[k] < SIGNAL_THRESHOLD_OFF) && (_buffer[k] > -SIGNAL_THRESHOLD_OFF) ) ) ; ++k ) {
				_fftw_in_time[_fftw_in_time_index++] = _buffer[k];
			}
		}

		// check if the level has been triggered
		if ( (k == _buffer_size) || (_fftw_in_time_index == _fftw_in_time_size) ) {
			// if the array end has been hit the level of the signal is too low, so drop all the buffer
			_fftw_in_time_index = 0;

			if ( _visualizationStatus == RUNNING ) {
				_visualizationStatus = STOP_REQUEST;
			}
		} else {
			// read the remaining of the buffer
			for (  ; ( (k < _buffer_size) && (_fftw_in_time_index < _fftw_in_time_size) ) ; ++k ) {
				_fftw_in_time[_fftw_in_time_index++] = _buffer[k];
			}

			if ( _visualizationStatus == STOPPED ) {
				_visualizationStatus = START_REQUEST;
			}

			// process the external buffer if required
			if ( _fftw_in_time_index == _fftw_in_time_size ) {
				// downsample factor used to extract a buffer with a time range of 50 milliseconds
				unsigned int fftw_in_downsampleFactor;
				if ( _sampleFrequency == 44100.0 ) {
					fftw_in_downsampleFactor = 4;
				} else if ( _sampleFrequency == 22050.0 ) {
					fftw_in_downsampleFactor = 2;
				}

				for ( unsigned int k = 0 ; k < _plotData_size ; ++k ) {
					Q_ASSERT( (k * fftw_in_downsampleFactor) < (_fftw_in_time_size) );
					_plotSample[k] = _fftw_in_time[k * fftw_in_downsampleFactor];
				}
				emit updatePlotSamples( _plotSample, _fftw_in_time_size / _sampleFrequency );

				// reset the index in the external buffer
				_fftw_in_time_index = 0;

				// compute the autocorrelation and find the best matching frequency
				double estimatedFrequency = fftw_pitchDetectionAlgorithm( );
				emit updateEstimatedFrequency( estimatedFrequency );

				// extract autocorrelation samples for the oscilloscope view in the range [40, 1000] Hz --> [0, 25] msec
				unsigned int fftw_out_downsampleFactor;
				if ( _sampleFrequency == 44100.0 ) {
					fftw_out_downsampleFactor = 2 * ZERO_PADDING_FACTOR;
				} else if ( _sampleFrequency == 22050.0 ) {
					fftw_out_downsampleFactor = 1 * ZERO_PADDING_FACTOR;
				}

				for ( unsigned int k = 0 ; k < _plotData_size ; ++k ) {
					Q_ASSERT( (k * fftw_out_downsampleFactor) < (ZERO_PADDING_FACTOR * _fftw_in_time_size) );
					_plotAutoCorr[k] = _fftw_in_time[k * fftw_out_downsampleFactor];
				}
				emit updatePlotAutoCorr( _plotAutoCorr, estimatedFrequency );
			}
		}

		// manage the visualization status
		if ( _visualizationStatus == STOP_REQUEST ) {
			emit updateSignalPresence( false );
			_visualizationStatus = STOPPED;
		} else if ( _visualizationStatus == START_REQUEST ) {
			emit updateSignalPresence( true );
			_visualizationStatus = RUNNING;
		}

		// release the buffer and wait
		_waitCond->wait( _mutex );
		_mutex->unlock( );
	}
}


double QPitchCore::fftw_pitchDetectionAlgorithm( )
{
	// ** ENSURE THAT FFTW STRUCTURES ARE VALID ** //
	Q_ASSERT( _fftw_plan_FFT	!= NULL );
	Q_ASSERT( _fftw_plan_IFFT 	!= NULL );
	Q_ASSERT( _fftw_in_time		!= NULL );
	Q_ASSERT( _fftw_out_freq	!= NULL );

	// ** COMPUTE THE AUTOCORRELATION ** //
	// compute the FFT of the input signal
	fftw_execute( _fftw_plan_FFT );

	/*
	 * compute the transform of the autocorrelation given in time domain by
	 *
	 *        k=-N
	 * r[t] = sum( x[k] * x[t-k] )
	 *         N
	 *
	 * or in the frequency domain (for a real signal) by
	 *
	 * R[f] = X[f] * X[f]' = |X[f]|^2 = Re(X[f])^2 + Im(X[f])^2
	 *
	 * when computing the FFT with fftw_plan_dft_r2c_1d there are only N/2
	 * significant samples so we only need to compute the |.|^2 for
	 * _fftw_in_time_Size/2 samples
	 */

	// compute |.|^2 of the signal
	for( unsigned int k = 0 ; k < (_fftw_in_time_size / 2 + 1) ; ++k ) {
		_fftw_out_freq[k][0] = (_fftw_out_freq[k][0] * _fftw_out_freq[k][0]) + (_fftw_out_freq[k][1] * _fftw_out_freq[k][1]);
		_fftw_out_freq[k][1] = 0.0;
	}

	// pad the FFT with zeros to increase resolution
	memset( &(_fftw_out_freq[_fftw_in_time_size/ 2 + 1][0]), 0, ( (ZERO_PADDING_FACTOR - 1) * _fftw_in_time_size + _fftw_in_time_size/ 2 - 1) * sizeof(fftw_complex) );

	// compute the IFFT to obtain the autocorrelation in time domain
	fftw_execute( _fftw_plan_IFFT );

	// find the maximum of the autocorrelation (rejecting the first peak)
	/*
	 * the main problem with autocorrelation techniques is that a peak may also
	 * occur at sub-harmonics or harmonics, but right now I can't come up with
	 * anything better =(
	 */

	// search for a minimum in the autocorrelation to reject the peak centered around 0
	unsigned int l;
    for ( l = 0 ; (l < ( (ZERO_PADDING_FACTOR / 2) * _fftw_in_time_size + 1)) && ( (_fftw_in_time[l+1] < _fftw_in_time[l]) || (_fftw_in_time[l+1] > 0.0) ) ; ++l ) {};

	// search for the maximum
	double 			maxAutoCorrelation			= 0.0;
	unsigned int	maxAutoCorrelation_index	= 0;
	for (  ; l < ( (ZERO_PADDING_FACTOR / 2) * _fftw_in_time_size + 1) ; ++l ) {
		if ( _fftw_in_time[l] > maxAutoCorrelation ) {
			maxAutoCorrelation			= _fftw_in_time[l];
			maxAutoCorrelation_index	= l;
		}
	}

	// compute the frequency of the maximum considering the padding factor
	return ( (ZERO_PADDING_FACTOR / 2) * (2.0 * _sampleFrequency) / (double) maxAutoCorrelation_index );
}

