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

#include "qsettingsdlg.h"


QSettingsDlg::QSettingsDlg( const QPitchParameters& qPitchParameters, QWidget* parent ) : QDialog( parent )
{
	// ** SETUP THE MAIN WINDOW ** //
	_sd.setupUi( this );

	// ** SETUP CONNECTIONS ** //
	connect( _sd.buttonBox, SIGNAL( accepted() ),
		this, SLOT( acceptSettings() ) );
	connect( (QObject*) _sd.buttonBox->button( QDialogButtonBox::RestoreDefaults ), SIGNAL( pressed() ),
		this, SLOT( restoreDefaultSettings() ) );

	// ** INITIALIZE WIDGETS ** //
	_sd.comboBox_sampleFrequency->setCurrentIndex( _sd.comboBox_sampleFrequency->findText( QString::number( qPitchParameters.sampleFrequency ) ) );
	_sd.comboBox_frameSize->setCurrentIndex( _sd.comboBox_frameSize->findText( QString::number( qPitchParameters.fftFrameSize ) ) );
	_sd.doubleSpinBox_fundamentalFrequency->setValue( qPitchParameters.fundamentalFrequency );

	switch( qPitchParameters.tuningNotation ) {
		default:
		case QLogView::NOTATION_US:
			_sd.radioButton_scaleUs->setChecked( true );
			break;

		case QLogView::NOTATION_FRENCH:
			_sd.radioButton_scaleFrench->setChecked( true );
			break;

		case QLogView::NOTATION_GERMAN:
			_sd.radioButton_scaleGerman->setChecked( true );
			break;

	}
}


void QSettingsDlg::acceptSettings( )
{
	// ** UPDATE THE APPLICATION SETTINGS ** //
	QLogView::TuningNotation tuningNotation = QLogView::NOTATION_US;

	if ( _sd.radioButton_scaleUs->isChecked( ) ) {
		tuningNotation = QLogView::NOTATION_US;
	} else if ( _sd.radioButton_scaleFrench->isChecked( ) ) {
		tuningNotation = QLogView::NOTATION_FRENCH;
	} else if ( _sd.radioButton_scaleGerman->isChecked( ) ) {
		tuningNotation = QLogView::NOTATION_GERMAN;
	}

	emit updateApplicationSettings( _sd.comboBox_sampleFrequency->currentText( ).toUInt( ), _sd.comboBox_frameSize->currentText( ).toUInt( ),
		_sd.doubleSpinBox_fundamentalFrequency->value( ), (const unsigned int) tuningNotation );
}


void QSettingsDlg::restoreDefaultSettings( )
{
	// ** RESTORE THE PROPERTIES OF THE AUDIO STREAM TO THE INITIAL VALUE ** //
	_sd.comboBox_sampleFrequency->setCurrentIndex( 0 );				// 44100 Hz
	_sd.comboBox_frameSize->setCurrentIndex( 1 );					// 4096 samples
	_sd.doubleSpinBox_fundamentalFrequency->setValue( 440.0 );		// A4 = 440 Hz for standard pitch
	_sd.radioButton_scaleUs->setChecked( true );					// US notation
}
