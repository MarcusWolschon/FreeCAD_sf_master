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


#include <PreCompiled.h>
#ifndef _PreComp_
#endif

#include <App/Document.h>
#include <App/Application.h>

#include <sstream>
#include <math.h>

// NOTE: These BOOST SPIRIT DEBUG macros MUST be defined BEFORE the include
// files.  Otherwise the default values, defined within the include files,
// will be used instead.

#include "LinuxCNC.h"
#include "TPG/ToolPath.h"
#include "Graphics/Paths.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <gp_Pnt.hxx>

using namespace Cam;
LinuxCNC::LinuxCNC(MachineProgram *machine_program, TPGFeature* rpgFeature) : GCode(machine_program, rpgFeature)
{
}

LinuxCNC::~LinuxCNC()
{
}

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phx = boost::phoenix;

typedef std::string::iterator Iterator;


template <typename Iter, typename Skipper = qi::blank_type> 
	struct linuxcnc_grammar : qi::grammar<Iter, Skipper> 
{

	// Accumulate a map of command line arguments for the motion commands
	// so that we know what was specified on each block.  This map is
	// cleaned out (after being duplicated) once the 'EndOfBlock' processing
	// is complete.
	// The 'value' isn't passed in directly here.  Instead, a symbol ID
	// is given that indicates which entry in the symbol table contains
	// the value.  We need to do this so that we can handle literal values,
	// mathematical expressions and variables.
	void AddMotionArgument(std::vector<char> c, LinuxCNC::Variables::SymbolId_t symbol_id)
	{
		std::string name;
		std::copy( c.begin(), c.end(), std::inserter(name,name.begin()));

		if (c.size() == 1)
		{
			switch (tolower(c[0]))
			{
			case 'x':	this->x = Value(symbol_id); this->x_specified = true; break;
			case 'y':	this->y = Value(symbol_id); this->y_specified = true; break;
			case 'z':	this->z = Value(symbol_id); this->z_specified = true; break;

			case 'a':	this->a = Value(symbol_id); this->a_specified = true; break;
			case 'b':	this->b = Value(symbol_id); this->b_specified = true; break;
			case 'c':	this->c = Value(symbol_id); this->c_specified = true; break;

			case 'u':	this->u = Value(symbol_id); this->u_specified = true; break;
			case 'v':	this->v = Value(symbol_id); this->v_specified = true; break;
			case 'w':	this->w = Value(symbol_id); this->w_specified = true; break;

			case 'l':	this->l = Value(symbol_id); this->l_specified = true; break;
			case 'p':	this->p = Value(symbol_id); this->p_specified = true; break;
			case 'r':	this->r = Value(symbol_id); this->r_specified = true; break;
			case 'q':	this->q = Value(symbol_id); this->q_specified = true; break;

			case 'i':	this->i = Value(symbol_id); this->i_specified = true; break;
			case 'j':	this->j = Value(symbol_id); this->j_specified = true; break;
			case 'k':	this->k = Value(symbol_id); this->k_specified = true; break;

			default:
				{
					QString warning;
					warning = QString::fromAscii("Unexpected motion argument '") + QString::fromStdString(name) + QString::fromAscii("' found");
					pLinuxCNC->AddWarning(warning);
				}
				break;
			}
		}
		else
		{
			QString warning;
			warning = QString::fromAscii("Invalid motion argument '") + QString::fromStdString(name) + QString::fromAscii("' found");
			pLinuxCNC->AddWarning(warning);
		}
	}

	void SetLineNumber( const int value ) { line_number = value; }

	/**
		The statement type is really the feed from the grammar parsing to what's done
		about it in the ProcessBlock() method.  The ProcessBlock() method is the thing
		that actually generates the graphics representing the tool movements and it
		decides which pattern of movement to generate based on the statement type.
		We do this separately as various sub-statement types may appear on the same
		line as a more important one.  eg: we can be setting the spindle speed
		on a line that contains a feed (G01) command.  In this case the feed command
		will result in a tool movement but the spindle speed command will not.
	 */
	void StatementType( const LinuxCNC::eStatement_t type )
	{
		// Only override the statement type if this integer is larger than the
		// current value.  This allows the order of the statement type
		// enumeration to determine the precendence used for statement
		// types (for colour etc.)

		if (int(type) > int(statement_type)) statement_type = type;
	}

	/**
		This is the constructor for the template and is effectively the 'main' routine
		for the grammar parsing.  This is where we define the grammar itself as well as
		the 'actions' (i.e. method calls) that result where various patterns of grammar
		are detected.  It's the side-effects of these 'actions' that produce our real
		output.  Mostly these come from the ProcessBlock() method.
	 */
	linuxcnc_grammar(LinuxCNC *pLinuxCNC) : linuxcnc_grammar::base_type(Start)
	{
		// Variables declared here are created and destroyed for each rule parsed.
		// Use member variables of the structure for long-lived variables instead.

		this->pLinuxCNC = pLinuxCNC;
		this->machine_program = this->pLinuxCNC->machine_program;

		current_coordinate_system = LinuxCNC::csG53;
		modal_coordinate_system = LinuxCNC::csG54;
		
		plane = LinuxCNC::eXYPlane;
		tool_length_offset = 0.0;

		line_offset = 0;

		// The 'units' are 1.0 for metric and 25.4 for imperial. These are the units of the
		// current graphics.  It's used for the graphics that represents tool movements.
		units = 1.0;	// Metric by default. TODO Take this from some application settings.

		// The 'emc2 variables units' indicates what units are relevant to the linuxcnc.var
		// variables file read in at startup.
		m_emc2_variables_units = LinuxCNC::eMetric;	// TODO Take this from some application settings.

		this->statement_type = LinuxCNC::stUndefined;

		this->previous[0] = this->x = 0.0;
		this->previous[1] = this->y = 0.0;
		this->previous[2] = this->z = 0.0;
		this->previous[3] = this->a = 0.0;
		this->previous[4] = this->b = 0.0;
		this->previous[5] = this->c = 0.0;
		this->previous[6] = this->u = 0.0;
		this->previous[7] = this->v = 0.0;
		this->previous[8] = this->w = 0.0;

		this->l = 0.0;
		this->p = 0.0;
		this->r = 0.0;
		this->q = 0.0;

		this->i = 0.0;
		this->j = 0.0;
		this->k = 0.0;

		ResetForEndOfBlock();
		InitializeGCodeVariables();

		// N110 - i.e. use qi::lexeme to avoid qi::space_type skipper which allows the possibility of interpreting 'N 110'.  We don't want spaces here.
		LineNumberRule = (ascii::no_case[qi::lit("N")] >> qi::int_)
			[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::SetLineNumber, phx::ref(*this), qi::_1) ] // call this->SetLineNumber(_2);
			;

