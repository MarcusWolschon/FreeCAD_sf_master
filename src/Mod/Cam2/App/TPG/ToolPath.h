/***************************************************************************
 *   Copyright (c) 2012 Andrew Robinson <andrewjrobinson@gmail.com>        *
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

#ifndef TOOLPATH_H_
#define TOOLPATH_H_

#include <PreCompiled.h>

#include <QStringList>
#include <QString>

#include <Area.h>

namespace Cam {
class CamExport ToolPath;
}

#include "TPG.h"

namespace Cam {

/**
 * Stores the Tool Path output from a single TPG.
 *
 * A ToolPath, in this context, is really a Python program that, when executed, will produce
 * a GCode file.  The PostProcessor layer is responsible for executing this Python program.  This
 * allows a set of Python scripts to implement the various methods called by this Python
 * program.  eg: rapid(x=1.0, y=2.0, z=3.0)
 *
 * One instance of the rapid() method might produce "G00X1.0Y2.0Z3.0" while another
 * might produce "N1000 G00 X 1.0 Y 2.0 Z 3.0".  In any case, the implementation of the rapid()
 * method can be done in a way that suits an individual CNC controller.
 * The class inheritance mechanism available within Python allows the easy replacement of
 * methods such as rapid() if a particular CNC controller requires the GCode presented in
 * a particular manner.
 *
 * The 'toolpath' member is a QStringList.  Each string in this list represents a single
 * row of Python code.
 */
class CamExport ToolPath {

protected:
    TPG *source;
	QString line_buffer;		// an accumulation of multiple operator<< calls for a single line until we find a newline character at the end.
	unsigned int required_decimal_places;	// need 3 for metric and 4 for imperial so that arc definitions are valid.

    int refcnt;
    virtual ~ToolPath();

public:
    QStringList *toolpath;
    ToolPath(TPG* source);
    ToolPath(const std::vector<std::string> & commands);

    /**
     * Add a single toolpath command to the ToolPath
     */
    void addToolPath(QString tp);

	/**
	 * Add the python construction version of the libArea graphics.
	 */
	void addAreaDefinition( const area::CArea graphics, const char *python_area_object_name );
	void addCurveDefinition( const area::CCurve graphics, const char *python_curve_object_name );

    /**
     * Clear out the toolpath.
     */
    void clear();

    /**
     * Get the TPG that created this toolpath
     */
    TPG *getSource();

    /**
     * Get the Toolpath as strings
     */
    QStringList *getToolPath();

    /**
     * Increases reference count
     * Note: it returns a pointer to this for convenience.
     */
    ToolPath *grab() {
        refcnt++;
        return this;
    }

    /**
     * Decreases reference count and deletes self if no other references
     */
    void release() {
        refcnt--;
        if (refcnt == 0) {
            qDebug("Freeing: %p\n", this);
            delete this;
            delete this;
        }
    }

public:
	QString PythonString( const QString value ) const;
	QString PythonString( const char * value ) const;
	QString PythonString( const double value ) const;

	// The operator<< methods all allow the accumulation of parts of a single line
	// of Python script.  Each one will look to see if the accumulated line ends
	// with a newline character ('\n').  If it does then the line will be
	// added to the list of lines (this->toolpath) and the buffer (this->line_buffer)
	// will be cleared ready to start accumulating the next line.  They're just
	// utility methods to help make the construction of the Python program
	// a little easier to read.

	ToolPath & operator<< ( const ToolPath & value );
	ToolPath & operator<< ( const double value );
	ToolPath & operator<< ( const float value );
	ToolPath & operator<< ( const QString value );
	ToolPath & operator<< ( const char *value );
	ToolPath & operator<< ( const int value );

	const unsigned int RequiredDecimalPlaces() const;
	void RequiredDecimalPlaces(const unsigned int value);

private:
	double Round(double number,int place) const;
	unsigned int Precision( const double value ) const;

public:
	friend QString CamExport operator<< ( QString & buf, const ToolPath & tool_path );

public:
	#ifdef FC_DEBUG
		/**
			Define a class that contains some basic unit testing for the ToolPath
			class.
		 */
		class CamExport Test
		{
		public:
			Test(ToolPath * tool_path);
			~Test();

			bool Run();
			bool Rounding();
			bool StringHandling();

		private:
			ToolPath *pToolPath;
		}; // End TestToolPath class definition.
	#endif // FC_DEBUG
};


} /* namespace Cam */
#endif /* TOOLPATH_H_ */
