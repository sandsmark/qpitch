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

//! About dialog to show application details
/*!
 * This class implements a simple dialog to display a brief
 * description of the application and some information about
 * its authors.
 */

#ifndef __QABOUTDLG_H_
#define __QABOUTDLG_H_

#include "ui_qaboutdlg.h"


class QAboutDlg : public QDialog {
	Q_OBJECT

public:
	//! Deafult constructor.
	/*!
	 * \param[in] parent handle to the parent widget
	 */
	QAboutDlg( QWidget* parent = 0 );

private:
	// ** Qt WIDGETS ** //
	Ui::QAboutDlg		_ab;							//!< Dialog created with Qt-Designer
};

#endif /* __QABOUTDLG_H_ */