		// X 1.1 etc.
		MotionArgument = (ascii::no_case[qi::repeat(1,1)[qi::char_("XYZABCUVWLPRQIJK")]] >> MathematicalExpression )
			[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::AddMotionArgument, phx::ref(*this), qi::_1, qi::_2) ] // call this->AddMotionArgument(_1, _2);
			;

		// The G00 to G09 commands need some special treatment due to the optional '0'. i.e. G01 and G1 are both valid while G10
		// can only ever be expressed as G10.  We use the ascii::no_case to handle the 'G' or 'g' equivalence.
		G00 = qi::lexeme[ascii::no_case[qi::lit("G")] >> qi::repeat(1,2)[qi::char_("0")]];
		G01 = qi::lexeme[ascii::no_case[qi::lit("G")] >> qi::repeat(0,1)[qi::char_("0")] >> qi::lit("1")];
		G02 = qi::lexeme[ascii::no_case[qi::lit("G")] >> qi::repeat(0,1)[qi::char_("0")] >> qi::lit("2")];
		G03 = qi::lexeme[ascii::no_case[qi::lit("G")] >> qi::repeat(0,1)[qi::char_("0")] >> qi::lit("3")];
		G04 = qi::lexeme[ascii::no_case[qi::lit("G")] >> qi::repeat(0,1)[qi::char_("0")] >> qi::lit("4")];

		CoordinateSystems =
				(ascii::no_case[qi::lit("G53")])
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::SetCurrentCoordinateSystem, phx::ref(*this), LinuxCNC::csG53) ]
			|	(ascii::no_case[qi::lit("G54")])
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::SetModalCoordinateSystem, phx::ref(*this), LinuxCNC::csG54) ]
			|	(ascii::no_case[qi::lit("G55")])
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::SetModalCoordinateSystem, phx::ref(*this), LinuxCNC::csG55) ]
			|	(ascii::no_case[qi::lit("G56")])
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::SetModalCoordinateSystem, phx::ref(*this), LinuxCNC::csG56) ]
			|	(ascii::no_case[qi::lit("G57")])
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::SetModalCoordinateSystem, phx::ref(*this), LinuxCNC::csG57) ]
			|	(ascii::no_case[qi::lit("G58")])
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::SetModalCoordinateSystem, phx::ref(*this), LinuxCNC::csG58) ]
			|	(ascii::no_case[qi::lit("G59.1")])
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::SetModalCoordinateSystem, phx::ref(*this), LinuxCNC::csG59_1) ]
			|	(ascii::no_case[qi::lit("G59.2")])
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::SetModalCoordinateSystem, phx::ref(*this), LinuxCNC::csG59_2) ]
			|	(ascii::no_case[qi::lit("G59.3")])
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::SetModalCoordinateSystem, phx::ref(*this), LinuxCNC::csG59_3) ]
			|	(ascii::no_case[qi::lit("G59")])
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::SetModalCoordinateSystem, phx::ref(*this), LinuxCNC::csG59) ]
			;


		// A newline character represents an 'end of block' in RS274.  We must have gathered up all of our
		// command line arguments as well as identified the dominant statement type at this point.  Go ahead
		// and generate any graphics that represents the tool's movement as a result of this command.
		EndOfBlock = qi::eol
			[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::ProcessBlock, phx::ref(*this) ) ]	// call this->EndOfBlock()
			;

		// Register statement type values for these.
		G43_1 =(ascii::no_case[qi::lit("G43.1")]) [ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stToolLengthEnabled ) ];
		G43   =(ascii::no_case[qi::lit("G43")])   [ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stToolLengthEnabled ) ];
		G49   =(ascii::no_case[qi::lit("G49")])   [ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stToolLengthDisabled ) ];

		// These won't result in extra graphics but they will change some 'variables' used
		// to store the state of the machine.  These 'variables' will also be used to adjust
		// any position arguments specified in subsequent movement commands.
		ToolLengthOffset =  (G43_1 >> +(MotionArgument))
			|				(G43   >> +(MotionArgument))
			|				(G49)
			;

		Motion =
					(G00 >> +(MotionArgument)) [ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stRapid ) ]
				|	(G01 >> +(MotionArgument)) [ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stFeed ) ]
				|	(G02 >> +(MotionArgument)) [ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stArcClockwise ) ]
				|	(G03 >> +(MotionArgument)) [ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stArcCounterClockwise ) ]
				|	(G04 >> +(MotionArgument)) [ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stFeed ) ]
				|	(ascii::no_case[qi::lit("G80")])

				|	((ascii::no_case[qi::lit("G33.1")]) >> +(MotionArgument))
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stFeed ) ]

				|	((ascii::no_case[qi::lit("G38.2")]) >> +(MotionArgument))
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stProbe ) ]

				|	((ascii::no_case[qi::lit("G38.3")]) >> +(MotionArgument))
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stProbe ) ]

				|	((ascii::no_case[qi::lit("G38.4")]) >> +(MotionArgument))
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stProbe ) ]

				|	((ascii::no_case[qi::lit("G38.5")]) >> +(MotionArgument))
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stProbe ) ]

				|	((ascii::no_case[qi::lit("G33")]) >> +(MotionArgument))
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stFeed ) ]
				
				|	((ascii::no_case[qi::lit("G81")]) >> +(MotionArgument))
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stDrilling ) ]

				|	((ascii::no_case[qi::lit("G82")]) >> +(MotionArgument))
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stDrilling ) ]

				|	((ascii::no_case[qi::lit("G83")]) >> +(MotionArgument))
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stDrilling ) ]

				|	((ascii::no_case[qi::lit("G84")]) >> +(MotionArgument))
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stTapping ) ]

				|	((ascii::no_case[qi::lit("G85")]) >> +(MotionArgument))
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stBoring ) ]

				|	((ascii::no_case[qi::lit("G86")]) >> +(MotionArgument))
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stBoring ) ]

				|	((ascii::no_case[qi::lit("G89")]) >> +(MotionArgument))
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stBoring ) ]
				;

		NonModalCodes =	
					(ascii::no_case[qi::lit("G28.1")]) // Save current position.
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stG28 ) ]

				|	((ascii::no_case[qi::lit("G28")]) >> *(MotionArgument))
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stG28 ) ]

				|	(ascii::no_case[qi::lit("G30.1")]) // Save current position.
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stG30 ) ]

				|	((ascii::no_case[qi::lit("G30")]) >> *(MotionArgument))
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stG30 ) ]

				|	(ascii::no_case[qi::lit("G92.1")])
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stG92_1 ) ]

				|	(ascii::no_case[qi::lit("G92.2")])
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stG92_2 ) ]

				|	(ascii::no_case[qi::lit("G92.3")])
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stG92_3 ) ]

				|	((ascii::no_case[qi::lit("G92")]) >> +(MotionArgument))
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stG92 ) ]

				|	((ascii::no_case[qi::lit("G10")]) >> +(MotionArgument))
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stDataSetting ) ]
				;

		Comment =	(qi::lit("(") >> +(qi::char_ - qi::lit(")")) >> qi::lit(")"))
			;

		PlaneSelection = 
					(ascii::no_case[qi::lit("G17")])
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::Plane, phx::ref(*this), LinuxCNC::eXYPlane ) ]
			|		(ascii::no_case[qi::lit("G18")])
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::Plane, phx::ref(*this), LinuxCNC::eXZPlane ) ]
			|		(ascii::no_case[qi::lit("G19")])
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::Plane, phx::ref(*this), LinuxCNC::eYZPlane ) ]
			;

		PathControlMode = 
					(ascii::no_case[qi::lit("G61.1")])
			|		(ascii::no_case[qi::lit("G61")])
			|		(ascii::no_case[qi::lit("G64")] >> +(MotionArgument))
			;

		// Any time we refer to a floating point number, we use this MathematicalExpression rule instead.  This allows
		// the floating point number to be expressed using literal numbers, mathematical expressions, variable names
		// or any combination of the above.  In order to support all these we store all variables and numbers in a
		// 'symbol table'.   Since we also use this to hold linuxcnc variables by name, we use the 'variables' object
		// for all such values.  The result of any of these expressions is really an integer ID that forms the key
		// into this 'variables' symbol table.
		MathematicalExpression = 
			  (qi::double_) 
				[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::AddSymbol, phx::ref(*this), qi::_val, "", qi::_1 ) ]

			// NOTE: We use _1 and _3 because qi::lit() does not produce an attribute and so it doesn't count as an argument.
			| (qi::lit("[") >> MathematicalExpression >> qi::char_("+") >> MathematicalExpression >> qi::lit("]"))
				[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::LHSplusRHS, phx::ref(*this), qi::_val, qi::_1, qi::_3 ) ]

			| (qi::lit("[") >> MathematicalExpression >> qi::char_("-") >> MathematicalExpression >> qi::lit("]"))
				[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::LHSminusRHS, phx::ref(*this), qi::_val, qi::_1, qi::_3 ) ]

			| (qi::lit("[") >> MathematicalExpression >> qi::char_("*") >> MathematicalExpression >> qi::lit("]"))
				[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::LHStimesRHS, phx::ref(*this), qi::_val, qi::_1, qi::_3 ) ]

			| (qi::lit("[") >> MathematicalExpression >> qi::char_("/") >> MathematicalExpression >> qi::lit("]"))
				[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::LHSdividedbyRHS, phx::ref(*this), qi::_val, qi::_1, qi::_3 ) ]

			| Functions [ qi::_val = qi::_1 ]

			| Variable [ qi::_val = qi::_1 ]

			| (qi::lit("[") >> MathematicalExpression >> qi::lit("]")) [ qi::_val = qi::_1 ]
			;


		// These functions also result in an integer ID which forms the key into the 'variables' symbol table.
		// We use the phoenix::bind method to tie the arguments seen in the GCode to arguments in local methods.  It
		// then calls these methods to accumulate the information.  In any case qi::_val needs to end up
		// being the symbol ID.
		Functions = 
				(ascii::no_case[qi::lit("ABS")] >> qi::lit("[") >> MathematicalExpression >> qi::lit("]"))
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::AbsoluteValue, phx::ref(*this), qi::_val, qi::_1) ] // call this->Abs(_val, _1);

			|	(ascii::no_case[qi::lit("ACOS")] >> qi::lit("[") >> MathematicalExpression >> qi::lit("]"))
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::ACos, phx::ref(*this), qi::_val, qi::_1) ] // call this->ACOS(_val, _1);

			|	(ascii::no_case[qi::lit("ASIN")] >> qi::lit("[") >> MathematicalExpression >> qi::lit("]"))
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::ASin, phx::ref(*this), qi::_val, qi::_1) ] // call this->ASIN(_val, _1);

			|	(ascii::no_case[qi::lit("COS")] >> qi::lit("[") >> MathematicalExpression >> qi::lit("]"))
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::Cos, phx::ref(*this), qi::_val, qi::_1) ] // call this->COS(_val, _1);

			|	(ascii::no_case[qi::lit("EXP")] >> qi::lit("[") >> MathematicalExpression >> qi::lit("]"))
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::Exp, phx::ref(*this), qi::_val, qi::_1) ] // call this->EXP(_val, _1);

			|	(ascii::no_case[qi::lit("FIX")] >> qi::lit("[") >> MathematicalExpression >> qi::lit("]"))
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::Fix, phx::ref(*this), qi::_val, qi::_1) ] // call this->FIX(_val, _1);

			|	(ascii::no_case[qi::lit("FUP")] >> qi::lit("[") >> MathematicalExpression >> qi::lit("]"))
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::Fup, phx::ref(*this), qi::_val, qi::_1) ] // call this->FUP(_val, _1);

			|	(ascii::no_case[qi::lit("ROUND")] >> qi::lit("[") >> MathematicalExpression >> qi::lit("]"))
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::Round, phx::ref(*this), qi::_val, qi::_1) ] // call this->ROUND(_val, _1);

			|	(ascii::no_case[qi::lit("LN")] >> qi::lit("[") >> MathematicalExpression >> qi::lit("]"))
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::Ln, phx::ref(*this), qi::_val, qi::_1) ] // call this->LN(_val, _1);

			|	(ascii::no_case[qi::lit("SIN")] >> qi::lit("[") >> MathematicalExpression >> qi::lit("]"))
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::Sin, phx::ref(*this), qi::_val, qi::_1) ] // call this->SIN(_val, _1);

			|	(ascii::no_case[qi::lit("SQRT")] >> qi::lit("[") >> MathematicalExpression >> qi::lit("]"))
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::Sqrt, phx::ref(*this), qi::_val, qi::_1) ] // call this->SQRT(_val, _1);

			|	(ascii::no_case[qi::lit("TAN")] >> qi::lit("[") >> MathematicalExpression >> qi::lit("]"))
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::Tan, phx::ref(*this), qi::_val, qi::_1) ] // call this->TAN(_val, _1);

			|	(ascii::no_case[qi::lit("ATAN")] >> qi::lit("[") >> MathematicalExpression >> qi::lit("]") >> qi::lit("/") >> qi::lit("[") >> MathematicalExpression >> qi::lit("]"))
				[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::ATan, phx::ref(*this), qi::_val, qi::_1, qi::_2) ] // call this->ATAN(_val, _1, _2);
			;

		// These are similar to the Functions definition above.  All of these (qi::_val) result in a symbol ID that holds
		// the actual value calculated.
		Comparison = 
				(qi::lit("[") >> MathematicalExpression >> ascii::no_case[qi::lit("EQ")] >> MathematicalExpression >> qi::lit("]"))
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::LHSequivalenttoRHS, phx::ref(*this), qi::_val, qi::_1, qi::_2) ]

			|	(qi::lit("[") >> MathematicalExpression >> ascii::no_case[qi::lit("NE")] >> MathematicalExpression >> qi::lit("]"))
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::LHSnotequaltoRHS, phx::ref(*this), qi::_val, qi::_1, qi::_2) ]

			|	(qi::lit("[") >> MathematicalExpression >> ascii::no_case[qi::lit("GT")] >> MathematicalExpression >> qi::lit("]"))
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::LHSgreaterthanRHS, phx::ref(*this), qi::_val, qi::_1, qi::_2) ]
			
			|	(qi::lit("[") >> MathematicalExpression >> ascii::no_case[qi::lit("LT")] >> MathematicalExpression >> qi::lit("]"))
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::LHSlessthanRHS, phx::ref(*this), qi::_val, qi::_1, qi::_2) ]
			;
			

		// We add the Boolean rule so that these constructs are recognised in the grammar but this grammar parsing does not
		// really do program flow.  Maybe at some later date it will but, for now, we just look for tool movement commands
		// directly.  So far we've got away with this by virtue of the fact that the generated GCode does not add subroutine
		// definitions or much program-flow code.
		Boolean = (ascii::no_case[qi::lit("EXISTS")] >> qi::lit("[") >> Variable >> qi::lit("]"))
				[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::Exists, phx::ref(*this), qi::_val, qi::_1) ] // call this->Exists(_val, _1);

			|	(Comparison)
			|	(qi::lit("[") >> Comparison >> ascii::no_case[qi::lit("AND")] >> Comparison >> qi::lit("]"))
			|	(qi::lit("[") >> Comparison >> ascii::no_case[qi::lit("OR")] >> Comparison >> qi::lit("]"))
			|	(qi::lit("[") >> Boolean >> ascii::no_case[qi::lit("OR")] >> Boolean >> qi::lit("]"))
			|	(qi::lit("[") >> Boolean >> ascii::no_case[qi::lit("AND")] >> Boolean >> qi::lit("]"))
			;

		// The 'variable' is added to the 'variables' symbol table by name.  If it's an integer name then the
		// name forms the symbol ID.  If it's a character string variable name then it's converted to an integer
		// via a LinuxCNC::Variables::Hash() method so that we always end up using the same symbol each time
		// this variable name is mentioned in the GCode.  As with all the MathematicalExpressions defined above,
		// this construct results in (i.e. qi::_val) a symbol ID into the 'variables' symbol table.  In the case
		// of the integer variables, it's possible that such symbols were already created when the linuxcnc.var
		// variables file was read in before parsing began.  If they're not found, they will be created though.
		Variable = 
					// #5061
					(qi::lit("#") >> qi::int_)
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::AddIntegerSymbol, phx::ref(*this), qi::_val, qi::_1 ) ]

					// #<myvariable>
			|		(qi::lit("#") >> qi::lit("<") >> +(ascii::no_case[qi::char_("a-z0-9_")]) >> qi::lit(">"))
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::AddSymbolByName, phx::ref(*this), qi::_val, qi::_1 ) ]
			;

		Assignment = 
					(Variable >> qi::lit("=") >> MathematicalExpression)
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::LHSassignmentfromRHS, phx::ref(*this), qi::_val, qi::_1, qi::_2 ) ]
			;
			
		Spindle =	(ascii::no_case[qi::lit("S")] >> MathematicalExpression)
						[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stPreparation ) ]
			;

		FeedRate =	(ascii::no_case[qi::lit("F")] >> MathematicalExpression) [ phx::bind(&linuxcnc_grammar<Iter, Skipper>::SetFeedRate, phx::ref(*this), qi::_1 ) ]
			|		(ascii::no_case[qi::lit("G93")]) [ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stPreparation ) ]
			|		(ascii::no_case[qi::lit("G94")]) [ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stPreparation ) ]
			|		(ascii::no_case[qi::lit("G95")]) [ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stPreparation ) ]
			;

		Units =	
					(ascii::no_case[qi::lit("G20")]) [ phx::bind(&linuxcnc_grammar<Iter, Skipper>::SwitchParseUnits, phx::ref(*this), 0 ) ]
			|		(ascii::no_case[qi::lit("G21")]) [ phx::bind(&linuxcnc_grammar<Iter, Skipper>::SwitchParseUnits, phx::ref(*this), 1 ) ]
			;

		DistanceMode =	
					(ascii::no_case[qi::lit("G90")]) [ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stPreparation ) ]
			|		(ascii::no_case[qi::lit("G91")]) [ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stPreparation ) ]
			;

		MCodes =	((ascii::no_case[qi::lit("M66")]) >> +(MotionArgument))	// M66 - test digital signal
			|		(ascii::no_case[qi::lit("M60")])	// M60
			|		(qi::lexeme[ascii::no_case[qi::lit("M")] >> qi::repeat(0,1)[qi::lit("0")] >> qi::lit("0")])	// M00 - Pause Program
			|		(qi::lexeme[ascii::no_case[qi::lit("M")] >> qi::repeat(0,1)[qi::lit("0")] >> qi::lit("1")])	// M01
			// Add M02 (end of program) as its own rule as its location is quite specific.
			|		(qi::lexeme[ascii::no_case[qi::lit("M")] >> qi::repeat(0,1)[qi::lit("0")] >> qi::lit("3")])	// M03 - turn spindle clockwise
			|		(qi::lexeme[ascii::no_case[qi::lit("M")] >> qi::repeat(0,1)[qi::lit("0")] >> qi::lit("4")])	// M04 - turn spindle counter-clockwise
			|		(qi::lexeme[ascii::no_case[qi::lit("M")] >> qi::repeat(0,1)[qi::lit("0")] >> qi::lit("5")])	// M05 - stop spindle
			|		(qi::lexeme[ascii::no_case[qi::lit("M")] >> qi::repeat(0,1)[qi::lit("0")] >> qi::lit("6")])	// M06 - enable selected tool
			|		(qi::lexeme[ascii::no_case[qi::lit("M")] >> qi::repeat(0,1)[qi::lit("0")] >> qi::lit("7")])	// M07 - mist coolant on
			|		(qi::lexeme[ascii::no_case[qi::lit("M")] >> qi::repeat(0,1)[qi::lit("0")] >> qi::lit("8")])	// M08 - flood coolant on
			|		(qi::lexeme[ascii::no_case[qi::lit("M")] >> qi::repeat(0,1)[qi::lit("0")] >> qi::lit("9")])	// M09 - coolant off
			;

		ProgramFlow = (qi::lexeme[(ascii::no_case[qi::lit("O")] >> qi::int_)] >> (ascii::no_case[qi::lit("IF")] >> Boolean))
			|		  (qi::lexeme[(ascii::no_case[qi::lit("O")] >> qi::int_)] >> (ascii::no_case[qi::lit("ELSE")] ))
			|		  (qi::lexeme[(ascii::no_case[qi::lit("O")] >> qi::int_)] >> (ascii::no_case[qi::lit("ENDIF")] ))
			|		  (qi::lexeme[(ascii::no_case[qi::lit("O")] >> qi::int_)] >> (ascii::no_case[qi::lit("DO")] ))
			|		  (qi::lexeme[(ascii::no_case[qi::lit("O")] >> qi::int_)] >> (ascii::no_case[qi::lit("WHILE")] >> Boolean))
			|		  (qi::lexeme[(ascii::no_case[qi::lit("O")] >> qi::int_)] >> (ascii::no_case[qi::lit("ENDWHILE")] ))
			;

		EndOfProgram = (qi::lexeme[ascii::no_case[qi::lit("M")] >> qi::repeat(0,1)[qi::lit("0")] >> qi::lit("2")])	// M02 - end of program
							[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::ProcessBlock, phx::ref(*this) ) ]	// call this->EndOfBlock()
			;

		ToolSelection = (ascii::no_case[qi::lit("T")] >> qi::int_)
					[ phx::bind(&linuxcnc_grammar<Iter, Skipper>::StatementType, phx::ref(*this),  LinuxCNC::stToolChange ) ]
			;

		RS274 = (LineNumberRule)
			|	(Motion)
			|	(Comment)
			|	(Spindle)
			|	(MCodes)
			|	(NonModalCodes)
			|	(FeedRate)
			|	(Boolean)
			|	(DistanceMode)
			|	(ToolLengthOffset)
			|	(Assignment)
			|	(Units)
			|	(PlaneSelection)
			|	(PathControlMode)
			|	(CoordinateSystems)
			|	(ToolSelection)
			|	(ProgramFlow)
			|	(EndOfProgram)
			;

		// one or more RS274 sequences followed by a newline (end of block)
		Expression = (+(RS274) >> EndOfBlock)
			;

		// This 'Start' rule was passed into the base class during construction via the
		// linuxcnc_grammar::base_type(Start) sequence at the top of this method.  This defines
		// the top-most level of the hierarchy of rules that make up the grammar for the GCode.
		Start = +(Expression)
			;

		// As the boost::spirit::qi parser consumes input, it generates 'trees' of possible
		// matches between the input and the hierarchy of rules that make up the grammar.
		// If it finds a perfect rule to fit the input then that rule's "action" code is called.
		// If it cannot find a perfect rule to fit the input sequence then it generates
		// a std::string that represents the 'tree' of rules along with the character sequences
		// it tried to apply to each.  This std::string was defined by defining the
		// BOOST_SPIRIT_DEBUG_OUT as GCode_GrammarDebugOutputBuffer.  The GCode_GrammarDebugOutputBuffer, in turn,
		// was defined as a std::ostringstream.  All this was done in GCode.h.  This is how
		// we intercept this debug string that's generated for us.  Mostly we can use this
		// to debug the grammar but, even at runtime, it would be helpful to identify
		// which line of GCode could not be interpreted and why.
		// The rules that will appear in this debug string MUST be defined using
		// the BOOST_SPIRIT_DEBUG_NODE macro.  To that end we really want to define
		// ALL the rules that make up this grammar so that, if something goes wrong, we can
		// figure out why.
		//
		// NOTE: This mechanism is 'global' to the application and is, therefore, NOT thread-safe.
		// this might be a motivation to turn it off.  Perhaps we should end up surrounding these
		// definitions in an '#ifdef FC_DEBUG' sequence so that the delivered version is threadsafe
		// while the development version can be debugged.

		BOOST_SPIRIT_DEBUG_NODE(Start);
		BOOST_SPIRIT_DEBUG_NODE(G00);
		BOOST_SPIRIT_DEBUG_NODE(G01);
		BOOST_SPIRIT_DEBUG_NODE(G02);
		BOOST_SPIRIT_DEBUG_NODE(G03);
		BOOST_SPIRIT_DEBUG_NODE(G04);
		BOOST_SPIRIT_DEBUG_NODE(Motion);
		BOOST_SPIRIT_DEBUG_NODE(MotionArgument);
		BOOST_SPIRIT_DEBUG_NODE(LineNumberRule);
		BOOST_SPIRIT_DEBUG_NODE(MathematicalExpression);
		BOOST_SPIRIT_DEBUG_NODE(Assignment);
		BOOST_SPIRIT_DEBUG_NODE(Functions);
		BOOST_SPIRIT_DEBUG_NODE(Comment);
		BOOST_SPIRIT_DEBUG_NODE(RS274);
		BOOST_SPIRIT_DEBUG_NODE(Expression);
		BOOST_SPIRIT_DEBUG_NODE(Spindle);
		BOOST_SPIRIT_DEBUG_NODE(MCodes);
		BOOST_SPIRIT_DEBUG_NODE(CoordinateSystems);
		BOOST_SPIRIT_DEBUG_NODE(EndOfBlock);
		BOOST_SPIRIT_DEBUG_NODE(EndOfProgram);
		BOOST_SPIRIT_DEBUG_NODE(FeedRate);
		BOOST_SPIRIT_DEBUG_NODE(NonModalCodes);
		BOOST_SPIRIT_DEBUG_NODE(Boolean);
		BOOST_SPIRIT_DEBUG_NODE(Variable);
		BOOST_SPIRIT_DEBUG_NODE(Comparison);
		BOOST_SPIRIT_DEBUG_NODE(G43);
		BOOST_SPIRIT_DEBUG_NODE(G43_1);
		BOOST_SPIRIT_DEBUG_NODE(G49);
		BOOST_SPIRIT_DEBUG_NODE(DistanceMode);
		BOOST_SPIRIT_DEBUG_NODE(Units);
		BOOST_SPIRIT_DEBUG_NODE(ToolLengthOffset);
		BOOST_SPIRIT_DEBUG_NODE(PlaneSelection);
		BOOST_SPIRIT_DEBUG_NODE(PathControlMode);
		BOOST_SPIRIT_DEBUG_NODE(ToolSelection);
		BOOST_SPIRIT_DEBUG_NODE(ProgramFlow);
	}

	public:
		// Define all of our member variables used both to define this grammar
		// and to be used during the execution of this grammar.  By keeping
		// such variables local we allow for thread-safe operation.

		qi::rule<Iter, Skipper> Start;
		qi::rule<Iter, Skipper> G00;
		qi::rule<Iter, Skipper> G01, G02, G03, G04, G43, G43_1, G49;
		qi::rule<Iter, Skipper> G80;
		qi::rule<Iter, Skipper> G83;
		qi::rule<Iter, Skipper> Motion, ToolLengthOffset, DistanceMode, Units;
		qi::rule<Iter, Skipper> MotionArgument;
		qi::rule<Iter, int(), Skipper> LineNumberRule;
		qi::rule<Iter, LinuxCNC::Variables::SymbolId_t(), Skipper> MathematicalExpression, Variable, Functions, Assignment;
		qi::rule<Iter, Skipper> Comment;
		qi::rule<Iter, Skipper> RS274;
		qi::rule<Iter, Skipper> Expression;
		qi::rule<Iter, Skipper> Spindle, FeedRate, PlaneSelection, PathControlMode, ToolSelection;
		qi::rule<Iter, Skipper> MCodes, NonModalCodes, ProgramFlow, CoordinateSystems;
		qi::rule<Iter, Skipper> EndOfBlock, EndOfProgram;
		qi::rule<Iter, bool(), Skipper> Boolean, Comparison;

		LinuxCNC	*pLinuxCNC;	// pointer to owning object into which we accumulate results.
		MachineProgram *machine_program;

		double previous[9]; // in parse_units

		LinuxCNC::eCoordinateSystems_t current_coordinate_system;
		LinuxCNC::eCoordinateSystems_t modal_coordinate_system;

		// This doesn't change the generated graphics but, if it has not been set, it shows invalid GCode. 
		// eg: if we see a G01 (feed) movement without having seen a feedrate definition first.
		double feed_rate;
		double spindle_speed;
		double units;
		double tool_length_offset;

		int	 line_offset;			// Which line in the GCode file are we processing?  i.e. index into ToolPath::toolpath (QStringList)
		int  line_number;			// GCode line number.  i.e. N30, N40 etc.  Used for any error/warning messages presented to the user.

		LinuxCNC::ePlane_t plane;	// XY, YZ or XZ.  Used as a 'modal' setting upon which arc movements are defined.

		// The statement type defines both the colour of the GCode text and the type of
		// motion used.  It's the key into the ProcessBlock() method that's used to define which sequence of graphics
		// represents the tool's movement.
		LinuxCNC::eStatement_t statement_type;
		LinuxCNC::eStatement_t previous_statement_type;	// from previous block. kept for modal commands such as G01 or drilling cycles etc.
		
		LinuxCNC::Variables	variables;	// Symbol table that holds both GCode variables and any transient numbers found in the program.

		LinuxCNC::eUnits_t m_emc2_variables_units;

		// Argument values specified within this block.  These are in 'parse units'.
		double x;
		double y;
		double z;
		double a;
		double b;
		double c;
		double u;
		double v;
		double w;
		double i;
		double j;
		double k;
		double l;
		double p;
		double q;
		double r;

		// True/False flag indicating whether each argument was mentioned within this block.
		int x_specified;
		int y_specified;
		int z_specified;
		int a_specified;
		int b_specified;
		int c_specified;
		int u_specified;
		int v_specified;
		int w_specified;
		int i_specified;
		int j_specified;
		int k_specified;
		int l_specified;
		int p_specified;
		int q_specified;
		int r_specified;

	private:
		// Define the series of methods that form 'actions' that are called when various inputs accurately match
		// a rule.  Many of these will be of the form;
		//
		// void myMethod(Symbol_t & returned_symbol_id, int arg1, double arg2).
		//
		// This is so that we can use the phoenix::bind() method to call them from the grammar's "action" sequence.  We
		// use the phoenix::bind() method to handle the type conversions between the arguments found in the grammar
		// parsing and those used in these methods.
		// In our exmaple above, the 'action' would be something like;
		//
		// [ phx::bind(&linuxcnc_grammar<Iter, Skipper>::myMethod, phx::ref(*this), qi::_val, qi::_1, qi::_2) ]
		// It's a cryptic way of getting the "action" code to call qi::_val = this->myMethod(int(qi::_1), double(qi::_2))
		// The qi::_val is always the return type defined as part of the rule's declaration.  eg: this rule is
		// returning an integer symbol ID so it would be defined something lile;
		//
		// qi::rule<Iter, LinuxCNC::Variables::SymbolId_t(), Skipper> MyRule;
		// i.e. the second argument to the template indicates the 'type' returned by the rule.  This defines the type of qi::_val
		

		// EMC2 units are defined via a setting (TODO Make this statement true)
		// It defines the units for which values in the 'variables' symbol table
		// are relevant.
		double Emc2Units(const double value_in_parse_units)
		{
			double value_in_mm = value_in_parse_units * this->units;

			switch (this->m_emc2_variables_units)
			{
				case LinuxCNC::eImperial:
					return(value_in_mm / 25.4); // imperial - inches

				case LinuxCNC::eMetric:
				default:
					return(value_in_mm);
			}
		}

		void Debug(const char *message)
		{
			qDebug("%s\n", message);
		}

		// Heeks uses millimeters
		double HeeksUnits( const double value_in_emc2_units )
		{
			switch (this->m_emc2_variables_units)
			{
				case LinuxCNC::eImperial:
					return(value_in_emc2_units * 25.4);


				case LinuxCNC::eMetric:
				default:
					return(value_in_emc2_units);
			}
		}

		// The Parse Units are defined by any G20 or G21 commands seen.  i.e. they're the
		// units for which the various command arguments are specified in.
		double ParseUnits( const double value_in_emc2_units )
		{
			double value_in_mm = HeeksUnits( value_in_emc2_units );
			return( value_in_mm / this->units );
		}

		double ParseUnitsFromHeeksUnits( const double value_in_heeks_units )
		{
			double value_in_mm = value_in_heeks_units;	// always.
			return( value_in_mm / this->units );
		}


		/**
			When the GCode switches from metric to imperial (or vice-versa) we need
			to change the values in out ParseState_t structure to the new standards
			so that we're comparing 'apples with apples' when interpreting
			subsequent coordinates.
		 */
		void SwitchParseUnits( const int metric )
		{
			if (metric)
			{
				if (this->units == 25.4)
				{
					// Need to adjust the existing settings from imperial to metric.

					this->x *= 25.4;
					this->y *= 25.4;
					this->z *= 25.4;

					/*
					Do not adjust a,b or c values as these are rotational units (degrees) rather than linear units
					this->a *= 25.4;
					this->b *= 25.4;
					this->c *= 25.4;
					*/

					this->u *= 25.4;
					this->v *= 25.4;
					this->w *= 25.4;

					this->i *= 25.4;
					this->j *= 25.4;
					this->k *= 25.4;

					this->l *= 25.4;
					this->p *= 25.4;
					this->q *= 25.4;
					this->r *= 25.4;

					for (::size_t i=0; i<sizeof(this->previous)/sizeof(this->previous[0]); i++)
					{
						this->previous[i] *= 25.4;
					}

					this->feed_rate *= 25.4;
				}

				this->units = 1.0;  // Switch to metric
			}
			else
			{
				if (this->units == 1.0)
				{
					// Need to adjust the existing settings from metric to imperial.

					this->x /= 25.4;
					this->y /= 25.4;
					this->z /= 25.4;

					/*
					Do not adjust a,b or c values as these are rotational units (degrees) rather than linear units
					this->a /= 25.4;
					this->b /= 25.4;
					this->c /= 25.4;
					*/

					this->u /= 25.4;
					this->v /= 25.4;
					this->w /= 25.4;

					this->i /= 25.4;
					this->j /= 25.4;
					this->k /= 25.4;

					this->l /= 25.4;
					this->p /= 25.4;
					this->q /= 25.4;
					this->r /= 25.4;

					for (::size_t i=0; i<sizeof(this->previous)/sizeof(this->previous[0]); i++)
					{
						this->previous[i] /= 25.4;
					}

					this->feed_rate /= 25.4;
				}

				this->units = 25.4;  // Switch to imperial
			}
		}

		void Plane( LinuxCNC::ePlane_t plane )
		{
			this->plane = plane;
		}

		void AddSymbol( LinuxCNC::Variables::SymbolId_t & returned_symbol_id, const char *name, const double value )
		{
			if ((name == NULL) || (*name == '\0'))
			{
				int id = variables.new_id();
				variables[id] = value;
				returned_symbol_id = id;
				return;
			}
			else
			{
				double unused = variables[name];	// This either finds or creates an entry.
				returned_symbol_id = variables.hash(name);
				return;
			}
		}

		void SetCurrentCoordinateSystem( LinuxCNC::eCoordinateSystems_t coordinate_system )
		{
			this->current_coordinate_system = coordinate_system;
		}

		void SetModalCoordinateSystem( LinuxCNC::eCoordinateSystems_t coordinate_system )
		{
			this->modal_coordinate_system = coordinate_system;
		}

		// The +(qi::char_) rule produces a std::vector<char> argument.  Convert that into
		// a std::string and then add the rule 'by name'.  i.e. use the Variables::Hash() method
		// to produce an integer that represents this 'name' and add the symbol against
		// that integer.  Return the integer used as the symbol ID in the returned_symbol_id
		// argument so that it's assigned to the qi::_val variable within the grammar.
		void AddSymbolByName( LinuxCNC::Variables::SymbolId_t & returned_symbol_id, std::vector<char> name )
		{
			std::string _name;
			std::copy( name.begin(), name.end(), std::inserter( _name, _name.begin() ) );
			AddSymbol( returned_symbol_id, _name.c_str(), 0.0 );
		}

		void AddIntegerSymbol( LinuxCNC::Variables::SymbolId_t & returned_symbol_id, const int value )
		{
			int id = variables.new_id();
			variables[id] = double(value);
			returned_symbol_id = id;
		}


		// Either find an existing symbol with this name and return it's ID or
		// add a new symbol with this name.
		int Symbol( const char *name )
		{
			if ((name == NULL) || (*name == '\0'))
			{
				int id = variables.new_id();
				variables[id] = 0.0;
				return(id);
			}
			else
			{
				double unused = variables[name];	// This either finds or creates an entry.
				return(variables.hash(name));
			}
		}

		


		void LHSequivalenttoRHS(bool & returned_boolean, const LinuxCNC::Variables::SymbolId_t lhs, const LinuxCNC::Variables::SymbolId_t rhs)
		{
			returned_boolean = ( variables[lhs] == variables[rhs] );
		}

		void LHSnotequaltoRHS(bool & returned_boolean, const LinuxCNC::Variables::SymbolId_t lhs, const LinuxCNC::Variables::SymbolId_t rhs)
		{
			returned_boolean = ( variables[lhs] != variables[rhs] );
		}

		void LHSgreaterthanRHS(bool & returned_boolean, const LinuxCNC::Variables::SymbolId_t lhs, const LinuxCNC::Variables::SymbolId_t rhs)
		{
			returned_boolean = ( variables[lhs] > variables[rhs] );
		}

		void LHSlessthanRHS(bool & returned_boolean, const LinuxCNC::Variables::SymbolId_t lhs, const LinuxCNC::Variables::SymbolId_t rhs)
		{
			returned_boolean = ( variables[lhs] < variables[rhs] );
		}


		void LHSplusRHS(LinuxCNC::Variables::SymbolId_t & returned_symbol_id, const LinuxCNC::Variables::SymbolId_t lhs, const LinuxCNC::Variables::SymbolId_t rhs)
		{
			int id = variables.new_id();
			variables[id] = variables[lhs] + variables[rhs];
			returned_symbol_id = id;
		}

		void LHSminusRHS(LinuxCNC::Variables::SymbolId_t & returned_symbol_id, const LinuxCNC::Variables::SymbolId_t lhs, const LinuxCNC::Variables::SymbolId_t rhs)
		{
			int id = variables.new_id();
			variables[id] = variables[lhs] - variables[rhs];
			returned_symbol_id = id;
		}

		void LHStimesRHS(LinuxCNC::Variables::SymbolId_t & returned_symbol_id, const LinuxCNC::Variables::SymbolId_t lhs, const LinuxCNC::Variables::SymbolId_t rhs)
		{
			int id = variables.new_id();
			variables[id] = variables[lhs] * variables[rhs];
			returned_symbol_id = id;
		}

		void LHSdividedbyRHS(LinuxCNC::Variables::SymbolId_t & returned_symbol_id, const LinuxCNC::Variables::SymbolId_t lhs, const LinuxCNC::Variables::SymbolId_t rhs)
		{
			int id = variables.new_id();
			if (variables[rhs] == 0.0)
			{
				// We don't want to get into too much trouble just for backplotting.
				variables[id] = DBL_MAX;
			}
			else
			{
				variables[id] = variables[lhs] / variables[rhs];
			}

			returned_symbol_id = id;
		}


		void LHSassignmentfromRHS( LinuxCNC::Variables::SymbolId_t & returned_symbol_id, const LinuxCNC::Variables::SymbolId_t lhs, const LinuxCNC::Variables::SymbolId_t rhs )
		{
			variables[lhs] = variables[rhs];
			returned_symbol_id = lhs;
		}


		double Value(const LinuxCNC::Variables::SymbolId_t name)
		{
			return( variables[name] );
		}


		double radians_to_degrees( const double radians )
		{
			double a = radians;
			return( (radians / (2.0 * M_PI)) * 360.0 );
		}

		double degrees_to_radians( const double degrees )
		{
			double a = degrees;
			return( (degrees / 360.0) * (2.0 * M_PI) );
		}


		void ASin(LinuxCNC::Variables::SymbolId_t & returned_symbol_id, const LinuxCNC::Variables::SymbolId_t symbol_id)
		{
			int id=variables.new_id();
			variables[id] = radians_to_degrees( asin(degrees_to_radians(variables[symbol_id] )) );
			returned_symbol_id = id;
		}

		void ACos(LinuxCNC::Variables::SymbolId_t & returned_symbol_id, const LinuxCNC::Variables::SymbolId_t symbol_id)
		{
			int id=variables.new_id();
			variables[id] = radians_to_degrees( acos(degrees_to_radians(variables[symbol_id] )) );
			returned_symbol_id = id;
		}

		void ATan(LinuxCNC::Variables::SymbolId_t & returned_symbol_id, const LinuxCNC::Variables::SymbolId_t lhs, const LinuxCNC::Variables::SymbolId_t rhs)
		{
			int id=variables.new_id();
			double radians = atan2(variables[lhs], variables[rhs]);
			variables[id] = radians_to_degrees( radians );
			returned_symbol_id = id;
		}

		void Sin(LinuxCNC::Variables::SymbolId_t & returned_symbol_id, const LinuxCNC::Variables::SymbolId_t symbol_id)
		{
			int id=variables.new_id();
			variables[id] = radians_to_degrees( sin(degrees_to_radians(variables[symbol_id] )) );
			returned_symbol_id = id;
		}

		void Cos(LinuxCNC::Variables::SymbolId_t & returned_symbol_id, const LinuxCNC::Variables::SymbolId_t symbol_id)
		{
			int id=variables.new_id();
			variables[id] = radians_to_degrees( cos(degrees_to_radians(variables[symbol_id] )) );
			returned_symbol_id = id;
		}

		void Tan(LinuxCNC::Variables::SymbolId_t & returned_symbol_id, const LinuxCNC::Variables::SymbolId_t symbol_id)
		{
			int id=variables.new_id();
			variables[id] = radians_to_degrees( tan(degrees_to_radians(variables[symbol_id] )) );
			returned_symbol_id = id;
		}

		void AbsoluteValue(LinuxCNC::Variables::SymbolId_t & returned_symbol_id, const LinuxCNC::Variables::SymbolId_t symbol_id)
		{
			int id=variables.new_id();
			variables[id] = abs(variables[symbol_id]);
			returned_symbol_id = id;
		}

		void	Sqrt(LinuxCNC::Variables::SymbolId_t & returned_symbol_id, const LinuxCNC::Variables::SymbolId_t symbol_id)
		{
			int id=variables.new_id();
			variables[id] = sqrt(variables[symbol_id]);
			returned_symbol_id = id;
		}

		void	Exp(LinuxCNC::Variables::SymbolId_t & returned_symbol_id, const LinuxCNC::Variables::SymbolId_t symbol_id)
		{
			int id=variables.new_id();
			variables[id] = exp((variables[symbol_id]));
			returned_symbol_id = id;
		}

		void	Fix(LinuxCNC::Variables::SymbolId_t & returned_symbol_id, const LinuxCNC::Variables::SymbolId_t symbol_id)
		{
			int id=variables.new_id();
			variables[id] = int(floor(((variables[symbol_id]))));
			returned_symbol_id = id;
		}

		void	Fup(LinuxCNC::Variables::SymbolId_t & returned_symbol_id, const LinuxCNC::Variables::SymbolId_t symbol_id)
		{
			int id=variables.new_id();
			variables[id] = int(ceil(((variables[symbol_id]))));
			returned_symbol_id = id;
		}

		void	Round(LinuxCNC::Variables::SymbolId_t & returned_symbol_id, const LinuxCNC::Variables::SymbolId_t symbol_id)
		{
			int id=variables.new_id();
			variables[id] = int(((variables[symbol_id])));
			returned_symbol_id = id;
		}

		void	Ln(LinuxCNC::Variables::SymbolId_t & returned_symbol_id, const LinuxCNC::Variables::SymbolId_t symbol_id)
		{
			int id=variables.new_id();
			variables[id] = log(variables[symbol_id]);
			returned_symbol_id = id;
		}

		void Exists(bool return_value, const LinuxCNC::Variables::SymbolId_t symbol_id)
		{
			return_value = variables.exists(symbol_id);
		}

		void SetFeedRate(LinuxCNC::Variables::SymbolId_t id)
		{
			this->feed_rate = variables[id];
			StatementType( LinuxCNC::stPreparation );
		}

		/**
			Adjust the value for the active coordinate system.  The parameter_offset is 0 for x, 1 for y etc.
			We want to arrange the GCode so that the G54 (the first relative coordinate system) lines up
			with the HeeksCAD drawing coordinate system.  All other coordinates are moved around that
			relative position.  We're using the fact that the gcode_variables map was initialized
			from the emc.var file to set the offsets of the various coordinate systems.

			The parameter_offset is added to the modal_coordinate_system base number (eg: 5221 for G54)
			to indicate which variable within that coordinate system we're referring to.  eg: 0 = x,
			1 = y etc.
		 */
		double adjust( const LinuxCNC::Variables::SymbolId_t parameter_offset, const double value_in_parse_units )
		{
			double value_in_emc2_units = Emc2Units(value_in_parse_units);
			double tool_length_offset = 0.0;

			if (parameter_offset == 2)
			{
				// We're adjusting a Z coordinate so add the tool length offset
				tool_length_offset = this->tool_length_offset;
			}

			if (parameter_offset >= 9)
			{
				qDebug("Parameter offset must be less than 9.  It is %d instead\n", parameter_offset);
				return( HeeksUnits(value_in_emc2_units) );
			}

			double g54_offset = variables[LinuxCNC::eG54VariableBase + parameter_offset];
			double g92_offset = variables[LinuxCNC::eG92VariableBase + parameter_offset];

			if (variables[LinuxCNC::eG92Enabled] > 0.0)
			{
				// Copy the values from the gcode_variables into the local cache.
				return(HeeksUnits(value_in_emc2_units - g54_offset + g92_offset + tool_length_offset));
			}

			if (this->current_coordinate_system == LinuxCNC::csG53)
			{
				// We need to move the point so that it's relavitve to the eG54 coordinate system.
				return(HeeksUnits(value_in_emc2_units - g54_offset + tool_length_offset));
			}

			int coordinate_system_offset = LinuxCNC::eG54VariableBase + ((this->modal_coordinate_system - 1) * 20);
			int name = coordinate_system_offset + parameter_offset;

			return(HeeksUnits(value_in_emc2_units - g54_offset + variables[name] + tool_length_offset));
		}


		double PX()
		{
			if (pLinuxCNC->machine_location.get())
			{
				return( adjust(0, pLinuxCNC->machine_location->X()) );
			}
			else
			{
				double x = adjust(0, this->previous[0]);
				return(x);
			}
		}

		double X()
		{
			double x = PX();
			if (this->x_specified) x = adjust(0, this->x);
			return(x);
		}

		double PY()
		{
			if (pLinuxCNC->machine_location.get())
			{
				return( adjust(1, pLinuxCNC->machine_location->Y()) );
			}
			else
			{
				double y = adjust(1, this->previous[1]);
				return(y);
			}
		}

		double Y()
		{
			double y = PY();
			if (this->y_specified) y = adjust(1, this->y);
			return(y);
		}

		double PZ()
		{
			if (pLinuxCNC->machine_location.get())
			{
				return( adjust(2, pLinuxCNC->machine_location->Z()) );
			}
			else
			{
				double z = adjust(2, this->previous[2]);
				return(z);
			}
		}

		double Z()
		{
			double z = PZ();
			if (this->z_specified) z = adjust(2, this->z);
			return(z);
		}

		double R()
		{
			double r = HeeksUnits(Emc2Units(this->r));
			return(r);
		}

		Cam::Point PreviousLocation()
		{
			return(Cam::Point( PX(), PY(), PZ() ));
		}

		/**
			Use the previous location's X, Y and Z values initially and then
			replace each with those specified within this command.  If each is
			not specified then the equivalent previous value is used.  This allows
			for modal commands where all of the X, Y and Z values are NOT specified
			on every command line.
		 */
		Cam::Point NewLocation()
		{
			return(Cam::Point( X(), Y(), Z() ));
		}

		void UpdateMachineLocation()
		{
			if (this->x_specified) pLinuxCNC->machine_location->X(this->x);
			if (this->y_specified) pLinuxCNC->machine_location->Y(this->y);
			if (this->z_specified) pLinuxCNC->machine_location->Z(this->z);

			if (this->a_specified) pLinuxCNC->machine_location->A(this->a);
			if (this->b_specified) pLinuxCNC->machine_location->B(this->b);
			if (this->c_specified) pLinuxCNC->machine_location->C(this->c);

			if (this->u_specified) pLinuxCNC->machine_location->U(this->u);
			if (this->v_specified) pLinuxCNC->machine_location->V(this->v);
			if (this->w_specified) pLinuxCNC->machine_location->W(this->w);
		}

		/**
			We've accumulated all the command line arguments as well as the statement
			type for this line (block).  Go ahead and adjust any of the command line
			arguments based on current machine settings and generate the graphics
			that represent this tool movement.
		 */
		void ProcessBlock()
		{
			if (! this->machine_program) return;
			QStringList *pPython = this->machine_program->getMachineProgram();
			if (pPython == NULL) return;

			if (this->statement_type == LinuxCNC::stUndefined)
			{
				switch (this->previous_statement_type)
				{
					// If it's one of the 'modal' commands then we don't actually NEED to see
					// that command again in the next block.  We could just be given the next
					// set of coordinates for the same command.  In this case, keep the
					// statement type as it is.  If another is seen, it will be overridden.
					case LinuxCNC::stProbe:
					case LinuxCNC::stFeed:
					case LinuxCNC::stArcClockwise:
					case LinuxCNC::stArcCounterClockwise:
					case LinuxCNC::stDrilling:
					case LinuxCNC::stBoring:
					case LinuxCNC::stTapping:
						this->statement_type = this->previous_statement_type;	// Reinstate the previous statement type.
					break;
				}
			} // End if - then


			if (::size_t(this->line_offset) < ::size_t(this->machine_program->getMachineProgram()->size()))
			{
				GCode::ToolMovement graphics(this->machine_program);
				graphics.Index( this->line_offset );
				graphics.CoordinateSystem(this->modal_coordinate_system);

				switch (this->statement_type)
				{
				case LinuxCNC::stUndefined:
					break;

				case LinuxCNC::stDataSetting:
					// The G10 statement has been specified.  Look at the L argument to see what needs to be set.
					if (this->l_specified && this->p_specified)
					{
						if (this->l == 20)
						{
							// We need to make the current coordinate be whatever the arguments say they are.  i.e.
							// adjust the appropriate coordinate system offset accordingly.

							int coordinate_system = this->p;

							if (this->x_specified)
							{
								int parameter_offset = 0;	// x
								int coordinate_system_offset = LinuxCNC::eG54VariableBase + ((this->p - 1) * 20);
								int name = coordinate_system_offset + parameter_offset;
								double offset_in_heeks_units = HeeksUnits( variables[name] );
								double offset_in_emc2_units = Emc2Units( this->x ) - variables[name];
								variables[name] = variables[name] + offset_in_emc2_units;
							}

							if (this->y_specified)
							{
								int parameter_offset = 1;	// y
								int coordinate_system_offset = LinuxCNC::eG54VariableBase + ((this->p - 1) * 20);
								int name = coordinate_system_offset + parameter_offset;
								double offset_in_heeks_units = HeeksUnits( variables[name] );
								double offset_in_emc2_units = Emc2Units( this->y ) - variables[name];
								variables[name] = variables[name] + offset_in_emc2_units;
							}

							if (this->z_specified)
							{
								int parameter_offset = 2;	// z
								int coordinate_system_offset = LinuxCNC::eG54VariableBase + ((this->p - 1) * 20);
								int name = coordinate_system_offset + parameter_offset;
								double offset_in_heeks_units = HeeksUnits( variables[name] );
								double offset_in_emc2_units = Emc2Units( this->z ) - variables[name];
								variables[name] = variables[name] + offset_in_emc2_units;
							}
						}
					}
					break;

				case LinuxCNC::stToolLengthEnabled:
					// The Z parameters given determine where we should think
					// we are right now.
					// this->tool_length_offset = this->k - ParseUnits(variables[eG54VariableBase+2]);
					/*
					xml << _T("<path col=\"rapid\" fixture=\"") << int(this->modal_coordinate_system) << _T("\">\n")
						<< _T("<line z=\"") << adjust(2,this->z) << _T("\" ")
						<< _T("/>\n")
						<< _T("</path>\n");
						*/
					break;

				case LinuxCNC::stToolLengthDisabled:
					// The Z parameters given determine where we should think
					// we are right now.
					this->tool_length_offset = 0.0;
					/*
					xml << _T("<path col=\"rapid\" fixture=\"") << int(this->modal_coordinate_system) << _T("\">\n")
						<< _T("<line z=\"") << adjust(2,this->previous[2]) << _T("\" ")
						<< _T("/>\n")
						<< _T("</path>\n");
						*/
					break;

				case LinuxCNC::stRapid:
					if (pLinuxCNC->machine_location.get() == NULL)
					{
						pLinuxCNC->machine_location = std::auto_ptr<GCode::MachineLocation>(new GCode::MachineLocation());
						UpdateMachineLocation();
					}
					else
					{
						// Create a line that represents this rapid movement.
						GCode::SingleCommandGeometry_t movement;
						GCode::ToolMovement step(graphics);
						step.Edge( Cam::Edge( PreviousLocation(), NewLocation() ) );
						step.Type( GCode::ToolMovement::eRapid );
						movement.push_back(step);
						pLinuxCNC->geometry.insert( std::make_pair(this->line_offset, movement) );
						UpdateMachineLocation();

						/*
						xml << _T("<path col=\"rapid\" fixture=\"") << int(this->modal_coordinate_system) << _T("\">\n")
							<< _T("<line ");
						if (this->x_specified) xml << _T("x=\"") << adjust(0,this->x) << _T("\" ");
						if (this->y_specified) xml << _T("y=\"") << adjust(1,this->y) << _T("\" ");
						if (this->z_specified) xml << _T("z=\"") << adjust(2,this->z) << _T("\" ");
						xml << _T("/>\n")
							<< _T("</path>\n");
							*/
					}
					break;

				case LinuxCNC::stFeed:
					if (pLinuxCNC->machine_location.get() == NULL)
					{
						pLinuxCNC->machine_location = std::auto_ptr<GCode::MachineLocation>(new GCode::MachineLocation());
						UpdateMachineLocation();
					}
					else
					{
						GCode::SingleCommandGeometry_t movement;
						GCode::ToolMovement step(graphics);
						step.Edge( Cam::Edge( PreviousLocation(), NewLocation() ) );
						step.Type( GCode::ToolMovement::eFeed );
						movement.push_back(step);
						pLinuxCNC->geometry.insert( std::make_pair(this->line_offset, movement) );
						UpdateMachineLocation();
						
						/*
						xml << _T("<path col=\"feed\" fixture=\"") << int(this->modal_coordinate_system) << _T("\">\n")
							<< _T("<line ");
						if (this->x_specified) xml << _T("x=\"") << adjust(0,this->x) << _T("\" ");
						if (this->y_specified) xml << _T("y=\"") << adjust(1,this->y) << _T("\" ");
						if (this->z_specified) xml << _T("z=\"") << adjust(2,this->z) << _T("\" ");
						xml << _T("/>\n")
							<< _T("</path>\n");
							*/
						if (this->feed_rate <= 0.0) 
						{
							QString warning;
							warning = QString::fromAscii("Zero feed rate found for feed movement - line ") + this->line_number;
							pLinuxCNC->AddWarning(warning);
						}
					}
					break;

				case LinuxCNC::stProbe:
					if (pLinuxCNC->machine_location.get() == NULL)
					{
						pLinuxCNC->machine_location = std::auto_ptr<GCode::MachineLocation>(new GCode::MachineLocation());
						UpdateMachineLocation();
					}
					else
					{
						GCode::SingleCommandGeometry_t movement;
						GCode::ToolMovement step(graphics);
						step.Edge( Cam::Edge( PreviousLocation(), NewLocation() ) );
						step.Type( GCode::ToolMovement::eFeed );
						movement.push_back(step);
						pLinuxCNC->geometry.insert( std::make_pair(this->line_offset, movement) );
						UpdateMachineLocation();
					}
					/*
					xml << _T("<path col=\"feed\" fixture=\"") << int(this->modal_coordinate_system) << _T("\">\n")
						<< _T("<line ");
					if (this->x_specified) xml << _T("x=\"") << adjust(0,this->x) << _T("\" ");
					if (this->y_specified) xml << _T("y=\"") << adjust(1,this->y) << _T("\" ");
					if (this->z_specified) xml << _T("z=\"") << adjust(2,this->z) << _T("\" ");
					xml << _T("/>\n")
						<< _T("</path>\n");
						*/

					// Assume that the furthest point of probing tripped the switch.  Store this location
					// as though we found our probed object here.
					variables[LinuxCNC::eG38_2VariableBase + 0] = ParseUnitsFromHeeksUnits(adjust(0,this->x));
					variables[LinuxCNC::eG38_2VariableBase + 1] = ParseUnitsFromHeeksUnits(adjust(1,this->y));
					variables[LinuxCNC::eG38_2VariableBase + 2] = ParseUnitsFromHeeksUnits(adjust(2,this->z));

					variables[LinuxCNC::eG38_2VariableBase + 3] = adjust(3,this->a);
					variables[LinuxCNC::eG38_2VariableBase + 4] = adjust(4,this->b);
					variables[LinuxCNC::eG38_2VariableBase + 5] = adjust(5,this->c);

					variables[LinuxCNC::eG38_2VariableBase + 6] = ParseUnitsFromHeeksUnits(adjust(6,this->u));
					variables[LinuxCNC::eG38_2VariableBase + 7] = ParseUnitsFromHeeksUnits(adjust(7,this->v));
					variables[LinuxCNC::eG38_2VariableBase + 8] = ParseUnitsFromHeeksUnits(adjust(8,this->w));

					if (this->feed_rate <= 0.0) 
					{
						QString warning;
						warning = QString::fromAscii("Zero feed rate found for probe movement - line ") + this->line_number;
						pLinuxCNC->AddWarning(warning);
					}
					break;

				case LinuxCNC::stArcClockwise:
					if (pLinuxCNC->machine_location.get() == NULL)
					{
						pLinuxCNC->machine_location = std::auto_ptr<GCode::MachineLocation>(new GCode::MachineLocation());
						UpdateMachineLocation();
					}
					else
					{
						GCode::SingleCommandGeometry_t movement;
						GCode::ToolMovement step(graphics);

						gp_Pnt start(PreviousLocation().Location());
						gp_Pnt end(NewLocation().Location());
						gp_Pnt centre(adjust(0, this->i + this->previous[0]), adjust(1, this->j + this->previous[1]), adjust(2, this->k + this->previous[2]));

						gp_Pnt orthogonal_start;
						gp_Pnt orthogonal_centre;

						gp_Circ circle;
						switch (this->plane)
						{
						case GCode::eXYPlane:
							orthogonal_start  = gp_Pnt(adjust(0, this->previous[0]),    adjust(1, this->previous[1]),  adjust(2, 0.0));
							orthogonal_centre = gp_Pnt(adjust(0, this->i + this->previous[0]),	adjust(1, this->j + this->previous[1]),	adjust(2, 0.0));
							circle.SetAxis(gp_Ax1(gp_Pnt(0.0,0.0,0.0), gp_Dir(0.0, 0.0, -1.0)));
							break;

						case GCode::eXZPlane:
							orthogonal_start  = gp_Pnt(adjust(0, this->previous[0]), adjust(1, 0.0), adjust(2, this->previous[2]));
							orthogonal_centre = gp_Pnt(adjust(0, this->i + this->previous[0]),	adjust(1, 0.0), adjust(2, this->k + this->previous[2]));
							circle.SetAxis(gp_Ax1(gp_Pnt(0.0,0.0,0.0), gp_Dir(0.0, -1.0, 0.0)));
							break;

						case GCode::eYZPlane:
							orthogonal_start  = gp_Pnt(adjust(0, 0.0), adjust(1, this->previous[1]), adjust(2, this->previous[2]));
							orthogonal_centre = gp_Pnt(adjust(0, 0.0), adjust(1, this->j + this->previous[1]),	adjust(2, this->k + this->previous[2]));
							circle.SetAxis(gp_Ax1(gp_Pnt(0.0,0.0,0.0), gp_Dir(-1.0, 0.0, 0.0)));
							break;
						} // End switch

						circle.SetLocation(centre);
						circle.SetRadius(orthogonal_start.Distance(orthogonal_centre));

						TopoDS_Edge edge = Cam::Edge( start, end, circle );
						step.Edge(edge);
						step.Type( GCode::ToolMovement::eFeed );
						movement.push_back(step);
						pLinuxCNC->geometry.insert( std::make_pair(this->line_offset, movement) );
						UpdateMachineLocation();
					}

					/*
					xml << _T("<path col=\"feed\" fixture=\"") << int(this->modal_coordinate_system) << _T("\">\n")
						<< _T("<arc x=\"") << adjust(0,this->x) << _T("\" ")
						<< _T("y=\"") << adjust(1,this->y) << _T("\" ")
						<< _T("z=\"") << adjust(1,this->z) << _T("\" ")
						<< _T("i=\"") << HeeksUnits(Emc2Units(this->i)) << _T("\" ")
						<< _T("j=\"") << HeeksUnits(Emc2Units(this->j)) << _T("\" ")
						<< _T("k=\"") << HeeksUnits(Emc2Units(this->k)) << _T("\" ")
						<< _T("d=\"-1\" ")
						<< _T("/>\n")
						<< _T("</path>\n");
					*/
						{
							// Confirm that the previous location, the center-point and the final location all make
							// sense for an arc movement.  If we have a rounding error then we want to know it.
							gp_Pnt start(PreviousLocation().Location());
							gp_Pnt end(NewLocation().Location());
							gp_Pnt centre(this->i + this->previous[0], this->j + this->previous[1], this->k + this->previous[2]);

							switch (this->plane)
							{
							case LinuxCNC::eXYPlane:
								start  = gp_Pnt(this->previous[0],                  this->previous[1],                  0.0);
								end    = gp_Pnt(this->x,                            this->y,                            0.0);
								centre = gp_Pnt(this->i + this->previous[0], this->j + this->previous[1], 0.0);
								break;

							case LinuxCNC::eXZPlane:
								start  = gp_Pnt(this->previous[0],                  0.0, this->previous[2]);
								end    = gp_Pnt(this->x,                            0.0, this->y);
								centre = gp_Pnt(this->i + this->previous[0], 0.0, this->k + this->previous[2]);
								break;

							case LinuxCNC::eYZPlane:
								start  = gp_Pnt(0.0, this->previous[1],                  this->previous[2]);
								end    = gp_Pnt(0.0, this->y,                            this->y);
								centre = gp_Pnt(0.0, this->j + this->previous[1], this->k + this->previous[2]);
								break;
							} // End switch

							
							double error = fabs(fabs(centre.Distance(end)) - fabs(start.Distance(centre)));
							double tolerance = pLinuxCNC->Tolerance();
							if (error > pLinuxCNC->Tolerance())
							{
								double magnitude = (fabs(error) - fabs(tolerance)) * pow(10, double(pLinuxCNC->RequiredDecimalPlaces()-1));
								if (magnitude > 1.0)
								{
									pLinuxCNC->AddWarning(QString::fromAscii("Clockwise arc offset between start and centre does not match that between centre and end."));
								}
							}
						}

						if (this->feed_rate <= 0.0) pLinuxCNC->AddWarning(QString::fromAscii("Zero feed rate found for arc movement"));
					break;

				case LinuxCNC::stArcCounterClockwise:
					if (pLinuxCNC->machine_location.get() == NULL)
					{
						pLinuxCNC->machine_location = std::auto_ptr<GCode::MachineLocation>(new GCode::MachineLocation());
						UpdateMachineLocation();
					}
					else
					{
						GCode::SingleCommandGeometry_t movement;
						GCode::ToolMovement step(graphics);

						gp_Pnt start(PreviousLocation().Location());
						gp_Pnt end(NewLocation().Location());
						gp_Pnt centre(adjust(0, this->i + this->previous[0]), adjust(1, this->j + this->previous[1]), adjust(2, this->k + this->previous[2]));

						gp_Pnt orthogonal_start;
						gp_Pnt orthogonal_centre;

						gp_Circ circle;
						switch (this->plane)
						{
						case GCode::eXYPlane:
							orthogonal_start  = gp_Pnt(adjust(0, this->previous[0]),    adjust(1, this->previous[1]),  adjust(2, 0.0));
							orthogonal_centre = gp_Pnt(adjust(0, this->i + this->previous[0]),	adjust(1, this->j + this->previous[1]),	adjust(2, 0.0));
							circle.SetAxis(gp_Ax1(gp_Pnt(0.0,0.0,0.0), gp_Dir(0.0, 0.0, +1.0)));
							break;

						case GCode::eXZPlane:
							orthogonal_start  = gp_Pnt(adjust(0, this->previous[0]), adjust(1, 0.0), adjust(2, this->previous[2]));
							orthogonal_centre = gp_Pnt(adjust(0, this->i + this->previous[0]),	adjust(1, 0.0), adjust(2, this->k + this->previous[2]));
							circle.SetAxis(gp_Ax1(gp_Pnt(0.0,0.0,0.0), gp_Dir(0.0, +1.0, 0.0)));
							break;

						case GCode::eYZPlane:
							orthogonal_start  = gp_Pnt(adjust(0, 0.0), adjust(1, this->previous[1]), adjust(2, this->previous[2]));
							orthogonal_centre = gp_Pnt(adjust(0, 0.0), adjust(1, this->j + this->previous[1]),	adjust(2, this->k + this->previous[2]));
							circle.SetAxis(gp_Ax1(gp_Pnt(0.0,0.0,0.0), gp_Dir(+1.0, 0.0, 0.0)));
							break;
						} // End switch

						circle.SetLocation(centre);
						circle.SetRadius(orthogonal_start.Distance(orthogonal_centre));

						TopoDS_Edge edge = Cam::Edge( start, end, circle );
						step.Edge(edge);
						step.Type( GCode::ToolMovement::eFeed );
						movement.push_back(step);
						pLinuxCNC->geometry.insert( std::make_pair(this->line_offset, movement) );
						UpdateMachineLocation();
					}
					/*
					xml << _T("<path col=\"feed\" fixture=\"") << int(this->modal_coordinate_system) << _T("\">\n")
						<< _T("<arc x=\"") << adjust(0,this->x) << _T("\" ")
						<< _T("y=\"") << adjust(1,this->y) << _T("\" ")
						<< _T("z=\"") << adjust(1,this->z) << _T("\" ")
						<< _T("i=\"") << HeeksUnits(Emc2Units(this->i)) << _T("\" ")
						<< _T("j=\"") << HeeksUnits(Emc2Units(this->j)) << _T("\" ")
						<< _T("k=\"") << HeeksUnits(Emc2Units(this->k)) << _T("\" ")
						<< _T("d=\"1\" ")
						<< _T("/>\n")
						<< _T("</path>\n");
					*/
						{
							// Confirm that the previous location, the center-point and the final location all make
							// sense for an arc movement.  If we have a rounding error then we want to know it.
							gp_Pnt start(PreviousLocation().Location());
							gp_Pnt end(NewLocation().Location());
							gp_Pnt centre(adjust(0, this->i + this->previous[0]), adjust(1, this->j + this->previous[1]), adjust(2, this->k + this->previous[2]));

							switch (this->plane)
							{
							case LinuxCNC::eXYPlane:
								start  = gp_Pnt(this->previous[0],                  this->previous[1],                  0.0);
								end    = gp_Pnt(this->x,                            this->y,                            0.0);
								centre = gp_Pnt(this->i + this->previous[0], this->j + this->previous[1], 0.0);
								break;

							case LinuxCNC::eXZPlane:
								start  = gp_Pnt(this->previous[0],                  0.0, this->previous[2]);
								end    = gp_Pnt(this->x,                            0.0, this->y);
								centre = gp_Pnt(this->i + this->previous[0], 0.0, this->k + this->previous[2]);
								break;

							case LinuxCNC::eYZPlane:
								start  = gp_Pnt(0.0, this->previous[1],                  this->previous[2]);
								end    = gp_Pnt(0.0, this->y,                            this->y);
								centre = gp_Pnt(0.0, this->j + this->previous[1], this->k + this->previous[2]);
								break;
							} // End switch

							double error = fabs(fabs(centre.Distance(end)) - fabs(start.Distance(centre)));
							double tolerance = pLinuxCNC->Tolerance();
							if (error > pLinuxCNC->Tolerance())
							{
								double magnitude = (fabs(error) - fabs(tolerance)) * pow(10, double(pLinuxCNC->RequiredDecimalPlaces()-1));
								if (magnitude > 1.0)
								{
									pLinuxCNC->AddWarning(QString::fromAscii("Counter-clockwise arc offset between start and centre does not match that between centre and end."));
								}
							}
						}

						if (this->feed_rate <= 0.0) pLinuxCNC->AddWarning(QString::fromAscii("Zero feed rate found for arc movement"));
					break;

				case LinuxCNC::stBoring:
				case LinuxCNC::stDrilling:
					{
						GCode::SingleCommandGeometry_t movement;

						if (pLinuxCNC->machine_location.get() == NULL)
						{
							pLinuxCNC->machine_location = std::auto_ptr<GCode::MachineLocation>(new GCode::MachineLocation());
							UpdateMachineLocation();
						}
						else
						{
							Cam::Point from( PreviousLocation() );
							Cam::Point to( X(), Y(), PZ() );
							GCode::ToolMovement step(graphics);
							step.Edge( Cam::Edge( from, to ) );
							step.Type( GCode::ToolMovement::eRapid );
							movement.push_back(step);
						}

						{
							Cam::Point from( X(), Y(), PZ() );
							Cam::Point to( X(), Y(), R() );
							GCode::ToolMovement step(graphics);
							step.Edge( Cam::Edge( from, to ) );
							step.Type( GCode::ToolMovement::eRapid );
							movement.push_back(step);
						}

						{
							Cam::Point from( X(), Y(), R());
							Cam::Point to( NewLocation() );
							GCode::ToolMovement step(graphics);
							step.Edge( Cam::Edge( from, to ) );
							step.Type( GCode::ToolMovement::eFeed );
							movement.push_back(step);
						}

						{
							Cam::Point from( NewLocation() );
							Cam::Point to( X(), Y(), R() );
							GCode::ToolMovement step(graphics);
							step.Edge( Cam::Edge( from, to ) );
							step.Type( GCode::ToolMovement::eRapid );
							movement.push_back(step);
						}
						pLinuxCNC->geometry.insert( std::make_pair(this->line_offset, movement) );
						UpdateMachineLocation();

						/*
						xml << _T("<path col=\"rapid\" fixture=\"") << int(this->modal_coordinate_system) << _T("\">\n")
							<< _T("<line x=\"") << adjust(0,this->x) << _T("\" ")
							<< _T("y=\"") << adjust(1,this->y) << _T("\" ")
							<< _T("/>\n")
							<< _T("</path>\n");
						xml << _T("<path col=\"rapid\" fixture=\"") << int(this->modal_coordinate_system) << _T("\">\n")
							<< _T("<line z=\"") << adjust(2,this->r) << _T("\" ")
							<< _T("/>\n")
							<< _T("</path>\n");
						xml << _T("<path col=\"feed\" fixture=\"") << int(this->modal_coordinate_system) << _T("\">\n")
							<< _T("<line z=\"") << adjust(2,this->z) << _T("\" ")
							<< _T("/>\n")
							<< _T("</path>\n");
						xml << _T("<path col=\"rapid\" fixture=\"") << int(this->modal_coordinate_system) << _T("\">\n")
							<< _T("<line z=\"") << adjust(2,this->r) << _T("\" ")
							<< _T("/>\n")
							<< _T("</path>\n");
						*/
						this->z = this->r;	// We end up at the clearance (r) position.
						if (this->feed_rate <= 0.0) pLinuxCNC->AddWarning(QString::fromAscii("Zero feed rate found for arc movement"));
					}
					break;

				case LinuxCNC::stTapping:
					{
						GCode::SingleCommandGeometry_t movement;

						if (pLinuxCNC->machine_location.get() == NULL)
						{
							pLinuxCNC->machine_location = std::auto_ptr<GCode::MachineLocation>(new GCode::MachineLocation());
							UpdateMachineLocation();
						}
						else
						{
							Cam::Point from( PreviousLocation() );
							Cam::Point to  ( X(), Y(), PZ() );
							GCode::ToolMovement step(graphics);
							step.Edge( Cam::Edge( from, to ) );
							step.Type( GCode::ToolMovement::eRapid );
							movement.push_back(step);
						}

						{
							Cam::Point from  ( X(), Y(), PZ() );
							Cam::Point to( X(), Y(), R() );
							GCode::ToolMovement step(graphics);
							step.Edge( Cam::Edge( from, to ) );
							step.Type( GCode::ToolMovement::eRapid );
							movement.push_back(step);
						}

						{
							Cam::Point from( X(), Y(), R() );
							Cam::Point to( NewLocation() );
							GCode::ToolMovement step(graphics);
							step.Edge( Cam::Edge( from, to ) );
							step.Type( GCode::ToolMovement::eFeed );
							movement.push_back(step);
						}

						{
							Cam::Point from( NewLocation() );
							Cam::Point to( X(), Y(), R() );
							GCode::ToolMovement step(graphics);
							step.Edge( Cam::Edge( from, to ) );
							step.Type( GCode::ToolMovement::eFeed );
							movement.push_back(step);
						}
						pLinuxCNC->geometry.insert( std::make_pair(this->line_offset, movement) );
						UpdateMachineLocation();
					}

					/*
					xml << _T("<path col=\"rapid\" fixture=\"") << int(this->modal_coordinate_system) << _T("\">\n")
						<< _T("<line x=\"") << adjust(0,this->x) << _T("\" ")
						<< _T("y=\"") << adjust(1,this->y) << _T("\" ")
						<< _T("/>\n")
						<< _T("</path>\n");
					xml << _T("<path col=\"rapid\" fixture=\"") << int(this->modal_coordinate_system) << _T("\">\n")
						<< _T("<line z=\"") << adjust(2,this->r) << _T("\" ")
						<< _T("/>\n")
						<< _T("</path>\n");
					xml << _T("<path col=\"feed\" fixture=\"") << int(this->modal_coordinate_system) << _T("\">\n")
						<< _T("<line z=\"") << adjust(2,this->z) << _T("\" ")
						<< _T("/>\n")
						<< _T("</path>\n");
					xml << _T("<path col=\"feed\" fixture=\"") << int(this->modal_coordinate_system) << _T("\">\n")
						<< _T("<line z=\"") << adjust(2,this->r) << _T("\" ")
						<< _T("/>\n")
						<< _T("</path>\n");
					*/
					this->z = this->r;	// We end up at the clearance (r) position.
					if (this->feed_rate <= 0.0) pLinuxCNC->AddWarning(QString::fromAscii("Zero feed rate found for arc movement"));
					break;


				case LinuxCNC::stG28:
					// The saved position can be found in variables 5161 to 5169.
					this->x = ParseUnits(variables[ LinuxCNC::eG28VariableBase + 0 ] - variables[ LinuxCNC::eG54VariableBase + 0 ]);
					this->y = ParseUnits(variables[ LinuxCNC::eG28VariableBase + 1 ] - variables[ LinuxCNC::eG54VariableBase + 1 ]);
					this->z = ParseUnits(variables[ LinuxCNC::eG28VariableBase + 2 ] - variables[ LinuxCNC::eG54VariableBase + 2 ]);
					this->a = ParseUnits(variables[ LinuxCNC::eG28VariableBase + 3 ] - variables[ LinuxCNC::eG54VariableBase + 3 ]);
					this->b = ParseUnits(variables[ LinuxCNC::eG28VariableBase + 4 ] - variables[ LinuxCNC::eG54VariableBase + 4 ]);
					this->c = ParseUnits(variables[ LinuxCNC::eG28VariableBase + 5 ] - variables[ LinuxCNC::eG54VariableBase + 5 ]);
					this->u = ParseUnits(variables[ LinuxCNC::eG28VariableBase + 6 ] - variables[ LinuxCNC::eG54VariableBase + 6 ]);
					this->v = ParseUnits(variables[ LinuxCNC::eG28VariableBase + 7 ] - variables[ LinuxCNC::eG54VariableBase + 7 ]);
					this->w = ParseUnits(variables[ LinuxCNC::eG28VariableBase + 8 ] - variables[ LinuxCNC::eG54VariableBase + 8 ]);

					if (pLinuxCNC->machine_location.get() == NULL)
					{
						pLinuxCNC->machine_location = std::auto_ptr<GCode::MachineLocation>(new GCode::MachineLocation());
						UpdateMachineLocation();
					}
					else
					{
						GCode::SingleCommandGeometry_t movement;

						{
							Cam::Point from  ( PreviousLocation() );
							Cam::Point to( NewLocation() );
							GCode::ToolMovement step(graphics);
							step.Edge( Cam::Edge( from, to ) );
							step.Type( GCode::ToolMovement::eRapid );
							movement.push_back(step);
						}

						pLinuxCNC->geometry.insert( std::make_pair(this->line_offset, movement) );
						UpdateMachineLocation();
					}
					/*
					xml << _T("<path col=\"rapid\" fixture=\"") << int(this->modal_coordinate_system) << _T("\">\n")
						<< _T("<line x=\"") << adjust(0,this->x) << _T("\" ")
						<< _T("y=\"") << adjust(1,this->y) << _T("\" ")
						<< _T("z=\"") << adjust(2,this->z) << _T("\" ")
						<< _T("/>\n")
						<< _T("</path>\n");
					*/
					break;

				case LinuxCNC::stG30:
					// The saved position can be found in variables 5181 to 5189.
					this->x = ParseUnits(variables[ LinuxCNC::eG30VariableBase + 0 ] - variables[ LinuxCNC::eG54VariableBase + 0 ]);
					this->y = ParseUnits(variables[ LinuxCNC::eG30VariableBase + 1 ] - variables[ LinuxCNC::eG54VariableBase + 1 ]);
					this->z = ParseUnits(variables[ LinuxCNC::eG30VariableBase + 2 ] - variables[ LinuxCNC::eG54VariableBase + 2 ]);
					this->a = ParseUnits(variables[ LinuxCNC::eG30VariableBase + 3 ] - variables[ LinuxCNC::eG54VariableBase + 3 ]);
					this->b = ParseUnits(variables[ LinuxCNC::eG30VariableBase + 4 ] - variables[ LinuxCNC::eG54VariableBase + 4 ]);
					this->c = ParseUnits(variables[ LinuxCNC::eG30VariableBase + 5 ] - variables[ LinuxCNC::eG54VariableBase + 5 ]);
					this->u = ParseUnits(variables[ LinuxCNC::eG30VariableBase + 6 ] - variables[ LinuxCNC::eG54VariableBase + 6 ]);
					this->v = ParseUnits(variables[ LinuxCNC::eG30VariableBase + 7 ] - variables[ LinuxCNC::eG54VariableBase + 7 ]);
					this->w = ParseUnits(variables[ LinuxCNC::eG30VariableBase + 8 ] - variables[ LinuxCNC::eG54VariableBase + 8 ]);

					if (pLinuxCNC->machine_location.get() == NULL)
					{
						pLinuxCNC->machine_location = std::auto_ptr<GCode::MachineLocation>(new GCode::MachineLocation());
						UpdateMachineLocation();
					}
					else
					{
						GCode::SingleCommandGeometry_t movement;

						{
							Cam::Point from  ( PreviousLocation() );
							Cam::Point to( NewLocation() );
							GCode::ToolMovement step(graphics);
							step.Edge( Cam::Edge( from, to ) );
							step.Type( GCode::ToolMovement::eRapid );
							movement.push_back(step);
						}

						pLinuxCNC->geometry.insert( std::make_pair(this->line_offset, movement) );
						UpdateMachineLocation();
					}

					/*
					xml << _T("<path col=\"rapid\" fixture=\"") << int(this->modal_coordinate_system) << _T("\">\n")
						<< _T("<line x=\"") << adjust(0,this->x) << _T("\" ")
						<< _T("y=\"") << adjust(1,this->y) << _T("\" ")
						<< _T("z=\"") << adjust(2,this->z) << _T("\" ")
						<< _T("/>\n")
						<< _T("</path>\n");
					*/
					break;

				case LinuxCNC::stG92:
					// Make the axis values just read in become the current machine position by
					// figuring out what these values are (in machine coordinates) and adding
					// the offsets into a new set of coordinate system variables.  That's so
					// we can use the adjust() routine to allow for them in subsequent
					// commands.  This must occur until we find a G92.1 command which turns
					// this 'temporary coordinate system' functionality off.
					this->current_coordinate_system = LinuxCNC::csG92;
					variables[LinuxCNC::eG92Enabled] = 1.0;

					variables[LinuxCNC::eG92VariableBase + 0] = Emc2Units(this->previous[0]) - Emc2Units(this->x);
					variables[LinuxCNC::eG92VariableBase + 1] = Emc2Units(this->previous[1]) - Emc2Units(this->y);
					variables[LinuxCNC::eG92VariableBase + 2] = Emc2Units(this->previous[2]) - Emc2Units(this->z);

					variables[LinuxCNC::eG92VariableBase + 3] = Emc2Units(this->previous[3]) - Emc2Units(this->a);
					variables[LinuxCNC::eG92VariableBase + 4] = Emc2Units(this->previous[4]) - Emc2Units(this->b);
					variables[LinuxCNC::eG92VariableBase + 5] = Emc2Units(this->previous[5]) - Emc2Units(this->c);

					variables[LinuxCNC::eG92VariableBase + 6] = Emc2Units(this->previous[6]) - Emc2Units(this->u);
					variables[LinuxCNC::eG92VariableBase + 7] = Emc2Units(this->previous[7]) - Emc2Units(this->v);
					variables[LinuxCNC::eG92VariableBase + 8] = Emc2Units(this->previous[8]) - Emc2Units(this->w);
					break;

				case LinuxCNC::stG92_1:
					// Turn off the 'temporary coordinate system' functionality.
					this->current_coordinate_system = LinuxCNC::csUndefined;

					// Disable the G92 offset function.
					variables[LinuxCNC::eG92Enabled] = 0.0;

					// Reset the G92 offsets to all zero.
					for (int i=LinuxCNC::eG92VariableBase; i<LinuxCNC::eG92VariableBase+9; i++)
					{
						variables[i] = 0.0;
					}

					break;

				case LinuxCNC::stG92_2:
					// Disable the G92 offset function but don't reset the offsets in the memory locations.
					variables[LinuxCNC::eG92Enabled] = 0.0;
					break;

				case LinuxCNC::stG92_3:
					// Re-Enable the G92 offset function and don't change the offsets in the memory locations.
					variables[LinuxCNC::eG92Enabled] = 1.0;
					break;

				case LinuxCNC::stAxis:
					// Nothing extra special to do here.
					break;

				} // End switch

				// xml << _T("</ncblock>\n");
			}

			ResetForEndOfBlock();
		} // End ProcessBlock() routine


		void ResetForEndOfBlock()
		{
			/*
			FILE *fp = fopen("c:\\temp\\david.log","a+t");
			fprintf(fp,"%s\n", this->line_number);
			fprintf(fp,"%s\n", emc_variables.log().c_str());
			fclose(fp);
			*/

			this->previous_statement_type = this->statement_type;
			this->statement_type = LinuxCNC::stUndefined;

			this->line_number = 0;
			this->line_offset += 1;

			this->previous[0] = this->x;
			this->previous[1] = this->y;
			this->previous[2] = this->z;
			this->previous[3] = this->a;
			this->previous[4] = this->b;
			this->previous[5] = this->c;
			this->previous[6] = this->u;
			this->previous[7] = this->v;
			this->previous[8] = this->w;

			this->l = 0.0;
			this->p = 0.0;
			this->r = 0.0;
			this->q = 0.0;

			this->i = 0.0;
			this->j = 0.0;
			this->k = 0.0;

			this->x_specified = 0;
			this->y_specified = 0;
			this->z_specified = 0;

			this->a_specified = 0;
			this->b_specified = 0;
			this->c_specified = 0;

			this->u_specified = 0;
			this->v_specified = 0;
			this->w_specified = 0;

			this->i_specified = 0;
			this->j_specified = 0;
			this->k_specified = 0;

			this->l_specified = 0;
			this->p_specified = 0;
			this->q_specified = 0;
			this->r_specified = 0;

		} // End ResetForEndOfBlock() routine



		/**
			Initialize all nine variables from this base number to the value given.
		 */
		void InitOneCoordinateSystem( const LinuxCNC::Variables::SymbolId_t base, const double value )
		{
			for (LinuxCNC::Variables::SymbolId_t var=base; var<=base + 8; var++)
			{
				variables[var] = value;
			}
		}

		void InitializeGCodeVariables()
		{
			InitOneCoordinateSystem(LinuxCNC::eG38_2VariableBase, 0.0);
			InitOneCoordinateSystem(LinuxCNC::eG28VariableBase, 0.0);
			InitOneCoordinateSystem(LinuxCNC::eG30VariableBase, 0.0);
			InitOneCoordinateSystem(LinuxCNC::eG92VariableBase, 0.0);

			// Coordinate system number (1 = G54, 2=G55 etc.)
			{
				variables[LinuxCNC::eCoordinateSystemInUse] = 1.0;
			}

			/*
			const wxChar *not_used = _("Not Used");
			if (PROGRAM->m_emc2_variables_file_name != not_used)
			{
				FILE *fp = fopen(PROGRAM->m_emc2_variables_file_name.utf8_str(),"r");
				if (fp == NULL)
				{
					wxString error;
					error << _("Could not open ") << PROGRAM->m_emc2_variables_file_name << _(" for reading. Would you like to turn off the use of the EMC2 variables file?");
					int answer = wxMessageBox( error, _("EMC2 Variables File"), wxYES_NO );
					if (answer == wxYES)
					{
						PROGRAM->SetEmc2VariablesFileName( not_used );
					} // End if - then
				}
				else
				{
					char buf[1024];
					memset( buf, '\0', sizeof(buf) );
					while (fgets(buf, sizeof(buf)-1, fp) != NULL)
					{
						std::string line(buf);
						std::string::size_type offset=line.find_first_of("\t ");
						if (offset != std::string::npos)
						{
							std::string name = line.substr(0, offset);
							while (isspace(line[offset])) offset++;
							std::string value = line.substr(offset);

							variables[name.c_str()] = atof(value.c_str());
						} // End if - then
					} // End while
					fclose(fp);
					fp = NULL;
				} // End if - then
			} // End if - then
			*/
		}

};

