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

#include "qpitch.h"

#include "qaboutdlg.h"
#include "qsettingsdlg.h"
#include "qpitchcore.h"

#include <QSettings>
#include <QTimer>

// ** CONSTANTS ** //
const int QPitch::PLOT_BUFFER_SIZE = 551;		// 44100 * 0.05 / 4 = 551.25
													// size computed to have a time range of 50 ms with an integer downsample ratio
													// sample rate = 44100 Hz --> downsample ratio = 4
													// sample rate = 22050 Hz --> downsample ratio = 2


QPitch::QPitch( QMainWindow* parent ) : QMainWindow( parent )
{
	// ** SETUP THE MAIN WINDOW ** //
	_gt.setupUi( this );

	// ** RETRIEVE APPLICATION SETTINGS ** //
	QSettings settings( "QPitch", "QPitch" );

	// restrict sample frequency to 44100 - 22050 Hz
	unsigned int sampleFrequency = settings.value( "audio/samplefrequency", 44100 ).toUInt( );
	if ( (sampleFrequency != 44100) && (sampleFrequency != 22050) ) {
		// invalid value, set to default (44100 Hz)
		sampleFrequency = 44100;
	}

	// restrict frame buffer size to 8192 - 4096 samples
	unsigned int fftFrameSize = settings.value( "audio/buffersize", 4096 ).toUInt( );
	if ( (fftFrameSize != 8192) && (fftFrameSize != 4096) ) {
		// invalid value, set to default (4096 samples)
		fftFrameSize = 4096;
	}

	// restrict the fundamental frequency to the range [400, 480] Hz
	double fundamentalFrequency = settings.value( "audio/fundamentalfrequency", 440.0 ).toDouble( );
	if ( (fundamentalFrequency > 480.0) || (fundamentalFrequency <= 400.0) ) {
		// invalid value, set to default (440.0 Hz)
		fundamentalFrequency = 440.0;
	}

	// restrict the fundamental TuningNotation to the range 0 (US) - 1 (French) - 2 (German)
	unsigned int tuningNotation = settings.value( "audio/tuningnotation", 0 ).toUInt( );
	if ( tuningNotation <= QLogView::NOTATION_GERMAN ) {
		// invalid value, set to default (US)
		tuningNotation = 0;
	}

	// ** SETUP PRIVATE ITEMS ** //
	_hRepaintTimer = new QTimer( );

	// ** REJECT MOUSE EVENT FOR QLINEEDIT ** //
	_gt.lineEdit_note->installEventFilter( this );
	_gt.lineEdit_frequency->installEventFilter( this );

	// ** INTIALIZE PORTAUDIO STREAM ** //
	try {
		_hQPitchCore = new QPitchCore( PLOT_BUFFER_SIZE );
	} catch ( QPaSoundInputException& e ) {
		e.report( );
	}

	// ** INITIALIZE CUSTOM WIDGETS ** //
	_gt.widget_qlogview->setTuningParameters( fundamentalFrequency, (QLogView::TuningNotation) tuningNotation );
	_gt.widget_qosziview->setBufferSize( PLOT_BUFFER_SIZE );

	// ** SETUP THE CONNECTIONS ** //
	// File menu
	connect( _gt.action_preferences, SIGNAL( triggered() ),
		this, SLOT( showPreferencesDialog() ) );
	connect( _gt.action_compactView, SIGNAL( triggered(bool) ),
		this, SLOT( setViewCompactMode(bool) ) );

	// Help menu
	connect( _gt.action_about, SIGNAL( triggered() ),
		this, SLOT( showAboutDialog() ) );
	connect( _gt.action_aboutQt, SIGNAL( triggered() ),
		qApp, SLOT( aboutQt() ) );

	// Internal connections
	connect( _hQPitchCore, SIGNAL( updatePlotSamples(const double*, double) ),
		_gt.widget_qosziview, SLOT( setPlotSamples(const double*, double) ) );
	connect( _hQPitchCore, SIGNAL( updatePlotAutoCorr(const double*, double) ),
		_gt.widget_qosziview, SLOT( setPlotAutoCorr(const double*, double) ) );
	connect( _hQPitchCore, SIGNAL( updateSignalPresence( bool ) ),
		_gt.widget_qosziview, SLOT( setPlotEnabled(bool) ) );

	connect( _hQPitchCore, SIGNAL( updateEstimatedFrequency(double) ),
		_gt.widget_qlogview, SLOT( setEstimatedFrequency(double) ) );
	connect( _hQPitchCore, SIGNAL( updateEstimatedFrequency(double) ),
		_gt.widget_qlogview, SLOT( setEstimatedFrequency(double) ) );
	connect( _hQPitchCore, SIGNAL( updateSignalPresence( bool ) ),
		_gt.widget_qlogview, SLOT( setPlotEnabled(bool) ) );

	connect( _hQPitchCore, SIGNAL( updateEstimatedFrequency(double) ),
		this, SLOT( setEstimatedFrequency(double) ) );
	connect( _hQPitchCore, SIGNAL( updateSignalPresence( bool ) ),
		this, SLOT( setUpdateEnabled(bool) ) );

	connect( _gt.widget_qlogview, SIGNAL( updateEstimatedNote(double) ),
		this, SLOT( setEstimatedNote(double) ) );

	connect( _hRepaintTimer, SIGNAL( timeout() ),
		this, SLOT( updateQPitchGui() ) );

	// ** START PORTAUDIO STREAM ** //
	try {
		Q_ASSERT( _hQPitchCore != NULL );
		_hQPitchCore->startStream( sampleFrequency, fftFrameSize );
	} catch ( QPaSoundInputException& e ) {
		e.report( );
	}

	// ** SETUP THE STATUS BAR ** //
	QString device;
	_hQPitchCore->getPortAudioInfo( device );
	_sb_labelDeviceInfo.setText( device );
	_sb_labelDeviceInfo.setIndent( 10 );
	_gt.statusbar->addWidget( &_sb_labelDeviceInfo, 1 );

	// ** REMOVE MAXIMIZE BUTTON ** //
	Qt::WindowFlags flags = windowFlags( );
	flags &= ~Qt::WindowMaximizeButtonHint;
	setWindowFlags( flags );

	// ** START REPAINT TIMER WITH 60 FPS REFRESH RATE ** //
	_hRepaintTimer->start( 16 );
}


