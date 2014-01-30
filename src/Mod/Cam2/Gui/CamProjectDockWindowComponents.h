/***************************************************************************
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

#ifndef CAMPROJECTDOCKWINDOWCOMPONENTS_H_
#define CAMPROJECTDOCKWINDOWCOMPONENTS_H_

#include <PreCompiled.h>

#include <QLineEdit>
#include <QFormLayout>
#include <QList>
#include <QWidget>
#include <QRadioButton>
#include <QMap>
#include <QPushButton>
#include <QToolButton>
#include <QComboBox>

#include "../App/TPG/TPGSettings.h"

namespace CamGui {


/**
	The CamComponent class is the base class used to represent a set of widgets that combine
	to represent a single Cam::Settings::Definition object (in this->tpgsetting).  Each one
	of these objects may hold pointers to a number of QWidget (based) objects.  eg: a single
	setting may be represented by  QLabelText, QLineEdit and/or QPushButton widgets.

	This class includes a Q_SIGNAL method (UpdatedCamComponentSignal()) that is tied to
	the CamProjectDockWindow object that owns all of the CamComponent objects.  Each time
	one of these CamComponent objects is updated, it emits a signal to the CamProjectDocWindow
	object indicating such.  The CamProjectDocWindow lets all the corresponding CamComponent
	objects know that one of the settings has changed.  This is the mechanism by which the
	Cam::Settings::Definition::visible flag is reflected in the various QWidget::setVisible()
	flag.  i.e. if a setting is changed then the onChanged() method may decide that another
	one of the setting's visibility flags should be turned on or off.  Such changes need
	to be reflected in the CamComponent objects (and hence its QWidgets) that represent
	that setting.
 */
class CamGuiExport CamComponent : public QObject {

Q_OBJECT;

protected:
    Cam::Settings::Definition *tpgsetting;
    QFormLayout* form;
    QList<QWidget*> rootComponents;
	QList<QWidget*> widgets_to_be_signalled;	// This differs from the rootComponents list only by the fact that one of the QLineEdit objects
												// needs to be signalled but, since it was added to a QLayout object, it must NOT be deleted
												// by us directly.

	/**
		The Validator class allows the QValidator mechanisms supported by the Qt library to ask
		the Cam::Settings::Definition class whether the value entered into the user interface
		is valid.  This allows the Cam::Settings::Definition::SettingType values to each implement
		their own validation rules without the need to link with the Qt library.
	 */
	class Validator : public QValidator
	{
	public:
		Validator( Cam::Settings::Definition *setting_definition, QObject *widget ) : QValidator(widget)
		{
			this->setting_definition = setting_definition->grab();
		}

		~Validator()
		{
			if (this->setting_definition)
			{
				this->setting_definition->release();
			}
		}

	virtual QValidator::State validate(QString &input, int &position) const;

	private:
		Cam::Settings::Definition *setting_definition;
	}; // End Validator class definition

	Validator *validator;

public:
    CamComponent();
    virtual ~CamComponent();

    /**
     * Creates the UI for this component and loads the initial value
     */
    virtual bool makeUI(Cam::Settings::Definition *tpgsetting, QFormLayout* form);

    /**
     * Saves the values on the UI to the TPGSetting instance
     */
    virtual bool close();

    /**
     * Get the name of this component's setting
     */
    const char* name() {
    	return tpgsetting->name.toStdString().c_str();
    }

	static QString FractionalRepresentation( const double original_value, const int max_denominator = 64 );

	/// Ensure the QWidgets that represent this setting have a visible flag that matches the setting's visible flag's value.
	void setVisibleFlag();

private:
	bool gui_signalling_enabled;

public:
	bool guiSignalling() const { return(this->gui_signalling_enabled); }
	void guiSignalling(const bool value) { this->gui_signalling_enabled = value; }

	virtual void refresh() {}	// Refresh the values displayed in the widgets based on the current settings values.

