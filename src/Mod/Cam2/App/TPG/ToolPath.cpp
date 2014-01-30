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

#include <PreCompiled.h>
#ifndef _PreComp_
#include <cstring>
#endif

#include "ToolPath.h"
#include "Area.h"


namespace Cam {


ToolPath::ToolPath(TPG* source) {
    this->source = source;
    this->toolpath = new QStringList();
    refcnt = 1;
	required_decimal_places = 3;	// TODO: Implement this properly.  Assume metric for now.
}


ToolPath::ToolPath(const std::vector<std::string>& commands) {
    source = NULL;
    toolpath = new QStringList();
    refcnt = 1;
    required_decimal_places = 3;    // TODO: Implement this properly.  Assume metric for now.

    std::vector<std::string>::const_iterator it;
    for (it = commands.begin(); it != commands.end(); ++it) {
        toolpath->append(QString::fromStdString(*it));
    }
}

ToolPath::~ToolPath() {
    if (this->toolpath != NULL)
        delete this->toolpath;
}

/**
 * Add a single toolpath command to the ToolPath
 */
void ToolPath::addToolPath(QString tp) {
    if (this->toolpath == NULL)
        this->toolpath = new QStringList();
    this->toolpath->push_back(tp);
}

/**
 * Clear out the toolpath.
 */
void ToolPath::clear() {
    if (this->toolpath != NULL)
        this->toolpath->clear();
    else
        this->toolpath = new QStringList();
}

/**
 * Get the TPG that created this toolpath
 */
TPG *ToolPath::getSource() {
    return this->source;
}

/**
 * Get the Toolpath as strings
 */
QStringList *ToolPath::getToolPath() {
    return this->toolpath;
}





/**
	When a string is passed into a Python routine, it needs to be surrounded by
	single (or double) quote characters.  If the contents of the string contain
	backslash characters then these may be interpreted by Python as part of
	a special character representation.  eg: '\n' represents a single newline
	character.  If we pass a Windows path in then the backslash characters that
	separate directory names may be interpreted rather than be taken literaly.
	This routine also adds the single quotes to the beginning and end of the
	string passed in.

	eg: if value = "abcde" then the returned string would be 'abcde'
	    if value = "c:\temp\myfile.txt" then the returned string would be 'c:\\temp\\myfile.txt'
		if value = "abc'de" then the returned string would be 'abc\'de'.
 */
QString ToolPath::PythonString( const QString value ) const
{
	QString _value(value);
	QString result;

	if (_value.startsWith(QString::fromAscii("\'")) && (_value.endsWith(QString::fromAscii("\'"))))
	{
		_value.remove(0, 1);
		_value.remove(_value.length()-1, 1);
	}

	if (_value.startsWith(QString::fromAscii("\"")) && (_value.endsWith(QString::fromAscii("\""))))
	{
		_value.remove(0, 1);
		_value.remove(_value.length()-1, 1);
	}

	_value.replace(QString::fromAscii("\\"), QString::fromAscii("\\\\") );
	_value.replace(QString::fromAscii("\'"), QString::fromAscii("\\\'") );
	_value.replace(QString::fromAscii("\""), QString::fromAscii("\\\"") );
	_value.replace(QString::fromAscii("("), QString::fromAscii("{") );
	_value.replace(QString::fromAscii(")"), QString::fromAscii("}") );

	result = QString::fromAscii("\'") + _value + QString::fromAscii("\'");
	return(result);
}

QString ToolPath::PythonString( const char *value ) const
{
	return(PythonString(QString::fromUtf8(value)));
}


/**
	Round the floating point number based on the number of decimal
	places currently configured and return the QString representation
	of that.
 */
QString ToolPath::PythonString( const double value ) const
{
	double rounded_value = Round(value,RequiredDecimalPlaces());
	int field_width = 0;
	char format = 'g';
	int precision = Precision(rounded_value);

	return(QString(QString::fromAscii("%1")).arg(rounded_value, field_width, format, precision));
}


/**
	Round to the nearest floating point number for the number of decimal places specified.
 */
double ToolPath::Round(double number,int place) const
{
	/*place = decimal point. putting in 0 will make it round to whole number. putting in 1 will round to the tenths digit.*/

	double sign = (number < 0)?-1:+1;
	number *= pow(double(10.0),place);
	int istack = 0;
	if (sign < 0.0)
	{
		istack = (int)ceil(number);
	}
	else
	{
		istack = (int)floor(number);
	}

	double out = fabs(number-istack);

	if (sign < 0.0)
	{
		// Negative number.
		if (out < 0.5)
		{
			number = ceil(number);
			number /= pow(double(10.0),place);
			return number;
		}
		if (out > 0.4) 
		{
			number = floor(number);
			number /= pow(double(10.0),place);
			return number;
		}
	}
	else
	{
		// Positive number.
		if (out < 0.5)
		{
			number = floor(number);
			number /= pow(double(10.0),place);
			return number;
		}
		if (out > 0.4) 
		{
			number = ceil(number);
			number /= pow(double(10.0),place);
			return number;
		}
	}
	return(number);
}

/**
	How many characters required for both the digits left of the decimal point, the decimal
	point, the +/- sign character and those digits required after the decimal point.
	It's used for rounding floating point numbers during the conversion to string format.
 */
unsigned int ToolPath::Precision( const double value ) const
{
    unsigned int lhs = 0;
    unsigned int one_for_plus_minus_sign = 1;

    lhs = 0;
    double temp = fabs(value);
    while (temp >= 1.0)
    {
        lhs++;
        temp /= 10.0;
    }

    return( lhs + RequiredDecimalPlaces() + one_for_plus_minus_sign );
}





ToolPath & ToolPath::operator<<( const double value )
{
	this->line_buffer.append(PythonString(value));
	return(*this);
}

ToolPath & ToolPath::operator<< ( const ToolPath & value )
{
	if (this != &value)
	{
		if (this->toolpath == NULL)
			this->toolpath = new QStringList();

		if (value.toolpath)
		{
			std::copy( value.toolpath->begin(), value.toolpath->end(), std::inserter( *(this->toolpath), this->toolpath->begin() ) );
		}
	}

	return(*this);
}


ToolPath & ToolPath::operator<< ( const QString value )
{
	this->line_buffer.append(value);
	if (this->line_buffer.endsWith(QString::fromAscii("\n")))
	{
		if (this->toolpath == NULL)
			this->toolpath = new QStringList();

		this->line_buffer.remove(this->line_buffer.size()-1, 1);	// Remove newline character.
		this->toolpath->push_back(this->line_buffer);
		this->line_buffer.clear();
	}
	return(*this);
}

ToolPath & ToolPath::operator<< ( const char *value )
{
	return( (*this) << QString::fromAscii(value) );
}


ToolPath & ToolPath::operator<< ( const int value )
{
	this->line_buffer.append(QString::fromAscii("%1").arg(value));
	return(*this);
}


/* friend */ QString operator<< ( QString & buf, const ToolPath & tool_path )
{
	if (tool_path.toolpath)
	{
		for (QStringList::const_iterator itString = tool_path.toolpath->constBegin(); itString != tool_path.toolpath->constEnd(); itString++)
		{
			buf.append(*itString);
			buf.append(QString::fromAscii("\n"));
		}
	}
	return(buf);
}


const unsigned int ToolPath::RequiredDecimalPlaces() const
{
	// Look for this as one of the TPG settings.  If it's not
	// found then look for the units used by this TPG.  Finally, just
	// assume metric and use 3+1 decimal places.
	// TODO - implement this functionality.

	return(required_decimal_places);
}

/**
	If the operator changes units from one setting to another, we need
	to change the number of decimal places used both for the generation of
	the Python/GCode and within the libArea module.  The LinuxCNC machine
	controller (and I assume others are similar) will look at arc definitions
	to see if the distance from the start to the centre is the same as
	the distance from the centre to the end of the arc.  If these values differ
	then an error is generated.  We need to ensure that the values we generate
	are correct for this.  If the GCode is in metric mode (G21) then
	the arc's distances must be accurate to the third decimal place.  If it's
	in imperial mode (G20) then it must be correct to the fourth decimal place.
	These accuracies are implicit in LinuxCNC.

	It's also important to itterate through input graphics at this resolution
	so we don't end up generating lines and arcs that are smaller than this
	resolution.  i.e. their start and end locations are effectively zero
	when viewed with these implicit accuracy by the CNC controlling software.

	Finally, it's important that we set the accuracy values used in the libArea
	module.  These accuracy values are used when deciding if one point matches
	another.  When the libArea code performs an 'offset' function, it converts
	all the input graphics into line segments.  i.e. it interpolates line segments
	from the arc definitions.  It then performs the offset using those.  It then
	attempts to re-interpret the resultant points as arcs.  This is an important
	step as a single arc can be used to accurately represent quite a large number
	of small line segments.  The accuracy value set by this method affects whether
	these small line segments will correctly be interpreted as arcs by the
	libArea code.

	We expect the 'value' specified here to be 3 for metric and 4 for imperial
	based on the implicit accuracies used by LinuxCNC.  If anyone knows of a better
	way to define the number of decimal places based on the units setting then
	we should use it.
 */
void ToolPath::RequiredDecimalPlaces(const unsigned int value)
{
	this->required_decimal_places = value;
	area::CArea::m_accuracy = 1.0 / pow(10.0, double(value));
	area::Point::tolerance = 1.0 / pow(10.0, double(value));
}


/**
	If the input graphics exists in the cam::Paths object then it can be used
	to generate an area::CArea object.  This method accepts an area::CArea object
	(i.e. the graphical representation within the libArea library) and generates
	Python definition that may be read by the libArea python module.
 */
void ToolPath::addAreaDefinition( const area::CArea graphics, const char *python_object_name )
{
	*this << python_object_name << " = area.Area()\n";

	for (std::list<area::CCurve>::const_iterator itCurve = graphics.m_curves.begin(); itCurve != graphics.m_curves.end(); itCurve++)
	{
		std::ostringstream l_ossCurveName;
		l_ossCurveName << python_object_name << "_curve_" << std::distance(graphics.m_curves.begin(), itCurve);

		addCurveDefinition( *itCurve, l_ossCurveName.str().c_str() );
		*this << python_object_name << ".append(" << l_ossCurveName.str().c_str() << ")\n";
	} // End for

	// Call the Reorder() method to make sure internal and external curves are oriented correctly
	// for the libArea library code.
	*this << python_object_name << ".Reorder()\n";
}


void ToolPath::addCurveDefinition( const area::CCurve curve, const char *python_object_name )
{
	*this << python_object_name << " = area.Curve()\n";

	std::list<area::CVertex>::const_iterator itPrevious;
	for (std::list<area::CVertex>::const_iterator itVertex = curve.m_vertices.begin(); itVertex != curve.m_vertices.end(); itVertex++)
	{
		switch (itVertex->m_type)
		{
		case 0:	// line (or start point)
			*this << python_object_name << ".append(area.Point(" << itVertex->m_p.x << "," << itVertex->m_p.y << "))\n";
			break;

		case -1:	// CW arc
			*this << python_object_name << ".append(area.Vertex(-1, area.Point(" << itVertex->m_p.x << "," << itVertex->m_p.y << "), area.Point(" << itVertex->m_c.x << "," << itVertex->m_c.y << ")))\n";
			break;

		case 1:	// CCW arc
			*this << python_object_name << ".append(area.Vertex(1, area.Point(" << itVertex->m_p.x << "," << itVertex->m_p.y << "), area.Point(" << itVertex->m_c.x << "," << itVertex->m_c.y << ")))\n";
			break;
		}
		
		itPrevious = itVertex;
	} // End for
}




#ifdef FC_DEBUG
	ToolPath::Test::Test(ToolPath * tool_path)
	{
		pToolPath = tool_path;
		if (pToolPath)
		{
			pToolPath->grab();	// we want our own copy
		}
	}