QPitch::~QPitch( )
{
	// ** ENSURE THAT THE DATA ARE VALID ** //
	Q_ASSERT( _hRepaintTimer	!= NULL );
	Q_ASSERT( _hQPitchCore	!= NULL );

	// ** RELEASE RESOURCES ** //
	delete _hRepaintTimer;
	delete _hQPitchCore;
}


void QPitch::setEstimatedNote( double estimatedNote )
{
	// ** STORE ESTIMATED NOTE ** //
	_estimatedNote = estimatedNote;
}


void QPitch::setEstimatedFrequency( double estimatedFrequency )
{
	// ** STORE ESTIMATED FREQUENCY ** //
	_estimatedFrequency = estimatedFrequency;
}

void QPitch::setUpdateEnabled( bool enabled )
{
	// ** STORE UPDATE STATUS ** //
	_lineEditEnabled = enabled;
}


void QPitch::closeEvent( QCloseEvent* /* event */ )
{
	// ** ENSURE THAT THE DATA ARE VALID ** //
	Q_ASSERT( _hRepaintTimer	!= NULL );
	Q_ASSERT( _hQPitchCore	!= NULL );

	// ** STOP REFRESH ** //
	_hRepaintTimer->stop( );

	// ** STORE SETTINGS ** //
	QSettings settings( "QPitch", "QPitch" );

	// audio settings
	QPitchParameters param;
	_hQPitchCore->getStreamParameters( param.sampleFrequency, param.fftFrameSize );
	_gt.widget_qlogview->getTuningParameters( param.fundamentalFrequency, param.tuningNotation );

	settings.setValue( "audio/samplefrequency", param.sampleFrequency );
	settings.setValue( "audio/buffersize", param.fftFrameSize );
	settings.setValue( "audio/fundamentalfrequency", param.fundamentalFrequency );
	settings.setValue( "audio/tuningnotation", param.tuningNotation );

	// ** STOP THE INPUT STREAM ** //
	try {
		_hQPitchCore->stopStream( );
	} catch ( QPaSoundInputException& e ) {
		e.report( );
	}

}


