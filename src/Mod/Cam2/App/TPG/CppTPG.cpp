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
#endif

#include "CppTPG.h"

namespace Cam {


CppTPG::CppTPG() {
    plugin = NULL;
}
CppTPG::~CppTPG() {
    if (plugin != NULL)
	{
        plugin->release();
		plugin = NULL;
	}
}

/**
 * Set the plugin that was used to create this create this instance.
 *
 * This is so it can obtain a reference to it and thus cause the plugin to
 * stay open until this instance is released.
 */
void CppTPG::setPlugin(CppTPGPlugin* plugin) {
    this->plugin = plugin->grab();
}

/* virtual */ void CppTPG::initialise(TPGFeature *tpgFeature)
{
	TPG::initialise(tpgFeature);

	// Now that the TPG::initialise() method has been called, it's safe to add any settings
	// that are specific to this class.

}

/* virtual */ void CppTPG::onChanged( Settings::Definition *tpgSettingDefinition, QString previous_value, QString new_value )
{
	qDebug("CppTPG::onChanged(%s changed from %s to %s)\n", 
				tpgSettingDefinition->getFullname().toAscii().constData(),
				previous_value.toAscii().constData(), 
				new_value.toAscii().constData());

	TPG::onChanged( tpgSettingDefinition, previous_value, new_value );
}

} // End Cam namespace


