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


#include "qlogview.h"

#include <cmath>

#include <QPainter>
#include <QPainterPath>

// ** MUSICAL NOTATIONS ** //
const QString QLogView::NoteLabel[6][12] = {
	{  "A",  QString("A%1").arg(QChar(0x266F)),  "B",  "C",  QString("C%1").arg(QChar(0x266F)),  "D",  QString("D%1").arg(QChar(0x266F)),  "E",  "F",   QString("F%1").arg(QChar(0x266F)),   "G",   QString("G%1").arg(QChar(0x266F)) },	/* US  */
	{  "A",  QString("B%1").arg(QChar(0x266D)),  "B",  "C",  QString("D%1").arg(QChar(0x266D)),  "D",  QString("E%1").arg(QChar(0x266D)),  "E",  "F",   QString("G%1").arg(QChar(0x266D)),   "G",   QString("A%1").arg(QChar(0x266D)) },	/* US alternate */
	{ "La", QString("La%1").arg(QChar(0x266F)), "Si", "Do", QString("Do%1").arg(QChar(0x266F)), "Re", QString("Re%1").arg(QChar(0x266F)), "Mi", "Fa",  QString("Fa%1").arg(QChar(0x266F)), "Sol", QString("Sol%1").arg(QChar(0x266F)) },	/* French */
	{ "La", QString("Si%1").arg(QChar(0x266D)), "Si", "Do", QString("Re%1").arg(QChar(0x266D)), "Re", QString("Mi%1").arg(QChar(0x266D)), "Mi", "Fa", QString("Sol%1").arg(QChar(0x266D)), "Sol",  QString("La%1").arg(QChar(0x266D)) },	/* French alternate */
	{  "A",                                "B",  "H",  "C",  QString("C%1").arg(QChar(0x266F)),  "D",  QString("D%1").arg(QChar(0x266F)),  "E",  "F",   QString("F%1").arg(QChar(0x266F)),   "G",   QString("G%1").arg(QChar(0x266F)) },	/* German */
	{  "A",                                "B",  "H",  "C",  QString("D%1").arg(QChar(0x266D)),  "D",  QString("E%1").arg(QChar(0x266D)),  "E",  "F",   QString("G%1").arg(QChar(0x266D)),   "G",   QString("A%1").arg(QChar(0x266D)) }		/* German alternate */
};

// ** NOTE RATIOS ** //
const double	QLogView::D_NOTE				= pow( 2.0, 1.0/12.0 );
const double	QLogView::D_NOTE_LOG			= log10( pow( 2.0, 1.0/12.0 ) ) / log10( 2.0 );

// ** WIDGET SIZES ** //
const double	QLogView::SIDE_MARGIN			= 0.02;
const int		QLogView::BAR_HEIGHT			= 8;
const int		QLogView::MINOR_TICK_HEIGHT		= 3;
const int		QLogView::MIDDLE_TICK_HEIGHT	= 5;
const int		QLogView::MAJOR_TICK_HEIGHT		= 7;
const int		QLogView::LABEL_OFFSET			= 12;
const int		QLogView::CARET_BORDER			= 5;
const int		QLogView::CURSOR_WIDTH			= 6;
const double	QLogView::ACCEPTED_DEVIATION	= 0.025;


QLogView::QLogView( QWidget* parent ) : QWidget( parent )
{
	// ** INITIALIZE PRIVATE VARIABLES ** //
	_tuningNotation			= NOTATION_US;
	_fundamentalFrequency	= 440.0;

	// redraw everything the first time and disable cursor
	_drawBackground			= true;
	_drawForeground			= false;
	_currentPitch			= -1;

	// initialize the pixmap
	_pixmap					= new QPixmap( );
}


void QLogView::setTuningParameters( const double fundamentalFrequency, const TuningNotation tuningNotation )
{
	// ** CHECK LIMITS AND UPDATE PARAMETERS ** //
	if ( tuningNotation <= NOTATION_GERMAN ) {
		_tuningNotation = (TuningNotation) tuningNotation;
	}

	if ( (_fundamentalFrequency >= 400.0) && (_fundamentalFrequency <= 480.0) ) {
		_fundamentalFrequency = fundamentalFrequency;
	}

	// ** UPDATE PITCH DETECTION CONSTANTS ** //
	for ( unsigned int k = 0 ; k < 12 ; ++k ) {
		_noteFrequency[k]	= _fundamentalFrequency * pow( D_NOTE, (int) k );						// set frequencies for pitch detection
		_noteScale[k]		= log10( _fundamentalFrequency ) / log10( 2.0 ) + (k * D_NOTE_LOG);		// set frequencies for visualization
	}

	// ** UPDATE THE GUI ** //
	_drawBackground = true;
	update( );
}


