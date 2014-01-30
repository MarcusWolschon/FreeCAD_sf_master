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

#include "PreCompiled.h"
#ifndef _PreComp_
#endif

#include <Base/Interpreter.h>		// For Python runtime
#include <App/PropertyUnits.h>
#include <TPG/CppTPGDescriptor.h>
#include <Mod/Part/App/PrimitiveFeature.h>

#include <App/Document.h>
#include <App/DocumentObject.h>
#include <App/Application.h>

#include "CppExampleTPG.h"
#include "Graphics/Paths.h"
#include <Mod/Cam2/App/GCode.h>

#include <QRegExp>

#define myID   "95744f1e-360f-11e2-bcd3-08002734b94f"
#define myName "Example CPP TPG"
#define myDesc "A simple example CPP TPG to demonstrating how to create one. "


#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_increment_actor.hpp>

using namespace BOOST_SPIRIT_CLASSIC_NS;


/**
 * Implement the Cpp shared library interface functions
 * Note: this must be outside the namespace declaration
 */
//CPPTPG_API_SOURCE(Cam::CppExampleTPG, myID, myName, myDesc)

//TODO: put this back once the memory corruption in getTPG() is located.
extern "C" Q_DECL_EXPORT Cam::TPGDescriptorCollection* getDescriptors() {
    Cam::TPGDescriptorCollection* descriptors = new Cam::TPGDescriptorCollection();
    Cam::TPGDescriptor *descriptor = new Cam::CppTPGDescriptor(QString::fromAscii("95744f1e-360f-11e2-bcd3-08002734b94f"), QString::fromAscii("Example CPP TPG"), QString::fromAscii("A simple example CPP TPG to demonstrating how to create one. "));
    descriptors->add(descriptor);
    descriptor->release();
    return descriptors;
}
extern "C" Q_DECL_EXPORT Cam::CppTPG* getTPG(QString id) {
    if (id.compare(QString::fromAscii(myID)) == 0)
        return new Cam::CppExampleTPG();
    return 0;
}

