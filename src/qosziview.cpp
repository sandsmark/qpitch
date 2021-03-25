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

#include "qosziview.h"

#include <cmath>

#include <QPainter>
#include <iostream>

// ** CUSTOM CONSTANTS ** //
const double	QOsziView::SIDE_MARGIN			= 0.01;
const double	QOsziView::TOP_MARGIN			= 0.055;
const double 	QOsziView::AXIS_HALF_HEIGHT		= 0.195;
const int		QOsziView::MINOR_TICK_HEIGHT	= 3;
const int		QOsziView::MIDDLE_TICK_HEIGHT	= 5;
const int		QOsziView::MAJOR_TICK_HEIGHT	= 7;
const int		QOsziView::LABEL_SPACING		= 3;


QOsziView::QOsziView( QWidget* parent ) : QWidget( parent )
{
	// ** SETUP PRIVATE VARIABLES ** //
	// redraw everything the first time and disable signals
	_drawBackground			= true;
	_drawForeground			= false;
	_timeRangeSample		= 1.0;					// dummy values to avoid division by 0

	//** INITIALIZE BUFFERS ** //
	_plotBuffer_size		= 0;					// empty buffers
	_plotSample				= NULL;
	_plotAutoCorr			= NULL;
}


QOsziView::~QOsziView()
{
	// ** RELEASE RESOURCES ** //
	delete[] _plotSample;
	delete[] _plotAutoCorr;
}


void QOsziView::setBufferSize( const unsigned int plotBuffer_size )
{
	// ** DELETE OLD BUFFERS ** //
	delete[] _plotSample;
	delete[] _plotAutoCorr;

	//** INITIALIZE BUFFERS ** //
	_plotBuffer_size	= plotBuffer_size;
	_plotSample			= new double[_plotBuffer_size];
	_plotAutoCorr		= new double[_plotBuffer_size];
	_estimatedFrequency	= 0.0;

	// ** CLEAR BUFFERS ** //
	memset( _plotSample, 0, sizeof( double ) * _plotBuffer_size );
	memset( _plotAutoCorr, 0, sizeof( double ) * _plotBuffer_size );
}


void QOsziView::setPlotEnabled( bool enabled )
{
	// ** SET THE ACTIVAITON STATUS ** //
	_drawForeground = enabled;
}


void QOsziView::setPlotSamples( const double* plotSample, double timeRangeSample )
{
	// ** ENSURE THAT THE BUFFERS ARE VALID ** //
	Q_ASSERT( plotSample		!= NULL );
	Q_ASSERT( _plotSample		!= NULL );
	Q_ASSERT( _plotBuffer_size	!= 0 );

	// ** STORE SIGNAL ** //
	memcpy( _plotSample, plotSample, _plotBuffer_size * sizeof( double ) );
	_timeRangeSample = timeRangeSample;
}


void QOsziView::setPlotAutoCorr( const double* plotAutoCorr, double estimatedFrequency )
{
	// ** ENSURE THAT THE BUFFERS ARE VALID ** //
	Q_ASSERT( plotAutoCorr		!= NULL );
	Q_ASSERT( _plotAutoCorr		!= NULL );
	Q_ASSERT( _plotBuffer_size	!= 0 );

	// ** STORE AUTOCORRELATION ** //
	memcpy( _plotAutoCorr, plotAutoCorr, _plotBuffer_size * sizeof( double ) );
	_estimatedFrequency	= estimatedFrequency;
}


