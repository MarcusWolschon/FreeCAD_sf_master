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

#ifndef TPGLISTMODEL_H_
#define TPGLISTMODEL_H_

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include <vector>

#include "../App/TPG/TPGFactory.h"

namespace CamGui {

/**
 * A model that wraps the contents of a TPGDescriptor list
 */
class TPGListModel: public QAbstractListModel {

  Q_OBJECT

protected:
  Cam::TPGDescriptorCollection *tpgs;

public:
  TPGListModel(Cam::TPGDescriptorCollection *tpgs, QObject *parent=0);
  virtual ~TPGListModel();

  /**
   * Simplified getter for the descriptors
   */
  Cam::TPGDescriptor *get(int i) {
    if (tpgs != NULL && i >= 0 && size_t(i) < tpgs->size())
      return tpgs->at(i);
    return NULL;
  }

  //List model methods
  int rowCount(const QModelIndex &parent=QModelIndex() ) const;
  QVariant data(const QModelIndex &index, int role=Qt::DisplayRole ) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole ) const;
};

} /* namespace CamGui */
#endif /* TPGLISTMODEL_H_ */