	/**
		This method emits the UpdatedCamComponentSignal() signal to call the CamProjectDocWindow::UpdatedCamComponent() method.

		It should be called by all parent classes when any of their values changes.
	 */
	void signalUpdated();

Q_SIGNALS:
	// This signal is tied to the CamProjectDockWindow::UpdateCamComponent() method via the QObject::connect() method.
	// It indicates that the setting represented by this CamComponent has changed in some way.  The
	// CamProjectDocWindow::UpdateCamComponent() method signals all the other CamComponent objects (of the
	// Settings editor dialog box) in case they need to change anything (such as their visibility flags).
	void UpdatedCamComponentSignal(CamComponent *camComponent);

};


/**
	The CamLineEdit class is only used so that we can register callback functions for the QLineEdit
	objects to handle the validation as well as a mechanism to set the TPGFeature::PropTPGSettings map
	based on the newly changed value.
 */
class CamLineEdit : public QLineEdit
{
public:
	CamLineEdit(QWidget *parent, Cam::Settings::Definition *tpgsetting) : QLineEdit(parent) 
	{
		this->tpgSetting = tpgsetting->grab();
	}

	~CamLineEdit()
	{
		if (this->tpgSetting)
		{
			this->tpgSetting->release();
		}
	}

	virtual void focusOutEvent ( QFocusEvent * e );
	virtual void refresh();

private:
	Cam::Settings::Definition *tpgSetting;
};

// ----- CamTextBoxComponent ---------------------------------------------------------
/**
 * Object that manages a Cam::TextBox setting
 *
 * This is used to represent the ObjectNamesForType, Integer, Text and Double settings
 * types.  This association is made in the CamProjectDockWindow::editSettings() method.               
 */
class CamGuiExport CamTextBoxComponent: public CamComponent {

	Q_OBJECT

protected:
    CamLineEdit *widget;

public:

    CamTextBoxComponent();

    /**
     * Creates the UI for this component and loads the initial value
     */
    virtual bool makeUI(Cam::Settings::Definition *tpgsetting, QFormLayout* form);

    /**
     * Saves the values on the UI to the TPGSetting instance
     */
    virtual bool close();
	virtual void refresh();

public Q_SLOTS:

    /**
     * Slot to receive messages when the user changes the text value
     */
    void editingFinished();
	void handleButton();
	void handleAddObjectNameButton();
	void handleOKButton();

};



// ----- CamLengthComponent ---------------------------------------------------------
/**
 * Object that manages a Cam::Length setting
 */
class CamGuiExport CamLengthComponent: public CamComponent {

	Q_OBJECT

protected:
    CamLineEdit *camLineEdit;
	QComboBox	*combo_box;

public:

    CamLengthComponent();

    /**
     * Creates the UI for this component and loads the initial value
     */
    virtual bool makeUI(Cam::Settings::Definition *tpgsetting, QFormLayout* form);

    /**
     * Saves the values on the UI to the TPGSetting instance
     */
    virtual bool close();
	virtual void refresh();

public Q_SLOTS:

    /**
     * Slot to receive messages when the user changes the text value
     */
    void editingFinished();
	void currentIndexChanged(int index);

private:
	typedef Cam::Settings::Definition::Units_t Id_t;
	typedef QString Label_t;
	std::vector< std::pair< Id_t, Label_t > > values;

};



// ----- CamRateComponent ---------------------------------------------------------
/**
 * Object that manages a Cam::Rate setting
 */
class CamGuiExport CamRateComponent: public CamComponent {

	Q_OBJECT

protected:
    CamLineEdit *camLineEdit;
	QComboBox	*combo_box;

public:

    CamRateComponent();

    /**
     * Creates the UI for this component and loads the initial value
     */
    virtual bool makeUI(Cam::Settings::Definition *tpgsetting, QFormLayout* form);

    /**
     * Saves the values on the UI to the TPGSetting instance
     */
    virtual bool close();
	virtual void refresh();

public Q_SLOTS:

    /**
     * Slot to receive messages when the user changes the text value
     */
    void editingFinished();
	void currentIndexChanged(int index);

private:
	typedef Cam::Settings::Definition::Units_t Id_t;
	typedef QString Label_t;
	std::vector< std::pair< Id_t, Label_t > > values;

};