/// TPG Implementation ///
namespace Cam {

/* friend */ QString & operator<< ( QString & buf, const CppExampleTPG::RetractMode_t & retract_mode )
{
	switch (retract_mode)
	{
	case CppExampleTPG::eRapidRetract:
		buf += QString::fromAscii("Rapid");
		return(buf);

	case CppExampleTPG::eFeedRetract:
		buf += QString::fromAscii("At Feed Rate");
		return(buf);
	} // End switch

	return(buf);
}


/**
	Itterate through the various RetractMode_t values and use the operator<<(QString) method
	to generate the text string for each value in turn.  That way we have a single place in
	the code that associates the RetractMode_t enumerated value to the QString representation.
 */
CppExampleTPG::RetractMode_t CppExampleTPG::toRetractMode( const QString string_representation ) const
{
	for (RetractMode_t mode = eRapidRetract; mode <= eFeedRetract; mode = RetractMode_t(int(mode)+1))
	{
		QString str;
		str << mode;
		if (str == string_representation) return(mode);
	}

	return(eRapidRetract);	// Should never get here.
}


CppExampleTPG::CppExampleTPG()
        : CppTPG() { // important to call parent constructor
    id = QS(myID);
    name = QS(myName);
    description = QS(myDesc);

	this->toolpath = NULL;

    QString qaction = QS("default");
    actions.push_back(qaction);

	depth = NULL;
	standoff = NULL;
	dwell = NULL;
	peck_depth = NULL;
	retract_mode = NULL;
	clearance_height = NULL;
	spindle_speed = NULL;
	feed_rate = NULL;
	sometimes_hidden = NULL;
}

CppExampleTPG::~CppExampleTPG() {
	if (depth) depth->release();
	if (standoff) standoff->release();
	if (dwell) dwell->release();
	if (peck_depth) peck_depth->release();
	if (retract_mode) retract_mode->release();
	if (clearance_height) clearance_height->release();
	if (spindle_speed) spindle_speed->release();
	if (feed_rate) feed_rate->release();    
}


/* virtual */ void CppExampleTPG::initialise(TPGFeature *tpgFeature)
{
	CppTPG::initialise(tpgFeature);	// We must do this first so that we have somewhere to store our properties.

	QString qaction = QS("default");

	if (settings != NULL)
	{
		// Define the names used for this TPG's settings once here to avoid inconsistencies.



		// We should have a settings pointer by now due to the CppTPG::initialise() call
		this->depth = new Settings::Length(	"Relative Depth", 
											 "Relative Depth",
											 "Distance from the current Z location to the bottom of the hole.  Must be positive",
											 5.0,
											 Settings::Definition::Metric );
		depth->Minimum(0.0);	// must be positive.  No maximum.
		settings->addSettingDefinition(qaction, depth);

		this->standoff = new Settings::Length(	"Standoff", 
												"Standoff",
												"Distance above the drilling point location to retract to following the drilling cycle.",
												5.0,
												Settings::Definition::Metric );

		settings->addSettingDefinition(qaction, standoff);

		this->dwell = new Settings::Double(	"Dwell", 
											"Dwell",
											"Time (in seconds) for which the machine pauses at the bottom of a drilling cycle to break 'stringers'",
											0.0,
											"seconds" );
		this->dwell->Minimum(0.0);
		
		settings->addSettingDefinition(qaction, this->dwell);




	
		this->peck_depth = new Settings::Length( "Peck Depth", 
												 "Peck Depth",
												 "Distance used for itterative movements down into the hole with retractions between each.  If this is zero then peck drilling is disabled.",
												 5.0,
												 Settings::Definition::Metric);
		settings->addSettingDefinition(qaction, this->peck_depth);

		this->retract_mode = new Settings::Enumeration(	 "Retract Mode", 
														 "Retract Mode",
														 int(eRapidRetract),
														 "mode",
														 "0 represents a rapid ratract movement.  1 represents a retraction at the current feed rate.");

		// Enumerated types MUST have one option for each different value.  For each option, the Id must be the integer form and the Label must
		// be the string (verbose) form.  Only the verbose forms are used on the user interface but the values used in the TPGSettingDefinition.value will
		// always be the integer form.
		// The integer forms need not start from zero or be sequential.  The values will appear in the combo-box in the order that
		// they're defined in the options list.  Their position in the list will be used by the combo-box.

		for (RetractMode_t mode = eRapidRetract; mode <= eFeedRetract; mode = RetractMode_t(int(mode)+1))
		{
			QString label;
			label << mode;		// use the operator<< override to convert from the enum to the string form.

			this->retract_mode->Add(int(mode), label);
		}

		settings->addSettingDefinition(qaction, this->retract_mode);


		this->sometimes_hidden = new Settings::Text("Gets Hidden","Gets Hidden", "Gets hidden when retract mode is rapid", "", "Gets hidden when retract mode is rapid");

		settings->addSettingDefinition(qaction, this->sometimes_hidden);

		if (this->retract_mode->get().first == eRapidRetract) 
		{
			this->sometimes_hidden->visible = false;
		}
		else
		{
			this->sometimes_hidden->visible = true;
		}


		this->clearance_height = new Settings::Length(	 "Clearance Relative Height", 
														 "Clearance Relative Height",
														 "Relative distance in Z to move to between holes to ensure the tool does not interfere with fixtures or other parts of the workpiece.",
														 30.0,
														 Settings::Definition::Metric );
		settings->addSettingDefinition(qaction, this->clearance_height);

		this->spindle_speed = new Settings::Double(	"Spindle Speed", 
													"Spindle Speed",
													"Spindle Speed.", 
													700.0,
													"RPM" );
		settings->addSettingDefinition(qaction, this->spindle_speed);

		this->feed_rate = new Settings::Rate(	"Feed Rate", 
												"Feed Rate",
												"Feed Rate.", 
												55.0,
												Settings::Definition::Metric );
		settings->addSettingDefinition(qaction, this->feed_rate);

		this->speed = new Settings::Radio("speed", "Speed", "normal", "The speed of the algorithm.  Faster will use less accurate algorithm.");
		settings->addSettingDefinition(qaction, this->speed);

		this->speed->Add("Fast");
		this->speed->Add("Normal");
		this->speed->Add("Slow");

		settings->addSettingDefinition(qaction, new Settings::Filename("Filename", 
																		 "My Special Filename",
																		 "c:\\temp\\david.txt",
																		 "Filename",
																		 "Dummy setting to test the new SettingType_Filename enumeration."));

		settings->addSettingDefinition(qaction, new Settings::Directory("Directory", 
																		 "My Special Directory",
																		 "c:\\temp",
																		 "Directory",
																		 "Dummy setting to test the new SettingType_Directory enumeration."));

		settings->addSettingDefinition(qaction, new Settings::Color("Colour", 
																		 "My Special Colour",
																		 "Dummy setting to test the new SettingType_Color enumeration."));

		Settings::Integer *test_integer_setting = new Settings::Integer("Integer", 
																		 "My Special Integer",
																		 3,
																		 "",
																		 "Dummy setting to test the new SettingType_Integer. Must be between 0 and 10");
		test_integer_setting->Minimum(1);
		test_integer_setting->Maximum(10);

		settings->addSettingDefinition(qaction, test_integer_setting);


		
		// Just as a hack for now, find all input object names and pass them in as input geometry.
		App::Document *document = App::GetApplication().getActiveDocument();
		if (document)
		{
			std::vector<App::DocumentObject*> input_geometry = document->getObjectsOfType(Part::Feature::getClassTypeId());
			for (std::vector<App::DocumentObject *>::const_iterator itGeometry = input_geometry.begin(); itGeometry != input_geometry.end(); itGeometry++)
			{
				QString value = settings->getValue(qaction, settingName_Geometry());
				if (value.contains(QString::fromAscii((*itGeometry)->getNameInDocument())) == false)
				{
					value.append(QString::fromAscii(" "));
					value.append(QString::fromAscii((*itGeometry)->getNameInDocument()));
					this->geometry->setValue(value);
				}
			}
		}
	}
}

/**
 * Run the TPG to generate the ToolPath code.
 *
 * Note: the return will change once the TP Language has been set in stone
 */
void CppExampleTPG::run(Settings::TPGSettings *settings, ToolPath *toolpath, QString action= QString::fromAscii(""))
{
    qDebug("This is where the TPG would generate the tool-path! \n");
//	if (this->toolpath != NULL)
//	{
//		qDebug("Releasing previously generated toolpath\n");
//		this->toolpath->release();	// release the previous copy and generate a new one.
//	}

	// Look at the list of object names and see if we can use any of them as input geometry.
	// If so, arrange them into the Cam::Paths object for use later.	
	Cam::Paths paths;
	paths.Add( this->geometry->GetNames() );

	ToolPath &python = *(toolpath);	// for readability only.

	// TODO We really need to define which machine post processor is used at the CamFeature
	// level (or above would be better)  We DO NOT want this import to remain in the TPG itself.
	python << "import sys\n";
	python << "sys.path.insert(0,'C:\\David\\src\\FreeCAD_sf_master\\src\\Mod\\Cam2\\App\\PyPostProcessor\\PostProcessor')\n";
	python << "import math\n";
	python << "from nc import *" << "\n";
	python << "import hm50" << "\n";
	
	python.RequiredDecimalPlaces(3);	// assume metric.

	/*
	python << "rapid(x=" << 12.3456789 << ")\n";
	python << "feed(x=" << 4 << ")\n";

	#ifdef FC_DEBUG
		// Run the unit tests for the ToolPath class.  This is for debug only.
		ToolPath::Test test(this->toolpath);
		if (test.Run())
		{
			python << "comment(" << python.PythonString("Tests for ToolPath succeeded") << ")\n";
		}
		else
		{
			python << "comment(" << python.PythonString("Tests for ToolPath failed") << ")\n";
		}
	#endif // FC_DEBUG

	python << "feed(x=" << 1.23456789 << ")\n";
	python << "comment(" << python.PythonString("we should be at x=1.235") << ")\n";

	python << "feed(x=" << 1.2355 << ")\n";
	python << "comment(" << python.PythonString("we should be at x=1.236") << ")\n";

	python << "feed(x=" << 1.2354 << ")\n";
	python << "comment(" << python.PythonString("we should be at x=1.235") << ")\n";

	python << "feed(x=" << -1.2355 << ")\n";
	python << "comment(" << python.PythonString("we should be at x=-1.236") << ")\n";

	python << "feed(x=" << -1.2354 << ")\n";
	python << "comment(" << python.PythonString("we should be at x=-1.235") << ")\n";

	QString some_setting(QString::fromUtf8("'Something (that) needs cleaning up befor it's included in a python comment() call'"));
	python << "comment(" << python.PythonString(some_setting) << ")\n";

	python << "comment(" << python.PythonString("the cat's mat has a single quote in it") << ")\n";
	python << "comment(" << python.PythonString("this one has (something in brackets) within it.") << ")\n";
	python << "comment(" << python.PythonString("'this one already has single quotes bounding it'") << ")\n";
	python << "comment(" << python.PythonString("\"this one already has double quotes bounding it\"") << ")\n";
	*/

	// Define a couple of intersecting lines just to play with the drilling cycle code.  This is
	// really to test the calling of the Python code held in the ToolPath through to the GCode
	// held in the MachineProgram.

	/*
	Cam::Paths paths;
	paths.Add( Cam::Edge( Cam::Point(0.0, 0.0, 0.0), Cam::Point(100.0, 0.0, 0.0) ) );
	paths.Add( Cam::Edge( Cam::Point(50.0, 0.0, 0.0), Cam::Point(50.0, 100.0, 0.0) ) );
	*/

	Settings::Definition::Units_t units = Settings::Definition::Metric;	// TODO Get some setting that indicates metric or imperial units for GCode.
	// For now, figure out the units by looking at the resolution of the Python code.
	if (python.RequiredDecimalPlaces() == 3)
	{
		units = Settings::Definition::Metric;	// metric.
	}
	else
	{
		units = Settings::Definition::Imperial;	// imperial
	}

	// Mark the beginning and end of the GCode generated by this TPG just in case we want to find
	// it again from the whole GCode program (for highlighting etc.)
	QString tpg_reference = QString::fromAscii("TpgBegin ");
	tpg_reference += QString::fromAscii("id='") + this->getId() + QString::fromAscii("'");
	tpg_reference += QString::fromAscii(", name='") + this->getName() + QString::fromAscii("'");
	python << "comment(" << python.PythonString(tpg_reference) << ")\n";

	// Add some 'modal' values for the remainder of this machine operation's use.
	// TODO - select the appropriate fixture
	// TODO - select the appropriate tool

	python << "spindle(s=" << this->spindle_speed->get() << ", clockwise=True)\n";
	// python << "feedrate(" << this->feed_rate.get(units) << ")\n";	

	// Generate the drilling locations from the Paths objects.  i.e. points as well as intersecting sketches etc.
	Cam::Paths::Locations_t locations = paths.PointLocationData();
	for (Cam::Paths::Locations_t::iterator itLocation = locations.begin(); itLocation != locations.end(); itLocation++)
	{
		// Align this point with the fixture (i.e. translation and/or rotation) so that the fixture's coordinate
		// system in the graphics file translates to the X,Y,Z coordinate system on the physical CNC machine.

		// gp_Pnt point = pMachineState->Fixture().Adjustment( l_itLocation->Location() );

		gp_Pnt point = itLocation->Location();	// Just for now...

		// TODO: Figure out where our GCode units settings are defined and convert the coordinates
		// from the graphics into those units required for GCode.

		python	<< "drill("
				<< "x=" << point.X() << ", "
				<< "y=" << point.Y() << ", "
				<< "z=" << point.Z() << ", "
				<< "depth=" << this->depth->get(units) << ", "
				<< "standoff=" << this->standoff->get(units) << ", "
				<< "dwell=" << this->dwell->get() << ", "
				<< "peck_depth=" << this->peck_depth->get(units) << ", "
				<< "retract_mode=" << this->retract_mode->get().first << ", "
				<< "clearance_height=" << this->clearance_height->get(units)
				<< ")\n";
        // pMachineState->Location(point); // Remember where we are.
	} // End for

	python << "end_canned_cycle()\n";

	/*
	python << "# coding=CP1252\n";
	python << "#English language or it's variant detected in Microsoft Windows\n";
	python << "import sys\n";
	python << "sys.path.insert(0,'C:\\David\\src\\jdcnc\\heekscnc')\n";
	python << "import math\n";
	python << "from nc.nc import *\n";
	python << "import nc.hm50\n";
	*/

	/*
	python << "program_begin(123, 'Generated by JDCNC version 0 17 133 - Monday 12 August 2013 - 09:45:38')\n";
	python << "absolute()\n";
	python << "metric()\n";
	python << "set_plane(0)\n";
	python << "machine_units_metric(False)\n";

	python << "set_path_control_mode(2,0.001,0.001)\n";
	python << "comment('Feeds and Speeds set for machining Mild Steel')\n";
	python << "#(1.6 mm HSS Drill Bit (for M2 x 0.4 tap))\n";
	python << "tool_defn( id=25, name='1.6 mm HSS Drill Bit (for M2 x 0.4 tap)', radius=0.8, length=15, gradient=0)\n";
	python << "#(3 mm HSS Drill Bit)\n";
	python << "tool_defn( id=27, name='3 mm HSS Drill Bit', radius=1.5, length=20, gradient=0)\n";
	python << "#(3.3 mm HSS Drill Bit (for M4 x 0.7 tap hole))\n";
	python << "tool_defn( id=28, name='3.3 mm HSS Drill Bit (for M4 x 0.7 tap hole)', radius=1.65, length=25, gradient=0)\n";
	python << "#(4.2 mm HSS Drill Bit (for 5mm tapping))\n";
	python << "tool_defn( id=4, name='4.2 mm HSS Drill Bit (for 5mm tapping)', radius=2.1, length=65, gradient=0)\n";
	python << "#(5 mm HSS Drill Bit (for M6 x 1 tapping))\n";
	python << "tool_defn( id=35, name='5 mm HSS Drill Bit (for M6 x 1 tapping)', radius=2.5, length=40, gradient=0)\n";
	python << "#(5.15937 mm (13/64\") HSS Drill Bit)\n";
	python << "tool_defn( id=31, name='5.15937 mm (13/64\") HSS Drill Bit', radius=2.58, length=63.5, gradient=0)\n";
	python << "#(5.5 mm HSS Drill Bit)\n";
	python << "tool_defn( id=5, name='5.5 mm HSS Drill Bit', radius=2.75, length=65, gradient=0)\n";
	python << "#(6 mm HSS Drill Bit)\n";
	python << "tool_defn( id=33, name='6 mm HSS Drill Bit', radius=3, length=50, gradient=0)\n";
	python << "#(6.35 mm (1/4\") HSS Drill Bit)\n";
	python << "tool_defn( id=12, name='6.35 mm (1/4\") HSS Drill Bit', radius=3.175, length=63.5, gradient=0)\n";
	python << "#(6.8 mm HSS Drill Bit (for M8x1.25 tap hole))\n";
	python << "tool_defn( id=20, name='6.8 mm HSS Drill Bit (for M8x1.25 tap hole)', radius=3.4, length=63.5, gradient=0)\n";
	python << "#(8 mm HSS Drill Bit)\n";
	python << "tool_defn( id=30, name='8 mm HSS Drill Bit', radius=4, length=63.5, gradient=0)\n";
	python << "#(8.5 mm HSS Drill Bit (for 10mm tap))\n";
	python << "tool_defn( id=15, name='8.5 mm HSS Drill Bit (for 10mm tap)', radius=4.25, length=40, gradient=0)\n";
	python << "#(9.525 mm (3/8\") HSS Drill Bit)\n";
	python << "tool_defn( id=37, name='9.525 mm (3/8\") HSS Drill Bit', radius=4.763, length=63.5, gradient=0)\n";
	python << "#(10 mm HSS Drill Bit)\n";
	python << "tool_defn( id=17, name='10 mm HSS Drill Bit', radius=5, length=63.5, gradient=0)\n";
	python << "#(12.7 mm (1/2\") HSS Drill Bit)\n";
	python << "tool_defn( id=6, name='12.7 mm (1/2\") HSS Drill Bit', radius=6.35, length=110, gradient=0)\n";
	python << "#(Size #4 (1/8\"=3.17mm) HSS Centre Drill Bit)\n";
	python << "tool_defn( id=3, name='Size #4 (1/8\"=3.17mm) HSS Centre Drill Bit', radius=1.588, length=53.975, gradient=0)\n";
	python << "#(6 mm HSS End Mill)\n";
	python << "tool_defn( id=7, name='6 mm HSS End Mill', radius=3, length=25, gradient=-0.02)\n";
	python << "#(6 mm HSS Long Series End Mill)\n";
	python << "tool_defn( id=11, name='6 mm HSS Long Series End Mill', radius=3, length=35, gradient=-0.02)\n";
	python << "#(10 mm HSS Roughing End Mill)\n";
	python << "tool_defn( id=8, name='10 mm HSS Roughing End Mill', radius=5, length=20, gradient=-0.02)\n";
	python << "#(45 degree HSS Chamfering Bit)\n";
	python << "tool_defn( id=10, name='45 degree HSS Chamfering Bit', radius=6.25, length=63.5, gradient=0)\n";
	python << "#(M4 x 0.7 mm coarse Tap Tool)\n";
	python << "tool_defn( id=29, name='M4 x 0.7 mm coarse Tap Tool', radius=2, length=25, gradient=0)\n";
	python << "#(1/4 x 20 UNC Tap Tool)\n";
	python << "tool_defn( id=32, name='1/4 x 20 UNC Tap Tool', radius=3.175, length=20, gradient=0)\n";
	python << "#(Tool Length Switch)\n";
	python << "tool_defn( id=1, name='Tool Length Switch', radius=6.35, length=63.5, gradient=0)\n";
	python << "#(Touch Probe)\n";
	python << "tool_defn( id=2, name='Touch Probe', radius=1.455, length=50, gradient=0)\n";
	python << "#(16 mm Carbide End Mill)\n";
	python << "tool_defn( id=9, name='16 mm Carbide End Mill', radius=8, length=55, gradient=-0.02)\n";
	python << "#(M5 x 0.8 mm coarse Tap Tool)\n";
	python << "tool_defn( id=13, name='M5 x 0.8 mm coarse Tap Tool', radius=2.5, length=63.5, gradient=0)\n";
	python << "#(Boring head)\n";
	python << "tool_defn( id=14, name='Boring head', radius=16, length=63.5, gradient=0)\n";
	python << "#(M10 x 1.25mm fine Tap Tool)\n";
	python << "tool_defn( id=16, name='M10 x 1.25mm fine Tap Tool', radius=5, length=63.5, gradient=0)\n";
	python << "#(20 mm Carbide End Mill)\n";
	python << "tool_defn( id=18, name='20 mm Carbide End Mill', radius=10, length=15.875, gradient=-0.02)\n";
	python << "#(45 degree Carbide Insert Chamfering Bit)\n";
	python << "tool_defn( id=19, name='45 degree Carbide Insert Chamfering Bit', radius=12.5, length=63.5, gradient=0)\n";
	python << "#(M8 x 1.25mm coarse Tap Tool)\n";
	python << "tool_defn( id=21, name='M8 x 1.25mm coarse Tap Tool', radius=4, length=63.5, gradient=0)\n";
	python << "#(0.79375 mm (1/32\") Carbide End Mill)\n";
	python << "tool_defn( id=22, name='0.79375 mm (1/32\") Carbide End Mill', radius=0.397, length=63.5, gradient=-0.02)\n";
	python << "#(0.5 mm Carbide End Mill)\n";
	python << "tool_defn( id=23, name='0.5 mm Carbide End Mill', radius=0.25, length=63.5, gradient=-0.02)\n";
	python << "#(100 mm Carbide End Mill)\n";
	python << "tool_defn( id=24, name='100 mm Carbide End Mill', radius=50, length=63.5, gradient=-0.02)\n";
	python << "#(M2 x 0.4 mm coarse Tap Tool)\n";
	python << "tool_defn( id=26, name='M2 x 0.4 mm coarse Tap Tool', radius=1, length=15, gradient=0)\n";
	python << "#(2 mm Carbide End Mill)\n";
	python << "tool_defn( id=36, name='2 mm Carbide End Mill', radius=1, length=20, gradient=-0.02)\n";
	python << "#(M6 x 1 mm coarse Tap Tool)\n";
	python << "tool_defn( id=34, name='M6 x 1 mm coarse Tap Tool', radius=3, length=63.5, gradient=0)\n";
	python << "set_plane(0)\n";
	python << "work_offset(workplane=1, xy_plane_rotation=0.0)\n";
	python << "message('Confirm that the tool length switch position has been marked with G28.1')\n";
	python << "program_stop(optional=False)\n";
	python << "message('Confirm that the X,Y touch-off point for G54 has been MANUALLY set <Bottom left corner of flat plate>')\n";
	python << "program_stop(optional=False)\n";
	python << "comment('tool change to Touch Probe')\n";
	python << "tool_change(id=2, description='Tool change to Touch Probe')\n";
	python << "feedrate(300)\n";
	python << "comment('Move back up to machine safety height')\n";
	python << "rapid(z=10, machine_coordinates=True)\n";
	python << "predefined_position(type='G28')\n";
	python << "comment('Move back up to machine safety height')\n";
	python << "rapid(z=10, machine_coordinates=True)\n";
	python << "comment('Probe to find the height of the tool length switch.  NOTE: We MUST do this from the machine safety height for consistency across fixtures')\n";
	python << "probe_downward_point(depth=-300, intersection_variable_z='<tool_length_switch_offset>', feedrate=300, use_m66_to_confirm_probe_state='True', m66_input_pin_number=0)\n";
	python << "comment('Move back up to machine safety height')\n";
	python << "rapid(z=10, machine_coordinates=True)\n";
	python << "workplane(1)\n";
	python << "feedrate(300)\n";
	python << "comment('Probing for height of G54 fixture')\n";
	python << "comment('Move back up to machine safety height')\n";
	python << "rapid(z=10, machine_coordinates=True)\n";
	python << "workplane(1)\n";
	python << "rapid(x=0,y=0)\n";
	python << "rapid(z=10, machine_coordinates=True)\n";
	python << "probe_downward_point(depth=-300, intersection_variable_z='<G54_fixture_height_offset>', touch_off_as_z=0, rapid_down_to_height=5, feedrate=300, use_m66_to_confirm_probe_state='True', m66_input_pin_number=0)\n";
	python << "comment('Move back up to machine safety height')\n";
	python << "rapid(z=10, machine_coordinates=True)\n";
	python << "workplane(1)\n";
	python << "rapid(x=0,y=0)\n";
	python << "rapid(z=10, machine_coordinates=True)\n";
	python << "workplane(1)\n";
	python << "comment('Move to safe Z height and then to G28 location')\n";
	python << "rapid(z=10, machine_coordinates=True)\n";
	python << "predefined_position(type='G28')\n";
	python << "comment('tool change to Size #4 (1/8\"=3.17mm) HSS Centre Drill Bit')\n";
	python << "tool_change( id=3, description='Tool change to Size #4 (1/8\"=3.17mm) HSS Centre Drill Bit')\n";
	python << "rapid(z=10, machine_coordinates=True)\n";
	python << "measure_and_offset_tool(distance=300, switch_offset_variable_name='<tool_length_switch_offset>', fixture_offset_variable_name='<G54_fixture_height_offset>', feed_rate=300)\n";
	python << "comment('Move back up to safety height before continuing with program')\n";
	python << "rapid(z=10, machine_coordinates=True)\n";
	python << "workplane(1)\n";
	python << "message('Press cycle start when ready to continue with Size #4 (1/8\"=3.17mm) HSS Centre Drill Bit')\n";
	python << "program_stop(optional=False)\n";
	python << "comment('Move above the touch-off point for G54')\n";
	python << "workplane(1)\n";
	python << "rapid(x=0, y=0)\n";
	python << "spindle(s=1300, clockwise=True)\n";
	python << "feedrate_hv(0, 55.809)\n";
	python << "drill(x=120.18, y=94.352, z=0, depth=1, standoff=2, dwell=0, peck_depth=0, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=99.62, y=92, z=0, depth=1, standoff=2, dwell=0, peck_depth=0, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=99.648, y=68.009, z=0, depth=1, standoff=2, dwell=0, peck_depth=0, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=99.62, y=40, z=0, depth=1, standoff=2, dwell=0, peck_depth=0, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=120.627, y=39.881, z=0, depth=1, standoff=2, dwell=0, peck_depth=0, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=180.214, y=39.881, z=0, depth=1, standoff=2, dwell=0, peck_depth=0, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=197.903, y=40, z=0, depth=1, standoff=2, dwell=0, peck_depth=0, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=197.845, y=68.456, z=0, depth=1, standoff=2, dwell=0, peck_depth=0, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=197.903, y=92, z=0, depth=1, standoff=2, dwell=0, peck_depth=0, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=180.438, y=91.896, z=0, depth=1, standoff=2, dwell=0, peck_depth=0, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=241, y=112, z=0, depth=1, standoff=2, dwell=0, peck_depth=0, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=240, y=20, z=0, depth=1, standoff=2, dwell=0, peck_depth=0, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=59.923, y=37.425, z=0, depth=1, standoff=2, dwell=0, peck_depth=0, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=21, y=20, z=0, depth=1, standoff=2, dwell=0, peck_depth=0, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=20, y=112, z=0, depth=1, standoff=2, dwell=0, peck_depth=0, retract_mode=0, clearance_height=20)\n";
	python << "end_canned_cycle()\n";
	python << "spindle(s=0, clockwise=False)\n";
	python << "comment('Move to safe Z height and then to G28 location')\n";
	python << "rapid(z=10, machine_coordinates=True)\n";
	python << "predefined_position(type='G28')\n";
	python << "comment('tool change to 6.35 mm (1/4\") HSS Drill Bit')\n";
	python << "tool_change( id=12, description='Tool change to 6.35 mm (1/4\") HSS Drill Bit')\n";
	python << "rapid(z=10, machine_coordinates=True)\n";
	python << "measure_and_offset_tool(distance=300, switch_offset_variable_name='<tool_length_switch_offset>', fixture_offset_variable_name='<G54_fixture_height_offset>', feed_rate=300)\n";
	python << "comment('Move back up to safety height before continuing with program')\n";
	python << "rapid(z=10, machine_coordinates=True)\n";
	python << "workplane(1)\n";
	python << "message('Press cycle start when ready to continue with 6.35 mm (1/4\") HSS Drill Bit')\n";
	python << "program_stop(optional=False)\n";
	python << "comment('Move above the touch-off point for G54')\n";
	python << "workplane(1)\n";
	python << "rapid(x=0, y=0)\n";
	python << "spindle(s=1126.211, clockwise=True)\n";
	python << "feedrate_hv(0, 75.51)\n";
	python << "drill(x=120.18, y=94.352, z=0, depth=10, standoff=2, dwell=0, peck_depth=3.175, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=99.648, y=68.009, z=0, depth=10, standoff=2, dwell=0, peck_depth=3.175, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=120.627, y=39.881, z=0, depth=10, standoff=2, dwell=0, peck_depth=3.175, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=180.214, y=39.881, z=0, depth=10, standoff=2, dwell=0, peck_depth=3.175, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=197.845, y=68.456, z=0, depth=10, standoff=2, dwell=0, peck_depth=3.175, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=180.438, y=91.896, z=0, depth=10, standoff=2, dwell=0, peck_depth=3.175, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=241, y=112, z=0, depth=10, standoff=2, dwell=0, peck_depth=3.175, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=240, y=20, z=0, depth=10, standoff=2, dwell=0, peck_depth=3.175, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=59.923, y=37.425, z=0, depth=10, standoff=2, dwell=0, peck_depth=3.175, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=21, y=20, z=0, depth=10, standoff=2, dwell=0, peck_depth=3.175, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=20, y=112, z=0, depth=10, standoff=2, dwell=0, peck_depth=3.175, retract_mode=0, clearance_height=20)\n";
	python << "end_canned_cycle()\n";
	python << "spindle(s=0, clockwise=False)\n";
	python << "comment('Move to safe Z height and then to G28 location')\n";
	python << "rapid(z=10, machine_coordinates=True)\n";
	python << "predefined_position(type='G28')\n";
	python << "comment('tool change to 5.15937 mm (13/64\") HSS Drill Bit')\n";
	python << "tool_change( id=31, description='Tool change to 5.15937 mm (13/64\") HSS Drill Bit')\n";
	python << "rapid(z=10, machine_coordinates=True)\n";
	python << "measure_and_offset_tool(distance=300, switch_offset_variable_name='<tool_length_switch_offset>', fixture_offset_variable_name='<G54_fixture_height_offset>', feed_rate=300)\n";
	python << "comment('Move back up to safety height before continuing with program')\n";
	python << "rapid(z=10, machine_coordinates=True)\n";
	python << "workplane(1)\n";
	python << "message('Press cycle start when ready to continue with 5.15937 mm (13/64\") HSS Drill Bit')\n";
	python << "program_stop(optional=False)\n";
	python << "comment('Move above the touch-off point for G54')\n";
	python << "workplane(1)\n";
	python << "rapid(x=0, y=0)\n";
	python << "spindle(s=1243.042, clockwise=True)\n";
	python << "feedrate_hv(0, 65.317)\n";
	python << "drill(x=99.62, y=92, z=0, depth=10, standoff=2, dwell=0, peck_depth=2.58, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=197.903, y=92, z=0, depth=10, standoff=2, dwell=0, peck_depth=2.58, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=197.903, y=40, z=0, depth=10, standoff=2, dwell=0, peck_depth=2.58, retract_mode=0, clearance_height=20)\n";
	python << "drill(x=99.62, y=40, z=0, depth=10, standoff=2, dwell=0, peck_depth=2.58, retract_mode=0, clearance_height=20)\n";
	python << "end_canned_cycle()\n";
	python << "spindle(s=0, clockwise=False)\n";
	python << "rapid(z=10, machine_coordinates=True)\n";
	python << "program_end()\n";
	*/



	/*
	// Generate some graphics, convert it into the libArea representation and generate
	// the Python equivalent of that.
	Cam::Paths graphics;

	// Define a square
	graphics.Add( Cam::Edge( Cam::Point(0.0, 0.0, 0.0), Cam::Point(100.0, 0.0, 0.0)));
	graphics.Add( Cam::Edge( Cam::Point(100.0, 0.0, 0.0), Cam::Point(100.0, 100.0, 0.0)));
	graphics.Add( Cam::Edge( Cam::Point(100.0, 100.0, 0.0), Cam::Point(0.0, 100.0, 0.0)));
	graphics.Add( Cam::Edge( Cam::Point(0.0, 100.0, 0.0), Cam::Point(0.0, 0.0, 0.0)));

	// And add an arc into the middle of it.
	gp_Circ circle;
	gp_Pnt	centre(50.0, 50.0, 0.0);
	circle.SetLocation(centre);
	circle.SetAxis(gp_Ax1(centre, gp_Dir(gp_XYZ(0.0, 0.0, 1.0))));
	circle.SetRadius(10.0);
	graphics.Add( Cam::Edge(Cam::Point(centre.X() - circle.Radius(), centre.Y(), 0.0), Cam::Point(centre.X() + circle.Radius(), centre.Y(), 0.0), circle));

	python << "import sys\n";
	// python << "sys.path.insert(0, 'C:\\\\David\\\\src\\\\FreeCAD_sf_master\\\\VC9\\\\src\\Mod\\\\Cam2\\\\App\\\\Libs\\\\libarea-clipper')\n";
	python << "sys.path.insert(0, 'C:\\\\David\\\\src\\\\jdcnc\\\\trunk\\\\heekscnc')\n";
	python << "sys.path.insert(0, 'C:\\\\David\\\\src\\\\jdcnc\\\\trunk\\\\heekscnc\\\\Boolean')\n";
           	                       
	python << "import AreaClipper_d as area\n";
	// python << "import area\n";
	python << "import kurve_funcs as kurve_funcs\n";

	python << "comment('tool change to 1/8 inch End Mill')\n";
	python << "#tool_change( id=1)\n";
	python << "spindle(7000)\n";
	python << "feedrate_hv(6889.76378, 7)\n";
	python << "clearance = float(1.25)\n";
	python << "rapid_safety_space = float(0.07874015748)\n";
	python << "start_depth = float(0)\n";
	python << "step_down = float(0.035)\n";
	python << "final_depth = float(-0.07)\n";
	python << "tool_diameter = float(0.125)\n";
	python << "cutting_edge_angle = float(0)\n";

	python << "roll_radius = float(0)\n";
	python << "offset_extra = 0\n";

	// Convert the graphics into their Python representation so that the kurve_funcs Python script
	// can receive them in the format it expects.
	// This addAreaDefinition() adds an Area object and includes all the Curve objects within it.
	python.addAreaDefinition( graphics.Area(), "area_description" );

	// This addCurveDefinition() ONLY adds the Curve object represenations.  We do NOT need BOTH
	// the area and the curve represenation normally.  The area call above is really just so we
	// can see what the Area represenation looks like - for debugging.
	const char *l_pszObjectName = "outer_profile";
	python.addCurveDefinition( graphics[0].AreaCurve(), l_pszObjectName );

	// And back to the example taken from test.py
	python << "kurve_funcs.make_smaller( " << l_pszObjectName << ", start = area.Point(1.8, 0))\n";
	python << "roll_on = 'auto'\n";
	python << "roll_off = 'auto'\n";
	python << "extend_at_start= 0\n";
	python << "extend_at_end= 0\n";
	python << "lead_in_line_len= 0\n";
	python << "lead_out_line_len= 0\n";
	python << "kurve_funcs.profile(outer_profile, 'left', tool_diameter/2, offset_extra, roll_radius, roll_on, roll_off, rapid_safety_space, clearance, start_depth, step_down, final_depth,extend_at_start,extend_at_end,lead_in_line_len,lead_out_line_len )\n";
	python << "absolute()\n";
	python << "program_end()\n";
	*/

	// Mark the end of the GCode generated by this TPG just in case we want to find
	// it again from the whole GCode program (for highlighting etc.)
	tpg_reference.replace(QString::fromAscii("TpgBegin"), QString::fromAscii("TpgEnd"));
	python << "comment(" << python.PythonString(tpg_reference) << ")\n";
}


/* virtual */ void CppExampleTPG::onChanged( Settings::Definition *tpgSettingDefinition, QString previous_value, QString new_value )
{
	if (tpgSettingDefinition == depth)
	{
		qDebug("CppExampleTPG::onChanged(%s changed to %lf)\n", 
				tpgSettingDefinition->getFullname().toAscii().constData(),
				depth->get(depth->getUnits()));
	}
	else if (tpgSettingDefinition == standoff)
	{
		qDebug("CppExampleTPG::onChanged(%s changed to %lf)\n", 
				tpgSettingDefinition->getFullname().toAscii().constData(),
				standoff->get(standoff->getUnits()));
	}
	else if (tpgSettingDefinition == dwell)
	{
		qDebug("CppExampleTPG::onChanged(%s changed to %lf)\n", 
				tpgSettingDefinition->getFullname().toAscii().constData(),
				dwell->get());
	}
	else if (tpgSettingDefinition == peck_depth)
	{
		qDebug("CppExampleTPG::onChanged(%s changed to %lf)\n", 
				tpgSettingDefinition->getFullname().toAscii().constData(),
				peck_depth->get(peck_depth->getUnits()));
	}
	else if (tpgSettingDefinition == retract_mode)
	{
		qDebug("CppExampleTPG::onChanged(%s changed to %s)\n", 
				tpgSettingDefinition->getFullname().toAscii().constData(),
				retract_mode->get().second.toAscii().constData());

		// If we set the visible flag here, the settings editor dialog box will be updated
		// to reflect the change.  We can use this to hide or display settings when other
		// settings change value.  eg: If one setting describes the 'number of tabs' in a
		// Contour operation then the setting for 'tab height' and/or 'tab width' need only
		// be displayed if the 'number of tabs' value is greater than zero.

		if (this->retract_mode->get().first == eRapidRetract) 
		{
			this->sometimes_hidden->visible = false;
			this->peck_depth->set(7.4);
			this->speed->setValue(QString::fromAscii("Fast"));
		}
		else
		{
			this->sometimes_hidden->visible = true;
			this->peck_depth->set(3.3);
			this->speed->setValue(QString::fromAscii("Slow"));
		}
	}
	else if (tpgSettingDefinition == clearance_height)
	{
		qDebug("CppExampleTPG::onChanged(%s changed to %lf)\n", 
				tpgSettingDefinition->getFullname().toAscii().constData(),
				clearance_height->get(clearance_height->getUnits()));
	}
	else if (tpgSettingDefinition == spindle_speed)
	{
		qDebug("CppExampleTPG::onChanged(%s changed to %lf)\n", 
				tpgSettingDefinition->getFullname().toAscii().constData(),
				spindle_speed->get());
	}
	else if (tpgSettingDefinition == feed_rate)
	{
		qDebug("CppExampleTPG::onChanged(%s changed to %lf)\n", 
				tpgSettingDefinition->getFullname().toAscii().constData(),
				feed_rate->get(feed_rate->getUnits()));
	}
	else
	{
		qDebug("CppExampleTPG::onChanged(%s changed from %s to %s)\n", 
					tpgSettingDefinition->getFullname().toAscii().constData(),
					previous_value.toAscii().constData(), 
					new_value.toAscii().constData());
	}

	CppTPG::onChanged( tpgSettingDefinition, previous_value, new_value );
}


///* virtual */ ToolPath *CppExampleTPG::getToolPath()
//{
//	if (this->toolpath)
//	{
//		return(this->toolpath->grab());
//	}
//	else
//	{
//		return(NULL);
//	}
//}


} /* namespace Cam */
