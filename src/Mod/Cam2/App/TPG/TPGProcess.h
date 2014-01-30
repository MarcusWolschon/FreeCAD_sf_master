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

#ifndef CAM_TPGPROCESS_H
#define CAM_TPGPROCESS_H


#include "TPGFactory.h"

namespace Cam
{

class ProcessTPGDescriptor : public TPGDescriptor
{
public:
  ProcessTPGDescriptor(QString id, QString name, QString description)
    : TPGDescriptor(id, name, description, QString::fromAscii("ProcessTPG"))
  {}

  ProcessTPGDescriptor(const char *id, const char * name, const char * description)
    : TPGDescriptor(id, name, description, "ProcessTPG")
  {}
  TPG* make();
};


/**
 * For implementing TPG plugins that are spawn new processes and require asynchronous operation
 */
class CamExport ProcessTPG : public TPG
{

public:
    ProcessTPG() {}
    ProcessTPG(TPGDescriptor *descriptor);
    ~ProcessTPG();    
protected:
};

} //namespace Cam


#endif //CAM_TPGProcess_H

