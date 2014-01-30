/***************************************************************************
 *   Copyright (c) Luke Parry (l.parry@warwick.ac.uk) 2012                 *
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

#ifndef _CAM_TPGFACTORYINST_h_
#define _CAM_TPGFACTORYINST_h_

#include <PreCompiled.h>

#include <Base/Factory.h>
#include <Base/Exception.h>
//#include <QStringList>
#include <boost/iterator/iterator_concepts.hpp>

#include "TPG.h"
//#include "../Support.h"
#include "TPGDescriptor.h"
#include "TPGDescriptorCollection.h"

#define PYTHON_TPG  1
#define CPP_TPG     2
#define ALL_TPG     3

namespace Cam
{

//class TPG;

/**
  * The actual static initialisation of the TPGFactory that stores the list of tpgLists
  */
class TPGFactoryInstP
{
public:
    Cam::TPGDescriptorCollection* descriptors;

    TPGFactoryInstP() {
        descriptors = NULL;
    }
};

class CamExport TPGFactoryInst
{

public:
  static TPGFactoryInst& instance(void);
  static void destruct (void);
  TPG * getPlugin(QString id);

  /**
   * Searches for TPG's (both Python and Cpp).
   */
  void scanPlugins(short typetype = ALL_TPG);

  /**
   * Get a vector of all Python TPG's that are known about
   */
  Cam::TPGDescriptorCollection* getDescriptors();

//  /**
//   * @deprecated, use getDescriptors() instead
//   */
//  const std::vector<TPGDescriptor*> & getPluginList() { return d->descriptors; }

private:
  TPGFactoryInst(void);
  ~TPGFactoryInst(void);
  static TPGFactoryInst* _pcSingleton;
  
  /// Used for clearing the registered descriptors for describing a TPG plugin
  void clearDescriptors();

  TPGFactoryInstP* d;
};

/// Get the global instance
inline TPGFactoryInst& TPGFactory(void)
{
    return TPGFactoryInst::instance();
}

}

#endif //_CAM_TPGFACTORYINST_h_