void QOsziView::paintEvent( QPaintEvent* /* event */ )
{
	// ** ENSURE THAT THE BUFFERS ARE VALID ** //
	Q_ASSERT( _plotSample		!= NULL );
	Q_ASSERT( _plotAutoCorr		!= NULL );
	Q_ASSERT( _plotBuffer_size	!= 0 );

	// ** INITIALIZE PAINTER ** //
	QPainter painter;

	// ** COMPUTE AXIS SIZE ** //
	int plotArea_width		= (int)(width( ) * (1.0 - 2 * SIDE_MARGIN));
	int plotArea_sideMargin	= (int)(width( ) * SIDE_MARGIN);
	int plotArea_height		= (int)(height( ) * AXIS_HALF_HEIGHT);
	int plotArea_topMargin	= (int)(height( ) * TOP_MARGIN);

	// ** DRAW THE OFFSCREEN BUFFER ** //
	if ( _drawBackground == true ) {
		// do not redraw background next time
		_drawBackground	= false;

		// create a new pixmap with the right size
		_pixmap	= QPixmap( size( ) );
#ifdef Q_WS_X11
		// do not use a trasparent background on X11 to avoid a **big** performance hit (on my machine...)
		_pixmap.fill( palette( ).window( ).color( ) );
#else
		_pixmap.fill( Qt::transparent );
#endif

		// setup the painter
		painter.begin( &_pixmap );

		// ** UPPER AXIS ** //
		painter.translate( plotArea_sideMargin, plotArea_topMargin + plotArea_height );
		drawLinearAxis( painter, plotArea_width, plotArea_height );

		// draw title
		painter.setPen( QPen( palette( ).text( ), 0, Qt::SolidLine ) );
		painter.drawText( plotArea_width / 2 - painter.fontMetrics( ).width( QString("Audio signal [ms]") ) / 2 + 1,
			- plotArea_height - painter.fontMetrics( ).descent( ) - LABEL_SPACING,
			QString("Audio signal [ms]") );

		// ** LOWER AXIS ** //
		// move the painter (the horizontal translation is fine)
		painter.translate( 0, 2 * (plotArea_topMargin + plotArea_height) );
		drawReversedLogAxis( painter, plotArea_width, plotArea_height );

		// draw title
		painter.setPen( QPen( palette( ).text( ), 0, Qt::SolidLine ) );
		painter.drawText( plotArea_width / 2 - painter.fontMetrics( ).width( QString("Autocorrelation [Hz]") ) / 2 + 1,
			- plotArea_height - painter.fontMetrics( ).descent( ) - LABEL_SPACING,
			QString("Autocorrelation [Hz]") );

		painter.end( );
	}

	// ** DISPLAY THE OFFSCREEN BUFFER ** //
	painter.begin( this );
	painter.drawPixmap( 0, 0, _pixmap );

	if ( _drawForeground == true ) {
		// ** UPPER AXIS ** //
		painter.translate( plotArea_sideMargin, plotArea_topMargin + plotArea_height );
		drawCurve( painter, _plotSample, _plotBuffer_size, plotArea_width, plotArea_height, Qt::darkGreen, 2048.0 );

		// ** LOWER AXIS ** //
		painter.translate( 0, 2 * (plotArea_topMargin + plotArea_height) );
		drawCurve( painter, _plotAutoCorr, _plotBuffer_size, plotArea_width, plotArea_height, Qt::darkBlue, 0 );

		// draw cursor
		if ( (_estimatedFrequency >= 40.0) && (_estimatedFrequency <= 2000.0) ) {
			painter.setPen( QPen( Qt::red, 0, Qt::SolidLine ) );
			painter.setRenderHint( QPainter::Antialiasing, true );
			painter.drawLine( QPointF( 40.0 / _estimatedFrequency * plotArea_width, -plotArea_height ),
				QPointF( 40.0 / _estimatedFrequency * plotArea_width, plotArea_height - 1 ) );
			painter.setRenderHint( QPainter::Antialiasing, false );
		}
	}
}


void QOsziView::resizeEvent( QResizeEvent* /* event */ )
{
	// ** REQUEST A BACKGROUND REPAINT ** //
	_drawBackground = true;
}


void QOsziView::drawLinearAxis( QPainter& painter, const int plotArea_width, const int plotArea_height )
{
	// axis range
	const double xAxisRange = 50.0;

	// plot axis
	drawAxisBox( painter, plotArea_width, plotArea_height );

	// plot ticks
	painter.setPen( QPen( palette( ).dark( ), 0, Qt::SolidLine ) );

	for ( unsigned int k = 1 ; k < 50 ; ++k ) {
		int xTick = (int)(k * 0.02 * plotArea_width);
		if ( (k % 10) == 0 ) {
			// plot major ticks
			painter.drawLine( xTick, plotArea_height, xTick, plotArea_height - MAJOR_TICK_HEIGHT );
		} else if ( (k % 5) == 0 ) {
			// plot middle ticks
			painter.drawLine( xTick, plotArea_height, xTick, plotArea_height - MIDDLE_TICK_HEIGHT );
		} else {
			// plot minor ticks
			painter.drawLine( xTick, plotArea_height, xTick, plotArea_height - MINOR_TICK_HEIGHT );
		}
	}

	// plot labels
	painter.save( );
	QFont font = painter.font( );
	font.setPointSize( font.pointSize( ) - 2 );
	painter.setFont( font );
	painter.setPen( QPen( palette( ).text( ), 0, Qt::SolidLine ) );
	for ( unsigned int k = 0 ; k <= 10 ; ++k ) {
		painter.drawText( (unsigned int)(k * 0.1 * plotArea_width) - painter.fontMetrics( ).width( QString("%1").arg( (unsigned int)(k * 0.1 * xAxisRange) ) ) / 2 + 1,
			plotArea_height + painter.fontMetrics( ).ascent( ) + LABEL_SPACING,
			QString("%1").arg( (unsigned int)(k * 0.1 * xAxisRange) ) );
	}
	painter.restore( );
}


