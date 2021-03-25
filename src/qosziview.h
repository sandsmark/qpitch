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

//! Signal visualization in the time domain.
/*!
 * This class implements an oscilloscope-like widget for the
 * visualization of signals in the time domain.
 * The input signal acquired from the microphone or from the
 * line-in input is plotted in the upper axis. The x-axis has
 * a fixed range of 50 milliseconds, while the y-axis has a
 * variable range (not indicated) which starts from 2048 and
 * goes to 32768.
 * The autocorrelation of the input signal instead is plotted
 * in the lower axis. The x-axis has a (somehow) logarithmic
 * scale ranging from 40 Hz to 1000 Hz. The peak of the
 * autocorrelation used to detect the frequency of the input
 * signal is indicated by a red line.
 */

#ifndef __QOSZIVIEW_H_
#define __QOSZIVIEW_H_

#include <QWidget>

class QOsziView : public QWidget {
	Q_OBJECT


public: /* methods */
	//! Deafult constructor.
	/*!
	 * \param[in] parent handle to the parent widget
	 */
	QOsziView( QWidget* parent = 0 );

	//! Default destructor.
	~QOsziView( );

	//! Set the size of the buffer used for visualization.
	/*!
	 * \param[in] plotBuffer_size size of the buffer used for visualization
	 */
	void setBufferSize( const unsigned int plotBuffer_size );


public slots:
	//! Update the graph of the input signal displayed in the oscilloscope.
	/*!
	 * \param[in] plotSample the array with the new samples of the input signal to display
	 * \param[in] timeRangeSample time range used to place the tick marks on the signal axis
	 */
	void setPlotSamples( const double* plotSample, double timeRangeSample );

	//! Update the graph of the autocorrelation displayed in the oscilloscope.
	/*!
	 * \param[in] plotAutoCorr the array with the new samples of the autocorrelation to display
	 * \param[in] estimatedFrequency frequency of the input signal estimated as the first autocorrelation peak
	 */
	void setPlotAutoCorr( const double* plotAutoCorr, double estimatedFrequency );

	//! Enable the plot area, and display a blank axis box if disabled.
	/*!
	 * \param[in] enabled flag with the activation status of the plot area
	 */
	void setPlotEnabled( bool enabled );


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
	static const double	SIDE_MARGIN;					//!< Percent width of the horizontal margin of the plot area
	static const double	TOP_MARGIN;						//!< Percent height of the vertical margin of the plot area
	static const double AXIS_HALF_HEIGHT;				//!< Percent height of half the plot area (used to simplify coding)
	static const int	MINOR_TICK_HEIGHT;				//!< Pixel height of the minor ticks
	static const int	MIDDLE_TICK_HEIGHT;				//!< Pixel height of the minor ticks
	static const int	MAJOR_TICK_HEIGHT;				//!< Pixel height of the major ticks
	static const int	LABEL_SPACING;					//!< Pixel distance of the labels from the axis


private: /* members */
	// ** VISUALIZATION BUFFERS ** //
	double*				_plotSample;					//!< Buffer used to store time samples for visualization
	double*				_plotAutoCorr;					//!< Buffer used to store autocorrelation samples for visualization
	unsigned int		_plotBuffer_size;				//!< Size of the buffer used for visualization

	// ** REPAINT FLAG **//
	bool				_drawBackground;				//!< Redraw everything when true, otherwise redraw only the note scale
	bool				_drawForeground;				//!< Draw the note cursor when true, otherwise draw nothing
	QPixmap				_pixmap;						//!< Pixmap used to store the background to reduce the load

	// ** PLOT PARAMETERS ** //
	double				_timeRangeSample;				//!< Time range of the signal axis
	double				_estimatedFrequency;			//!< Value of the estimated frequency


private: /* methods */
	//! Draws an axis box with a linear scale with a fixed range [0, 50] ms.
	/*!
	 * \param[in] painter reference to the painter object used to draw on screen
	 * \param[in] plotArea_width width of the plot area
	 * \param[in] plotArea_height half the height of the plot area (used instead of the height to simplify coding)
	 */
	void drawLinearAxis( QPainter& painter, const int plotArea_width, const int plotArea_height );

	//! Draw an axis box with a reversed logarithmic scale with a fixed range [1000, 40] Hz.
	/*!
	 * \param[in] painter reference to the painter object used to draw on screen
	 * \param[in] plotArea_width width of the plot area
	 * \param[in] plotArea_height half the height of the plot area (used instead of the height to simplify coding)
	 */
	void drawReversedLogAxis( QPainter& painter, const int plotArea_width, const int plotArea_height );

	//! Draws an axis box with an optional x-axis.
	/*!
	 * \param[in] painter reference to the painter object used to draw on screen
	 * \param[in] plotArea_width width of the plot area
	 * \param[in] plotArea_height half the height of the plot area (used instead of the height to simplify coding)
	 */
	void drawAxisBox( QPainter& painter, const int plotArea_width, const int plotArea_height );

	//! Draws an axis box with an optional x-axis and ticks in linear or logarithmic scale
	/*!
	 * \param[in] painter reference to the painter object used to draw on screen
	 * \param[in] plotData buffer with the data to draw
	 * \param[in] plotData_size size of the data buffer
	 * \param[in] plotArea_width width of the plot area
	 * \param[in] plotArea_height half the height of the plot area (used instead of the height to simplify coding)
	 * \param[in] color the color used to draw the data
	 * \param[in] autoScaleThreshold threshold under which autoscale is disabled (0 to always enable autoscale)
	 */
	void drawCurve( QPainter& painter, const double* plotData, const unsigned int plotData_size,
		const int plotArea_width, const int plotArea_height, const QColor& color,
		const double autoScaleThreshold );
};

#endif /* __QOSZIVIEW_H_ */
