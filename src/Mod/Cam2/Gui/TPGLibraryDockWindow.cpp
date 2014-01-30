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
# include <QObject>
# include <QMessageBox>

#endif
# include <Gui/MainWindow.h>

#include "ui_TPGLibraryDockWindow.h"
#include "TPGLibraryDockWindow.h"
#include "UIManager.h"

namespace CamGui {

TPGLibraryDockWindow::TPGLibraryDockWindow(Gui::Document* pcDocument, QWidget *parent)
  : DockWindow(pcDocument,parent), ui(new Ui_TPGLibraryDockWindow)
{
  setWindowTitle(tr("TPG Library"));

  ui->setupUi(this);

  updatedTPGList(NULL);

  /// Internal ///
  // connect add button to internal handler
  QObject::connect(ui->AddTPG, SIGNAL(clicked()),
      this, SLOT(addBtnClick()));
  // selection changes
  QObject::connect(ui->TPGList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
      this, SLOT(selectionChanged(const QItemSelection &, const QItemSelection &)));

  /// External ///
  // connect addTPG to UIManager
  QObject::connect(this, SIGNAL(addTPG(Cam::TPGDescriptor*)),
      &UIManager(), SLOT(addTPG(Cam::TPGDescriptor*)));
  // connect reload button to UIManager
  QObject::connect(ui->ReloadLibrary, SIGNAL(clicked()),
      &UIManager(), SLOT(reloadTPGs()));
  // subscribe to tpg list updates updatedTPGList
  QObject::connect(&UIManager(), SIGNAL(updatedTPGList(TPGListModel*)),
      this, SLOT(updatedTPGList(TPGListModel*)));
}

TPGLibraryDockWindow::~TPGLibraryDockWindow() {
}

/**
 * A slot to update the TPG model.
 */
void TPGLibraryDockWindow::updatedTPGList(TPGListModel* tpgs)
{
  this->tpgs = tpgs;
  ui->TPGList->setModel(tpgs);
  // Reconnect the signal after a model change
  QObject::connect(ui->TPGList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
      this, SLOT(selectionChanged(const QItemSelection &, const QItemSelection &)));
}

/**
 * [Internal Use Only] Slot to receive Add TPG requests from the button
 */
void TPGLibraryDockWindow::addBtnClick()
{
  QItemSelectionModel *selectionModel = ui->TPGList->selectionModel();
  QModelIndexList idxs = selectionModel->selectedIndexes();
  for (int i = 0; i < idxs.size(); i++) {
    Cam::TPGDescriptor *tpg = tpgs->get(idxs[i].row());
    if (tpg != NULL)
      Q_EMIT addTPG(tpg);
  }
}

/**
 * [Internal Use Only] Slot to receive selection change signals from the TPGTreeView.
 */
void TPGLibraryDockWindow::selectionChanged(const QItemSelection &newselection, const QItemSelection &oldselection)
{
  //TODO: move the hard-coded HTML into resource files
  //TODO: translate on-screen text
  QString html;
  if (newselection.size() == 1) {
    QModelIndexList idxs = newselection.indexes();

    Cam::TPGDescriptor *tpg = tpgs->get(idxs[0].row());
	html = QString::fromUtf8("<html><body style=\"font-family:'Cantarell'; font-size:11pt; margin: 0;\">"\
		"<p style=\"margin:0;\"><span style=\"font-weight:bold;\">Name</span>: ") + tpg->name + QString::fromUtf8("</p>"\
        "<p style=\"margin:0;\"><span style=\"font-weight:bold;\">Description</span>: <br/>") + tpg->description + QString::fromUtf8("</p>"\
        "<hr style=\"margin:0;\"/>"\
        "<p style=\"font-size:8pt; font-style:italic; color:#868686; margin: 0;\">{") + tpg->id + QString::fromUtf8("}</p>"\
        "</body></html>");
  }
  else if (newselection.size() == 0)
    html = QString::fromAscii("<html><body><p style=\"font-family:'Cantarell'; font-size:8pt; font-style:italic; color:#868686;\">No TPG selected</p></body></html>");
  else
    html = QString::fromAscii("<html><body><p style=\"font-family:'Cantarell'; font-size:8pt; font-style:italic; color:#868686;\">Multiple TPG selection</p></body></html>");
  ui->TPGDetails->setHtml(html);
}

#include "moc_TPGLibraryDockWindow.cpp"

} /* namespace CamGui */
