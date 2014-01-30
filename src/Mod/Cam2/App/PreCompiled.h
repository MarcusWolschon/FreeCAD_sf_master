/***************************************************************************
 *   Copyright (c) 2012 Luke Parry    (l.parry@warwick.ac.uk)              *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef __PRECOMPILED__
#define __PRECOMPILED__

#include <FCConfig.h>

#include <QLibrary>
#include <QtCore>	// For the Q_DECL_IMPORT/EXPORT macros.


#ifdef FC_OS_WIN32
	#ifdef FCAppCam
		// We're in the 'Cam' application module so we want to export the various classes.
		// Exporting of App classes
		# define CamExport    Q_DECL_EXPORT
		# define PartExport   Q_DECL_IMPORT
		# define MeshExport   Q_DECL_IMPORT
	#else
		// We're inside either a plugin or the GUI modules.  We want to import the CAM classes instead.
		# define CamExport    Q_DECL_IMPORT
		# define PartExport   Q_DECL_IMPORT
		# define MeshExport   Q_DECL_IMPORT
	#endif // FCAppCam
#else // for Linux
	# define CamExport
	# define PartExport
	# define MeshExport
#endif



#ifdef _PreComp_

// standard
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <assert.h>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <bitset>

#include <Mod/Part/App/OpenCascadeAll.h>
#include <Python.h>

#elif defined(FC_OS_WIN32)
#include <windows.h>
#endif // _PreComp_
#endif

