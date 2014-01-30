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

#include "TPGListModel.h"

namespace CamGui {

TPGListModel::TPGListModel(Cam::TPGDescriptorCollection *tpgs, QObject *parent)
  : QAbstractListModel(parent)
{
    this->tpgs = tpgs->grab();
}

TPGListModel::~TPGListModel() {
    if (tpgs != NULL)
        tpgs->release();
}

int TPGListModel::rowCount(const QModelIndex& parent) const {
  if (tpgs != NULL)
    return tpgs->size();
  return 0;
}

QVariant TPGListModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid())
    return QVariant();

  if (size_t(index.row()) >= tpgs->size())
    return QVariant();
  if (role == Qt::DisplayRole) {
    if (tpgs != NULL && tpgs->size() > size_t(index.row())) {
      Cam::TPGDescriptor* tpg = tpgs->at(index.row());
      return QVariant(tpg->name);
    }
  }
  return QVariant();
}

QVariant TPGListModel::headerData(int section, Qt::Orientation orientation,
    int role) const {
  if (role == Qt::DisplayRole && section == 0) {
	  return QVariant(QString::fromUtf8("TPG Name"));
  }
  return QVariant();
}


#include "moc_TPGListModel.cpp"

}/* namespace CamGui */
