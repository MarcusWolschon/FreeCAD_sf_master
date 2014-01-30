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

#include <PreCompiled.h>
#ifndef _PreComp_
#endif

#include <cstdlib>
#include <QLabel>
#include <QIntValidator>
#include <QMessageBox>
#include <QComboBox>
#include <QFileDialog>
#include <QColor>
#include <QColorDialog>
#include <QPalette>

#include <Base/Console.h>

#include <App/Document.h>
#include <App/Application.h>
#include <App/DocumentObject.h>

#include "CamProjectDockWindowComponents.h"

namespace CamGui {


// ----- CamComponent ---------------------------------------------------------
CamComponent::CamComponent() {
    form = NULL;
    tpgsetting = NULL;
	validator = NULL;
}

CamComponent::~CamComponent() {

	close();

    // release my hold on the settings object
    if (tpgsetting != NULL) {
        tpgsetting->release();
        tpgsetting = NULL;
    }

    // Remove my components
    QList<QWidget*>::iterator it = rootComponents.begin();
    for (; it != rootComponents.end(); ++it)
	{
        delete *it;
	}
    rootComponents.clear();

	// NOTE: There is no need to explicitly delete the Validator object as the deletion of
	// the QWidget implicitly deletes the Validator object for us.
}

void CamComponent::setVisibleFlag()
{
	if (this->tpgsetting)
	{
		// For all QWidget objects that represent this Settings::Definition object in the settings
		// editor dialog...
		for (QList<QWidget *>::iterator it = widgets_to_be_signalled.begin(); it != widgets_to_be_signalled.end(); it++)
		{
			// Only force the update if the value has changed.  I don't know if it would flicker if
			// we re-set it to the original value but it's simply enough to avoid such a possibility
			// by checking first.

			if ((*it)->isVisible() != this->tpgsetting->visible)
			{
				// Signal the QWidget to either become visible or become invisible.  Something has
				// changed the Settings::Definition::visible flag so we want the widgets representing
				// that setting to reflect this change.

				(*it)->setVisible(this->tpgsetting->visible);
			}
		}
	}
}

/**
	This CamComponent's setting has changed.  Signal the CamProjectDocWindow object so that
	it can let all the corresponding CamComponent objects know that one of their partners
	has changed.  This ends up calling the CamProjectDocWindow::UpdatedCamComponent() method
	via the QObject::connect() mechanism.
 */
void CamComponent::signalUpdated()
{
	// We disable GUI signalling when the program changes settings as opposed to the operator changing
	// them using the GUI.  When we change the values without the GUI, we don't want all the GUI
	// signalling, and hence updating, that goes on when the operator uses the GUI to change values.

	if (this->guiSignalling())
	{
		// The GUI was used to change something.  Let everyone know that this has occured just
		// in case they want to update themselves as a result.
		Q_EMIT UpdatedCamComponentSignal(this);
	}
}



/**
	Call the registered validate() method for this setting and convert the results
	into a QValidator::State value.  The Cam::Setting::Definition::validate() method
	doesn't return a QValidator::State value directly because the Cam::Setting::Definition
	objects can exist in a non-gui environment while the QValidator class cannot.
 */
QValidator::State CamComponent::Validator::validate(QString & input, int & position) const
{
	switch(this->setting_definition->validate(input, position))
	{
	case Cam::Settings::Definition::Acceptable:
		return(QValidator::Acceptable);

	case Cam::Settings::Definition::Intermediate:
		return(QValidator::Intermediate);

	default:
	case Cam::Settings::Definition::Invalid:
		return(QValidator::Invalid);
	}
}

/**
 * Creates the UI for this component and loads the initial value
 */
bool CamComponent::makeUI(Cam::Settings::Definition *tpgsetting, QFormLayout* form) {
    Base::Console().Log("Warning: Unimplemented makeUI() method or unnecessary call to base method.");
    return false;
}

/**
 * Saves the values on the UI to the TPGSetting instance
 */
bool CamComponent::close() {
    Base::Console().Log("Warning: Unimplemented save() method or unnecessary call to base method.");
    return false;
}

/**
	Find a fraction that represents this floating point number.  We use this
	purely for readability purposes.  It only looks accurate to the nearest 1/64th

	eg: 0.125 -> "1/8"
	    1.125 -> "1+(1/8)"
	    0.109375 -> "7/64"

	We NEED the QString value returned by this method to be something that can be interpreted
	by the internal Python interpreter to produce a floating point number.  That's why we need
	to include the plus (+) character when the value is greater than one.
 */
/* static */ QString CamComponent::FractionalRepresentation( const double original_value, const int max_denominator /* = 64 */ )
{

	std::ostringstream result;
	double _value(original_value);
	double tolerance = 0.00001;

	if (floor(_value) > 0)
	{
		result << floor(_value);
		_value -= floor(_value);
	} // End if - then

	double fraction = 0.0;
	if ( ((_value > fraction) && ((_value - fraction) < tolerance)) ||
	     ((_value < fraction) && ((fraction - _value) < tolerance)) ||
	     (_value == fraction) )
	{
		return(QString::fromStdString(result.str()));
	} // End if - then

	if (result.str().length() > 0)
	{
	    result << "+(";
	}

	// We only want even numbers between 2 and 64 for the denominator.  The others just look 'wierd'.
	for (int denominator = 2; denominator <= max_denominator; denominator *= 2)
	{
		for (int numerator = 1; numerator < denominator; numerator++)
		{
			double fraction = double( double(numerator) / double(denominator) );
			if ( ((_value > fraction) && ((_value - fraction) < tolerance)) ||
			     ((_value < fraction) && ((fraction - _value) < tolerance)) ||
			     (_value == fraction) )
			{
				result << numerator << "/" << denominator;
				if (result.str().find('+') != std::string::npos)
				{
					result << ")";
				}
				return(QString::fromStdString(result.str()));
			} // End if - then
		} // End for
	} // End for

	return(QString::null); // It's not a recognisable fraction.  Return nothing to indicate such.
} // End FractionalRepresentation() method


// ----- CamTextBoxComponent ---------------------------------------------------------


CamTextBoxComponent::CamTextBoxComponent()
: CamComponent() {
    this->widget = NULL;
}


/**
 * Slot to receive messages when the user changes the text value
 */
void CamTextBoxComponent::editingFinished() {
	if (widget != NULL && tpgsetting != NULL) {
		QString qvalue = widget->text();
		Cam::Settings::Integer *pIntSetting = dynamic_cast<Cam::Settings::Integer *>(tpgsetting);
		Cam::Settings::Double *pDoubleSetting = dynamic_cast<Cam::Settings::Double *>(tpgsetting);
		if (pIntSetting)
		{
			// Interpret the value using the Python interpreter.

			int value;
			if (pIntSetting->Evaluate(qvalue.toAscii().constData(), &value))
			{
				pIntSetting->set( value );
				std::ostringstream ossValue;
				ossValue << value;
				widget->setText( QString::fromStdString(ossValue.str()) );
			}
			else
			{
				Base::Console().Error("Could not evaluate '%s'\n", qvalue.toStdString().c_str());
				widget->setText(tpgsetting->getValue());
			}
		}
		else if (pDoubleSetting)
		{
			// Interpret the value using the Python interpreter.

			double value;
			if (pDoubleSetting->Evaluate(qvalue.toAscii().constData(), &value))
			{
				pDoubleSetting->set( value );
				std::ostringstream ossValue;
				ossValue << value;
				widget->setText( QString::fromStdString(ossValue.str()) );
			}
			else
			{
				Base::Console().Error("Could not evaluate '%s'\n", qvalue.toStdString().c_str());
				widget->setText(tpgsetting->getValue());
			}
		}
		else {
			if (!tpgsetting->setValue(qvalue))
			{
				Base::Console().Error("Saving failed: '%s'\n", tpgsetting->name.toStdString().c_str());
				widget->setText(tpgsetting->getValue());
			}
		}
	}
	else
		Base::Console().Error("Saving a setting failed!\n");

	// Signal the parent dialog that we may have changed something so that it can let all the other
	// CamComponent objects know to re-check their visible flags for changes too.
	signalUpdated();
}

void CamLineEdit::focusOutEvent ( QFocusEvent * e )
{
	if (this->hasAcceptableInput() == false)
	{
		QString message = QString::fromAscii("Value for ");
		message += this->tpgSetting->name;
		message += QString::fromAscii(" has been rejected as invalid");

		QMessageBox message_box;
		message_box.setText(message);
		message_box.setInformativeText(QString::fromAscii("Do you want to keep the value entered or revert to the previous value?"));
		message_box.setStandardButtons( QMessageBox::Save | QMessageBox::Discard );
		message_box.setDefaultButton( QMessageBox::Discard );
		int response = message_box.exec();
		if (response == QMessageBox::Discard)
		{
			this->refresh();
		}
		else
		{
			QLineEdit::focusOutEvent(e);
		}
	}
	else
	{
		QLineEdit::focusOutEvent(e);
	}
}

void CamLineEdit::refresh()
{
	switch(this->tpgSetting->type)
	{
	case Cam::Settings::Definition::SettingType_Length:
		{
			Cam::Settings::Length *length = dynamic_cast<Cam::Settings::Length *>(this->tpgSetting);
			if (length != NULL)
			{
				std::ostringstream ossValue;
				ossValue << length->get(length->getUnits());
				this->setText(QString::fromStdString(ossValue.str()));
				
			}
		}
		break;

	case Cam::Settings::Definition::SettingType_Rate:
		{
			Cam::Settings::Rate *rate = dynamic_cast<Cam::Settings::Rate *>(this->tpgSetting);
			if (rate != NULL)
			{
				std::ostringstream ossValue;
				ossValue << rate->get(rate->getUnits());
				this->setText(QString::fromStdString(ossValue.str()));
			}
		}
		break;

	case Cam::Settings::Definition::SettingType_Double:
		{
			Cam::Settings::Double *double_setting = dynamic_cast<Cam::Settings::Double *>(this->tpgSetting);
			if (double_setting != NULL)
			{
				std::ostringstream ossValue;
				ossValue << double_setting->get();
				this->setText(QString::fromStdString(ossValue.str()));
			}
		}
		break;

	case Cam::Settings::Definition::SettingType_Integer:
		{
			Cam::Settings::Integer *integer_setting = dynamic_cast<Cam::Settings::Integer *>(this->tpgSetting);
			if (integer_setting != NULL)
			{
				std::ostringstream ossValue;
				ossValue << integer_setting->get();
				this->setText(QString::fromStdString(ossValue.str()));
			}
		}
		break;

	default:
		this->setText(this->tpgSetting->getValue());
	}
}

void CamTextBoxComponent::refresh()
{
	if ((widget != NULL) && (tpgsetting != NULL))
	{
		widget->refresh();
	}
}


/**
	This is a button handler for the dialog that is created
	within the CamTextBoxComponent::handleButton() method.  This
	handler is called when the add_button is pressed.
 */
void CamTextBoxComponent::handleAddObjectNameButton()
{
}

/**
	This is a button handler for the dialog that is created
	within the CamTextBoxComponent::handleButton() method.  This
	handler is called when the ok_button is pressed.
 */
void CamTextBoxComponent::handleOKButton()
{
}

/**
	This is a button handler for the small QToolButton presented next
	to a Cam::Text setting in the settings user interface.  The QToolButton
	has the three period (...) name in the hope that it denotes an 'edit' function.

	The intent of this method is to generate a dialog 'on the fly' that will allow
	the operator to view and select objects from the main document whose types
	are included within this setting's options.  eg: Cast the
	this->tpgsetting pointer to a Setting::ObjectNamesForType pointer and use
	the GetTypes() method to retrieve a list of object type strings.

	In the end, we need to re-create a single QString value that includes a comma
	delimited list of object names. This single QString will form the 'value' of
	the setting.

	At this stage, this is really just a place-holder for a proper dialog
	implementation.
 */
void CamTextBoxComponent::handleButton()
{
	if (this->tpgsetting->type == Cam::Settings::Definition::SettingType_ObjectNamesForType)
	{
		boost::scoped_ptr<QDialog> dialog(new QDialog);
		boost::scoped_ptr<QVBoxLayout> layout(new QVBoxLayout);
		boost::scoped_ptr<QPushButton> add_button(new QPushButton);
		boost::scoped_ptr<QPushButton> ok_button(new QPushButton);
		boost::scoped_ptr<QLineEdit> dummy_edit(new QLineEdit);

		add_button->setText(QString::fromAscii("Add"));
		ok_button->setText(QString::fromAscii("OK"));
		dummy_edit->setText(QString::fromAscii("One day this dialog will present a list of all objects whose types match this setting and allow the object names to be selected"));

		add_button->connect( add_button.get(), SIGNAL(clicked()), this, SLOT(handleAddObjectNameButton()));
		layout->addWidget(add_button.get());

		ok_button->connect( ok_button.get(), SIGNAL(clicked()), this, SLOT(handleOKButton()));
		layout->addWidget(ok_button.get());

		layout->addWidget(dummy_edit.get());

		dialog->setGeometry(0,0,200,100);
		QString title = QString::fromAscii("Edit object names list");
		dialog->setWindowTitle(title);
		dialog->setLayout(layout.get());
		dialog->setWindowFlags(Qt::Dialog);		

		dialog->show();
		dialog->exec();
	}
}

/**
 * Creates the UI for this component and loads the initial value
 *
 * This type of object is used for settings whose 'value' can be expressed
 * as a single string.  i.e. SettingType_Text, SettingType_Double, SettingType_Filename
 * SettingType_Directory and SettingType_ObjectNamesForType
 */
bool CamTextBoxComponent::makeUI(Cam::Settings::Definition *tpgsetting, QFormLayout* form) {
    if (tpgsetting != NULL)
    {
        // grab a copy of the setting so we can save it later
        this->tpgsetting = tpgsetting->grab();

        // construct the ui
        QWidget *parent = dynamic_cast<QWidget*>(form->parent());
        if (parent != NULL) {
            int row = form->rowCount();
            QString qname = tpgsetting->getFullname();

            // make the label
            QWidget *labelWidget = new QLabel(tpgsetting->label, parent);
			widgets_to_be_signalled.push_back(labelWidget);
			labelWidget->setObjectName(qname + QString::fromUtf8("Label"));
            form->setWidget(row, QFormLayout::LabelRole, labelWidget);
            labelWidget->setToolTip(tpgsetting->helptext);
			labelWidget->setVisible(tpgsetting->visible);

			// make the container
            QWidget *container = new QWidget(parent);
			widgets_to_be_signalled.push_back(container);
            QHBoxLayout *layout = new QHBoxLayout(container);
            layout->setContentsMargins(0,0,0,0);
            container->setLayout(layout);
            container->setObjectName(qname);
            container->setToolTip(tpgsetting->helptext);
			container->setVisible(tpgsetting->visible);
			form->setWidget(row, QFormLayout::FieldRole, container);

            // make the edit box
            this->widget = new CamLineEdit(parent, tpgsetting);
			widgets_to_be_signalled.push_back(this->widget);
            this->widget->setObjectName(qname);
			this->widget->setVisible(tpgsetting->visible);

			switch(tpgsetting->type)
			{
			case Cam::Settings::Definition::SettingType_Double:
			case Cam::Settings::Definition::SettingType_Text:
			case Cam::Settings::Definition::SettingType_ObjectNamesForType:
			case Cam::Settings::Definition::SettingType_Integer:
			default:
				if (tpgsetting->getValue() != QString::null)
				{
					this->widget->setText(tpgsetting->getValue());
				}
				else
				{
					this->widget->setText(tpgsetting->defaultvalue);
				}
			}

            this->widget->setToolTip(tpgsetting->helptext);
			this->widget->setPlaceholderText(tpgsetting->helptext);

			this->validator = new Validator(tpgsetting->grab(), this->widget);
			this->widget->setValidator(validator);

            form->setWidget(row, QFormLayout::FieldRole, this->widget);

            // connect events
        	QObject::connect(widget, SIGNAL(editingFinished()), this,
        			SLOT(editingFinished()));

			layout->addWidget(this->widget);

			if (this->tpgsetting->type == Cam::Settings::Definition::SettingType_ObjectNamesForType)
			{
				// make the push button
				QString qvalue = tpgsetting->getValue();
				QToolButton *button = new QToolButton(container);
				widgets_to_be_signalled.push_back(button);
				button->setObjectName(QString::fromAscii("SelectFile"));
				button->setText(QString::fromAscii("..."));
				button->setVisible(tpgsetting->visible);
				// connect events
        		QObject::connect(button, SIGNAL(pressed()), this, SLOT(handleButton()));
				layout->addWidget(button);
				rootComponents.push_back(button);
			}
			else
			{
				if (this->tpgsetting->units.length() > 0)
				{
					QLabel *units_widget = new QLabel(this->tpgsetting->units, parent);
					widgets_to_be_signalled.push_back(units_widget);
					layout->addWidget( units_widget );
					units_widget->setVisible(tpgsetting->visible);
					rootComponents.push_back( units_widget );
				}
			}

            // keep reference to widgets for later cleanup
            rootComponents.push_back(labelWidget);
            rootComponents.push_back(container);
			
            return true;
        }
        Base::Console().Warning("Warning: Unable to find parent widget for (%p)\n", form->parent());
    }

    Base::Console().Warning("Warning: Not given a TPGSettingDefinition\n");
    return false;
}

/**
 * Saves the values on the UI to the TPGSetting instance
 */
bool CamTextBoxComponent::close() {
//    if (widget != NULL && tpgsetting != NULL) {
//    	QString qvalue = widget->text();
//        return tpgsetting->setValue(qvalue);
//    }
    return true;
}


CamLengthComponent::CamLengthComponent()
: CamComponent() {
    this->camLineEdit = NULL;
	this->combo_box = NULL;

	QString metric, imperial;
	metric << Cam::Settings::Definition::Metric;
	imperial << Cam::Settings::Definition::Imperial;

	this->values.push_back( std::make_pair(Cam::Settings::Definition::Metric, metric) );
	this->values.push_back( std::make_pair(Cam::Settings::Definition::Imperial, imperial) );
}

/**
 * Slot to receive messages when the user changes the text value
 */
void CamLengthComponent::editingFinished() 
{
	if (camLineEdit != NULL && tpgsetting != NULL) {
		QString qvalue = camLineEdit->text();
		Cam::Settings::Length *length = dynamic_cast<Cam::Settings::Length *>(this->tpgsetting);
		if (length != NULL)
		{
			double value;
			if (length->Evaluate(camLineEdit->text().toAscii().constData(), &value))
			{
				length->set( value );

				if (length->getUnits() == Cam::Settings::Definition::Imperial)
				{
					// See if the value happens to also be a fractional value.  They just 'look' better for imperial measurements.
					QString fraction = FractionalRepresentation( value );
					if (fraction.isNull() == false)
					{
						camLineEdit->setText(fraction);
					}
					else
					{
						std::ostringstream ossValue;
						ossValue << length->get(length->getUnits());
						camLineEdit->setText(QString::fromStdString(ossValue.str()));
					}
				}
				else
				{
					std::ostringstream ossValue;
					ossValue << length->get(length->getUnits());
					camLineEdit->setText(QString::fromStdString(ossValue.str()));
				}
			}
		}
	}
	else
		Base::Console().Error("Saving a setting failed!\n");
}

void CamLengthComponent::refresh() 
{
	if (camLineEdit != NULL && tpgsetting != NULL) {
		Cam::Settings::Length *length = dynamic_cast<Cam::Settings::Length *>(this->tpgsetting);
		if (length != NULL)
		{
			if ((this->combo_box->currentIndex() >= 0) && (this->combo_box->currentIndex() < int(values.size())))
			{
				if (values[this->combo_box->currentIndex()].first != length->getUnits())
				{
					// We need the value in the setting (both 'value' and 'units') to be taken as correct.
					// Save the existing 'value' before we change the units as the changing of the units
					// may change the 'value' as well.  We then restore the original value with the new units
					// in place.

					double existingValue = length->get(length->getUnits());
					length->setUnits(values[this->combo_box->currentIndex()].first);
					length->set(existingValue);
				}
			}

			if (length->getUnits() == Cam::Settings::Definition::Imperial)
			{
				// See if the value happens to also be a fractional value.  They just 'look' better for imperial measurements.
				QString fraction = FractionalRepresentation( length->get(length->getUnits()) );
				if (fraction.isNull() == false)
				{
					camLineEdit->setText(fraction);
				}
				else
				{
					std::ostringstream ossValue;
					ossValue << length->get(length->getUnits());
					camLineEdit->setText(QString::fromStdString(ossValue.str()));
				}
			}
			else
			{
				std::ostringstream ossValue;
				ossValue << length->get(length->getUnits());
				camLineEdit->setText(QString::fromStdString(ossValue.str()));
			}
		}
	}
	else
		Base::Console().Error("Saving a setting failed!\n");
}

/**
 * Apply the changed setting to the TPGFeature so that any interested parties are notified of the change.
 */
void CamLengthComponent::currentIndexChanged ( int current_index )
{
	if ((current_index >= 0) && (current_index < int(values.size())))
	{
		Cam::Settings::Length *length = dynamic_cast<Cam::Settings::Length *>(this->tpgsetting);
		if (length != NULL)
		{
			// Call the one routine that converts from enumerated type to QString so that
			// we always produce a consistent result.
			QString imperial;
			imperial << Cam::Settings::Definition::Imperial;	

			if (values[current_index].first == Cam::Settings::Definition::Imperial)
			{
				length->setUnits(Cam::Settings::Definition::Imperial);

				// See if the value happens to also be a fractional value.  They just 'look' better for imperial measurements.
				QString fraction = FractionalRepresentation( length->get(length->getUnits()) );
				if (fraction.isNull() == false)
				{
					camLineEdit->setText(fraction);
				}
				else
				{
					std::ostringstream ossValue;
					ossValue << length->get(length->getUnits());
					camLineEdit->setText(QString::fromStdString(ossValue.str()));
				}
			}
			else
			{
				length->setUnits(Cam::Settings::Definition::Metric);
				std::ostringstream ossValue;
				ossValue << length->get(length->getUnits());
				camLineEdit->setText(QString::fromStdString(ossValue.str()));
			}
		}
	}

	// Signal the parent dialog that we may have changed something so that it can let all the other
	// CamComponent objects know to re-check their visible flags for changes too.
	signalUpdated();
}

/**
 * Creates the UI for this component and loads the initial value
 */
bool CamLengthComponent::makeUI(Cam::Settings::Definition *tpgsetting, QFormLayout* form) {
    if (tpgsetting != NULL)
    {
        // grab a copy of the setting so we can save it later
        this->tpgsetting = tpgsetting->grab();

        // construct the ui
        QWidget *parent = dynamic_cast<QWidget*>(form->parent());
        if (parent != NULL) {
			Cam::Settings::Length *length = dynamic_cast<Cam::Settings::Length *>(this->tpgsetting);
			if (length != NULL)
			{
				int row = form->rowCount();
				QString qname = tpgsetting->getFullname();

				// make the label
				QWidget *labelWidget = new QLabel(tpgsetting->label, parent);
				widgets_to_be_signalled.push_back(labelWidget);
				labelWidget->setObjectName(qname + QString::fromUtf8("Label"));
				form->setWidget(row, QFormLayout::LabelRole, labelWidget);
				labelWidget->setToolTip(tpgsetting->helptext);
				labelWidget->setVisible(tpgsetting->visible);

				// make the container
				QWidget *widget = new QWidget(parent);
				widgets_to_be_signalled.push_back(widget);
				QHBoxLayout *layout = new QHBoxLayout(widget);
				layout->setContentsMargins(0,0,0,0);
				widget->setLayout(layout);
				widget->setObjectName(qname);
				widget->setToolTip(tpgsetting->helptext);
				widget->setVisible(tpgsetting->visible);
				form->setWidget(row, QFormLayout::FieldRole, widget);

				// make the edit box
				camLineEdit = new CamLineEdit(parent, tpgsetting);
				widgets_to_be_signalled.push_back(camLineEdit);
				camLineEdit->setObjectName(qname);

				// See if the value happens to also be a fractional value.  They just 'look' better for imperial measurements.
				QString fraction = FractionalRepresentation( length->get(length->getUnits()) );
				if (fraction.isNull() == false)
				{
					camLineEdit->setText(fraction);
				}
				else
				{
					std::ostringstream ossValue;
					ossValue << length->get(length->getUnits());
					camLineEdit->setText(QString::fromStdString(ossValue.str()));
				}

				camLineEdit->setToolTip(tpgsetting->helptext);
				camLineEdit->setPlaceholderText(tpgsetting->helptext);
				this->validator = new Validator(tpgsetting->grab(), camLineEdit);
				camLineEdit->setValidator(validator);
				// connect events
        		QObject::connect(camLineEdit, SIGNAL(editingFinished()), this,
        				SLOT(editingFinished()));
				layout->addWidget(camLineEdit);

				this->combo_box = new QComboBox(parent);
				widgets_to_be_signalled.push_back(this->combo_box);
				int index = 0;
				for (std::vector< std::pair< Id_t, Label_t > >::const_iterator itValue = values.begin(); itValue != values.end(); itValue++)
				{
					this->combo_box->addItem(itValue->second);
					if (length->getUnits() == itValue->first)
					{
						this->combo_box->setCurrentIndex(index);
					}
					index++;
				}

				this->combo_box->setObjectName(qname);
				this->combo_box->setToolTip(tpgsetting->helptext);
				this->combo_box->setVisible(tpgsetting->visible);

				this->validator = new Validator(tpgsetting->grab(), this->combo_box);
				this->combo_box->setValidator(validator);

				form->setWidget(row, QFormLayout::FieldRole, this->combo_box);

				// connect events
        		QObject::connect(combo_box, SIGNAL(currentIndexChanged(int)), this,
        				SLOT(currentIndexChanged(int)));
				layout->addWidget(this->combo_box);

				// keep reference to widgets for later cleanup
				rootComponents.push_back(labelWidget);
				rootComponents.push_back(camLineEdit);
				rootComponents.push_back(this->combo_box);

				return true;
			}
        }
        Base::Console().Warning("Warning: Unable to find parent widget for (%p)\n", form->parent());
    }

    Base::Console().Warning("Warning: Not given a TPGSettingDefinition\n");
    return false;
}

/**
 * Saves the values on the UI to the TPGSetting instance
 */
bool CamLengthComponent::close() {
//    if (widget != NULL && tpgsetting != NULL) {
//    	QString qvalue = widget->text();
//        return tpgsetting->setValue(qvalue);
//    }
    return true;
}




//--- CamRateComponent ---

CamRateComponent::CamRateComponent()
: CamComponent() {
    this->camLineEdit = NULL;
	this->combo_box = NULL;

	QString metric, imperial;
	metric << Cam::Settings::Definition::Metric;
	metric += QString::fromAscii("/min");
	imperial << Cam::Settings::Definition::Imperial;
	imperial += QString::fromAscii("/min");

	this->values.push_back( std::make_pair(Cam::Settings::Definition::Metric, metric) );
	this->values.push_back( std::make_pair(Cam::Settings::Definition::Imperial, imperial) );
}

/**
 * Slot to receive messages when the user changes the text value
 */
void CamRateComponent::editingFinished() {
	if (camLineEdit != NULL && tpgsetting != NULL) {
		QString qvalue = camLineEdit->text();
		Cam::Settings::Rate *rate = dynamic_cast<Cam::Settings::Rate *>(this->tpgsetting);
		if (rate != NULL)
		{
			double value;
			if (rate->Evaluate(camLineEdit->text().toAscii().constData(), &value))
			{
				rate->set( value );
				std::ostringstream ossValue;
				ossValue << rate->get(rate->getUnits());
				camLineEdit->setText(QString::fromStdString(ossValue.str()));
			}
		}
	}
	else
		Base::Console().Error("Saving a setting failed!\n");

	// Signal the parent dialog that we may have changed something so that it can let all the other
	// CamComponent objects know to re-check their visible flags for changes too.
	signalUpdated();
}

void CamRateComponent::refresh() 
{
	if (camLineEdit != NULL && tpgsetting != NULL) {
		Cam::Settings::Rate *rate = dynamic_cast<Cam::Settings::Rate *>(this->tpgsetting);
		if (rate != NULL)
		{
			if ((this->combo_box->currentIndex() >= 0) && (this->combo_box->currentIndex() < int(values.size())))
			{
				if (values[this->combo_box->currentIndex()].first != rate->getUnits())
				{
					// We need the value in the setting (both 'value' and 'units') to be taken as correct.
					// Save the existing 'value' before we change the units as the changing of the units
					// may change the 'value' as well.  We then restore the original value with the new units
					// in place.

					double existingValue = rate->get(rate->getUnits());
					rate->setUnits(values[this->combo_box->currentIndex()].first);
					rate->set(existingValue);
				}
			}

			std::ostringstream ossValue;
			ossValue << rate->get(rate->getUnits());
			camLineEdit->setText(QString::fromStdString(ossValue.str()));
		}
	}
	else
		Base::Console().Error("Saving a setting failed!\n");
}

/**
 * Apply the changed setting to the TPGFeature so that any interested parties are notified of the change.
 */
void CamRateComponent::currentIndexChanged ( int current_index )
{
	if ((current_index >= 0) && (current_index < int(values.size())))
	{
		Cam::Settings::Rate *rate = dynamic_cast<Cam::Settings::Rate *>(this->tpgsetting);
		if (rate != NULL)
		{
			rate->setUnits(values[current_index].first);
			std::ostringstream ossValue;
			ossValue << rate->get(rate->getUnits());
			camLineEdit->setText(QString::fromStdString(ossValue.str()));
		}
	}

	// Signal the parent dialog that we may have changed something so that it can let all the other
	// CamComponent objects know to re-check their visible flags for changes too.
	signalUpdated();
}


/**
 * Creates the UI for this component and loads the initial value
 */
bool CamRateComponent::makeUI(Cam::Settings::Definition *tpgsetting, QFormLayout* form) {
    if (tpgsetting != NULL)
    {
        // grab a copy of the setting so we can save it later
        this->tpgsetting = tpgsetting->grab();

        // construct the ui
        QWidget *parent = dynamic_cast<QWidget*>(form->parent());
        if (parent != NULL) {
			Cam::Settings::Rate *rate = dynamic_cast<Cam::Settings::Rate *>(this->tpgsetting);
			if (rate != NULL)
			{
				int row = form->rowCount();
				QString qname = tpgsetting->getFullname();

				// make the label
				QWidget *labelWidget = new QLabel(tpgsetting->label, parent);
				widgets_to_be_signalled.push_back(labelWidget);
				labelWidget->setObjectName(qname + QString::fromUtf8("Label"));
				form->setWidget(row, QFormLayout::LabelRole, labelWidget);
				labelWidget->setToolTip(tpgsetting->helptext);
				labelWidget->setVisible(tpgsetting->visible);

				// make the container
				QWidget *widget = new QWidget(parent);
				widgets_to_be_signalled.push_back(widget);
				QHBoxLayout *layout = new QHBoxLayout(widget);
				layout->setContentsMargins(0,0,0,0);
				widget->setLayout(layout);
				widget->setObjectName(qname);
				widget->setToolTip(tpgsetting->helptext);
				widget->setVisible(tpgsetting->visible);
				form->setWidget(row, QFormLayout::FieldRole, widget);

				// make the edit box
				camLineEdit = new CamLineEdit(parent, tpgsetting);
				widgets_to_be_signalled.push_back(camLineEdit);
				camLineEdit->setObjectName(qname);
				camLineEdit->setVisible(tpgsetting->visible);

				std::ostringstream ossValue;
				ossValue << rate->get( rate->getUnits() );
				camLineEdit->setText(QString::fromStdString(ossValue.str()));
				camLineEdit->setToolTip(tpgsetting->helptext);
				camLineEdit->setPlaceholderText(tpgsetting->helptext);
				this->validator = new Validator(tpgsetting->grab(), camLineEdit);
				camLineEdit->setValidator(validator);
				// connect events
        		QObject::connect(camLineEdit, SIGNAL(editingFinished()), this,
        				SLOT(editingFinished()));
				layout->addWidget(camLineEdit);

				this->combo_box = new QComboBox(parent);
				widgets_to_be_signalled.push_back(this->combo_box);
				int index = 0;
				for (std::vector< std::pair< Id_t, Label_t > >::const_iterator itValue = values.begin(); itValue != values.end(); itValue++)
				{
					this->combo_box->addItem(itValue->second);
					if (rate->getUnits() == itValue->first)
					{
						this->combo_box->setCurrentIndex(index);
					}
					index++;
				}

				this->combo_box->setObjectName(qname);
				this->combo_box->setToolTip(tpgsetting->helptext);
				this->combo_box->setVisible(tpgsetting->visible);

				this->validator = new Validator(tpgsetting->grab(), combo_box);
				this->combo_box->setValidator(validator);

				form->setWidget(row, QFormLayout::FieldRole, this->combo_box);

				// connect events
        		QObject::connect(combo_box, SIGNAL(currentIndexChanged(int)), this,
        				SLOT(currentIndexChanged(int)));
				layout->addWidget(this->combo_box);

				// keep reference to widgets for later cleanup
				rootComponents.push_back(labelWidget);
				rootComponents.push_back(camLineEdit);
				rootComponents.push_back(this->combo_box);

				return true;
			}
        }
        Base::Console().Warning("Warning: Unable to find parent widget for (%p)\n", form->parent());
    }

    Base::Console().Warning("Warning: Not given a TPGSettingDefinition\n");
    return false;
}

/**
 * Saves the values on the UI to the TPGSetting instance
 */
bool CamRateComponent::close() {
//    if (widget != NULL && tpgsetting != NULL) {
//    	QString qvalue = widget->text();
//        return tpgsetting->setValue(qvalue);
//    }
    return true;
}



// ----- CamRadioComponent ---------------------------------------------------------

CamRadioComponent::CamRadioComponent()
: CamComponent() {
}

CamRadioComponent::~CamRadioComponent()
{
	for (QMap<QString, QRadioButton*>::iterator itRadio = radios.begin(); itRadio != radios.end(); itRadio++)
	{
		QObject::disconnect( itRadio.value(), SIGNAL(clicked(bool)), this, SLOT(clicked(bool)));
	}
}

void CamRadioComponent::refresh()
{
	for (QMap<QString, QRadioButton*>::iterator itRadio = radios.begin(); itRadio != radios.end(); itRadio++)
	{
		itRadio.value()->setChecked(bool(this->tpgsetting->getValue() == itRadio.key()));
	}
}

void CamRadioComponent::clicked(bool checked)
{
	QMap<QString, QRadioButton*>::const_iterator it = radios.constBegin();
	for (; it != radios.constEnd(); ++it) {
		if (it.value()->isChecked()) {
			QString qvalue = it.key();
			tpgsetting->setValue(qvalue);
		}
	}

	signalUpdated();
}



/**
 * Creates the UI for this component and loads the initial value
 */
bool CamRadioComponent::makeUI(Cam::Settings::Definition *tpgsetting, QFormLayout* form) {

    if (tpgsetting != NULL) {
        // grab a copy of the setting so we can save it later
        this->tpgsetting = tpgsetting->grab();

        // construct the ui
        QWidget *parent = dynamic_cast<QWidget*>(form->parent());
        if (parent != NULL) {
            int row = form->rowCount();
            QString qname = tpgsetting->getFullname();

            // make the label
            QWidget *labelWidget = new QLabel(tpgsetting->label, parent);
			widgets_to_be_signalled.push_back(labelWidget);
			labelWidget->setObjectName(qname + QString::fromUtf8("Label"));
            form->setWidget(row, QFormLayout::SpanningRole, labelWidget);
            labelWidget->setToolTip(tpgsetting->helptext);
			labelWidget->setVisible(tpgsetting->visible);

            // make the container
            QWidget *widget = new QWidget(parent);
			widgets_to_be_signalled.push_back(widget);
            QHBoxLayout *layout = new QHBoxLayout(widget);
            layout->setContentsMargins(0,0,0,0);
            widget->setLayout(layout);
            widget->setObjectName(qname);
            widget->setToolTip(tpgsetting->helptext);
			widget->setVisible(tpgsetting->visible);
            form->setWidget(row + 1, QFormLayout::SpanningRole, widget);

            // make the radio buttons
            QString qvalue = tpgsetting->getValue();
            QList<Cam::Settings::Option*>::iterator it = tpgsetting->options.begin();
            for (; it != tpgsetting->options.end(); ++it) {
                QRadioButton *btn = new QRadioButton(widget);
				widgets_to_be_signalled.push_back(btn);

				QObject::connect( btn, SIGNAL(clicked(bool)), this, SLOT(clicked(bool)));

                btn->setObjectName(qname + (*it)->id);
				btn->setVisible(tpgsetting->visible);
                if (qvalue.compare((*it)->id) == 0)
				{
                    btn->setChecked(true);
				}
                btn->setText((*it)->label);
                layout->addWidget(btn);
				radios.insert((*it)->id, btn);
            }

            // keep reference to widgets for later cleanup
            rootComponents.push_back(labelWidget);
            rootComponents.push_back(widget);

            return true;
        }
        Base::Console().Warning("Warning: Unable to find parent widget for (%p)", form->parent());
    }

    Base::Console().Warning("Warning: Not given a TPGSettingDefinition");
    return false;
}

/**
 * Saves the values on the UI to the TPGSetting instance
 */
bool CamRadioComponent::close() {

	//TODO: make this happen when radio buttons lose focus or different one is selected
	QMap<QString, QRadioButton*>::const_iterator it = radios.constBegin();
	for (; it != radios.constEnd(); ++it) {
		if (it.value()->isChecked()) {
			QString qvalue = it.key();
			return tpgsetting->setValue(qvalue);
		}
	}

    return false;
}




// ----- CamComboBoxComponent ---------------------------------------------------------

CamComboBoxComponent::CamComboBoxComponent()
: CamComponent() {
	this->combo_box = NULL;
}

/**
 * Creates the UI for this component and loads the initial value
 */
bool CamComboBoxComponent::makeUI(Cam::Settings::Definition *tpgsetting, QFormLayout* form) {

    if (tpgsetting != NULL) {
        // grab a copy of the setting so we can save it later
        this->tpgsetting = tpgsetting->grab();

        // construct the ui
        QWidget *parent = dynamic_cast<QWidget*>(form->parent());
        if (parent != NULL) {
            int row = form->rowCount();
            QString qname = tpgsetting->getFullname();

            // make the label
            QWidget *labelWidget = new QLabel(tpgsetting->label, parent);
			widgets_to_be_signalled.push_back(labelWidget);
			labelWidget->setObjectName(qname + QString::fromUtf8("Label"));
			form->setWidget(row, QFormLayout::LabelRole, labelWidget);
            labelWidget->setToolTip(tpgsetting->helptext);
			labelWidget->setVisible(tpgsetting->visible);

            // make the container
			// Keep our own copy of the id/label pairs in a vector so that we're sure to
			// associate the 'index' of the combo-box with the correct options in both
			// this method (which defines them) and in the currentIndexChanged() (which
			// looks for which one was chosen).
			this->combo_box = new QComboBox(parent);
			widgets_to_be_signalled.push_back(this->combo_box);
			int index = 0;
			if (this->tpgsetting->type == Cam::Settings::Definition::SettingType_SingleObjectNameForType)
			{
				Cam::Settings::SingleObjectNameForType *pSetting = (Cam::Settings::SingleObjectNameForType *) this->tpgsetting;
				// We need to search the current document for objects whose type matches GetType() and add their names
				// as options to the combo-box.

				App::Document *doc = App::GetApplication().getActiveDocument();
				if (doc != NULL)
				{
					Base::Type type = Base::Type::fromName( pSetting->GetType().toAscii().constData() );
					if (type.isBad() == false)
					{
						std::vector<App::DocumentObject *> objects = doc->getObjectsOfType( type );
						for (std::vector<App::DocumentObject *>::const_iterator itObject = objects.begin(); itObject != objects.end(); itObject++)
						{
							QString name = QString::fromAscii((*itObject)->getNameInDocument());
							this->combo_box->addItem( name );
							if (pSetting->GetName() == name)
							{
								this->combo_box->setCurrentIndex(index);
							}

							values.push_back( std::make_pair( name, name ));
							index++;
						}
					}
				}
			}
			else if (this->tpgsetting->type == Cam::Settings::Definition::SettingType_Enumeration)
			{
				for (QList<Cam::Settings::Option *>::const_iterator itOption = this->tpgsetting->options.begin(); itOption != this->tpgsetting->options.end(); itOption++)
				{
					this->combo_box->addItem((*itOption)->label);
					if (this->tpgsetting->getValue() == (*itOption)->id)
					{
						this->combo_box->setCurrentIndex(index);
					}
					values.push_back( std::make_pair( (*itOption)->id, (*itOption)->label ) );
					index++;
				}
			}

            this->combo_box->setObjectName(qname);
            this->combo_box->setToolTip(tpgsetting->helptext);
			this->combo_box->setVisible(tpgsetting->visible);

			this->validator = new Validator(tpgsetting->grab(), this->combo_box);
			this->combo_box->setValidator(validator);

            form->setWidget(row, QFormLayout::FieldRole, this->combo_box);

            // connect events
        	QObject::connect(combo_box, SIGNAL(currentIndexChanged(int)), this,
        			SLOT(currentIndexChanged(int)));

            // keep reference to widgets for later cleanup
            rootComponents.push_back(labelWidget);
            rootComponents.push_back(this->combo_box);

            return true;
        }
        Base::Console().Warning("Warning: Unable to find parent widget for (%p)", form->parent());
    }

    Base::Console().Warning("Warning: Not given a TPGSettingDefinition");
    return false;
}

void CamComboBoxComponent::refresh()
{
	switch (this->tpgsetting->type)
	{
	case Cam::Settings::Definition::SettingType_SingleObjectNameForType:
		{
			Cam::Settings::SingleObjectNameForType *pSetting = (Cam::Settings::SingleObjectNameForType *) this->tpgsetting;
			if (pSetting)
			{
				for (::size_t i=0; i<values.size(); i++)
				{
					if (values[i].second == pSetting->GetName())
					{
						this->combo_box->setCurrentIndex(int(i));
					}
				}
			}
		}
		break;

	case Cam::Settings::Definition::SettingType_Enumeration:
		{
			Cam::Settings::Enumeration *pSetting = (Cam::Settings::Enumeration *) this->tpgsetting;
			if (pSetting)
			{
				for (::size_t i=0; i<values.size(); i++)
				{
					if (values[i].second == pSetting->get().second)
					{
						this->combo_box->setCurrentIndex(int(i));
					}
				}
			}
		}
		break;
	}

}

/**
 * Apply the changed setting to the TPGFeature so that any interested parties are notified of the change.
 */
void CamComboBoxComponent::currentIndexChanged ( int current_index )
{
	if ((current_index >= 0) && (current_index < int(values.size())))
	{
		this->tpgsetting->setValue(values[current_index].first);
	}

	// Signal the parent dialog that we may have changed something so that it can let all the other
	// CamComponent objects know to re-check their visible flags for changes too.
	signalUpdated();
}

/**
 * Saves the values on the UI to the TPGSetting instance.
 * NOTE: We don't save it here as the value will already have been set in the currentIndexChanged() method.
 */
bool CamComboBoxComponent::close() {
	
    return true;	// Assume all is well.
}







CamFilenameComponent::CamFilenameComponent()
: CamComponent() {
    this->camLineEdit = NULL;
	this->button = NULL;
}

void CamFilenameComponent::refresh()
{
	this->camLineEdit->refresh();
}

/**
 * Slot to receive messages when the user changes the text value
 */
void CamFilenameComponent::editingFinished() {
	if (camLineEdit != NULL && tpgsetting != NULL) {
		QString qvalue = camLineEdit->text();
		if (!tpgsetting->setValue(qvalue))
		{
			Base::Console().Error("Saving failed: '%s'\n", tpgsetting->name.toStdString().c_str());
			camLineEdit->setText(tpgsetting->getValue());
		}
	}
	else
		Base::Console().Error("Saving a setting failed!\n");

	// Signal the parent dialog that we may have changed something so that it can let all the other
	// CamComponent objects know to re-check their visible flags for changes too.
	signalUpdated();
}

/**
 * Creates the UI for this component and loads the initial value
 */
bool CamFilenameComponent::makeUI(Cam::Settings::Definition *tpgsetting, QFormLayout* form) {
    if (tpgsetting != NULL)
    {
        // grab a copy of the setting so we can save it later
        this->tpgsetting = tpgsetting->grab();

        // construct the ui
        QWidget *parent = dynamic_cast<QWidget*>(form->parent());
        if (parent != NULL) {
            int row = form->rowCount();
            QString qname = tpgsetting->getFullname();

            // make the label
            QWidget *labelWidget = new QLabel(tpgsetting->label, parent);
			widgets_to_be_signalled.push_back(labelWidget);
			labelWidget->setObjectName(qname + QString::fromUtf8("Label"));
            form->setWidget(row, QFormLayout::LabelRole, labelWidget);
            labelWidget->setToolTip(tpgsetting->helptext);
			labelWidget->setVisible(tpgsetting->visible);

            // make the container
            QWidget *widget = new QWidget(parent);
			widgets_to_be_signalled.push_back(widget);
            QHBoxLayout *layout = new QHBoxLayout(widget);
            layout->setContentsMargins(0,0,0,0);
            widget->setLayout(layout);
            widget->setObjectName(qname);
            widget->setToolTip(tpgsetting->helptext);
			widget->setVisible(tpgsetting->visible);
			form->setWidget(row, QFormLayout::FieldRole, widget);

			// make the edit box
            camLineEdit = new CamLineEdit(parent, tpgsetting);
			widgets_to_be_signalled.push_back(camLineEdit);
            camLineEdit->setObjectName(qname);
            camLineEdit->setText(tpgsetting->getValue());
            camLineEdit->setToolTip(tpgsetting->helptext);
			camLineEdit->setPlaceholderText(tpgsetting->helptext);
			camLineEdit->setVisible(tpgsetting->visible);
			this->validator = new Validator(tpgsetting->grab(), camLineEdit);
			camLineEdit->setValidator(validator);
            // connect events
        	QObject::connect(camLineEdit, SIGNAL(editingFinished()), this,
        			SLOT(editingFinished()));
			layout->addWidget(camLineEdit);

            // make the push button
            QString qvalue = tpgsetting->getValue();
            button = new QToolButton(widget);
			widgets_to_be_signalled.push_back(button);
			button->setObjectName(QString::fromAscii("SelectFile"));
            button->setText(QString::fromAscii("..."));
			button->setVisible(tpgsetting->visible);
			// connect events
        	QObject::connect(button, SIGNAL(pressed()), this,
        			SLOT(handleButton()));
            layout->addWidget(button);

            // keep reference to widgets for later cleanup
            rootComponents.push_back(labelWidget);
            rootComponents.push_back(camLineEdit);
			rootComponents.push_back(button);
			rootComponents.push_back(widget);

            return true;
        }
        Base::Console().Warning("Warning: Unable to find parent widget for (%p)\n", form->parent());
    }

    Base::Console().Warning("Warning: Not given a TPGSettingDefinition\n");
    return false;
}

/**
 * Saves the values on the UI to the TPGSetting instance
 */
bool CamFilenameComponent::close() {
//    if (widget != NULL && tpgsetting != NULL) {
//    	QString qvalue = widget->text();
//        return tpgsetting->setValue(qvalue);
//    }
    return true;
}

void CamFilenameComponent::handleButton()
{
	QString caption = QString::fromAscii("Caption");
	QString directory = QString::fromAscii("c:\\david");
	QString filter = QString::fromAscii("*.*");

	QString filename = QFileDialog::getOpenFileName(NULL, caption, directory, filter);
	if (filename.isNull() == false)
	{
		this->camLineEdit->setText(filename);
		this->tpgsetting->setValue(filename);
	}

	// Signal the parent dialog that we may have changed something so that it can let all the other
	// CamComponent objects know to re-check their visible flags for changes too.
	signalUpdated();
}








CamDirectoryComponent::CamDirectoryComponent()
: CamComponent() {
    this->camLineEdit = NULL;
	this->button = NULL;
}

void CamDirectoryComponent::refresh()
{
	this->camLineEdit->refresh();
}


/**
 * Slot to receive messages when the user changes the text value
 */
void CamDirectoryComponent::editingFinished() {
	if (camLineEdit != NULL && tpgsetting != NULL) {
		QString qvalue = camLineEdit->text();
		if (!tpgsetting->setValue(qvalue))
		{
			Base::Console().Error("Saving failed: '%s'\n", tpgsetting->name.toStdString().c_str());
			camLineEdit->setText(tpgsetting->getValue());
		}
	}
	else
		Base::Console().Error("Saving a setting failed!\n");
}

/**
 * Creates the UI for this component and loads the initial value
 */
bool CamDirectoryComponent::makeUI(Cam::Settings::Definition *tpgsetting, QFormLayout* form) {
    if (tpgsetting != NULL)
    {
        // grab a copy of the setting so we can save it later
        this->tpgsetting = tpgsetting->grab();

        // construct the ui
        QWidget *parent = dynamic_cast<QWidget*>(form->parent());
        if (parent != NULL) {
            int row = form->rowCount();
            QString qname = tpgsetting->getFullname();

            // make the label
            QWidget *labelWidget = new QLabel(tpgsetting->label, parent);
			widgets_to_be_signalled.push_back(labelWidget);
			labelWidget->setObjectName(qname + QString::fromUtf8("Label"));
            form->setWidget(row, QFormLayout::LabelRole, labelWidget);
            labelWidget->setToolTip(tpgsetting->helptext);
			labelWidget->setVisible(tpgsetting->visible);

            // make the container
            QWidget *widget = new QWidget(parent);
			widgets_to_be_signalled.push_back(widget);
            QHBoxLayout *layout = new QHBoxLayout(widget);
            layout->setContentsMargins(0,0,0,0);
            widget->setLayout(layout);
            widget->setObjectName(qname);
            widget->setToolTip(tpgsetting->helptext);
			widget->setVisible(tpgsetting->visible);
			form->setWidget(row, QFormLayout::FieldRole, widget);

			// make the edit box
            camLineEdit = new CamLineEdit(parent, tpgsetting);
			widgets_to_be_signalled.push_back(camLineEdit);
            camLineEdit->setObjectName(qname);
            camLineEdit->setText(tpgsetting->getValue());
            camLineEdit->setToolTip(tpgsetting->helptext);
			camLineEdit->setPlaceholderText(tpgsetting->helptext);
			this->validator = new Validator(tpgsetting->grab(), camLineEdit);
			camLineEdit->setValidator(validator);
			camLineEdit->setVisible(tpgsetting->visible);
            // connect events
        	QObject::connect(camLineEdit, SIGNAL(editingFinished()), this,
        			SLOT(editingFinished()));
			layout->addWidget(camLineEdit);

            // make the push button
            QString qvalue = tpgsetting->getValue();
            button = new QToolButton(widget);
			widgets_to_be_signalled.push_back(button);
			button->setObjectName(QString::fromAscii("SelectFile"));
            button->setText(QString::fromAscii("..."));
			button->setVisible(tpgsetting->visible);
			// connect events
        	QObject::connect(button, SIGNAL(pressed()), this,
        			SLOT(handleButton()));
            layout->addWidget(button);

            // keep reference to widgets for later cleanup
            rootComponents.push_back(labelWidget);
			rootComponents.push_back(camLineEdit);
			rootComponents.push_back(button);
            rootComponents.push_back(widget);

            return true;
        }
        Base::Console().Warning("Warning: Unable to find parent widget for (%p)\n", form->parent());
    }

    Base::Console().Warning("Warning: Not given a TPGSettingDefinition\n");
    return false;
}

/**
 * Saves the values on the UI to the TPGSetting instance
 */
bool CamDirectoryComponent::close() {
//    if (widget != NULL && tpgsetting != NULL) {
//    	QString qvalue = widget->text();
//        return tpgsetting->setValue(qvalue);
//    }
    return true;
}

void CamDirectoryComponent::handleButton()
{
	QString caption = QString::fromAscii("Caption");
	QString directory = QDir::homePath();
	QString filter = QString::fromAscii("*.*");

	QString dir = QFileDialog::getExistingDirectory(NULL, caption, directory, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.isNull() == false)
	{
		this->camLineEdit->setText(dir);
		this->tpgsetting->setValue(dir);
	}

	// Signal the parent dialog that we may have changed something so that it can let all the other
	// CamComponent objects know to re-check their visible flags for changes too.
	signalUpdated();
}






CamColorComponent::CamColorComponent()
: CamComponent() {
	this->button = NULL;
}

void CamColorComponent::refresh()
{
	Cam::Settings::Color *pColorSetting = dynamic_cast<Cam::Settings::Color *>(this->tpgsetting);

	if (pColorSetting)
	{
		int red, green, blue, alpha;
		pColorSetting->get(red, green, blue, alpha);
		QColor color(red, green, blue, alpha);

		if (color.isValid())
		{
			QPalette palette(color);
			this->button->setPalette(palette);
			this->button->setAutoFillBackground(true);
			this->button->setFlat(true);
		}
		
		// Signal the parent dialog that we may have changed something so that it can let all the other
		// CamComponent objects know to re-check their visible flags for changes too.
		signalUpdated();
	}
}


/**
 * Creates the UI for this component and loads the initial value
 */
bool CamColorComponent::makeUI(Cam::Settings::Definition *tpgsetting, QFormLayout* form) {
    if (tpgsetting != NULL)
    {
        // grab a copy of the setting so we can save it later
        this->tpgsetting = tpgsetting->grab();

        // construct the ui
        QWidget *parent = dynamic_cast<QWidget*>(form->parent());
        if (parent != NULL) {
            int row = form->rowCount();
            QString qname = tpgsetting->getFullname();

            // make the label
            QWidget *labelWidget = new QLabel(tpgsetting->label, parent);
			widgets_to_be_signalled.push_back(labelWidget);
			labelWidget->setObjectName(qname + QString::fromUtf8("Label"));
            form->setWidget(row, QFormLayout::LabelRole, labelWidget);
            labelWidget->setToolTip(tpgsetting->helptext);
			labelWidget->setVisible(tpgsetting->visible);

            // make the container
            QWidget *widget = new QWidget(parent);
			widgets_to_be_signalled.push_back(widget);

            // make the push button
            QString qvalue = tpgsetting->getValue();
            button = new QPushButton(widget);
			widgets_to_be_signalled.push_back(button);
			button->setObjectName(QString::fromAscii("SelectFile"));
            button->setText(QString::fromAscii("   "));
			button->setVisible(tpgsetting->visible);

			int red, green, blue, alpha;
			Cam::Settings::Color *pColorSetting = (Cam::Settings::Color *) this->tpgsetting;
			pColorSetting->get( red, green, blue, alpha );
			QColor color(red, green, blue, alpha);
			QPalette palette(color);
			button->setPalette(palette);
			button->setAutoFillBackground(true);
			button->setFlat(true);

			// connect events
        	QObject::connect(button, SIGNAL(pressed()), this,
        			SLOT(handleButton()));
			form->setWidget(row, QFormLayout::FieldRole, widget);

            // keep reference to widgets for later cleanup
            rootComponents.push_back(labelWidget);
            rootComponents.push_back(widget);

            return true;
        }
        Base::Console().Warning("Warning: Unable to find parent widget for (%p)\n", form->parent());
    }

    Base::Console().Warning("Warning: Not given a TPGSettingDefinition\n");
    return false;
}

/**
 * Saves the values on the UI to the TPGSetting instance
 */
bool CamColorComponent::close() {
//    if (widget != NULL && tpgsetting != NULL) {
//    	QString qvalue = widget->text();
//        return tpgsetting->setValue(qvalue);
//    }
    return true;
}

void CamColorComponent::handleButton()
{
	Cam::Settings::Color *pColorSetting = dynamic_cast<Cam::Settings::Color *>(this->tpgsetting);

	if (pColorSetting)
	{
		int red, green, blue, alpha;
		pColorSetting->get(red, green, blue, alpha);
		QColor initial(red, green, blue, alpha);

		QColor color = QColorDialog::getColor(initial, NULL);
		if (color.isValid())
		{
			QPalette palette(color);
			this->button->setPalette(palette);
			this->button->setAutoFillBackground(true);
			this->button->setFlat(true);

			// Cast the setting pointer to a TPGColorSettingDefinition so that we can generate
			// the verbose (string) represenation of the QColor for storage in the PropTPGSettings property
			// within the TPGFeature.

			
			pColorSetting->set( color.red(), color.green(), color.blue(), color.alpha() );
		}
		
		// Signal the parent dialog that we may have changed something so that it can let all the other
		// CamComponent objects know to re-check their visible flags for changes too.
		signalUpdated();
	}
}




#include "moc_CamProjectDockWindowComponents.cpp"

} /* namespace Cam */
