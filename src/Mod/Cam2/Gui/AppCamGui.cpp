/***************************************************************************
 *   Copyright (c) 2007 Werner Mayer <wmayer@users.sourceforge.net>        *
 *   Copyright (c) 2013 Andrew Robinson <andrewjrobinson@gmail.com>        *
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

#include <Base/Console.h>
#include <Base/Interpreter.h>

#include <Gui/Application.h>
#include <Gui/BitmapFactory.h>
#include <Gui/WidgetFactory.h>
#include <Gui/MainWindow.h>

#include "ViewProviderCamFeature.h"
#include "ViewProviderMachineFeature.h"
#include "ViewProviderMachineProgramFeature.h"
#include "ViewProviderToolFeature.h"
#include "ViewProviderToolPathFeature.h"
#include "ViewProviderTPGFeature.h"

#include "Workbench.h"

#include <vector>
#include <qmessagebox.h>

#include "TPGListModel.h"
#include "UIManager.h"
#include "TPGLibraryDockWindow.h"
#include "../App/TPG/PyTPGFactory.h"

// use a different name to CreateCommand()
void CreateCamCommands(void);


static PyObject *test(PyObject *self, PyObject *args)
{
  // get the TPGs
//  std::vector<Cam::TPGDescriptor*> plugins = Cam::PyTPGManager().scanPlugins();
//  for (int i = 0; i < plugins.size(); i++)
//    plugins[i]->print();

  CamGui::UIManager().reloadTPGs();
//  CamGui::UIManager().addTPG(NULL);

//  // get the dock window
//  Gui::DockWindowManager* pDockMgr = Gui::DockWindowManager::instance();
//  QWidget *tpgdockwidget = pDockMgr->getDockWindow("Cam_TPGLibraryDockWindow");
//  if (tpgdockwidget != NULL)
//  {
//    CamGui::TPGLibraryDockWindow *dwin = (CamGui::TPGLibraryDockWindow*) tpgdockwidget;
//    CamGui::TPGListModel *model = new CamGui::TPGListModel(&plugins);
//    dwin->setTPGList(model);
//  }
//  else
//    QMessageBox::information(Gui::getMainWindow(), "Information", "Failed to get TPGLibrary window");

  Py_Return;
}

/**
 * Asks the UIManager to reload the TPG library
 */
static PyObject *reloadTPGs(PyObject *self, PyObject *args)
{
  CamGui::UIManager().reloadTPGs();
  Py_Return;
}


/* registration table  */
static struct PyMethodDef CamGui_methods[] =
{
    {"reloadTPGs", reloadTPGs, METH_VARARGS, "Instructs the UIManager to reload the TPG Library."},
    {"test", test, METH_VARARGS, "Perform some UI testing."},
    {NULL, NULL}                   /* end of table marker */
};

extern "C"
{
    void CamGuiExport initCamGui()
    {
        if (!Gui::Application::Instance)
        {
            PyErr_SetString(PyExc_ImportError, "Cannot load Gui module in console application.");
            return;
        }

        // load dependend module
        try {
            Base::Interpreter().loadModule("Cam");
        }
        catch(const Base::Exception& e) {
            PyErr_SetString(PyExc_ImportError, e.what());
            return;
        }

        (void) Py_InitModule("CamGui", CamGui_methods);   /* mod name, table ptr */
        Base::Console().Log("Loading GUI of Cam module... done\n");

        CamGui::ViewProviderCamFeature              ::init();
        CamGui::ViewProviderMachineFeature          ::init();
        CamGui::ViewProviderMachineProgramFeature   ::init();
        CamGui::ViewProviderTPGFeature              ::init();
        CamGui::ViewProviderToolFeature             ::init();
        CamGui::ViewProviderToolPathFeature         ::init();
        CamGui::Workbench                           ::init();

        // instantiating the commands
        CreateCamCommands();

        return;
    }
} // extern "C"