// ----- CamRadioComponent ---------------------------------------------------------
/**
 * Object that manages a Cam::Radio setting
 */


class CamGuiExport CamRadioComponent: public CamComponent {
	Q_OBJECT;

protected:
//    QList<QRadioButton *> radios;
    QMap<QString, QRadioButton*> radios;

public:

    CamRadioComponent();
	~CamRadioComponent();

    /**
     * Creates the UI for this component and loads the initial value
     */
    virtual bool makeUI(Cam::Settings::Definition *tpgsetting, QFormLayout* form);

    /**
     * Saves the values on the UI to the TPGSetting instance
     */
    virtual bool close();
	virtual void refresh();

public Q_SLOTS:
		void clicked(bool checked);

};


// ----- CamComboBoxComponent ---------------------------------------------------------
/**
 * Object that manages a Cam::Settings::Definition::SettingType_Enumeration setting
 */
class CamGuiExport CamComboBoxComponent: public CamComponent {

	Q_OBJECT

public:

    CamComboBoxComponent();
	QComboBox *combo_box;

    /**
     * Creates the UI for this component and loads the initial value
     */
    virtual bool makeUI(Cam::Settings::Definition *tpgsetting, QFormLayout* form);

    /**
     * Saves the values on the UI to the TPGSetting instance
     */
    virtual bool close();
	virtual void refresh();

public Q_SLOTS:

    /**
     * Slot to receive messages when the user changes the text value
     */
    void currentIndexChanged(int index);

private:
	typedef QString Id_t;
	typedef QString Label_t;
	typedef std::vector< std::pair< Id_t, Label_t > > Values_t;
	Values_t values;
};


// ----- CamFilenameComponent ---------------------------------------------------------
/**
 * Object that manages a Cam::Filename setting
 */
class CamGuiExport CamFilenameComponent: public CamComponent {

	Q_OBJECT

protected:
	QToolButton	*button;
    CamLineEdit *camLineEdit;

public:

    CamFilenameComponent();

    /**
     * Creates the UI for this component and loads the initial value
     */
    virtual bool makeUI(Cam::Settings::Definition *tpgsetting, QFormLayout* form);

    /**
     * Saves the values on the UI to the TPGSetting instance
     */
    virtual bool close();
	virtual void refresh();

private Q_SLOTS:

    /**
     * Slot to receive messages when the user changes the text value
     */
    void editingFinished();
	void handleButton();

};



// ----- CamDirectoryComponent ---------------------------------------------------------
/**
 * Object that manages a Cam::Directory setting
 */
class CamGuiExport CamDirectoryComponent: public CamComponent {

	Q_OBJECT

protected:
	QToolButton	*button;
    CamLineEdit *camLineEdit;

public:

    CamDirectoryComponent();

    /**
     * Creates the UI for this component and loads the initial value
     */
    virtual bool makeUI(Cam::Settings::Definition *tpgsetting, QFormLayout* form);

    /**
     * Saves the values on the UI to the TPGSetting instance
     */
    virtual bool close();
	virtual void refresh();

private Q_SLOTS:

    /**
     * Slot to receive messages when the user changes the text value
     */
    void editingFinished();
	void handleButton();

};




// ----- CamColorComponent ---------------------------------------------------------
/**
 * Object that manages a Cam::Directory setting
 */
class CamGuiExport CamColorComponent: public CamComponent {

	Q_OBJECT

protected:
	QPushButton	*button;

public:

    CamColorComponent();

    /**
     * Creates the UI for this component and loads the initial value
     */
    virtual bool makeUI(Cam::Settings::Definition *tpgsetting, QFormLayout* form);

    /**
     * Saves the values on the UI to the TPGSetting instance
     */
    virtual bool close();
	virtual void refresh();

private Q_SLOTS:

    /**
     * Slot to receive messages when the user changes the text value
     */
	void handleButton();

};

} /* namespace Cam */
#endif /* CAMPROJECTDOCKWINDOWCOMPONENTS_H_ */