void QOsziView::drawReversedLogAxis( QPainter& painter, const int plotArea_width, const int plotArea_height )
{
	// plot axis
	drawAxisBox( painter, plotArea_width, plotArea_height );

	// plot ticks
	painter.setPen( QPen( palette( ).dark( ), 0, Qt::SolidLine ) );

	int		xTick;
	double	freq = 10.0;

	for ( unsigned int k = 5 ; k <= 20 ; ++k ) {
		if ( (k % 10) == 0 ) {
			// move to next decade
			freq *= 10;
			xTick = (int)( 40.0 * (double) plotArea_width / freq );
		} else {
			xTick = (int)( 40.0 * (double) plotArea_width / ((k % 10) * freq) );
		}

		if ( (k % 10) == 0 ) {
			// plot major ticks
			painter.drawLine( xTick, plotArea_height, xTick, plotArea_height - MAJOR_TICK_HEIGHT );

			painter.save( );
			painter.setPen( QPen( palette( ).text( ), 0, Qt::SolidLine ) );
			QFont font = painter.font( );
			font.setPointSize( font.pointSize( ) - 2 );
			painter.setFont( font );
			painter.drawText( xTick - painter.fontMetrics( ).width( QString("%1").arg( (unsigned int) freq ) ) / 2 + 1,
				plotArea_height + painter.fontMetrics( ).ascent( ) + LABEL_SPACING,
				QString("%1").arg( ( (unsigned int) freq ) ) );
			painter.restore( );
		} else if ( (k % 10) == 5 ) {
			// plot middle ticks
			painter.drawLine( xTick, plotArea_height, xTick, plotArea_height - MIDDLE_TICK_HEIGHT );

			painter.save( );
			painter.setPen( QPen( palette( ).text( ), 0, Qt::SolidLine ) );
			QFont font = painter.font( );
			font.setPointSize( font.pointSize( ) - 2 );
			painter.setFont( font );
			painter.drawText( xTick - painter.fontMetrics( ).width( QString("%1").arg( (unsigned int)((k % 10) * freq) ) ) / 2 + 1,
				plotArea_height + painter.fontMetrics( ).ascent( ) + LABEL_SPACING,
				QString("%1").arg( ( (unsigned int)((k % 10) * freq) ) ) );
			painter.restore( );
		} else {
			// plot minor ticks
			painter.drawLine( xTick, plotArea_height, xTick, plotArea_height - MINOR_TICK_HEIGHT );
		}
	}

	// plot labels
	painter.setPen( QPen( palette( ).text( ), 0, Qt::SolidLine ) );
	freq = 10.0;
	for ( unsigned int k = 4 ; k <= 20 ; k+=5 ) {
		if ( (k % 10) == 0 ) {
			// move to next decade
			freq *= 10;
			xTick = (int)( 40.0 * (double) plotArea_width / freq );
		} else {
			xTick = (int)( 40.0 * (double) plotArea_width / ((k % 10) * freq) );
		}


	}
}


void QOsziView::drawAxisBox( QPainter& painter, const int plotArea_width, const int plotArea_height )
{
	// disable antialiasing and plot a light rectangle
	painter.fillRect( 0, -plotArea_height, plotArea_width, 2 * plotArea_height, palette( ).light( ) );

	// plot the x-axis
	painter.setPen( QPen( palette( ).dark( ), 0, Qt::DashLine ) );
	painter.drawLine( 0, 0, plotArea_width - 1, 0 );

	// plot axis box (slightly bigger than required so it is possible to clean only the inside of the box)
	painter.setPen( QPen( palette( ).dark( ), 0, Qt::SolidLine ) );
	painter.drawRect( 0, -plotArea_height - 1, plotArea_width - 1, 2 * plotArea_height + 1);
}


void QOsziView::drawCurve( QPainter& painter, const double* plotData, const unsigned int plotData_size,
	const int plotArea_width, const int plotArea_height, const QColor& color,
	const double autoScaleThreshold )
{
	// ** ENSURE THAT THE DATA ARE VALID ** //
 	Q_ASSERT( plotData != NULL );
 	Q_ASSERT( plotData_size != 0 );

	// enable antialiasing and plot signal samples
	painter.setPen( QPen( color, 0, Qt::SolidLine ) );

	// find min and max of the signal in order to autoscale the signal up to 16 times
	double minValue = plotData[0];
	double maxValue = plotData[0];

	for ( unsigned int k = 1 ; k < plotData_size ; ++k ) {
		if ( plotData[k] < minValue ) {
			minValue = plotData[k];
		}
		if ( plotData[k] > maxValue ) {
			maxValue = plotData[k];
		}
	}

	// find the actual limit value and disable plot if it is below a given threshold
	double limitValue	= (fabs(maxValue) > fabs(minValue)) ? fabs(maxValue) : fabs(minValue);

	// y-axis is upside-down so use a negative scale factor to mirror the plot
	double scaleFactor	= -(0.95 * plotArea_height) / ((limitValue > autoScaleThreshold) ? limitValue : autoScaleThreshold );

	double xIntervalStep = (double) plotArea_width / (double) plotData_size;
	for ( unsigned int k = 1 ; k < plotData_size ; ++k ) {
		painter.drawLine( QPointF( (k - 1) * xIntervalStep, plotData[k - 1] * scaleFactor ),
			QPointF( k * xIntervalStep, plotData[k] * scaleFactor) );
	}
}
