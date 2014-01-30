/***************************************************************************
 *   Copyright (c) 2012 Andrew Robinson  (andrewjrobinson@gmail.com)       *
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

#ifndef CAM_SUPPORT_H_
#define CAM_SUPPORT_H_
#include <QString>
#include <Python.h>

#ifdef WIN32
	#include <windows.h>	// For the Sleep() prototype
#endif // WIN32

/**
 * Convert a QString to a Python Object
 */
PyObject *QStringToPythonUC(const QString &str);

/**
 * Convert a Python Object (string/unicode) to a QString
 */
QString PythonUCToQString(PyObject *obj);

/**
 * Convert a QString to a const char *
 */
inline const char* ts(QString str)
{
  return str.toAscii().constData();
}
inline const char* ts(QString *str)
{
  if (str != NULL)
    return str->toAscii().constData();
  return "NULL";
}

inline void sleepSec(int seconds)
{
#ifdef WIN32
	Sleep(seconds * 1000);
#else
	sleep(seconds);
#endif
}

#endif /* SUPPORT_H_ */
