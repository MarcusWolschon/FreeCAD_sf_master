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

#ifndef CAM_LINUX_CNC_H
#define CAM_LINUX_CNC_H

#include <PreCompiled.h>

#include <Mod/Cam2/App/GCode.h>


///////////////////////////////////////////////////////////////////////////////

/// Base class
namespace Cam
{

class CamExport LinuxCNC : public GCode
{
public:
    LinuxCNC(MachineProgram *machine_program, TPGFeature* tpgFeature);
    ~LinuxCNC();

public:
	virtual bool Parse();

	/*
		NOTE: The order of the eStatement_t enumeration determines the 
		importance of each line of GCode.  Only the most 'important' (highest number)
		will affect both the colour and the NCCode object lines added as a result.

		eg: If a line of GCode includes;
		N50 G01 X1 Y2 Z3 (This is a movement)

		Then this line could be interpreted as a;
			- line number
			- feed rate movement
			- comment

		Of these, the feed movement (G01) is the most important so the stFeed statement
		type must appear AFTER the stComment value in this enumeration.
	 */
	typedef enum {
		stUndefined = 0,
		stProgramFlow,
		stIfStatement,
		stElseStatement,
		stEndifStatement,
		stWhileStatement,
		stEndWhileStatement,
		stComment,
		stProgram,
		stPreparation,
		stDataSetting,
		stAxis,
		stVariable,
		stG28,
		stG30,
		stG92,
		stG92_1,
		stG92_2,
		stG92_3,
		stToolLengthEnabled,
		stToolLengthDisabled,
		stToolChange,
		stProbe,
		stRapid,
		stFeed,
		stArcClockwise,
		stArcCounterClockwise,
		stDrilling,
		stTapping,
		stBoring
	} eStatement_t;

	enum {
		eG92Enabled = 5210,
		eG92VariableBase = 5211,    // 9 values here

		eCoordinateSystemInUse = 5220,  // 1 = g54 etc.

		eG54VariableBase = 5221,    // 9 values here
		eG55VariableBase = 5241,    // 9 values here
		eG56VariableBase = 5261,    // 9 values here
		eG57VariableBase = 5281,    // 9 values here
		eG58VariableBase = 5301,    // 9 values here
		eG59VariableBase = 5321,    // 9 values here
		eG59_1VariableBase = 5341,    // 9 values here
		eG59_2VariableBase = 5361,    // 9 values here
		eG59_3VariableBase = 5381,    // 9 values here

		eG38_2VariableBase = 5061,  // 9 values here - probing results.
		eG28VariableBase = 5161,    // 9 values here - offsets for G28 home location
		eG30VariableBase = 5181     // 9 values here - offsets for G30 home location
	} eVariableLocations;

	/**
		A LinuxCNC variable can either be named as an integer (eg: #5061)
		or as a string (eg: #<myvariable>)

		In this class, the 'name' used would be '5061' and '<myvariable>'
		in the above two cases.  i.e. the leading '#' character is removed
		as part of the parsing process.

		We also use this class to hold a symbol table of variable/value pairs
		for all literal and non-literal numbers in the GCode.  eg: when we
		see 'Z35', for example, the 'Z' is interpreted as a 'token' and the '35'
		is assigned a random variable 'name' that is then given the floating
		point value of '35'.  We then refer to this variable by a reference
		given to this newly created entry in the LinuxCNCVariables object.  i.e. a
		symbol ID.

		If the variable's name is given then the reference is an integer generated
		by a simple hashing algorithm from the variable's name.

		If the variable's name is automatically generated then it's name starts
		from 1000000 and counts up.

		If an emc.var file is provided then this object is initialized from that
		variables file so that the generated toolpath graphics makes sense in the
		context of the real machine.
	 */
	class Variables {
	public:
		typedef int SymbolId_t;

		Variables()
		{
			m_last_id = 1000000;
		}

