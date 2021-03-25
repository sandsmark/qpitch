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

#ifndef __QPITCHCORE_H_
#define __QPITCHCORE_H_

//! Definition used to feed the application with a reference squarewave
//#define _REFERENCE_SQUAREWAVE_INPUT

#include <cstring>
#include <iostream>
#include <stdexcept>

#include <fftw3.h>
#include <portaudio.h>

#include <QMessageBox>
#include <QThread>

class QMutex;
class QWaitCondition;


//! An exception thrown when a PortAudio error occurs
class QPaSoundInputException : public std::runtime_error {

public: /* methods */
	//! Default constructor.
	/*!
	 * \param[in] msg the error message to display
	 */
	QPaSoundInputException( const std::string& msg ) : std::runtime_error( msg ) {
		return;
	};

	//! Report exception.
	void report( ) {
		QMessageBox::critical( NULL, "QPitch", QString( "PortAudio error: %1." ).arg( this->what( ) ) );
		std::cerr << "PortAudio error: " << this->what( ) << "\n";
	};
};


//! Working thread for the QPitch application.
/*!
 * This class implements the main working thread for the QPitch
 * application.
 * The audio stream is acquired through the PortAudio library
 * (cross-platform) using a callback function. The size of the internal
 * buffer is set to the size suggested for robust non-interactive
 * application, since the latency is not an issue in this application.
 * In the current version the default audio input stream is used,
 * thus the selection of the audio input is performed using the control
 * panel of the operating system.
 * The pitch detection algorithm is based on the identification of the
 * first peak in the autocorrelation of the signal, which is computed
 * as the inverse FFT of the power spectral density of the signal
 * (the squared module of the signal FFT). The FFT is computed using
 * the FFTW3 library and prior to the inverse transform the signal is
 * zero-padded to increase the resolution of the autocorrelation in
 * order to have a better frequency identification.
 */

class QPitchCore : public QThread {
	Q_OBJECT


#ifdef _REFERENCE_SQUAREWAVE_INPUT
public: /* members */
	short int			_referenceSineWave[4410];				//!< Artificial sine-wave used for debug
	unsigned int		_referenceSineWave_index;				//!< Index incremented after each step to simulate time
#endif


public: /* methods */
	//! Default constructor.
	/*!
	 * \param[in] plotPlot_size the number of sample in the buffer used for visualization
	 * \param[in] parent a QObject* with the handle of the parent
	 */
	QPitchCore( const unsigned int plotPlot_size = 512, QObject* parent = 0 );

	//! Default destructor.
	~QPitchCore( );

	//! Start an input audio stream with the given properties.
	/*!
	 * \param[in] sampleFrequency the sample rate of the input stream (default 44100)
	 * \param[in] fftFrameSize the size of the frame used to compute the FFT and the note pitch (default 4096)
	 */
	void startStream( const unsigned int sampleFrequency = 44100, const unsigned int fftFrameSize = 4096 );

	//! Stop the input audio stream.
	void stopStream( );

	//! Retrieve the device parameters.
	/*!
	 * \param[out] device the name of the device used by PortAudio plus the name of the API used by PortAudio
	 */
	void getPortAudioInfo( QString& device ) const;

	//! Retrieve the audio stream parameters.
	/*!
	 * \param[out] sampleFrequency the sample rate of the input stream
	 * \param[out] fftBufferSize the size of the frame used to compute the FFT and the note pitch
	 */
	void getStreamParameters( unsigned int& sampleFrequency, unsigned int& fftBufferSize ) const;
	//	double& ) const;

    /*! \brief Dummy callback function to call the real non-static callback that does the work.
     *  \param[in] input Pointer to the interleaved input samples.
     *  \param[out] output Pointer to the interleaved output samples.
     *  \param[in] frameCount Number of sample frames to be processed.
     *  \param[in] timeInfo Time when the buffer is processed.
     *  \param[in] statusFlags Whether underflow or overflow occurred.
     *  \param[in] userData Pointer to user data.
     */
    static int paCallback( const void* input, void* output, unsigned long frameCount,
        const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData );

    /*! \brief Play the selected WAV file through the selected PortAudio device.
     *  \param[in] input Pointer to the interleaved input samples.
     *  \param[in] frameCount Number of sample frames to be processed.
     */
    int paStoreInputBufferCallback( const short int* output, unsigned long frameCount );

signals:
	//! Request an update in the audio stream signal graph.
	/*!
	 * \param[in] osziSample the array with the new samples of the input signal to display
	 * \param[in] osziSample_timeRange the time range used to place the tick marks on the x-axis
	 */
	void updatePlotSamples( const double* osziSample, double osziSample_timeRange );

	//! Request an update in the autocorrelation graph.
	/*!
	 * \param[in] osziAutoCorr the array with the new samples of the autocorrelation to display
 	 * \param[in] estimatedFrequency the value of the signal frequency estimated as the maximum of the autocorrelation
	 */
	void updatePlotAutoCorr( const double* osziAutoCorr, double estimatedFrequency );

	//! Request an update in the displayed value of the estimated frequency.
	/*!
	 * \param[in] estimatedFrequency the value of the signal frequency estimated as the maximum of the autocorrelation
	 */
	void updateEstimatedFrequency( double estimatedFrequency );

	//! Signal the level of the input signal.
	/*!
	 * \param[in] signalPresent flag with the current signal presence
	 */
	void updateSignalPresence( bool signalPresent );


protected:
	//! Main loop of the thread.
	virtual void run( );


private: /* enumerations */
	//! Status of the visualization used to handle silence in the input stream.
	enum VisualizationStatus {
		STOPPED,
		STOP_REQUEST,
		START_REQUEST,
		RUNNING
		};

private: /* static constants */
	static const int	ZERO_PADDING_FACTOR;					//!< Number of times that the FFT is zero-padded to increase frequency resolution
	static const int 	SIGNAL_THRESHOLD_ON;					//!< Value of the threshold above which the processing is activated
	static const int 	SIGNAL_THRESHOLD_OFF;					//!< Value of the threshold below which the input audio signal is deactivated


private: /* members */
	// ** PORTAUDIO STREAM ** //
	PaStreamParameters	_inputParameters;						//!< Parameters of the input audio stream
	PaStream*			_stream;								//!< Handle to the PortAudio stream
	double				_sampleFrequency;						//!< PortAudio stream
	short int*			_buffer;								//!< Internal buffer to store the input samples read in the callback
	unsigned int		_buffer_size;							//!< Size of the internal buffer

	// ** FFTW STRUCTURES ** //
	fftw_plan			_fftw_plan_FFT;							//!< Plan to compute the FFT of a given signal
	fftw_plan			_fftw_plan_IFFT;						//!< Plan to compute the IFFT of a given signal (with additional zero-padding
	double*				_fftw_in_time;							//!< External buffer used to store signals in the time domain (first the input signal and then its autocorrelation)
	unsigned int		_fftw_in_time_size;						//!< Size of the external buffer
	unsigned int		_fftw_in_time_index;					//!< Index in the external buffer
	fftw_complex*		_fftw_out_freq;							//!< Buffer used to store signals in the frequency domain (first the FFT of the input signal and later the FFT of its autocorrelation)
	// ** THREAD HANDLING ** //
	bool				_running;								//!< True when the thread is running
	QMutex*				_mutex;									//!< Mutex used by the wait condition
	QWaitCondition*		_waitCond;								//!< Wait condition used to put the thread to sleep while waiting for audio samples

	// ** TEMPORARY BUFFERS USED FOR VISUALIZATION ** //
	double*				_plotSample;							//!< Buffer used to store time samples used for visualization
	double*				_plotAutoCorr;							//!< Buffer used to store autocorrelation samples used for visualization
	unsigned int		_plotData_size;							//!< Total number of samples used for visualization
	VisualizationStatus	_visualizationStatus;					//!< Visualization status used to handle silence

private: /* methods */
	//! Estimate the pitch of the input signal finding the first peak of the autocorrelation.
	/*!
	 * \return the frequency value corresponding to the maximum of the autocorrelation
	 */
	double fftw_pitchDetectionAlgorithm( );
};
#endif