bool LinuxCNC::Parse()
{
	// The object exists...
	if (! this->machine_program)
	{
		return(false);
	}

	// The QStringList within the object exists...
	if (! this->machine_program->getMachineProgram())
	{
		return(false);
	}

	QString str;
	str << *(this->machine_program);	// Convert from QStringList to QString
	const std::string gcode(str.toAscii().constData());

	linuxcnc_grammar<std::string::const_iterator> parser(this);
	std::string::const_iterator gcode_program_location = gcode.begin();
	
	// Parse the GCode using the linuxcnc grammar.  The qi::blank skipper will skip all whitespace
	// except newline characters.  i.e. it allows newline characters to be included in the grammar
	// (which they need to be as they represent an 'end of block' marker)

	if ((qi::phrase_parse(gcode_program_location, gcode.end(), parser, qi::blank)) && (gcode_program_location == gcode.end()))
	{
		qDebug("last line number %d\n", parser.line_number );
		qDebug("%s\n", this->machine_program->TraceProgramLinkages().toAscii().constData());
	}
	else
	{
		// qDebug("Parsing failed at %c%d\n", linuxcnc.line_number[0].first, linuxcnc.line_number[0].second);
		qDebug("%s\n", GCode_GrammarDebugOutputBuffer.str().c_str());
		if (gcode_program_location != gcode.end())
		{
			qDebug("%s\n", std::string(gcode_program_location, gcode.end()).c_str());
		}
	}

	return(gcode_program_location == gcode.end());
}