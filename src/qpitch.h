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

#ifndef __QPITCH_H_
#define __QPITCH_H_

#include "ui_qpitch.h"

#include <QMainWindow>

class QPitchCore;
class QTimer;


//! Main window of the application.
/*!
 * This class implements the main window of the application.
 * It has been created using the QtDesigner and its role is
 * simply to display widgets and connect signals.
 */

class QPitch : public QMainWindow {
	Q_OBJECT


public: /* methods */
	//! Deafult constructor.
	/*!
	 * \param[in] parent handle to the parent widget
	 */
	QPitch( QMainWindow* parent = 0 );

	//! Deafult destructor.
	~QPitch( );


public slots:
	//! Update the displayed value of the estimated note.
	/*!
	 * \param[in] estimatedNote target frequency of the note identified
	 */
	void setEstimatedNote( double estimatedNote );

	//! Update the displayed value of the estimated frequency.
	/*!
	 * \param[in] estimatedFrequency the value of the signal frequency estimated as the maximum of the autocorrelation
	 */
	void setEstimatedFrequency( double estimatedFrequency );

	//! Enable the update of the estimated and detected frequencies.
	/*!
	 * \param[in] enabled the status of GUI
	 */
	void setUpdateEnabled( bool enabled );


protected: /* methods */
	//! Function called when the main window is closed.
	/*!
	 * \param[in] event details of the close event
	 */
	virtual void closeEvent( QCloseEvent* event );

	//! Function called to handle generic events.
	/*!
	 * \param[in] watched the object that has emitted the event
	 * \param[in] event details of the event to be processed
	 */
	virtual bool eventFilter( QObject* watched, QEvent* event );


private: /* static constants */
	// ** BUFFER SIZE ** //
	static const int	PLOT_BUFFER_SIZE;				//!< Size of the buffers used for visualization


private: /* members */
	// ** Qt WIDGETS ** //
	Ui::QPitch		_gt;							//!< Mainwindow created with Qt-Designer
	QPitchCore*		_hQPitchCore;					//!< Handle to the working thread

	// ** STATUS BAR ITEMS ** //
	QLabel				_sb_labelDeviceInfo;			//!< Label with the device information

	// ** UPDATE TIMERS ** //
	QTimer*				_hRepaintTimer;					//!< Support timer to trigger the repaint of children
	bool				_lineEditEnabled;				//!< Flag to disable the update of the frequency estimation
	bool				_compactModeActivated;			//!< Flag to request a widget resize to the compact mode

	// ** PITCH ESTIMATION ** //
	double				_estimatedFrequency;			//!< Estimated frequency for the input signal
	double				_estimatedNote;					//!< Estimated note closest to the estimated frequency

private slots:
	//! Open a dialog to configure the application settings.
	void showPreferencesDialog( );

	//! Open the about dialog.
	void showAboutDialog( );

	//! Update the application settings.
	/*!
	 * \param[in] sampleFrequency requested sample frequency
	 * \param[in] fftFrameSize requested size of the buffer used to compute the FFT
	 * \param[in] fundamentalFrequency requested fundamental frequency of the note A4
	 * \param[in] tuningNotation requested tuning notation
	 */
	void setApplicationSettings( unsigned int sampleFrequency, unsigned int fftFrameSize,
		double fundamentalFrequency, unsigned int tuningNotation );

	//! Set the compactmode for the application hiding the oscilloscope widget.
	/*!
	 * \param[in] enabled flag controlling the visualization of the oscilloscope widget
	 */
	void setViewCompactMode( bool enabled );

	//! Update all the elements in the GUI.
	void updateQPitchGui( );
};

#endif /* __QPITCH_H_ */
