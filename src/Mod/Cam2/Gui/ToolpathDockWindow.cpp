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
# include <QGridLayout>
# include <QListWidget>
#endif
#include <QListWidgetItem>

#include "ToolpathDockWindow.h"
#include "UIManager.h"

#include "sstream"

namespace CamGui {

ToolpathDockWindow::ToolpathDockWindow(Gui::Document*  pcDocument, QWidget *parent)
  : DockWindow(pcDocument, parent), ui(new Ui_ToolpathDockWindow)
{
	setWindowTitle(QString::fromUtf8("Toolpath"));

    ui->setupUi(this);

    clearSelection();

    // listen for ToolPath Selection events
    QObject::connect(&UIManager(), SIGNAL(updatedToolPathSelection(Cam::ToolPathFeature*)), this,
          SLOT(updatedToolPathSelection(Cam::ToolPathFeature*)));

    // listen for Machine Program Selection events
    QObject::connect(&UIManager(), SIGNAL(updatedMachineProgramSelection(Cam::MachineProgramFeature*)), this,
          SLOT(updatedMachineProgramSelection(Cam::MachineProgramFeature*)));
}

ToolpathDockWindow::~ToolpathDockWindow() {
}

/**
 * Set the Toolpath to be displayed.  It expects the toolpath to be in HTML
 * format already.
 */
void ToolpathDockWindow::setToolPath(const QString &toolpath)
{
//  textedit->setHtml(toolpath);

    ui->Toolpath->setSelectionMode(QAbstractItemView::MultiSelection);
}

/**
 * Clear the output (i.e. no toolpath or machine program selected)
 */
void ToolpathDockWindow::clearSelection() {
    ui->Toolpath->clear();
    ui->Toolpath->setSelectionMode(QAbstractItemView::NoSelection);
    ui->Toolpath->addItem(QString::fromUtf8("No tool-path/machine program selected"));
}


/**
 * Receive messages to update the toolpath display
 */
void ToolpathDockWindow::updatedToolPathSelection(Cam::ToolPathFeature* toolpath) {
    if (toolpath != NULL) {

        const std::vector<std::string> tpcmds = toolpath->TPCommands.getValues();
        if (tpcmds.size() > 0) {

            // format the toolpath
            //TODO: do a colour coded rendering
            std::vector<std::string>::const_iterator it;
            std::stringstream ss;
            ss << "<p><b>Toolpath</b>:</p>" << std::endl;
            ss << "<ol>" << std::endl;
            for (it = tpcmds.begin(); it != tpcmds.end(); ++it)
                ss << "  <li><i>" << (*it) << "</i></li>" << std::endl;
            ss << "</ol>" << std::endl;

            // update display
            setToolPath(QString::fromStdString(ss.str()));
        }
        else
            clearSelection();
    }
    else
        clearSelection();
}

/**
 * Receive messages to update the machineProgram display
 */
void ToolpathDockWindow::updatedMachineProgramSelection(Cam::MachineProgramFeature* machineProgram) {
    if (machineProgram != NULL) {

        const std::vector<std::string> mpcmds = machineProgram->MPCommands.getValues();
        if (mpcmds.size() > 0) {

            // format the toolpath
            //TODO: do a colour coded rendering
            std::vector<std::string>::const_iterator it;
            std::stringstream ss;
            ss << "<p><b>Machine Program</b>:</p>" << std::endl;
            ss << "<ol>" << std::endl;
            for (it = mpcmds.begin(); it != mpcmds.end(); ++it)
                ss << "  <li><i>" << (*it) << "</i></li>" << std::endl;
            ss << "</ol>" << std::endl;

            // update display
            setToolPath(QString::fromStdString(ss.str()));
        }
        else
            clearSelection();
    }
    else
        clearSelection();
}

#include "moc_ToolpathDockWindow.cpp"

} /* namespace Cam */
