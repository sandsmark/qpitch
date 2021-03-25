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

//! Pitch detection algorithm and note scale visualization.
/*!
 * This class implements a simple pitch detection algorithm to
 * identify the note corresponding to the frequency estimated as
 * the first peak of the autocorrelation function.
 * A linear note scale is displayed using the chosen musical
 * notation. A moving cursor gives a rough indication of the
 * detected note. The identified note is highlighted and
 * when the pitch deviation is smaller than 2.5% a red square
 * is drawn around the note label.
 */

#ifndef __QLOGVIEW_H_
#define __QLOGVIEW_H_

#include <QWidget>

class QLogView : public QWidget {
	Q_OBJECT


public: /* enumerations */
	//! Enumeration of the available tuning scales
	enum TuningNotation {
		NOTATION_US,
		NOTATION_FRENCH,
		NOTATION_GERMAN
	};


public: /* methods */
	//! Default constructor.
	/*!
	 * \param[in] parent handle to the parent widget
	 */
	QLogView( QWidget* parent = 0 );

	//! Set the parameters of the current pitch detection algorithm.
	/*!
	 * \param[in] fundamentalFrequency fundamental frequency of the note A4
	 * \param[in] tuningNotation tuning notation used to select the string to display
	 */
	void setTuningParameters( const double fundamentalFrequency, const TuningNotation tuningNotation );

	//! Get the parameters of the current pitch detection algorithm.
	/*!
	 * \param[out] fundamentalFrequency fundamental frequency of the note A4
	 * \param[out] tuningNotation tuning notation used to select the string to display
	 */
	void getTuningParameters( double& fundamentalFrequency, TuningNotation& tuningNotation ) const;


public slots:
	//! Set the estimated value of the frequency of the input signal.
	/*!
	 * \param[in] estimatedFrequency estimated frequency used to detect the current pitch
	 */
	void setEstimatedFrequency( double estimatedFrequency );

	//! Enable the visualization of the cursor on the note scale.
	/*!
	 * \param[in] enabled the status of the cursor
	 */
	void setPlotEnabled( bool enabled );


signals:
	//! Update the frequency of the estimated note.
	/*!
	 * \param[in] estimatedNote target frequency of the pitch identified
	 */
	void updateEstimatedNote( double estimatedNote );


protected: /* methods */
	//! Function called to handle a repaint request.
	/*!
	 * \param[in] event the details of the repaint event
	 */
	virtual void paintEvent( QPaintEvent* event );

	//! Function called to handle a resize request.
	/*!
	 * \param[in] event the details of the resize event
	 */
	virtual void resizeEvent( QResizeEvent* event );


private: /* static constants */
	// ** WIDGETS SIZES ** //
	static const double	SIDE_MARGIN;					//!< Percent width of the horizontal border
	static const int	BAR_HEIGHT;						//!< Half the height of the tuning bar
	static const int	MINOR_TICK_HEIGHT;				//!< Height of the minor ticks
	static const int	MIDDLE_TICK_HEIGHT;				//!< Height of the middle ticks
	static const int	MAJOR_TICK_HEIGHT;				//!< Height of the major ticks
	static const int	LABEL_OFFSET;					//!< Distance of the labels from the tuning bar
	static const int	CARET_BORDER;					//!< Space between the label and the rounded rectangle displayed when the error is below 2.5 percent
	static const int	CURSOR_WIDTH;					//!< Width of the cursor displayed in the tuning bar
	static const double	ACCEPTED_DEVIATION;				//!< Pitch deviation where the cursor becomes a rectangle


private: /* members */
	// ** TUNING NOTATIONS ** //
	static const QString NoteLabel[6][12];				//!< Labels of the note in different tuning scales

	// ** PITCH DETECTION PARAMETERS ** //
	static const double	D_NOTE;							//!< Ratio of two consecutive notes for pitch detection
	static const double	D_NOTE_LOG;						//!< Ratio of two consecutive notes for visualization
	double				_noteFrequency[12];				//!< Frequencies of the notes in the reference octave used for pitch detection
	double				_noteScale[12];					//!< Scale of the notes in the reference octave used for visualization
	double				_fundamentalFrequency;			//!< Fundamental frequency used as a reference to build the pitch scale
	TuningNotation		_tuningNotation;				//!< Musical notation used to select the string displayed
	int					_currentPitch;					//!< Frequency of the closest note in the scale
	double				_currentPitchDeviation;			//!< percentuale da -0.5 a +0.5 che dice di quanto va disegnato spostato

	// ** REPAINT FLAG **//
	bool				_drawBackground;				//!< Redraw everything when true, otherwise redraw only the note scale
	bool				_drawForeground;				//!< Draw the note cursor when true, otherwise draw nothing
	QPixmap*			_pixmap;						//!< Pixmap used to store the background to reduce the load
};

#endif /* __QLOGVIEW_H_ */