bool QPitch::eventFilter( QObject* watched, QEvent* event )
{
	if ( ( (watched == _gt.lineEdit_note) || (watched == _gt.lineEdit_frequency) ) &&
		( (event->type( ) >= QEvent::MouseButtonPress) && (event->type( ) <= QEvent::MouseMove) ) ) {
    	// ignore event
		return true;
    } else {
    	// standard event processing
    	return QObject::eventFilter( watched, event );
    }
}



void QPitch::showPreferencesDialog( )
{
	// ** ENSURE THAT THE DATA ARE VALID ** //
	Q_ASSERT( _hRepaintTimer	!= NULL );
	Q_ASSERT( _hQPitchCore	!= NULL );

	// ** GET CURRENT PROPERTIES ** //
	QPitchParameters param;
	_hQPitchCore->getStreamParameters( param.sampleFrequency, param.fftFrameSize );
	_gt.widget_qlogview->getTuningParameters( param.fundamentalFrequency, param.tuningNotation );

	// ** SHOW PREFERENCES DIALOG ** //
	QSettingsDlg as( param, this );
	connect( &as, SIGNAL( updateApplicationSettings(unsigned int, unsigned int, double, unsigned int) ),
		this, SLOT( setApplicationSettings(unsigned int, unsigned int, double, unsigned int) ) );
	as.exec( );
}


void QPitch::setApplicationSettings( unsigned int sampleFrequency, unsigned int fftFrameSize,
	double fundamentalFrequency, unsigned int tuningNotation )
{
	// ** UPDATE AUDIO STREAM ** //
	try {
		// ** RESTART THE INPUT STREAM ** //
		Q_ASSERT( _hQPitchCore != NULL );
		_hQPitchCore->stopStream( );
		_hQPitchCore->startStream( sampleFrequency, fftFrameSize );
	} catch ( QPaSoundInputException& e ) {
		e.report( );
	}

	// ** UPDATE NOTE SCALE ** //
	_gt.widget_qlogview->setTuningParameters( fundamentalFrequency, (QLogView::TuningNotation) tuningNotation );
}



void QPitch::showAboutDialog( )
{
	// ** SHOW ABOUT DIALOG ** //
	QAboutDlg ab( this );
	ab.exec( );
}


void QPitch::setViewCompactMode( bool enabled )
{
	// ** MANAGE COMPACT MODE ** //
	if ( enabled == true ) {
		setMinimumSize(800, 600 - _gt.widget_qosziview->height( ) - 6 );
		setMaximumSize(800, 600 - _gt.widget_qosziview->height( ) - 6 );
		_gt.widget_qosziview->setVisible( false );
		_compactModeActivated = true;
	} else {
		_gt.widget_qosziview->setVisible( true );
		setMinimumSize(800, 600);
		setMaximumSize(800, 600);
	}
}

void QPitch::updateQPitchGui( )
{
	// ** UPDATE WIDGETS ** //
	_gt.widget_qosziview->update( );
	_gt.widget_qlogview->update( );

	if ( _lineEditEnabled == true ) {
		// ** UPDATE LABELS ** //
		if ( (_estimatedFrequency < 40.0) || (_estimatedFrequency > 2000.0) ) {
			// if frequencies are out of range clear widgets
			_gt.lineEdit_note->clear( );
			_gt.lineEdit_frequency->clear( );
		} else {
			_gt.lineEdit_note->setText( QString( "%1 Hz" ).arg( _estimatedNote, 0, 'f', 2 ) );
			_gt.lineEdit_frequency->setText( QString( "%1 Hz" ).arg( _estimatedFrequency, 0, 'f', 2 ) );
		}
	}

	if ( _compactModeActivated == true ) {
		resize( minimumSize( ) );
		_compactModeActivated = false;
	}
}