void QLogView::getTuningParameters( double& fundamentalFrequency, TuningNotation& tuningNotation ) const
{
	fundamentalFrequency	= _fundamentalFrequency;
	tuningNotation			= _tuningNotation;
}


void QLogView::setEstimatedFrequency( double estimatedFrequency )
{
	// ** TRY TO ENSURE THAT THE ARRAY IS PROPERLY INITIALIZED ** //
	Q_ASSERT( _noteFrequency[0] != 0 );
	Q_ASSERT( _noteFrequency[11] != 0 );

	// process only notes within the range [40, 2000] Hz
	if ( (estimatedFrequency >= 40.0) && (estimatedFrequency <= 2000.0) ) {
		// ** ESTIMATE THE NEW PITCH ** //
		// compute the deviation of the estimated frequency with respect to the reference octave
		int octaveDeviation = 0;

		// rescale the estimatedFrequency to bring it inside the reference octave (4th octave)
		while ( estimatedFrequency > (_noteFrequency[11] + _fundamentalFrequency * D_NOTE_LOG / 2.0 ) ) {
			// higher frequency, higher octave
			estimatedFrequency /= 2.0;
			octaveDeviation++;
		}

		while ( estimatedFrequency < (_noteFrequency[0] - _fundamentalFrequency * D_NOTE_LOG / 2.0 ) ) {
			// lower frequency, lower octave
			estimatedFrequency *= 2.0;
			octaveDeviation--;
		}

		/*
		 * here estimatedFrequency is in the range ( _noteFrequency[0], _noteFrequency[11] )
		 * so it is possible to find the pitch in the scale as the one with the minimum
		 * distance in the LINEAR scale (logarithm of the frequencies)
		 */

		double			minPitchDeviation		= _fundamentalFrequency * D_NOTE_LOG / 2.0;
		unsigned int	minPitchDeviation_index	= 0;
		double			log_estimatedFrequency	= log10(estimatedFrequency) / log10( 2.0 );

		for ( unsigned int k = 0 ; k < 12 ; ++k ) {
			if ( fabs( log_estimatedFrequency - _noteScale[k] ) < minPitchDeviation ) {
				minPitchDeviation		= fabs( log_estimatedFrequency - _noteScale[k] );
				minPitchDeviation_index	= k;
			}
		}

		// save the details of the pitch deviation for the visualization
		_currentPitch 			= minPitchDeviation_index;
		_currentPitchDeviation	= ( log_estimatedFrequency - _noteScale[minPitchDeviation_index] ) / D_NOTE_LOG;

		// ** BROADCAST PITCH ESTIMATION ** //
		emit updateEstimatedNote( _noteFrequency[minPitchDeviation_index] * pow( 2.0, octaveDeviation ) );
	} else {
		// disable current selection
		_currentPitch = -1;
	}
}


void QLogView::setPlotEnabled( bool enabled )
{
	// ** SET THE ACTIVAITON STATUS ** //
	_drawForeground = enabled;
}


