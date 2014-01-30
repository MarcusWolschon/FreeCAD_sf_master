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

#ifndef CPPTPGWRAPPER_H_
#define CPPTPGWRAPPER_H_

#include "TPG.h"
#include "CppTPGPlugin.h"

namespace Cam {

/**
 * This class is used to wrap C++TPGs that are loaded from external modules
 * to ensure the implementation is deleted via the module's delete function.
 */
class CppTPGWrapper: public Cam::TPG {
protected:
    TPG *impl;
    CppTPGPlugin *plugin;

public:
    CppTPGWrapper(TPG *impl, CppTPGPlugin *plugin);
    virtual ~CppTPGWrapper();

    /**
     * Returns a list of action names supported by this TPG.
     *
     * Note: if this TPG only does one thing then just return a list containing 'default'
     */
    virtual std::vector<QString> &getActions();

    /**
     * Get the settings for a given action
     */
    virtual TPGSettings *getSettingDefinitions(QString &action);

    /**
     * Run the TPG to generate the ToolPath code.
     *
     * Note: the return will change once the TP Language has been set in store
     */
    virtual void run(TPGSettings *settings, QString action);
};

} /* namespace Cam */
#endif /* CPPTPGWRAPPER_H_ */