		bool exists( const int name )
		{
			int id=this->hash(name);
			return(m_variables.find(id) != m_variables.end());
		}

		double & operator[]( const char *name )
		{
			int id = this->hash(name);
			if (m_variables.find(id) == m_variables.end())
			{
				m_variables.insert( std::make_pair( id, 0.0 ) );
			}

			return(m_variables[id]);
		}

		double & operator[]( const int name )
		{
			int id=this->hash(name);
			if (m_variables.find(id) == m_variables.end())
			{
				m_variables.insert( std::make_pair( id, 0.0 ) );
			}

			std::ostringstream results;
			results << "name " << id << " value " << m_variables[id] << "\n";

			return(m_variables[id]);
		}

		// Allocate a new ID number for an 'unnamed' variable.
		int new_id()
		{
			int id=m_last_id;
			Variables_t::const_iterator itVar = m_variables.find(id);
			while (itVar != m_variables.end())
			{
				id++;
				itVar = m_variables.find(id);
			}
			m_last_id = id;
			return(id);
		}

		// Generate an integer representation of this variable name so that we
		// can find it later on.
		int hash(const char *name)
		{
			int id=0;
			if ((name != NULL) && (*name != '\0'))
			{
				char *p = NULL;
				unsigned long check = strtoul(name, &p, 10);
				if (*p == '\0')
				{
					// The name is completely made up of digits.  Use the name as the ID (in integer form)
					// This becomes more important when we refer to this variable from multiple
					// places in the GCode.  eg: if we say #1001 = 50 at one place and then
					// say G00 X [#1001] then we want the second reference to use the same symbol
					// as the first.  i.e. the symbol is 'named'.

					id = int(check);
				}
				else
				{
					// The name has at least one non-digit in it.  Assign a new ID for this variable.
					id = new_id();
				}

				if (m_name_id_map.find(name) == m_name_id_map.end())
				{
					m_name_id_map.insert( std::make_pair( name, id ) );
					return(id);
				}
				else
				{
					return(m_name_id_map[name]);
				}
			}
			else
			{
				// There is no name given.  This is probably just a 'temporary'
				// symbol (eg: "G00 Z30" would need to store "30" as a temporary symbol).
				// Just assign a new ID to this variable.
				id = new_id();
				m_name_id_map.insert( std::make_pair( name, id ) );
				return(id);
			}
		}

		int hash(const int name)
		{
			std::ostringstream ossName;
			ossName << name;
			return(this->hash(ossName.str().c_str()));
		}

		void clear()
		{
			m_variables.clear();
		}

		std::string name( const int id ) const
		{
			for (NameIdMap_t::const_iterator itPtr = m_name_id_map.begin(); itPtr != m_name_id_map.end(); itPtr++)
			{
				if (itPtr->second == id)
				{
					return(itPtr->first);
				}
			}

			return("UNKNOWN");
		}

		std::string log() const
		{
			std::ostringstream msg;

			msg << "<LINUX_CNC_VARIABLES NUM=\"" << m_variables.size() << "\">\n";
			for (Variables_t::const_iterator itVar = m_variables.begin(); itVar != m_variables.end(); itVar++)
			{
				msg << "   <VAR>\n"
					<< "     <NAME>" << name(itVar->first) << "</NAME>\n"
					<< "     <VALUE>" << itVar->second << "</VALUE>\n"
					<< "   </VAR>\n";
			}
			msg << "</LINUX_CNC_VARIABLES>\n";

			return(msg.str());
		}

	private:
		typedef std::map< int, double > Variables_t;
		typedef std::map< std::string, int > NameIdMap_t;

		Variables_t m_variables;
		NameIdMap_t m_name_id_map;
		int m_last_id;
	};

	typedef std::map<char, Variables::SymbolId_t > MotionArguments_t;

}; // End LinuxCNC class definition.


} //namespace Cam
#endif //CAM_LINUX_CNC_H