void QLogView::paintEvent( QPaintEvent* /* event */ )
{
	// ** ENSURE THAT THE PIXMAP IS VALID ** //
	Q_ASSERT( _pixmap != NULL );

	// ** INITIALIZE PAINTER ** //
	QPainter	painter;
	int			scaleWidth = (int)(width( ) * (1.0 - 2 * SIDE_MARGIN));

	/*
	 * redraw the background only when the widget is resized or when
	 * the application is hidden and then shown, using a pixmap to
	 * store the background and to redraw it the next time
	 */

	// ** DRAW THE OFFSCREEN BUFFER ** //
	if ( _drawBackground == true ) {
		// do not redraw background next time
		_drawBackground	= false;

		// create a new pixmap with the right size
		delete	_pixmap;
		_pixmap	= new QPixmap( size( ) );

#ifdef Q_WS_X11
		// do not use a trasparent background on X11 to avoid a **big** performance hit (on my machine...)
		_pixmap->fill( palette( ).window( ).color( ) );
#else
		_pixmap->fill( Qt::transparent );
#endif

		// setup the painter
		painter.begin( _pixmap );
		painter.translate( QPoint( (int)(width( ) * SIDE_MARGIN), height( ) / 2 ) );

		// plot axis frame
		painter.setPen( QPen( palette( ).dark( ), 0, Qt::SolidLine ) );
		painter.drawRect( 0, -BAR_HEIGHT, scaleWidth, 2 * BAR_HEIGHT );
		painter.fillRect( 1, -BAR_HEIGHT + 1, scaleWidth - 1, 2 * BAR_HEIGHT - 1, palette( ).light( ) );

		// plot ticks
		int xTick;
		int xTick_height;

		for ( unsigned int k = 0 ; k <= 120 ; ++k ) {
			xTick = (unsigned int) ( scaleWidth / 120.0 * k );

			if ( ((k+5) % 10) == 0 ) {
				xTick_height = MAJOR_TICK_HEIGHT;
			} else if ( ((k+5) % 5) == 0 ) {
				xTick_height = MIDDLE_TICK_HEIGHT;
			} else {
				xTick_height = MINOR_TICK_HEIGHT;
			}

			painter.drawLine( xTick, -BAR_HEIGHT, xTick, -BAR_HEIGHT - xTick_height );	// upper tick
			painter.drawLine( xTick,  BAR_HEIGHT, xTick,  BAR_HEIGHT + xTick_height );	// lower tick
		}

		// increase the font size
		QFont font = painter.font( );
		font.setPointSize( font.pointSize( ) + 2 );
		painter.setFont( font );

		// plot labels
		painter.setPen( QPen( palette( ).text( ), 0, Qt::SolidLine ) );
		for ( unsigned int k = 0 ; k < 12 ; ++k ) {
			xTick = (int) ( scaleWidth / 24.0 + scaleWidth / 12.0 * k );
			// label above the bar
			painter.drawText( xTick - (painter.fontMetrics( ).horizontalAdvance( NoteLabel[2 * _tuningNotation][k] ) / 2 + 1),
				- BAR_HEIGHT - painter.fontMetrics( ).descent( ) - LABEL_OFFSET,
				NoteLabel[2 * _tuningNotation][k] );
			// label below the bar
			painter.drawText( xTick - (painter.fontMetrics( ).horizontalAdvance( NoteLabel[2 * _tuningNotation + 1][k] ) / 2 + 1),
				BAR_HEIGHT + painter.fontMetrics( ).ascent( ) + LABEL_OFFSET,
				NoteLabel[2 * _tuningNotation + 1][k] );
		}
		painter.end( );
	}

	// ** DISPLAY THE OFFSCREEN BUFFER ** //
	painter.begin( this );
	painter.drawPixmap( 0, 0, *_pixmap );

	if ( (_drawForeground == true) && (_currentPitch >= 0) ) {
		// ** DRAW THE CURSOR IF REQUIRED ** //
		// draw labels
		painter.translate( QPoint( (int)(width( ) * SIDE_MARGIN), height( ) / 2 ) );

		// increase the font size
		QFont font = painter.font( );
		font.setPointSize( font.pointSize( ) + 2 );
		painter.setFont( font );

		int xTick = (int) ( scaleWidth / 24.0 + scaleWidth / 12.0 * _currentPitch );

		if ( fabs( _currentPitchDeviation ) < ACCEPTED_DEVIATION ) {
			// draw a square around the note when the error pitch is less than 2.5 percent
			painter.setRenderHint( QPainter::Antialiasing, true );
			painter.setPen( QPen( Qt::red, 0, Qt::SolidLine ) );
			painter.drawRoundedRect( QRectF ( xTick - painter.fontMetrics( ).horizontalAdvance( NoteLabel[2 * _tuningNotation][_currentPitch] ) / 2.0 - CARET_BORDER,
				-BAR_HEIGHT - painter.fontMetrics( ).ascent( ) - LABEL_OFFSET - CARET_BORDER,
				painter.fontMetrics( ).horizontalAdvance( NoteLabel[2 * _tuningNotation][_currentPitch] ) + 2 * CARET_BORDER,
				2 * ( BAR_HEIGHT + painter.fontMetrics( ).ascent( ) + LABEL_OFFSET + CARET_BORDER) ), 15, 15, Qt::RelativeSize );
			painter.setRenderHint( QPainter::Antialiasing, false );
		}

		// highlight the current note
		painter.setPen( QPen( Qt::red, 0, Qt::SolidLine ) );

		// label above the bar
		painter.drawText( xTick - (painter.fontMetrics( ).horizontalAdvance( NoteLabel[2 * _tuningNotation][_currentPitch] ) / 2 + 1),
			-BAR_HEIGHT - painter.fontMetrics( ).descent( ) - LABEL_OFFSET,
			NoteLabel[2 * _tuningNotation][_currentPitch] );
		// label below the bar
		painter.drawText( xTick - (painter.fontMetrics( ).horizontalAdvance( NoteLabel[2 * _tuningNotation + 1][_currentPitch] ) / 2 + 1),
			BAR_HEIGHT + painter.fontMetrics( ).ascent( ) + LABEL_OFFSET,
			NoteLabel[2 * _tuningNotation + 1][_currentPitch] );

		// draw the cursor
		painter.setPen( QPen( palette( ).text( ), 0, Qt::SolidLine ) );
		int xCursor = (int) ( scaleWidth / 24.0 + scaleWidth / 12.0 * ( _currentPitch + _currentPitchDeviation ) );
		QColor cursorColor( (int)(0xAA + 0x55 * (1.0 - 2.0 * fabs(_currentPitchDeviation))), 0x00, 0x00 );

		if ( _currentPitchDeviation < -ACCEPTED_DEVIATION ) {
			// draw a right arrow if the pitch is lower than the reference
			painter.setRenderHint( QPainter::Antialiasing, true );

			QPolygon rightArrow;
			rightArrow << QPoint( xCursor - CURSOR_WIDTH / 2, -BAR_HEIGHT + 1)
				<< QPoint( xCursor - CURSOR_WIDTH / 2, BAR_HEIGHT)
				<< QPoint( xCursor + CURSOR_WIDTH / 2, 0)
				<< QPoint( xCursor - CURSOR_WIDTH / 2, -BAR_HEIGHT + 1);

			QPainterPath path;
			path.addPolygon( rightArrow );

			painter.fillPath( path, cursorColor );
			painter.drawPath( path );

			painter.setRenderHint( QPainter::Antialiasing, false );
		} else if ( _currentPitchDeviation > ACCEPTED_DEVIATION ) {
			// draw a left arrow if the pitch is lower than the reference
			painter.setRenderHint( QPainter::Antialiasing, true );

			QPolygon rightArrow;
			rightArrow << QPoint( xCursor + CURSOR_WIDTH / 2, -BAR_HEIGHT + 1)
				<< QPoint( xCursor + CURSOR_WIDTH / 2, BAR_HEIGHT)
				<< QPoint( xCursor - CURSOR_WIDTH / 2, 0)
				<< QPoint( xCursor + CURSOR_WIDTH / 2, -BAR_HEIGHT + 1);

			QPainterPath path;
			path.addPolygon( rightArrow );

			painter.fillPath( path, cursorColor );
			painter.drawPath( path );

			painter.setRenderHint( QPainter::Antialiasing, false );
		} else {
			// draw a rectangular cursor
			painter.fillRect( xCursor - CURSOR_WIDTH / 2, -BAR_HEIGHT + 1, CURSOR_WIDTH, 2 * BAR_HEIGHT - 1, cursorColor );
			painter.drawRect( xCursor - CURSOR_WIDTH / 2, -BAR_HEIGHT + 1, CURSOR_WIDTH, 2 * BAR_HEIGHT - 2 );
			painter.drawLine( xCursor, -BAR_HEIGHT + 1, xCursor, BAR_HEIGHT - 1 );
		}
	}
}


void QLogView::resizeEvent( QResizeEvent* /* event */ )
{
	// ** REQUEST A BACKGROUND REPAINT ** //
	_drawBackground = true;
}
