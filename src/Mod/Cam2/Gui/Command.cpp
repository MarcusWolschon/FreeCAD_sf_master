/***************************************************************************
 *   Copyright (c) 2012 Luke Parry    (l.parry@warwick.ac.uk)              *
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

#include <App/DocumentObjectGroup.h>
#include <Gui/Command.h>
#include <Gui/Document.h>
#include <Gui/Application.h>
#include <Gui/MainWindow.h>
#include <QPointer>

#include "../App/Features/CamFeature.h"
#include "../App/Features/TPGFeature.h"

#include "UIManager.h"


//===========================================================================
// CmdCamFeature
//===========================================================================
DEF_STD_CMD_A(CmdCamFeature);

CmdCamFeature::CmdCamFeature()
  :Command("Cam_CamFeature")
{
    sAppModule    = "Cam";
    sGroup        = QT_TR_NOOP("Cam");
    sMenuText     = QT_TR_NOOP("Create a new Cam Feature");
    sToolTipText  = QT_TR_NOOP("Create a new Cam Feature");
    sWhatsThis    = sToolTipText;
    sStatusTip    = sToolTipText;
    sPixmap       = "Cam_CamFeature";
}

void CmdCamFeature::activated(int iMsg)
{
    openCommand("CamFeatureNew");
	if (CamGui::UIManager().CamFeature()) {
//		updateActive();
    	commitCommand();
	}
	else
		abortCommand();
}

bool CmdCamFeature::isActive(void)
{
    return hasActiveDocument();
}

//===========================================================================
// CmdCamTPGFeature
//===========================================================================
DEF_STD_CMD_A(CmdCamTPGFeature);

CmdCamTPGFeature::CmdCamTPGFeature()
  :Command("Cam_TPGFeature")
{
    sAppModule    = "Cam";
    sGroup        = QT_TR_NOOP("Cam");
    sMenuText     = QT_TR_NOOP("Create a new TPG Feature");
    sToolTipText  = QT_TR_NOOP("Create a new TPG Feature");
    sWhatsThis    = sToolTipText;
    sStatusTip    = sToolTipText;
    sPixmap       = "Cam_TPGFeature";
}

void CmdCamTPGFeature::activated(int iMsg)
{
    openCommand("New TPG Feature");
	if (CamGui::UIManager().TPGFeature()) {
    	commitCommand();
	}
	else
		abortCommand();
}

bool CmdCamTPGFeature::isActive(void)
{
    return hasActiveDocument();
}

//===========================================================================
// CmdCamToolFeature
//===========================================================================
DEF_STD_CMD_A(CmdCamToolFeature);

CmdCamToolFeature::CmdCamToolFeature()
  :Command("Cam_ToolFeature")
{
    sAppModule    = "Cam";
    sGroup        = QT_TR_NOOP("Cam");
    sMenuText     = QT_TR_NOOP("New Tool");
    sToolTipText  = QT_TR_NOOP("Create a new tool definition");
    sWhatsThis    = sToolTipText;
    sStatusTip    = sToolTipText;
    sPixmap       = "Cam_ToolFeature";
}

void CmdCamToolFeature::activated(int iMsg)
{
    openCommand("New Tool Feature");
    if (CamGui::UIManager().ToolFeature()) {
        commitCommand();
    }
    else
        abortCommand();
}

bool CmdCamToolFeature::isActive(void)
{
    return hasActiveDocument();
}

//===========================================================================
// CmdCamMachineFeature
//===========================================================================
DEF_STD_CMD_A(CmdCamMachineFeature);

CmdCamMachineFeature::CmdCamMachineFeature()
  :Command("Cam_MachineFeature")
{
    sAppModule    = "Cam";
    sGroup        = QT_TR_NOOP("Cam");
    sMenuText     = QT_TR_NOOP("New Machine");
    sToolTipText  = QT_TR_NOOP("Create a new machine definition");
    sWhatsThis    = sToolTipText;
    sStatusTip    = sToolTipText;
    sPixmap       = "Cam_MachineFeature";
}

void CmdCamMachineFeature::activated(int iMsg)
{
    openCommand("New Machine Feature");
    if (CamGui::UIManager().MachineFeature()) {
        commitCommand();
    }
    else
        abortCommand();
}

bool CmdCamMachineFeature::isActive(void)
{
    return hasActiveDocument();
}

//===========================================================================
// CmdCamTPGFeature
//===========================================================================
DEF_STD_CMD_A(CmdCamRunTPG);

CmdCamRunTPG::CmdCamRunTPG()
  :Command("Cam_RunTPG")
{
    sAppModule    = "Cam";
    sGroup        = QT_TR_NOOP("Cam");
    sMenuText     = QT_TR_NOOP("Run TPG");
    sToolTipText  = QT_TR_NOOP("Executes the selected TPG to (re)produce its Tool Path");
    sWhatsThis    = sToolTipText;
    sStatusTip    = sToolTipText;
    sPixmap       = "Cam_RunTPG";
}

void CmdCamRunTPG::activated(int iMsg)
{
    CamGui::UIManager().RunTPG();
}

bool CmdCamRunTPG::isActive(void)
{
    return hasActiveDocument();
}


//===========================================================================
// CmdCamPostProcess
//===========================================================================
DEF_STD_CMD_A(CmdCamPostProcess);

CmdCamPostProcess::CmdCamPostProcess()
  :Command("Cam_PostProcess")
{
    sAppModule    = "Cam";
    sGroup        = QT_TR_NOOP("Cam");
    sMenuText     = QT_TR_NOOP("Post Process");
    sToolTipText  = QT_TR_NOOP("Executes toolpath to (re)produce its Machine Program");
    sWhatsThis    = sToolTipText;
    sStatusTip    = sToolTipText;
    sPixmap       = "Cam_PostProcess";
}

void CmdCamPostProcess::activated(int iMsg)
{
    CamGui::UIManager().PostProcess();
}

bool CmdCamPostProcess::isActive(void)
{
    return hasActiveDocument();
}


//===========================================================================
// CmdCamWatchHighlight
//===========================================================================
DEF_STD_CMD_A(CmdCamWatchHighlight);

CmdCamWatchHighlight::CmdCamWatchHighlight()
  :Command("Cam_WatchHighlight")
{
    sAppModule    = "Cam";
    sGroup        = QT_TR_NOOP("Cam");
    sMenuText     = QT_TR_NOOP("WatchHighlight");
    sToolTipText  = QT_TR_NOOP("WatchHighlight");
    sWhatsThis    = sToolTipText;
    sStatusTip    = sToolTipText;
    sPixmap       = "Cam_WatchHighlight";
}

void CmdCamWatchHighlight::activated(int iMsg)
{
    CamGui::UIManager().WatchHighlight();
}

bool CmdCamWatchHighlight::isActive(void)
{
    return hasActiveDocument();
}

//===========================================================================
// Common Code
//===========================================================================
/**
 *
 */
void CreateCamCommands()
{
    Gui::CommandManager &rcCmdMgr = Gui::Application::Instance->commandManager();
    rcCmdMgr.addCommand(new CmdCamFeature());
    rcCmdMgr.addCommand(new CmdCamToolFeature());
    rcCmdMgr.addCommand(new CmdCamTPGFeature());
    rcCmdMgr.addCommand(new CmdCamMachineFeature());
    rcCmdMgr.addCommand(new CmdCamRunTPG());
	rcCmdMgr.addCommand(new CmdCamPostProcess());
    rcCmdMgr.addCommand(new CmdCamWatchHighlight());
}