	ToolPath::Test::~Test()
	{
		if (pToolPath)
		{
			pToolPath->release();	// Fly - be free!
		}
	}

	bool ToolPath::Test::Run()
	{
		if (! Rounding()) return(false);
		if (! StringHandling()) return(false);

		return(true);	// All tests passed.
	}

	bool ToolPath::Test::Rounding()
	{
		if (pToolPath == NULL) return(false);

		if (pToolPath->Round( 1.23456789, 3 ) != 1.235) return(false);
		if (pToolPath->Round( 1.2355, 3 ) != 1.236) return(false);
		if (pToolPath->Round( 1.2354, 3 ) != 1.235) return(false);

		if (pToolPath->Round( -1.2355, 3 ) != -1.236) return(false);
		if (pToolPath->Round( -1.2354, 3 ) != -1.235) return(false);

		pToolPath->RequiredDecimalPlaces(3);
		if (pToolPath->PythonString( 1.2355 ) != QString::fromAscii("1.236")) return(false);
		if (pToolPath->PythonString( 1.2354 ) != QString::fromAscii("1.235")) return(false);

		if (pToolPath->PythonString( -1.2355 ) != QString::fromAscii("-1.236")) return(false);
		if (pToolPath->PythonString( -1.2354 ) != QString::fromAscii("-1.235")) return(false);

		return(true);
	}

