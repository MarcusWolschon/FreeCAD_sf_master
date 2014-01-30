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

#ifndef CAM_GCODE_H
#define CAM_GCODE_H

#include <App/PropertyStandard.h>
#include <PreCompiled.h>

// NOTE: These BOOST SPIRIT DEBUG macros MUST be defined BEFORE the include
// files.  Otherwise the default values, defined within the include files,
// will be used instead.

#ifndef BOOST_SPIRIT_DEBUG
	#define BOOST_SPIRIT_DEBUG
	#define BOOST_SPIRIT_DEBUG_OUT GCode_GrammarDebugOutputBuffer
#endif

#include <Mod/Cam2/App/MachineProgram.h>
#include <Mod/Part/App/PartFeature.h>
#include <TopoDS_Edge.hxx>

extern std::ostringstream CamExport GCode_GrammarDebugOutputBuffer;

///////////////////////////////////////////////////////////////////////////////
/// Base class
namespace Cam
{

class CamExport GCode
{
public:
	typedef enum {
		csUndefined = -1,
		csG53 = 0,
		csG54,
		csG55,
		csG56,
		csG57,
		csG58,
		csG59,
		csG59_1,
		csG59_2,
		csG59_3,
		csG92,
		csG92_1
	} eCoordinateSystems_t;

	typedef enum {
		eXYPlane = 0,
		eXZPlane,
		eYZPlane
	} ePlane_t;

	typedef enum {
		eMetric = 0,
		eImperial
	} eUnits_t;

public:
    GCode(MachineProgram *machine_program, TPGFeature* rpgFeature);
    ~GCode();

public:
	/**
		Define a class that contains the full set of values for the
		machine's current location.
	 */
	class MachineLocation
	{
	public:
		MachineLocation()
		{
			memset( this, 0, sizeof(*this) );
		}

	public:
		void X(const double value) { x=value; }
		double X() const { return(x); }

		void Y(const double value) { y=value; }
		double Y() const { return(y); }

		void Z(const double value) { z=value; }
		double Z() const { return(z); }

		void A(const double value) { a=value; }
		double A() const { return(a); }

		void B(const double value) { b=value; }
		double B() const { return(b); }

		void C(const double value) { c=value; }
		double C() const { return(c); }

		void U(const double value) { u=value; }
		double U() const { return(u); }

		void V(const double value) { v=value; }
		double V() const { return(v); }

		void W(const double value) { w=value; }
		double W() const { return(w); }

	private:
		double x,y,z, a,b,c, u,v,w;
	}; // End MachineLocation class

public:
	/**
		This is the main 'engine' for the GCode class.  This method parses the MachineProgram
		and produces the 'geometry' as output.  If the machine_location is NULL then we must
		not know where the machine is currently located.  At the end, however, the machine_location
		should indicate the last known machine location.  If this is the second parsing call then
		the location of the machine from the previous execution should be planted into this object's
		machine_location pointer.
	 */
	virtual bool Parse() = 0;

	// These hold warnings found during parsing.  eg: Do we have a G01 feed movement without ever having seen a feedrate defined.
	void AddWarning(const QString warning);

	// These are initialized from the ToolPath object that's associated with the MachineProgram object passed into the constructor.
	double Tolerance() const { return(tolerance); }
	int RequiredDecimalPlaces() const { return(required_decimal_places); }

public:
	MachineProgram *machine_program;
	QStringList	warnings;
	double tolerance;
	int required_decimal_places;
	TPGFeature* tpgFeature;	// The owning object into which we place our results.
	std::auto_ptr<MachineLocation> machine_location;	// Current machine location.  NULL indicates that we don't know (i.e. an initial movement)

	// Define a type representing the index into the QStringList contained within the MachineProgram object.
	// We will assign specific graphics to these for later reference.
	typedef QStringList::size_type MachineProgramIndex_t;

	/**
		Define a class that contains a single graphical element representing
		tool movements defined by the GCode contained in the MachineProgram object.
		We will end up with a list of such graphical elements.
	 */
	class CamExport ToolMovement
	{
	public:
		typedef enum
		{
			eRapid,
			eFeed,
			eArcClockwise,
			eArcCounterClockwise
		} eMovement_t;

	public:
		ToolMovement(MachineProgram *machine_program);
		~ToolMovement();
		ToolMovement( const ToolMovement & rhs );
		ToolMovement & operator= ( const ToolMovement & rhs );

		void Index(const MachineProgramIndex_t index) { machine_program_index = index; }
		MachineProgramIndex_t Index() const { return(machine_program_index); }

		void PartFeature( Part::Feature *part_feature );
		Part::Feature *PartFeature() const { return(this->part_feature); }

		void Edge( TopoDS_Edge edge );
		TopoDS_Edge Edge() { return(this->edge); }

		void CoordinateSystem( const eCoordinateSystems_t value ) { this->coordinate_system = value; }
		eCoordinateSystems_t CoordinateSystem() const { return(this->coordinate_system); }

		void Type( const eMovement_t type ) { this->type = type; }
		eMovement_t Type() const { return(this->type); }

	private:
		MachineProgram *machine_program;
		MachineProgramIndex_t machine_program_index;
		TopoDS_Edge	edge;
		Part::Feature *part_feature;	// graphical element - line, arc etc.
		eCoordinateSystems_t coordinate_system;
		eMovement_t type;
	};

	typedef std::vector<ToolMovement> SingleCommandGeometry_t;
	typedef std::map< MachineProgramIndex_t, SingleCommandGeometry_t > Geometry_t;

	Geometry_t	geometry;

}; // End GCode class definition.


} //namespace Cam
#endif //CAM_GCODE_H
