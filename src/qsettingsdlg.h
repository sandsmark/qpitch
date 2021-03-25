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

#ifndef __QAUDIOSETTINGS_H_
#define __QAUDIOSETTINGS_H_

#include "ui_qsettingsdlg.h"

#include "qlogview.h"


//! Structure holding the application settings
struct QPitchParameters {
	unsigned int				sampleFrequency;		//!< Current sample rate
	unsigned int				fftFrameSize;			//!< Current size of the buffer used to compute the FFT
	double						fundamentalFrequency;	//!< The reference frequency of A4 used to estimate the pitch
	QLogView::TuningNotation	tuningNotation;			//!< Current tuning notation
};


//! Settings dialog to configure the application
/*!
 * This class implements a dialog to configure the parameters of
 * the audio stream and the parameters of the pitch detection
 * algorithm.
 * The configuration of the audio stream includes the selection
 * of the sample frequency and of the size of the frame used to
 * compute the FFT.
 * The configuration of the pitch detection algorithm includes
 * the selection of the fundamental frequency (A4 = 440Hz as the
 * default) used to build the note scale and the selection of the
 * tuning notation (US, French and German notation).
 */

class QSettingsDlg : public QDialog {
	Q_OBJECT


public: /* members */
	//! Deafult constructor.
    /*!
     * \param[in] qPitchParameters structure with the current audio stream and tuning parameters
	 * \param[in] parent handle to the parent widget
	 */
	QSettingsDlg( const QPitchParameters& qPitchParameters, QWidget* parent = 0 );


public slots:
	//! Restore default application settings.
	void restoreDefaultSettings( );

	//! Accept the application settings in the dialog.
	void acceptSettings( );


signals:
	//! Request an update in the application settings.
	/*!
	 * \param[in] sampleFrequency requested sample frequency
	 * \param[in] fftFrameSize requested size of the buffer used to compute the FFT
	 * \param[in] fundamentalFrequency requested fundamental frequency of the note A4
	 * \param[in] tuningNotation requested tuning notation
	 */
	void updateApplicationSettings( unsigned int sampleFrequency, unsigned int fftFrameSize,
		double fundamentalFrequency, unsigned int tuningNotation );


private: /* members */
	// ** Qt WIDGETS ** //
	Ui::QSettingsDlg	_sd;							//!< Dialog created with Qt-Designer
};

#endif /* __QSETTINGSDLG_H_ */
