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

#ifndef CAMSETTINGSDOCKWINDOW_H_
#define CAMSETTINGSDOCKWINDOW_H_

#include <Gui/DockWindow.h>

#include <QLabel>
#include <QList>
#include <QMap>
#include <QObject>
#include <QProgressBar>

#include "ui_CamProjectDockWindow.h"

#include "../App/TPG/TPG.h"
#include "../App/TPG/TPGSettings.h"

#include "CamProjectDockWindowComponents.h"

namespace CamGui {

class CamProjectDockWindow: public Gui::DockWindow {

Q_OBJECT

public:
	CamProjectDockWindow(Gui::Document* pcDocument, QWidget *parent = 0);
	virtual ~CamProjectDockWindow();

	/**
	 * Causes the setting area to display a form for editing the selected
	 * setting.  The form is populated with the current values from the setting;
	 * Caller needs to set the value to defaultvalue if that is desired.
	 *
	 * Provide a NULL newSettings to stop edit any settings.
	 */
	bool editSettings(Cam::Settings::TPGSettings* newSettings, bool saveOld = true);

	/**
	 * Saves the current values from the UI into the settings objects.
	 *
	 * Note: this is called by default on when changing settings (if saveOld is
	 * true).
	 */
	bool saveSettings();

protected:

	class ProgressBar {
		QString name;
		QWidget *parent;

		QLabel *progressLabel;
		QProgressBar *progressBar;

	public:
		ProgressBar(QWidget *parent, QString name) {
			this->parent = parent;
			this->name = name;

			progressLabel = new QLabel(parent);
			progressLabel->setObjectName(QString::fromUtf8("label") + name);
			progressLabel->setText(name); //TODO: get the name of the TPG

			progressBar = new QProgressBar(parent);
			progressBar->setObjectName(QString::fromUtf8("progressBar") + name);
			progressBar->setValue(0);
			progressBar->setTextVisible(true);
			progressBar->setRange(0, 100);

			QVBoxLayout *layout = (QVBoxLayout*) parent->layout();
			layout->insertWidget(layout->count()-1, progressLabel);
			layout->insertWidget(layout->count()-1, progressBar);
		}
		virtual ~ProgressBar() {
			if (progressLabel != NULL) {
				delete progressLabel;
				progressLabel = NULL;
			}
			if (progressBar != NULL) {
				delete progressBar;
				progressBar = NULL;
			}
		}
		/**
		 * Updates the TPG Status progressbar
		 */
		void updateState(Cam::TPG::State state, int progress) {
			progressLabel->setText(name + QString::fromUtf8(": ") + Cam::TPG::stateToStr(state));
			if (progress >= 0 && progress <= 100)
				progressBar->setValue(progress);
		}
	};

	Cam::Settings::TPGSettings *currentSettings;
	QList<CamComponent*> components;

	QMap<QString, ProgressBar*> progressBars;

public Q_SLOTS:
	/**
	 * Receive messages to update the settings area for a tpg
	 */
	void updatedTPGSelection(Cam::TPGFeature* tpgFeature);

	/**
	 * Receive messages to update a progress bar
	 */
	void updatedTPGState(QString tpgid, Cam::TPG::State state, int progress);

	/**
	 * Receive messages indicating that one of the CamComponent objects has changed.  We need to
	 * run through all CamComponents to see if any of their 'Settings::Definition::visible' flags has changed
	 * at which point we should change their QWidget::isVisible() flag to the same value.  i.e. to
	 * either hide or re-display the user interface items that represent that setting in this dialog.
	 */
	void UpdatedCamComponent(CamComponent *camComponent);

private:
	Ui_CamProjectDockWindow* ui;
};

} /* namespace CamGui */
#endif /* CAMSETTINGSDOCKWINDOW_H_ */