	bool ToolPath::Test::StringHandling()
	{
		if (pToolPath == NULL) return(false);

		{
			QString lhs(pToolPath->PythonString(QString::fromUtf8("'Something (that) needs cleaning up befor it's included in a python comment() call'")));
			QString rhs(QString::fromUtf8("'Something {that} needs cleaning up befor it\\'s included in a python comment{} call'"));
			if (lhs != rhs) return(false);
		}

		{
			QString lhs(pToolPath->PythonString(QString::fromUtf8("the cat's mat has a single quote in it")));
			QString rhs(QString::fromUtf8("'the cat\\'s mat has a single quote in it'"));
			if (lhs != rhs) return(false);
		}

		{
			QString lhs(pToolPath->PythonString(QString::fromUtf8("this one has (something in brackets) within it.")));
			QString rhs(QString::fromUtf8("'this one has {something in brackets} within it.'"));
			if (lhs != rhs) return(false);
		}

		{
			QString lhs(pToolPath->PythonString(QString::fromUtf8("this one already has single quotes bounding it")));
			QString rhs(QString::fromUtf8("'this one already has single quotes bounding it'"));
			if (lhs != rhs) return(false);
		}

		{
			QString lhs(pToolPath->PythonString(QString::fromUtf8("\"this one already has double quotes bounding it\"")));
			QString rhs(QString::fromUtf8("'this one already has double quotes bounding it'"));
			if (lhs != rhs) return(false);
		}

		return(true);	// All passed
	}
#endif // FC_DEBUG


} /* namespace Cam */
