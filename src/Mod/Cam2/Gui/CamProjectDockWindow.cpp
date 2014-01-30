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
#include <QList>

#include <Base/Console.h>

#include "ui_CamProjectDockWindow.h"
#include "CamProjectDockWindow.h"
#include "UIManager.h"
#include "../App/TPG/TPGSettings.h"

namespace CamGui {

CamProjectDockWindow::CamProjectDockWindow(Gui::Document* pcDocument,
		QWidget *parent) :
		DockWindow(pcDocument, parent), ui(new Ui_CamProjectDockWindow) {
	setWindowTitle(tr("Cam project"));

	ui->setupUi(this);

	currentSettings = NULL;

	QObject::connect(&UIManager(), SIGNAL(updatedTPGSelection(Cam::TPGFeature*)), this,
			SLOT(updatedTPGSelection(Cam::TPGFeature*)));

	// receive tpg running state changes from Cam layer.
	QObject::connect(&CamGui::UIManager(), SIGNAL(updatedTPGStateSig(QString, Cam::TPG::State, int)),
			this, SLOT(updatedTPGState(QString, Cam::TPG::State, int)));
}

CamProjectDockWindow::~CamProjectDockWindow()
{
	// clear old form components
    QList<CamComponent*>::iterator it = components.begin();
    for (; it != components.end(); ++it)
	{
		QObject::disconnect(*it, SIGNAL(UpdatedCamComponentSignal(CamComponent *)),
					this, SLOT(UpdatedCamComponent(CamComponent *)));
        delete *it;
	}
    components.clear();
}

/**
 * Causes the setting area to display a form for editing the selected
 * setting.  The form is populated with the current values from the setting;
 * Caller needs to set the value to defaultvalue if that is desired.
 *
 * Provide a NULL newSettings to stop edit any settings.
 */
bool CamProjectDockWindow::editSettings(Cam::Settings::TPGSettings* newSettings, bool saveOld /*=true*/)
{
    // save old settings
    if (saveOld && ! saveSettings()) {
        //TODO: tell the user saving settings failed
    	Base::Console().Warning("Failed to update settings store!\n");

        // clear old form components
        QList<CamComponent*>::iterator it = components.begin();
        for (; it != components.end(); ++it)
		{
			QObject::disconnect(*it, SIGNAL(UpdatedCamComponentSignal(CamComponent *camComponent)),
						this, SLOT(UpdatedCamComponent(CamComponent *camComponent)));
            delete *it;
		}
        components.clear();
        return false;
    }

    // clear old form components
    QList<CamComponent*>::iterator it = components.begin();
    for (; it != components.end(); ++it)
	{
		QObject::disconnect(*it, SIGNAL(UpdatedCamComponentSignal(CamComponent *)),
						this, SLOT(UpdatedCamComponent(CamComponent *)));
        delete *it;
	}
    components.clear();

    // create the new form
    if (newSettings != NULL)
    {
        currentSettings = newSettings->grab();
        CamComponent *comp = NULL;
        bool failure = false;
        std::vector<Cam::Settings::Definition*> settings = newSettings->getSettings();
        std::vector<Cam::Settings::Definition*>::iterator it = settings.begin();
        for (; it != settings.end(); ++it)
        {
            Cam::Settings::Definition* setting = *it;
            if (setting != NULL)
            {
				if (setting->type == Cam::Settings::Definition::SettingType_Text)
                    comp = new CamTextBoxComponent();
				else if (setting->type == Cam::Settings::Definition::SettingType_Radio)
                    comp = new CamRadioComponent();
				else if (setting->type == Cam::Settings::Definition::SettingType_ObjectNamesForType)
                    comp = new CamTextBoxComponent();
				else if (setting->type == Cam::Settings::Definition::SettingType_SingleObjectNameForType)
                    comp = new CamComboBoxComponent();
				else if (setting->type == Cam::Settings::Definition::SettingType_Enumeration)
					comp = new CamComboBoxComponent();
				else if (setting->type == Cam::Settings::Definition::SettingType_Length)
                    comp = new CamLengthComponent();
				else if (setting->type == Cam::Settings::Definition::SettingType_Rate)
                    comp = new CamRateComponent();
				else if (setting->type == Cam::Settings::Definition::SettingType_Filename)
                    comp = new CamFilenameComponent();
				else if (setting->type == Cam::Settings::Definition::SettingType_Directory)
                    comp = new CamDirectoryComponent();
				else if (setting->type == Cam::Settings::Definition::SettingType_Color)
                    comp = new CamColorComponent();
				else if (setting->type == Cam::Settings::Definition::SettingType_Integer)
                    comp = new CamTextBoxComponent();
				else if (setting->type == Cam::Settings::Definition::SettingType_Double)
                    comp = new CamTextBoxComponent();
                else
                    continue;
                components.append(comp);
                if (!comp->makeUI(setting,(QFormLayout*)ui->SettingsWidget->layout())) {
                    Base::Console().Warning("CamProjectDockWindow::editSettings(): Failed to make UI Component '%s'!\n", setting->name.toStdString().c_str());
                    failure = true;
                }
				else
				{
					// Connect this new CamComponent object to this owning dialog class so that we
					// can receive signals indicating that one of the components has changed.
					QObject::connect(comp, SIGNAL(UpdatedCamComponentSignal(CamComponent *)),
						this, SLOT(UpdatedCamComponent(CamComponent *)));
				}
            }
            else
                Base::Console().Log("Warning: NULL setting provided\n");
        }
        return !failure;
    }

    return true;
}

void CamProjectDockWindow::UpdatedCamComponent(CamComponent *camComponent)
{
	// One of the CamComponent objects (that represent a single setting) has changed.
	// We need to go through all of the other CamComponent objects to see if the visible
	// flag on the setting they represent has also changed.  If so then we need
	// to ensure the QWidget::setVisible() method is called for them.  i.e. we want
	// the Cam::Settings::Definition::visible flag to be reflected in the
	// CamComponent::isVisible() flag as well.  The onChanged() functionality
	// used in the settings classes may have changed the visible flag as part of its
	// functionality.  We want the GUI's representation of those settings to reflect
	// these changes.


	for (QList<CamComponent*>::iterator itCamComponent = components.begin(); itCamComponent != components.end(); itCamComponent++)
	{
		(*itCamComponent)->guiSignalling(false);	// Disable GUI signalling while we're refreshing values without the user interface.
		(*itCamComponent)->setVisibleFlag();		// Display/Hide the widgets for this setting's GUI representation.
		(*itCamComponent)->refresh();				// Force the widgets representing the setting to be refreshed based on the setting's current value (in case the onChanged() method changed one of them)
		(*itCamComponent)->guiSignalling(true);		// Re-enable GUI signalling so that updates occur when the operator uses the GUI
	}
}


/**
 * Saves the current values from the UI into the settings objects.
 *
 * Note: this is called by default on when changing settings (if saveOld is
 * true).
 */
bool CamProjectDockWindow::saveSettings()
{
    bool failure = false;
    QList<CamComponent*>::iterator it = components.begin();
    for (; it != components.end(); ++it)
        if (!(*it)->close()) {
        	Base::Console().Warning("Failed to save '%s'\n", (*it)->name());
            failure = true;
        }
    return !failure;
}

/**
 * Slot that is called from the UIManager when the selection is changed.
 */
void CamProjectDockWindow::updatedTPGSelection(Cam::TPGFeature* tpgFeature) {

    if (tpgFeature != NULL) {
		Cam::Settings::TPGSettings *settings = tpgFeature->getTPGSettings();
		QStringList sl = settings->getActions();
		Base::Console().Log("Actions: %i",sl.size());
		if (!this->editSettings(settings))
			Base::Console().Error("Failed to edit settings for '%s'\n", tpgFeature->Label.getValue());
    }
    else
    	this->editSettings(NULL);
}


/**
 * Receive messages to update a progress bar
 */
void CamProjectDockWindow::updatedTPGState(QString tpgid, Cam::TPG::State state, int progress) {

	ProgressBar *pb = NULL;
	if (!progressBars.contains(tpgid)) {
		pb = new ProgressBar(this->ui->scrollAreaWidgetContents, tpgid);
		progressBars.insert(tpgid, pb);
	} else {
		pb = progressBars[tpgid];
	}
	pb->updateState(state, progress);

	if (state == Cam::TPG::FINISHED) {
		progressBars.remove(tpgid);

		delete pb; //TODO: delay this by a timer (maybe 10 seconds)
	}
}

#include "moc_CamProjectDockWindow.cpp"

} /* namespace CamGui */
