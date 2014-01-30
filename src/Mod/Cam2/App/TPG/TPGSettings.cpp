/***************************************************************************
 *   Copyright (c) 2012 Luke Parry    (l.parry@warwick.ac.uk)              *
 *   Copyright (c) 2012-3 Andrew Robinson <andrewjrobinson@gmail.com>      *
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

#include <App/Application.h>
#include <App/Document.h>

#include "TPGSettings.h"

#include <cstdio>
#include <QList>

#include <set>

#include <Base/Console.h>

#include <math.h>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/exceptions.hpp>

#include <Base/Interpreter.h>

#include <Base/PyTools.h>
#include <Base/Exception.h>
#include <Base/PyObjectBase.h>

namespace Cam {
	namespace Settings {

const char* ts(QString str)
{
	return str.toAscii().constData();
}

const char* ts(QString *str)
{
	if (str != NULL)
		return str->toAscii().constData();
	return "NULL";
}

////////// TPGSetting //////////

#ifdef WIN32
#pragma region "Settings::Definition"
#endif

Definition::Definition(const char *name, const char *label, const SettingType type, const char *defaultvalue, const char *units, const char *helptext)
{
	this->refcnt = 1;
    this->parent = NULL;

	this->name = QString::fromAscii(name);
	this->label = QString::fromAscii(label);
	this->type = type;
	this->defaultvalue = QString::fromAscii(defaultvalue);
	this->units = QString::fromAscii(units);
	this->helptext = QString::fromAscii(helptext);
	this->visible = true;
}
Definition::Definition(QString name, QString label, SettingType type, QString defaultvalue, QString units, QString helptext)
{
	this->refcnt = 1;
    this->parent = NULL;

	this->name = name;
	this->label = label;
	this->type = type;
	this->defaultvalue = defaultvalue;
	this->units = units;
	this->helptext = helptext;
	this->visible = true;
}
Definition::Definition() {
	this->refcnt = 1;
    this->parent = NULL;
	this->visible = true;
}

Definition::~Definition() {
    QList<Settings::Option*>::iterator it = this->options.begin();
    for (; it != this->options.end(); ++it)
	{
        delete *it;
	}
    options.clear();

	#ifdef FCAppCamGui
		pValidator.reset(NULL);	// Discard the validator object.
	#endif // FCAppCamGui
}

bool Definition::operator== ( const Definition & rhs ) const
{
	if (name != rhs.name) return(false);
	if (label != rhs.label) return(false);
	if (type != rhs.type) return(false);

	return(true);
}


/**
 * Perform a deep copy of this class
 */
Definition* Definition::clone()
{
    Definition* clone = new Definition(name, label, type, defaultvalue, units, helptext);
	QList<Settings::Option*>::iterator it = this->options.begin();

    for (; it != this->options.end(); ++it)
        clone->addOption((*it)->id, (*it)->label);

	clone->action = this->action;
	clone->parent = this->parent;

    return clone;
}



Definition::Definition( Definition & rhs )
{
	this->refcnt = 1;
    this->parent = NULL;
	this->visible = true;

	*this = rhs;	// call the assignment operator.
}

Definition & Definition::operator= ( Definition & rhs )
{
	if (this != &rhs)
	{
		rhs.grab();	// Make sure it doesn't go away while we're looking at it.

		this->refcnt = 1;
		if (this->parent)
		{
			this->parent->release();
			this->parent = NULL;
		}

		this->parent = rhs.parent;
		if (this->parent)
		{
			this->parent->grab();
		}

		this->action = rhs.action;

		this->name = rhs.name;
		this->label = rhs.label;
		this->type = rhs.type;
		this->defaultvalue = rhs.defaultvalue;
		this->units = rhs.units;
		this->helptext = rhs.helptext;

		for (QList<Option *>::iterator itOption = options.begin(); itOption != options.end(); itOption++)
		{
			delete (*itOption);
		}
		options.clear();

		for (QList<Option *>::const_iterator itRhsOption = rhs.options.begin(); itRhsOption != rhs.options.end(); itRhsOption++)
		{
			this->options.push_back(new Settings::Option(*(*itRhsOption)));
		}

		this->visible = rhs.visible;
		rhs.release();
	}

	return(*this);
}


/**
 * add an option for the value of this setting
 */
void Definition::addOption(QString id, QString label) {
    this->options.append(new Settings::Option(id, label));
}

/**
 * add an option for the value of this setting
 */
void Definition::addOption(const char *id, const char *label) {
    this->options.append(new Settings::Option(id, label));
}


Definition::ValidationState Definition::validate(QString & input,int & position) const
{
	return(this->Acceptable);	// If no type-specific validation is required then the value must be acceptable.
}







void Definition::print() const
{
	qDebug("  - (%s, %s, %d, %s, %s, %s)\n",
			name.toAscii().constData(),
			label.toAscii().constData(),
			type,
			defaultvalue.toAscii().constData(),
			units.toAscii().constData(),
			helptext.toAscii().constData());
}

/**
 * Increases reference count
 * Note: it returns a pointer to 'this' for convenience.
 */
Definition *Definition::grab() {
    refcnt++;
    return this;
}

/**
 * Decreases reference count and deletes self if no other references
 */
void Definition::release() {
    refcnt--;
    if (refcnt == 0)
        delete this;
}

/**
 * Get the value associated with this setting
 */
QString Definition::getValue() const
{
	if (this->parent != NULL)
		return this->parent->getValue(action, name);
	return QString::null;
}

/**
 * Set the value associated with this setting
 */
bool Definition::setValue(QString value) {
	int position = value.length();
	if (position > 0) position -= 1;
	if (this->parent != NULL) {
		return parent->setValue(action, name, value);
	}
		
	Base::Console().Warning("Setting doesn't have parent!\n");
	return false;
}

/**
 * Get the namespaced name <action>::<name>
 */
QString Definition::getFullname() const
{
	return action + QString::fromAscii("::") + name;
}


Settings::Option *Settings::Definition::getOption(const QString id) const
{
	for (QList<Option*>::const_iterator itOption = options.begin(); itOption != options.end(); itOption++)
	{
		if ((*itOption)->id.toUpper() == id.toUpper())
		{
			return(*itOption);
		}
	}

	return(NULL);
}

std::string Definition::PythonName(const QString prefix) const
{
	// Replace any spaces within the variable name with underbars so that the
	// name can be used as a Python variable name.
	std::string processed_name;
	
	if (prefix.isNull() == false)
	{
		processed_name = prefix.toStdString();
		processed_name += "_";
	}

	processed_name += this->label.toStdString();
	std::string::size_type offset;
	while ((offset=processed_name.find(' ')) != std::string::npos)
	{
		processed_name[offset] = '_';
	}

	return(processed_name);
}


/**
	This method is called once for each setting in the TPGSettings object prior
	to any 'Length' or 'Double' setting's interpretation within the Python
	environment.  This method adds variable=value pairs to the Python dictionary
	passed in so that such variables may be used when calculating the value
	of other settings.

	eg: If we're changing the 'Peck Depth' of a drilling operation then we might
	enter "Tool_Diameter/2" into the dialog box.  The "Tool_Diameter" variable
	will have been added by this method and its value will be divided by two
	before assigning the result to the 'Peck Depth' setting.

	For text-based settings such as 'text', 'filename' and 'directory', the
	variable name will be the setting's label (with spaces replaced by
	underbar characters).  The value will simply be the setting's value.

	For floating point settings such as 'Double' and 'Length', the name
	will be the label (processed as above) and the value will be the value
	of the setting.

	For Color settings we define four separate variables.  The 'label' of
	the color setting will be pre-processed as mentioned above.  Appended
	to this label will be "_red", "_green", "_blue" and "_alpha".  eg:
	If the color setting's label is "My Color" then the following
	variable names will be added to the Python dictionary;
	"My_Color_red", "My_Color_green", "My_Color_blue" and "My_Color_alpha"

	NOTE: Any Length settings will be converted to the 'units' passed in prior to
	being used in the variable's value.  This allows one setting to refer to another
	setting's value without worrying about which units each one is defined in.
 */
bool Definition::AddToPythonDictionary(PyObject *pDictionary, const QString requested_units, const QString prefix) const
{
	return(false);
}


#ifdef WIN32
#pragma endregion "Settings::Definition"
#endif


// ----- TPGSettings ----------------------------------------------------------
#ifdef WIN32
#pragma region "Settings::TPGSettings"
#endif

TPGSettings::TPGSettings()
{
	this->refcnt = 1;
	this->tpgFeature = NULL;
}

TPGSettings::~TPGSettings()
{
	// The objects in the settingDefs vector used grab() to increment the reference count when they
	// were added in the addSettingDefinition() method.  Release this reference count now that we
	// don't need them any more.
	for (std::vector<Definition*>::iterator itSetting = settingDefs.begin(); itSetting != settingDefs.end(); /* increment within loop */ )
	{
		(*itSetting)->release();
		itSetting = settingDefs.erase(itSetting);
	}
}

/**
 * Perform a deep copy of this class
 */
TPGSettings* TPGSettings::clone()
{
	TPGSettings* settings = new TPGSettings();

	std::vector<Definition*>::iterator it = this->settingDefs.begin();
	while (it != this->settingDefs.end())
	{
		settings->addSettingDefinition((*it)->action, (*it)->clone());
		++it;
	}

	settings->tpgFeature = this->tpgFeature;

	return settings;
}

/**
 * Adds the setting to this setting group
 */
Definition* TPGSettings::addSettingDefinition(QString action, Definition* setting) {

	if (setting != NULL) {
		QString qname = makeName(action, setting->name);

		// store reference to setting
		settingDefs.push_back(setting->grab());
		settingDefsMap.insert(std::pair<QString, Definition*>(qname, setting));

		// take ownership of setting
		setting->action = action;
		setting->parent = this;

		// add setting to the required action
		std::map<QString, std::vector<Definition*> >::iterator it = settingDefsActionMap.find(action);
		if (it == settingDefsActionMap.end()) {
			std::vector<Definition*> settings;
			settings.push_back(setting);
			settingDefsActionMap.insert(std::pair<QString, std::vector<Definition*> >(action, settings));
		} else {
			// Check to see if we already have one.  If so, return a pointer to it rather than adding this new one.
			bool found = false;
			for (std::vector<Cam::Settings::Definition *>::iterator itSettingDef = it->second.begin(); (! found) && (itSettingDef != it->second.end()); itSettingDef++)
			{
				if (*(*itSettingDef) == *setting)
				{
					found = true;
					setting->release();	// We don't need this one any more.
					setting = *itSettingDef; // Return a pointer to the one we already had.
					break;
				}
			}

			if (! found)
			{
				it->second.push_back(setting);
			}
		}
	}

	// return setting for convenience
	return setting;
}

/**
 * Get the currently selected action.
 */
const QString TPGSettings::getAction() const {
	return action;
}

/**
 * Change the currently selected action.
 */
bool TPGSettings::setAction(QString action) {
	this->action = action;
	if (tpgFeature != NULL)
	{
		tpgFeature->PropTPGSettings.setValue("action", action.toAscii().constData());
		return(true);
	}
	return(false);
}

/**
 * Get the value of a given setting (by name)
 */
const QString TPGSettings::getValue(QString name) const
{
	if (action.isNull() || action.isEmpty())
		return QString::null;
	return getValue(action, name);
}

/**
 * Get the value of a given setting (by name)
 */
const QString TPGSettings::getValue(QString action, QString name) const
{

	if (action.isNull() || action.isEmpty()) {
		Base::Console().Message("action not set\n");
		return QString::null;
	}

	// compute full setting name (<action>::<name>)
	QString qname = makeName(action, name);

    // get setting value
	if (tpgFeature != NULL) {
		if (settingDefsMap.find(qname) != settingDefsMap.end()) {
			const std::map<std::string,std::string> vals = tpgFeature->PropTPGSettings.getValues();
			std::map<std::string,std::string>::const_iterator val = vals.find(qname.toStdString());
			if (val != vals.end()) {
				return QString::fromStdString(val->second);
			}
		}
	}
	Base::Console().Message("Can't find setting!\n");
    return QString::null;
}

/**
 * Set the value for the named setting
 */
bool TPGSettings::setValue(QString action, QString name, QString value) {

	if (action.isNull() || action.isEmpty())
		return false;

	// compute full setting name (<action>::<name>)
	QString qname = makeName(action, name);

	if (tpgFeature != NULL) {
		if (settingDefsMap.find(qname) != settingDefsMap.end()) {
			std::string strname = qname.toStdString();
			std::string strvalue = value.toStdString();
			tpgFeature->PropTPGSettings.setValue(strname, strvalue);
			return true;
		}
	}
	Base::Console().Message("Can't find setting!\n");
	return false;
}

/**
 * Set the value for the named setting
 */
bool TPGSettings::setValue(QString name, QString value) {
	return setValue(action, name, value);
}

void TPGSettings::print() const
{
	std::vector<Definition*>::const_iterator it = this->settingDefs.begin();
	while (it != this->settingDefs.end())
	{
		(*it)->print();
		++it;
	}
}


std::vector<Definition*> TPGSettings::getSettings() const
{
	return this->settingDefs;
}

/**
 * Sets the default value for any settings that are missing from the TPGFeature
 */
void TPGSettings::addDefaults() {

	if (tpgFeature != NULL) {
		const std::map<std::string,std::string> currentValues = tpgFeature->PropTPGSettings.getValues();

		// add action
		if (currentValues.find("action") == currentValues.end())
			tpgFeature->PropTPGSettings.setValue("action", "default");

		// add all settings for each available action
		std::vector<Definition*>::iterator it = this->settingDefs.begin();
		while (it != this->settingDefs.end()) {
			QString nsName = makeName((*it)->action, (*it)->name);
			if (currentValues.find(nsName.toStdString()) == currentValues.end())
			{
				tpgFeature->PropTPGSettings.setValue(nsName.toStdString(),(*it)->defaultvalue.toStdString());
			}

			++it;
		}
	}
}

QStringList TPGSettings::getActions() const
{

	QStringList result;

	std::map<QString, std::vector<Definition*> >::const_iterator it = settingDefsActionMap.begin();
	while (it != settingDefsActionMap.end()) {
		Base::Console().Log("%s\n", (*it).first.toAscii().constData());
		result.append((*it).first);
		++it;
	}

	return result;
}

/**
 * Sets the TPGFeature that the value will be saved-to/read-from.
 *
 * This will set the default values for any missing settings
 */
void TPGSettings::setTPGFeature(Cam::TPGFeature *tpgFeature) {
	this->tpgFeature = tpgFeature;

	// load the TPG's action
	tpgFeature->PropTPGSettings.getValues();

	addDefaults();
}

/**
 * Increases reference count
 * Note: it returns a pointer to this for convenience.
 */
TPGSettings *TPGSettings::grab() {
    refcnt++;
    return this;
}

/**
 * Decreases reference count and deletes self if no other references
 */
void TPGSettings::release() {
    refcnt--;
    if (refcnt == 0)
        delete this;
}

/**
 * Make a namespaced name (from <action>::<name>)
 */
QString TPGSettings::makeName(QString action, QString name) const {
	QString result = action;
	result.append(QString::fromAscii("::"));
	result.append(name);
	return result;
}


/**
	Keep a copy of the properties map before any changes occur so that we can
	compare it with the map of modified settings.  Only by comparing these two
	can we figure out which one of the settings changed.
 */
void TPGSettings::onBeforePropTPGSettingsChange(const App::PropertyMap* property_map)
{
	if (property_map != NULL)
	{
		this->previous_tpg_properties_version.clear();

		std::copy( property_map->getValues().begin(), property_map->getValues().end(),
			std::inserter( this->previous_tpg_properties_version, this->previous_tpg_properties_version.begin() ) );

		// qDebug("TPGSettings::onBeforePropTPGSettingsChange(%s) called\n", property_map->getName());
	}
}

/**
	Called when one of the TPGFeature::PropTPGSettings values changes.
 */
void TPGSettings::onPropTPGSettingsChanged(const App::PropertyMap* property_map)
{
	// One of the settings has changed.  Figure out which one and let any interested parties know.
	if (tpgFeature != NULL)
	{
		TPG *tpg = tpgFeature->getTPG();
		if (tpg)
		{
			tpg->grab();

			// qDebug("TPGSettings::onPropTPGSettingsChanged() called\n");
			for (std::map<QString, Definition*>::iterator itSettingsDef = settingDefsMap.begin(); itSettingsDef != settingDefsMap.end(); itSettingsDef++)
			{
				std::string name = itSettingsDef->first.toStdString();
				std::string previous_value = this->previous_tpg_properties_version[name];
				std::string new_value = itSettingsDef->second->getValue().toStdString();

				if (new_value != previous_value)
				{
					tpg->onChanged( itSettingsDef->second, QString::fromStdString(previous_value), QString::fromStdString(new_value));
				}
			}
			tpg->release();
		}
	}
}


static std::pair<std::string::size_type, std::string::size_type> next_possible_number(const std::string & value, std::string::size_type offset )
{
	std::pair<std::string::size_type, std::string::size_type> results = std::make_pair(std::string::npos, std::string::npos);

	for ( ; offset < value.length(); offset++)
	{
		if ((value[offset] == '-') ||
			(value[offset] == '+') ||
			(value[offset] == '.') ||
			((value[offset] >= '0') &&
			 (value[offset] <= '9')))
		{
			if (results.first == std::string::npos)
			{
				results.first = offset;
			}
			else
			{
				// We already have the starting point.  Just keep stepping through the number.
			}
		}
		else
		{
			if (results.first != std::string::npos)
			{
				results.second = offset-1;
				return(results);
			}
		}
	}

	if (results.first != std::string::npos)
	{
		results.second = value.length() - 1;
		return(results);
	}

	return(results);
}





/**
    Use the Python interpreter to evaluate the 'value' string passed in.  Before
    doing this, however, define all the variable=value pairs found in the settings
    list just in case the 'value' expression uses one of these properties as
    a reference.  eg: if a propertyDouble has 'diameter'='3.4' and the
    'value'='diameter / 2.0' then we need to end up returning '1.7' in a QString.
 */
bool TPGSettings::EvaluateWithPython( const Definition *definition, QString value, QString & evaluated_version ) const
{
    bool return_status = false;

    // Py_Initialize() and Py_Finalize() should only occur once per process.  Do it in the main application framework instead.
    // Py_Initialize();

	// Lock the 'Global Interpreter Lock' so that we're not interrupted during our execution.
	Base::PyGILStateLocker locker;	

    PyObject *pModule = PyImport_ImportModule("math");

    if (pModule != NULL)
    {
        PyObject *pDictionary = PyModule_GetDict(pModule);

        if (pDictionary != NULL)
        {
			for (std::vector<Definition *>::const_iterator itDef = this->settingDefs.begin(); itDef != this->settingDefs.end(); itDef++)
			{
				if ((*itDef) == definition) continue;	// Can't use our own value within the definition of our own value.

				(*itDef)->AddToPythonDictionary(pDictionary, definition->units, QString::null);
			}

			std::string interpreted_value(value.toStdString());

			// Run through the value looking for the various units (inch/mm) and
			// add the modification required to convert it into mm.
			//
			// eg: '1/8 inch' -> '((1/8) * 25.4)'  (assuming we're using mm)
			// eg: '1/8 inch' -> '((1/8) * 1.0)'  (assuming we're using inches)
			// eg: '1/8 mm' -> '((1/8) / 25.4)'  (assuming we're using inches)

			typedef std::list< std::pair<std::string, std::string> >  Patterns_t;
			Patterns_t patterns;
			Definition::Units_t units = Settings::Definition::Metric;
			
			const Settings::Length *length = dynamic_cast<const Settings::Length *>(definition);
			if (length) units = length->getUnits();

			const Settings::Rate *rate = dynamic_cast<const Settings::Rate *>(definition);
			if (rate) units = rate->getUnits();

			if (units == Settings::Definition::Metric)
			{
				// We're using mm.
				patterns.push_back( std::make_pair(std::string("Inches"), std::string(" * 25.4")));
				patterns.push_back( std::make_pair(std::string("inches"), std::string(" * 25.4")));
				patterns.push_back( std::make_pair(std::string("INCHES"), std::string(" * 25.4")));
				patterns.push_back( std::make_pair(std::string("Inch"), std::string(" * 25.4")));
				patterns.push_back( std::make_pair(std::string("INCH"), std::string(" * 25.4")));
				patterns.push_back( std::make_pair(std::string("inch"), std::string(" * 25.4")));
				patterns.push_back( std::make_pair(std::string("In"), std::string(" * 25.4")));
				patterns.push_back( std::make_pair(std::string("IN"), std::string(" * 25.4")));
				patterns.push_back( std::make_pair(std::string("in"), std::string(" * 25.4")));
				patterns.push_back( std::make_pair(std::string("mm"), std::string(" * 1.0")));
				patterns.push_back( std::make_pair(std::string("MM"), std::string(" * 1.0")));
				patterns.push_back( std::make_pair(std::string("ft"), std::string(" * 25.4 * 12")));
			}
			else
			{
				// We're using inches.
				patterns.push_back( std::make_pair(std::string("Inches"), std::string(" * 1.0")));
				patterns.push_back( std::make_pair(std::string("inches"), std::string(" * 1.0")));
				patterns.push_back( std::make_pair(std::string("INCHES"), std::string(" * 1.0")));
				patterns.push_back( std::make_pair(std::string("Inch"), std::string(" * 1.0")));
				patterns.push_back( std::make_pair(std::string("INCH"), std::string(" * 1.0")));
				patterns.push_back( std::make_pair(std::string("inch"), std::string(" * 1.0")));
				patterns.push_back( std::make_pair(std::string("In"), std::string(" * 1.0")));
				patterns.push_back( std::make_pair(std::string("IN"), std::string(" * 1.0")));
				patterns.push_back( std::make_pair(std::string("in"), std::string(" * 1.0")));
				patterns.push_back( std::make_pair(std::string("mm"), std::string(" / 25.4")));
				patterns.push_back( std::make_pair(std::string("MM"), std::string(" / 25.4")));
				patterns.push_back( std::make_pair(std::string("ft"), std::string(" / (25.4 * 12)")));
			}

			// NOTE: The patterns list must be arranged from most verbose to least verbose so that
			// a substitution of 'in' does not occur before the substitution of 'inches'
			patterns.sort();
			patterns.reverse();

			for (Patterns_t::iterator itPattern = patterns.begin(); itPattern != patterns.end(); itPattern++)
			{
				std::string::size_type offset = std::string::npos;
				while ((offset = interpreted_value.find(itPattern->first)) != std::string::npos)
				{
					// We need to see if the keyword was part of a variable name or a word by itself.
					// If it's surrounded by '_' or aphabetic characters then it must be part of
					// another word.
					if (offset > 1)
					{
						if ((interpreted_value[offset-1] == '_') || ((interpreted_value[offset-1] >= 'a') && (interpreted_value[offset-1] <= 'Z'))) break;
					}

					if (interpreted_value.length() > (offset + itPattern->first.length()))
					{
						if ((interpreted_value[offset + itPattern->first.length()] == '_') || ((interpreted_value[offset + itPattern->first.length()] >= 'a') && (interpreted_value[offset + itPattern->first.length()] <= 'Z'))) break;
					}

					interpreted_value.insert(0,std::string("(("));
					std::string replacement;
					replacement = std::string(")") + itPattern->second + std::string(")");
					interpreted_value.replace(offset+2, itPattern->first.length(), replacement);
				}
			}

			// Now step through the value and change all integer values (3) into floating point
			// values (3.0) so that values such as '1/8' will be interpreted as '1.0/8.0' to force floating
			// point arithmetic rather than integer arithmetic.

			// eg: '((1/8) * 25.4)' -> '((1.0/8.0) * 25.4)'

			for (std::string::size_type i = 0; i <= interpreted_value.length();)
			{
				std::pair<std::string::size_type, std::string::size_type> pointers = next_possible_number(interpreted_value, i);
				if ((pointers.first != std::string::npos) && (pointers.second != std::string::npos))
				{
					std::string replacement(interpreted_value.substr(0, pointers.first));
					std::string a(interpreted_value.substr(pointers.first, pointers.second - pointers.first + 1));
					if ((a.find('.') == std::string::npos) && (a.size() > 0) && (a[a.size()-1] >= '0') && (a[a.size()-1] <= '9'))
					{
						a += std::string(".0");
					}

					replacement += a;

					i = replacement.length();

					if (pointers.second < interpreted_value.length())
					{
						replacement.append(interpreted_value.substr(pointers.second+1));
					}

					interpreted_value.assign(replacement);
				}
				else
				{
					break;
				}
			}

			// Remove any leading or trailing spaces.
			while ((interpreted_value.size() > 0) && (interpreted_value[0] == ' '))
			{
			    interpreted_value.erase(0,1);
			}

			while ((interpreted_value.size() > 0) && (interpreted_value[interpreted_value.size()-1] == ' '))
			{
			    interpreted_value.erase(interpreted_value.size()-1,1);
			}

			qDebug("%s\n", interpreted_value.c_str());
            PyObject *pResult = PyRun_String(interpreted_value.c_str(), Py_eval_input, pDictionary, pDictionary);
            if (pResult != NULL)
            {
                double d;
                if (PyArg_Parse(pResult, "d", &d))
                {
					std::ostringstream oss_value;
					oss_value << d;
					evaluated_version = QString::fromStdString(oss_value.str());
                    return_status = true;
                    Py_XDECREF(pResult);
                    pResult = NULL;
                }
                else
                {
                    // wxMessageBox(_("Value does not evaluate to a floating point number"));
                    Py_XDECREF(pResult);
                    pResult = NULL;
                    return_status = false;
                }
            }
            else
            {
				/*
                QString message;
                message << _("Could not evaluate ") << interpreted_value << _(" using Python");
                wxMessageBox(message);
				*/
                return_status = false;
            }

            // Do NOT release the dictionary - I still don't know why.
            // Py_XDECREF(pDictionary);
            // pDictionary = NULL;
        }
        else
        {
            // wxMessageBox(_("Could not load dictionary from 'math' module in Python interpreter"));
            return_status = false;
        }

        Py_XDECREF(pModule);
        pModule = NULL;
    }
    else
    {
        // wxMessageBox(_("Could not load 'math' module in Python interpreter"));
        return_status = false;
    }

    // Py_Initialize() and Py_Finalize() should only occur once per process.  Do it in the main application framework instead.
    // Py_Finalize();

    // return(evaluated_version);
    return(return_status);
}



/**
	Interpret the entered_value as a Python script that should return a floating point number.
 */
bool TPGSettings::EvaluateLength( const Definition *definition, const char *entered_value, double *pResult ) const
{	
	QString evaluated_version;
	if (this->EvaluateWithPython( definition, QString::fromAscii(entered_value), evaluated_version ))
	{
		bool status;
		*pResult = evaluated_version.toDouble(&status);
		return(status);
	}
	else
	{
		return(false);
	}
}


bool TPGSettings::AddToPythonDictionary(PyObject *pDictionary, const QString requested_units, const QString prefix) const
{
	bool status = false;
	for (std::vector<Definition*>::const_iterator itDef = this->settingDefs.begin(); itDef != this->settingDefs.end(); itDef++)
	{
		if ((*itDef)->AddToPythonDictionary(pDictionary, requested_units, prefix))
		{
			status = true;	// We have at least one.
		}
	}
	return(status);
}


Settings::Definition *Settings::TPGSettings::getDefinition(const QString action, const QString name) const
{
	if (action.isNull() || action.isEmpty()) {
		Base::Console().Message("action not set\n");
		return(NULL);
	}

	// compute full setting name (<action>::<name>)
	QString qname = makeName(action, name);

    // get setting value
	if (tpgFeature != NULL) {
		std::map<QString, Settings::Definition *>::const_iterator itDef = settingDefsMap.find(qname);
		if (itDef != settingDefsMap.end()) {
			return(itDef->second);
		}
	}
	Base::Console().Message("Can't find setting!\n");
    return(NULL);
}


#ifdef WIN32
#pragma endregion "Settings::TPGSettings"
#endif


// ----------------Settings::Filename------------------------------
#ifdef WIN32
#pragma region "Settings::Filename"
#endif


bool Filename::AddToPythonDictionary(PyObject *pDictionary, const QString requested_units, const QString prefix) const
{
	// Replace any spaces within the variable name with underbars so that the
	// name can be used as a Python variable name.
	std::string processed_name = PythonName(prefix);

	// Name=Value
	bool status = false;
	PyObject *pName = PyString_FromString(processed_name.c_str());
	PyObject *pValue = PyString_FromString(this->getValue().toAscii().constData());

	if ((pName != NULL) && (pValue != NULL))
	{
		PyDict_SetItem(pDictionary, pName, pValue);
		status = true;
	}

	Py_XDECREF(pName); pName=NULL;
	Py_XDECREF(pValue); pValue=NULL;
	return(status);
}

Settings::Filename *Settings::TPGSettings::asFilename(const QString action, const QString name) const
{
	Settings::Definition *definition = this->getDefinition(action, name);
	if ((definition != NULL) && (definition->type == Settings::Definition::SettingType_Filename)) return((Settings::Filename *) definition);
	return(NULL);
}

#ifdef WIN32
#pragma endregion "Settings::Filename"
#endif


// ----------------Settings::Text------------------------------
#ifdef WIN32
#pragma region "Settings::Text"
#endif

Settings::Text	*Settings::TPGSettings::asText(const QString action, const QString name) const
{
	Settings::Definition *definition = this->getDefinition(action, name);
	if ((definition != NULL) && (definition->type == Settings::Definition::SettingType_Text)) return((Settings::Text *) definition);
	return(NULL);
}


bool Text::AddToPythonDictionary(PyObject *pDictionary, const QString requested_units, const QString prefix) const
{
	// Replace any spaces within the variable name with underbars so that the
	// name can be used as a Python variable name.
	std::string processed_name = PythonName(prefix);

	// Name=Value
	bool status = false;
	PyObject *pName = PyString_FromString(processed_name.c_str());
	PyObject *pValue = PyString_FromString(this->getValue().toAscii().constData());

	if ((pName != NULL) && (pValue != NULL))
	{
		PyDict_SetItem(pDictionary, pName, pValue);
		status = true;
	}

	Py_XDECREF(pName); pName=NULL;
	Py_XDECREF(pValue); pValue=NULL;
	return(status);
}

#ifdef WIN32
#pragma endregion "Settings::Text"
#endif





Settings::Color	*Settings::TPGSettings::asColor(const QString action, const QString name) const
{
	Settings::Definition *definition = this->getDefinition(action, name);
	if ((definition != NULL) && (definition->type == Settings::Definition::SettingType_Color)) return((Settings::Color *) definition);
	return(NULL);
}

Settings::ObjectNamesForType	*Settings::TPGSettings::asObjectNamesForType(const QString action, const QString name) const
{
	Settings::Definition *definition = this->getDefinition(action, name);
	if ((definition != NULL) && (definition->type == Settings::Definition::SettingType_ObjectNamesForType)) return((Settings::ObjectNamesForType *) definition);
	return(NULL);
}

Settings::SingleObjectNameForType	*Settings::TPGSettings::asSingleObjectNameForType(const QString action, const QString name) const
{
	Settings::Definition *definition = this->getDefinition(action, name);
	if ((definition != NULL) && (definition->type == Settings::Definition::SettingType_SingleObjectNameForType)) return((Settings::SingleObjectNameForType *) definition);
	return(NULL);
}


Settings::Enumeration *Settings::TPGSettings::asEnumeration(const QString action, const QString name) const
{
	Settings::Definition *definition = this->getDefinition(action, name);
	if ((definition != NULL) && (definition->type == Settings::Definition::SettingType_Enumeration)) return((Settings::Enumeration *) definition);
	return(NULL);
}

Settings::Length *Settings::TPGSettings::asLength(const QString action, const QString name) const
{
	Settings::Definition *definition = this->getDefinition(action, name);
	if ((definition != NULL) && (definition->type == Settings::Definition::SettingType_Length)) return((Settings::Length *) definition);
	return(NULL);
}




// ----------------Settings::Double------------------------------
#ifdef WIN32
#pragma region "Settings::Double"
#endif

bool Double::AddToPythonDictionary(PyObject *pDictionary, const QString requested_units, const QString prefix) const
{
	// Replace any spaces within the variable name with underbars so that the
	// name can be used as a Python variable name.
	std::string processed_name = PythonName(prefix);

	// Name=Value
	bool status = false;
	double value = this->get();
	
	PyObject *pName = PyString_FromString(processed_name.c_str());
	PyObject *pValue = PyFloat_FromDouble(value);

	if ((pName != NULL) && (pValue != NULL))
	{
		PyDict_SetItem(pDictionary, pName, pValue);
		status = true;
	}

	Py_XDECREF(pName); pName=NULL;
	Py_XDECREF(pValue); pValue=NULL;

	return(status);
}


Double::Double(
		const char *name, 
		const char *label, 
		const char *helptext,
		const double default_value,
		const double minimum, 
		const double maximum, 
		const char *units ):
	  Definition(name, label, SettingType_Double, "", units, helptext)
{
	std::ostringstream min;
	min << minimum;

	std::ostringstream max;
	max << maximum;

	this->options.push_back( new Option(QString::fromAscii("minimum"), QString::fromStdString(min.str()) ));
	this->options.push_back( new Option(QString::fromAscii("maximum"), QString::fromStdString(max.str()) ));

	std::ostringstream def_val;
	def_val << default_value;
	this->defaultvalue = QString::fromStdString(def_val.str());
}

Double::Double(
		const char *name, 
		const char *label, 
		const char *helptext,
		const double default_value,
		const char * units ):
	  Definition(name, label, SettingType_Double, "", units, helptext)
{
	std::ostringstream def_val;
	def_val << default_value;
	this->defaultvalue = QString::fromStdString(def_val.str());
}

	  
double Settings::Double::Minimum() const
{
	Option *option = this->getOption(QString::fromAscii("minimum"));
	if (option)
	{
		bool status;
		double value = option->label.toDouble(&status);
		if (status)
		{
			return(value);
		}
		else
		{
			return(0.0);
		}
	}
	else
	{
		return(0.0);
	}
}

void Settings::Double::Minimum(const double value)
{
	Option *option = this->getOption(QString::fromAscii("minimum"));
	if (option)
	{
		std::ostringstream ossValue;
		ossValue << value;
		option->label = QString::fromStdString(ossValue.str());
	}
}

double Settings::Double::Maximum() const
{
	Option *option = this->getOption(QString::fromAscii("maximum"));
	if (option)
	{
		bool status;
		double value = option->label.toDouble(&status);
		if (status)
		{
			return(value);
		}
		else
		{
			return(0.0);
		}
	}
	else
	{
		return(0.0);
	}
}
void Settings::Double::Maximum(const double value)
{
	Option *option = this->getOption(QString::fromAscii("maximum"));
	if (option)
	{
		std::ostringstream ossValue;
		ossValue << value;
		option->label = QString::fromStdString(ossValue.str());
	}
}


double Settings::Double::get() const
{
	if (this->getValue().isNull() == false)	return(this->getValue().toDouble());
	return(this->defaultvalue.toDouble());
}


void Settings::Double::set(const double value)
{
	std::ostringstream encoded_value;
	encoded_value << value;
	this->setValue(QString::fromStdString(encoded_value.str()));
}


bool Settings::Double::Evaluate( const char *formula, double *pResult ) const
{
	if (! this->parent)
	{
		return(false);
	}

	return(this->parent->EvaluateLength( this, formula, pResult ));
}


/* virtual */ Definition::ValidationState Double::validate(QString & input,int & position) const
{
	if (input.length() == 0) return(this->Intermediate);
	if ((input.length() == 1) && (input == QString::fromAscii("-"))) return(this->Intermediate);
	if ((input.length() == 1) && (input == QString::fromAscii("+"))) return(this->Intermediate);

	Option *minOption = this->getOption(QString::fromAscii("minimum"));
	Option *maxOption = this->getOption(QString::fromAscii("maximum"));

	double value;
	if (this->parent->EvaluateLength( this, input.toAscii().constData(), &value ))
	{
		if ((minOption) && (value < minOption->label.toDouble())) return(this->Invalid);
		if ((maxOption) && (value > maxOption->label.toDouble())) return(this->Invalid);
		return(this->Acceptable);
	}
	else
	{
		return(this->Intermediate);
	}
}


Settings::Double *Settings::TPGSettings::asDouble(const QString action, const QString name) const
{
	Settings::Definition *definition = this->getDefinition(action, name);
	if ((definition != NULL) && (definition->type == Settings::Definition::SettingType_Double)) return((Settings::Double *) definition);
	return(NULL);
}

#ifdef WIN32
#pragma endregion "Settings::Double"
#endif


// ----------------Settings::Rate------------------------------
#ifdef WIN32
#pragma region "Settings::Rate"
#endif

Settings::Rate::Rate(
		const char *name, 
		const char *label, 
		const char *helptext,
		const double default_value,
		const double minimum, 
		const double maximum, 
		const Definition::Units_t units ):
	  Definition(name, label, SettingType_Rate, "", "", helptext)
{
	std::ostringstream min;
	min << minimum;

	std::ostringstream max;
	max << maximum;

	this->options.push_back( new Option(QString::fromAscii("minimum"), QString::fromStdString(min.str()) ));
	this->options.push_back( new Option(QString::fromAscii("maximum"), QString::fromStdString(max.str()) ));

	Encode_t data = this->decode();
	boost::tuples::get<valueOffset>(data) = default_value;
	boost::tuples::get<unitsOffset>(data) = units;
	
	this->setValue(this->encode(data));
	this->defaultvalue = this->encode( data );
}

Settings::Rate::Rate(
		const char *name, 
		const char *label, 
		const char *helptext,
		const double default_value,
		const Definition::Units_t units ):
	  Definition(name, label, SettingType_Rate, "", "", helptext)
{
	Encode_t data;
	boost::tuples::get<valueOffset>(data) = default_value;
	boost::tuples::get<unitsOffset>(data) = units;

	this->defaultvalue = this->encode(data);
}


/**
	The Length setting can change both its 'value' and its 'units' so it
	is necessary to store both of these values in the TPGFeature::PropTPGSettings
	map.  This way, they are both saved/restored to/from the data file.
	Now that we're trying to get the value alone, we need to retrieve
	the encoded version (i.e. the string that includes both the value
	and the units) and decode just the value part of it.  We then need
	to interpret that value as a double and convert, if necessary, to
	the units the caller has asked the value to be expressed in.
*/
double Settings::Rate::get(const Definition::Units_t requested_units) const
{
	Encode_t data = this->decode();

	double value = boost::tuples::get<valueOffset>(data);
	Definition::Units_t these_units = boost::tuples::get<unitsOffset>(data);

	if ((requested_units == Settings::Definition::Metric) && (these_units == Settings::Definition::Imperial)) return(value * 25.4);
	if ((requested_units == Settings::Definition::Imperial) && (these_units == Settings::Definition::Metric)) return(value / 25.4);

	return(value);	// The units must be the same.
}

QString Settings::Rate::encode(const Encode_t data) const
{
	using boost::property_tree::ptree;
	ptree pt;

	pt.put("rate.value",  boost::tuples::get<valueOffset>(data));
	pt.put("rate.units",  int(boost::tuples::get<unitsOffset>(data)));

	std::ostringstream encoded_value;

	write_json(encoded_value, pt);
	return(QString::fromStdString(encoded_value.str()));
}


Settings::Rate::Encode_t Settings::Rate::decode() const
{
	std::list<QString> values;
	values.push_back( this->getValue() );
	values.push_back( this->defaultvalue );
	for (std::list<QString>::const_iterator itValue = values.begin(); itValue != values.end(); itValue++)
	{
		if (itValue->isNull() == false)
		{
			try
			{
				using boost::property_tree::ptree;
				ptree pt;

				std::stringstream encoded_value;
				encoded_value << itValue->toAscii().constData();
				
				read_json(encoded_value, pt);

				Encode_t data;

				boost::tuples::get<valueOffset>(data) = pt.get<double>("rate.value");
				boost::tuples::get<unitsOffset>(data) = Definition::Units_t(pt.get<int>("rate.units"));

				return(data);
			}
			catch(boost::property_tree::ptree_error const & error)
			{
				qWarning("%s\n", error.what());
				
			}
		}
	}

	Encode_t data;

	boost::tuples::get<valueOffset>(data) = 0.0;
	boost::tuples::get<unitsOffset>(data) = Definition::Metric;

	return(data);
}


void Settings::Rate::set(const double value)
{
	this->set(value, this->getUnits());
}


void Settings::Rate::set(const double value, const Settings::Definition::Units_t units)
{
	this->units << units;
	this->units += QString::fromAscii("/min");

	Encode_t data = this->decode();
	boost::tuples::get<unitsOffset>(data) = units;
	boost::tuples::get<valueOffset>(data) = value;
	this->setValue(this->encode(data));
}

Settings::Definition::Units_t Settings::Rate::getUnits() const
{
	Encode_t data = this->decode();
	return(boost::tuples::get<unitsOffset>(data));
}

void Settings::Rate::setUnits(const Settings::Definition::Units_t class_of_units)
{
	this->units << class_of_units;
	this->units += QString::fromAscii("/min");

	Encode_t data = this->decode();

	Definition::Units_t these_units = boost::tuples::get<unitsOffset>(data);
	if ((these_units == Definition::Metric) && (class_of_units == Definition::Imperial)) boost::tuples::get<valueOffset>(data) /= 25.4;
	if ((these_units == Definition::Imperial) && (class_of_units == Definition::Metric)) boost::tuples::get<valueOffset>(data) *= 25.4;

	boost::tuples::get<unitsOffset>(data) = class_of_units;
	this->setValue(this->encode(data));
}



bool Settings::Rate::Evaluate( const char *formula, double *pResult ) const
{
	if (! this->parent)
	{
		return(false);
	}

	return(this->parent->EvaluateLength( this, formula, pResult ));
}


/* virtual */ Definition::ValidationState Rate::validate(QString & input,int & position) const
{
	if (input.length() == 0) return(this->Intermediate);
	if ((input.length() == 1) && (input == QString::fromAscii("-"))) return(this->Intermediate);
	if ((input.length() == 1) && (input == QString::fromAscii("+"))) return(this->Intermediate);

	Option *minOption = this->getOption(QString::fromAscii("minimum"));
	Option *maxOption = this->getOption(QString::fromAscii("maximum"));

	double value;
	if (this->parent->EvaluateLength( this, input.toAscii().constData(), &value ))
	{
		if ((minOption) && (value < minOption->label.toDouble())) return(this->Invalid);
		if ((maxOption) && (value > maxOption->label.toDouble())) return(this->Invalid);
		return(this->Acceptable);
	}
	else
	{
		return(this->Intermediate);
	}
}


double Settings::Rate::Minimum() const
{
	Option *option = this->getOption(QString::fromAscii("minimum"));
	if (option)
	{
		bool status;
		double value = option->label.toDouble(&status);
		if (status)
		{
			return(value);
		}
		else
		{
			return(0.0);
		}
	}
	else
	{
		return(0.0);
	}
}

void Settings::Rate::Minimum(const double value)
{
	Option *option = this->getOption(QString::fromAscii("minimum"));
	if (option)
	{
		std::ostringstream ossValue;
		ossValue << value;
		option->label = QString::fromStdString(ossValue.str());
	}
}

double Settings::Rate::Maximum() const
{
	Option *option = this->getOption(QString::fromAscii("maximum"));
	if (option)
	{
		bool status;
		double value = option->label.toDouble(&status);
		if (status)
		{
			return(value);
		}
		else
		{
			return(0.0);
		}
	}
	else
	{
		return(0.0);
	}
}
void Settings::Rate::Maximum(const double value)
{
	Option *option = this->getOption(QString::fromAscii("maximum"));
	if (option)
	{
		std::ostringstream ossValue;
		ossValue << value;
		option->label = QString::fromStdString(ossValue.str());
	}
}


bool Rate::AddToPythonDictionary(PyObject *pDictionary, const QString requested_units, const QString prefix) const
{
	// Replace any spaces within the variable name with underbars so that the
	// name can be used as a Python variable name.
	std::string processed_name = PythonName(prefix);

	// Name=Value
	bool status = false;
	
	double value = this->get( this->getUnits() );

	QString metric, imperial;
	metric << Definition::Metric;
	metric += QString::fromAscii("/min");
	imperial << Definition::Imperial;
	imperial += QString::fromAscii("/min");

	if ((this->getUnits() == Definition::Metric) && (requested_units == imperial))
	{
		// We have mm but the setting being changed uses inches.  Convert now.
		value /= 25.4;
	}
	else if ((this->getUnits() == Definition::Imperial) && (requested_units == metric))	
	{
		// We're using inches but the setting being changed uses mm. Convert now.
		value *= 25.4;
	}	

	PyObject *pName = PyString_FromString(processed_name.c_str());
	PyObject *pValue = PyFloat_FromDouble(value);

	if ((pName != NULL) && (pValue != NULL))
	{
		PyDict_SetItem(pDictionary, pName, pValue);
		status = true;
	}

	Py_XDECREF(pName); pName=NULL;
	Py_XDECREF(pValue); pValue=NULL;

	return(status);
}



Settings::Rate *Settings::TPGSettings::asRate(const QString action, const QString name) const
{
	Settings::Definition *definition = this->getDefinition(action, name);
	if ((definition != NULL) && (definition->type == Settings::Definition::SettingType_Rate)) return((Settings::Rate *) definition);
	return(NULL);
}

#ifdef WIN32
#pragma endregion "Settings::Rate"
#endif


// ----------------Settings::Integer------------------------------
#ifdef WIN32
#pragma region "Settings::Integer"
#endif

Definition::ValidationState Integer::validate(QString & input,int & position) const
{
	if (input.length() == 0) return(this->Intermediate);
	if ((input.length() == 1) && (input == QString::fromAscii("-"))) return(this->Intermediate);
	if ((input.length() == 1) && (input == QString::fromAscii("+"))) return(this->Intermediate);

	Option *minOption = this->getOption(QString::fromAscii("minimum"));
	Option *maxOption = this->getOption(QString::fromAscii("maximum"));
	
	double value;
	if (this->parent->EvaluateLength( this, input.toAscii().constData(), &value ))
	{
		if ((minOption) && (value < minOption->label.toInt())) return(this->Invalid);
		if ((maxOption) && (value > maxOption->label.toInt())) return(this->Invalid);
		return(this->Acceptable);
	}
	else
	{
		return(this->Intermediate);
	}
}

int Settings::Integer::get() const
{
	if (this->getValue().isNull() == false)	return(this->getValue().toInt());
	return(this->defaultvalue.toInt());
}


void Settings::Integer::set(const int value)
{
	std::ostringstream encoded_value;

	encoded_value << value;
	this->setValue(QString::fromStdString(encoded_value.str()));
}

bool Settings::Integer::Evaluate( const char *formula, int *pResult ) const
{
	if (! this->parent)
	{
		return(false);
	}

	double value;
	if (this->parent->EvaluateLength( this, formula, &value ))
	{
		*pResult = int(value);
		return(true);
	}
	else
	{
		return(false);
	}
}

bool Integer::AddToPythonDictionary(PyObject *pDictionary, const QString requested_units, const QString prefix) const
{
	// Replace any spaces within the variable name with underbars so that the
	// name can be used as a Python variable name.
	std::string processed_name = PythonName(prefix);

	// Name=Value
	bool status = false;
	long value = this->get();

	PyObject *pName = PyString_FromString(processed_name.c_str());
	PyObject *pValue = PyInt_FromLong(value);

	if ((pName != NULL) && (pValue != NULL))
	{
		PyDict_SetItem(pDictionary, pName, pValue);
		status = true;
	}

	Py_XDECREF(pName); pName=NULL;
	Py_XDECREF(pValue); pValue=NULL;

	return(status);
}

int Settings::Integer::Minimum() const
{
	Option *option = this->getOption(QString::fromAscii("minimum"));
	if (option)
	{
		bool status;
		int value = option->label.toInt(&status);
		if (status)
		{
			return(value);
		}
		else
		{
			return(0.0);
		}
	}
	else
	{
		return(0.0);
	}
}

void Settings::Integer::Minimum(const int value)
{
	Option *option = this->getOption(QString::fromAscii("minimum"));
	if (option)
	{
		std::ostringstream ossValue;
		ossValue << value;
		option->label = QString::fromStdString(ossValue.str());
	}
}

int Settings::Integer::Maximum() const
{
	Option *option = this->getOption(QString::fromAscii("maximum"));
	if (option)
	{
		bool status;
		double value = option->label.toInt(&status);
		if (status)
		{
			return(value);
		}
		else
		{
			return(0.0);
		}
	}
	else
	{
		return(0.0);
	}
}
void Settings::Integer::Maximum(const int value)
{
	Option *option = this->getOption(QString::fromAscii("maximum"));
	if (option)
	{
		std::ostringstream ossValue;
		ossValue << value;
		option->label = QString::fromStdString(ossValue.str());
	}
}


Settings::Integer *Settings::TPGSettings::asInteger(const QString action, const QString name) const
{
	Settings::Definition *definition = this->getDefinition(action, name);
	if ((definition != NULL) && (definition->type == Settings::Definition::SettingType_Integer)) return((Settings::Integer *) definition);
	return(NULL);
}


#ifdef WIN32
#pragma endregion "Settings::Integer"
#endif


// ----------------Settings::Directory------------------------------
#ifdef WIN32
#pragma region "Settings::Directory"
#endif

Settings::Directory	*Settings::TPGSettings::asDirectory(const QString action, const QString name) const
{
	Settings::Definition *definition = this->getDefinition(action, name);
	if ((definition != NULL) && (definition->type == Settings::Definition::SettingType_Directory)) return((Settings::Directory *) definition);
	return(NULL);
}

bool Directory::AddToPythonDictionary(PyObject *pDictionary, const QString requested_units, const QString prefix) const
{
	// Replace any spaces within the variable name with underbars so that the
	// name can be used as a Python variable name.
	std::string processed_name = PythonName(prefix);

	// Name=Value
	bool status = false;
	PyObject *pName = PyString_FromString(processed_name.c_str());
	PyObject *pValue = PyString_FromString(this->getValue().toAscii().constData());

	if ((pName != NULL) && (pValue != NULL))
	{
		PyDict_SetItem(pDictionary, pName, pValue);
		status = true;
	}

	Py_XDECREF(pName); pName=NULL;
	Py_XDECREF(pValue); pValue=NULL;
	return(status);
}

#ifdef WIN32
#pragma endregion "Settings::Directory"
#endif


// ----------------Settings::ObjectNamesForType------------------------------
#ifdef WIN32
#pragma region "Settings::ObjectNamesForType"
#endif

/**
	Settings whose type is SettingType_ObjectNamesForType are expected to have a value that includes
	a list of object names.  The characters that delimit the names must be found in an option
	whose ID = "Delimiters".  Once the value has been parsed into separate object names
	using these delimiting characters, pointers to them are retrieved using their names alone.  Once
	found, their types MUST be included within the options whose ID = "TypeId".  eg: for this a
	name might be "Circle01, Line74", the options might be;
		- id = "Delimiters" label = " \n\t,"
		- id = "TypeId" label = "Part::Feature"
		- id = "TypeId" label = "Cam::TPGFeature"
 */
Definition::ValidationState ObjectNamesForType::validate(QString & input,int & position) const
{
	// Cast this object to a TPGObjectNamesForTypeSettingDefinition object so we can use the helper functions
	// to review the object's contents.
	ObjectNamesForType *pSetting = (ObjectNamesForType *) this;

	Option *delimiters_option = this->getOption(QString::fromAscii("Delimiters"));
	if (delimiters_option == NULL)
	{
		return(this->Invalid);
	}

	// Use a regular expression so that we use any one of the characters in the delimiters string
	// as a delimiting character.
	QString expression = QString::fromAscii("[") + delimiters_option->label + QString::fromAscii("]");
	QRegExp regular_expression(expression);
	QStringList object_names = input.split(regular_expression, QString::SkipEmptyParts);

	QStringList valid_type_names = pSetting->GetTypes();
	
	App::Document *document = App::GetApplication().getActiveDocument();
	if (document)
	{
		for (QStringList::size_type i=0; i<object_names.size(); i++)
		{
			QString name = object_names[i];
			App::DocumentObject *object = document->getObject(object_names[i].toAscii().constData());
			if(object)
			{
				bool is_valid = false;
				for (QStringList::const_iterator itTypeName = valid_type_names.begin(); itTypeName != valid_type_names.end(); itTypeName++)
				{
					if (object->isDerivedFrom(Base::Type::fromName((*itTypeName).toAscii().constData())))
					{
						is_valid = true;
					}
				}
				if (! is_valid)
				{
					return(this->Intermediate);	// They may be half way through typing it.  Give them the bennefit of the doubt.
				}
			}
			else
			{
				return(this->Intermediate);	// They may be half way through typing it.  Give them the bennefit of the doubt.
			}
		}
	}

	return(this->Acceptable);
}


bool ObjectNamesForType::AddToPythonDictionary(PyObject *pDictionary, const QString requested_units, const QString prefix) const
{
	return(false);
}

ObjectNamesForType::ObjectNamesForType(	const char *name, 
								const char *label, 
								const char *helptext,
								const char *delimiters,
								const char *object_type )
								 : Definition(name, label, SettingType_ObjectNamesForType, "", "", helptext)
{
	this->addOption( QString::fromAscii("Delimiters"), QString::fromAscii(delimiters) );
	this->addOption( QString::fromAscii("TypeId"), QString::fromAscii(object_type) );
}

void ObjectNamesForType::Add(const char * object_type)
{
	this->addOption( QString::fromAscii("TypeId"), QString::fromAscii(object_type) );
}

void ObjectNamesForType::SetDelimiters(const char * delimiters)
{

	Option *delimiters_option = this->getOption(QString::fromAscii("Delimiters"));
	if (delimiters_option != NULL)
	{
		delimiters_option->label = QString::fromAscii(delimiters);
		return;
	}

	// We couldn't find an existing option so add one now.
	this->addOption( QString::fromAscii("Delimiters"), QString::fromAscii(delimiters) );	
}


QStringList ObjectNamesForType::GetTypes() const
{
	QStringList types;

	for (QList<Option*>::const_iterator itOption = options.begin(); itOption != options.end(); itOption++)
	{
		if ((*itOption)->id.toUpper() == QString::fromAscii("TypeId").toUpper())
		{
			types.push_back((*itOption)->label);
		}
	}

	return(types);
}

QStringList ObjectNamesForType::GetNames() const
{
	QStringList names;
	Option *delimiters_option = this->getOption(QString::fromAscii("Delimiters"));
	if (delimiters_option != NULL)
	{
		// Use a regular expression so that we use any one of the characters in the delimiters string
		// as a delimiting character.
		QString value = this->getValue();
		QString expression = QString::fromAscii("[") + delimiters_option->label + QString::fromAscii("]");
		QRegExp regular_expression(expression);
		names = value.split(regular_expression, QString::SkipEmptyParts);
	}

	return(names);
}

#ifdef WIN32
#pragma endregion "Settings::ObjectNamesForType"
#endif




// ----------------Settings::SingleObjectNameForType------------------------------
#ifdef WIN32
#pragma region "Settings::SingleObjectNameForType"
#endif

/**
	Settings whose type is SettingType_SingleObjectNameForType are expected to have a value that includes
	a single object name.  That named object MUST be of the type found within the option whose ID = "TypeId".  eg: for this a
	name might be "Centre Drill #4 HSS", the options might be;
		- id = "TypeId" label = "Cam::Tool"
 */
Definition::ValidationState SingleObjectNameForType::validate(QString & input,int & position) const
{
	// Cast this object to a TPGObjectNamesForTypeSettingDefinition object so we can use the helper functions
	// to review the object's contents.
	SingleObjectNameForType *pSetting = (SingleObjectNameForType *) this;

	App::Document *document = App::GetApplication().getActiveDocument();
	if (document)
	{
		App::DocumentObject *object = document->getObject(pSetting->GetName().toAscii().constData());
		if(object)
		{
			if (object->isDerivedFrom(Base::Type::fromName(pSetting->GetType().toAscii().constData())))
			{
				return(this->Acceptable);
			}
			else
			{
				return(this->Intermediate);	// They may be half way through typing it.  Give them the bennefit of the doubt.
			}
		}
		else
		{
			return(this->Intermediate);	// They may be half way through typing it.  Give them the bennefit of the doubt.
		}
	}

	return(this->Intermediate);
}


bool SingleObjectNameForType::AddToPythonDictionary(PyObject *pDictionary, const QString requested_units, const QString prefix) const
{
	return(false);
}

SingleObjectNameForType::SingleObjectNameForType(	const char *name, 
								const char *label, 
								const char *helptext,
								const char *object_type ) : Definition(name, label, SettingType_SingleObjectNameForType, object_type, "", helptext)
{
	this->addOption( QString::fromAscii("TypeId"), QString::fromAscii(object_type) );
}

void SingleObjectNameForType::Add(const char * object_type)
{
	this->addOption( QString::fromAscii("TypeId"), QString::fromAscii(object_type) );
}

QString SingleObjectNameForType::GetType() const
{
	for (QList<Option*>::const_iterator itOption = options.begin(); itOption != options.end(); itOption++)
	{
		if ((*itOption)->id.toUpper() == QString::fromAscii("TypeId").toUpper())
		{
			return((*itOption)->label);
		}
	}

	return(QString::null);
}

QString SingleObjectNameForType::GetName() const
{
	return(this->getValue());
}

#ifdef WIN32
#pragma endregion "Settings::SingleObjectNameForType"
#endif


// ----------------Settings::Length------------------------------
#ifdef WIN32
#pragma region "Settings::Length"
#endif

/* virtual */ Definition::ValidationState Length::validate(QString & input,int & position) const
{
	if (input.length() == 0) return(this->Intermediate);
	if ((input.length() == 1) && (input == QString::fromAscii("-"))) return(this->Intermediate);
	if ((input.length() == 1) && (input == QString::fromAscii("+"))) return(this->Intermediate);

	Option *minOption = this->getOption(QString::fromAscii("minimum"));
	Option *maxOption = this->getOption(QString::fromAscii("maximum"));

	double value;
	if (this->parent->EvaluateLength( this, input.toAscii().constData(), &value ))
	{
		if ((minOption) && (value < minOption->label.toDouble())) return(this->Invalid);
		if ((maxOption) && (value > maxOption->label.toDouble())) return(this->Invalid);
		return(this->Acceptable);
	}
	else
	{
		return(this->Intermediate);
	}
}


bool Settings::Length::Evaluate( const char *formula, double *pResult ) const
{
	if (! this->parent)
	{
		return(false);
	}

	return(this->parent->EvaluateLength( this, formula, pResult ));
}


Settings::Length::Length(
		const char *name, 
		const char *label, 
		const char *helptext,
		const double default_value,
		const double minimum, 
		const double maximum, 
		const Definition::Units_t units ):
	  Definition(name, label, SettingType_Length, "", "", helptext)
{
	std::ostringstream min;
	min << minimum;

	std::ostringstream max;
	max << maximum;

	this->options.push_back( new Option(QString::fromAscii("minimum"), QString::fromStdString(min.str()) ));
	this->options.push_back( new Option(QString::fromAscii("maximum"), QString::fromStdString(max.str()) ));

	Encode_t data;
	boost::tuples::get<valueOffset>(data) = default_value;
	boost::tuples::get<unitsOffset>(data) = units;

	this->defaultvalue = this->encode( data );
}

Settings::Length::Length(
		const char *name, 
		const char *label, 
		const char *helptext,
		const double default_value,
		const Definition::Units_t units ):
	  Definition(name, label, SettingType_Length, "", "", helptext)
{
	Encode_t data;
	boost::tuples::get<valueOffset>(data) = default_value;
	boost::tuples::get<unitsOffset>(data) = units;

	this->defaultvalue = this->encode( data );
}

Settings::Length::Encode_t Settings::Length::decode() const
{
	std::list<QString> values;
	values.push_back( this->getValue() );
	values.push_back( this->defaultvalue );
	for (std::list<QString>::const_iterator itValue = values.begin(); itValue != values.end(); itValue++)
	{
		if (itValue->isNull() == false)
		{
			try
			{
				using boost::property_tree::ptree;
				ptree pt;

				std::stringstream encoded_value;
				encoded_value << itValue->toAscii().constData();
				
				read_json(encoded_value, pt);

				Encode_t data;
				boost::tuples::get<valueOffset>(data) = pt.get<double>("length.value");
				boost::tuples::get<unitsOffset>(data) = Definition::Units_t(pt.get<int>("length.units"));

				return(data);
			}
			catch(boost::property_tree::ptree_error const & error)
			{
				qWarning("%s\n", error.what());
			}
		}
	}

	Encode_t data;
	boost::tuples::get<valueOffset>(data) = 0.0;
	boost::tuples::get<unitsOffset>(data) = Settings::Definition::Metric;

	return(data);
}

/**
	The Length setting can change both its 'value' and its 'units' so it
	is necessary to store both of these values in the TPGFeature::PropTPGSettings
	map.  This way, they are both saved/restored to/from the data file.
	Now that we're trying to get the value alone, we need to retrieve
	the encoded version (i.e. the string that includes both the value
	and the units) and decode just the value part of it.  We then need
	to interpret that value as a double and convert, if necessary, to
	the units the caller has asked the value to be expressed in.
*/
double Settings::Length::get(const Definition::Units_t requested_units) const
{
	Encode_t data = this->decode();

	double value = boost::tuples::get<valueOffset>(data);
	Settings::Definition::Units_t units = boost::tuples::get<unitsOffset>(data);

	if ((units == Definition::Metric) && (requested_units == Definition::Imperial)) return(value / 25.4);
	if ((units == Definition::Imperial) && (requested_units == Definition::Metric)) return(value * 25.4);

	// The units must be the same.
	return(value);
}

QString Settings::Length::encode(const Settings::Length::Encode_t data) const
{
	using boost::property_tree::ptree;
	ptree pt;

	pt.put("length.value", boost::tuples::get<valueOffset>(data));
	pt.put("length.units", int(boost::tuples::get<unitsOffset>(data)));

	std::ostringstream encoded_value;

	write_json(encoded_value, pt);
	return(QString::fromStdString(encoded_value.str()));
}

void Settings::Length::set(const double value)
{
	Encode_t data = this->decode();
	boost::tuples::get<valueOffset>(data) = value;
	this->setValue(this->encode(data));
}

void Settings::Length::set(const double value, const Settings::Definition::Units_t class_of_units)
{
	Encode_t data = this->decode();
	boost::tuples::get<valueOffset>(data) = value;
	boost::tuples::get<unitsOffset>(data) = class_of_units;
	this->setValue(this->encode(data));

	this->units << class_of_units;
}


Settings::Definition::Units_t Settings::Length::getUnits() const
{
	Encode_t data = this->decode();
	return(boost::tuples::get<unitsOffset>(data));
}



void Settings::Length::setUnits(const Settings::Definition::Units_t class_of_units)
{
	Encode_t data = this->decode();
	
	Definition::Units_t units = boost::tuples::get<unitsOffset>(data);

	if (units == class_of_units) return;
	if ((units == Definition::Metric) && (class_of_units == Definition::Imperial))
	{
		boost::tuples::get<valueOffset>(data) /= 25.4;
		boost::tuples::get<unitsOffset>(data) = class_of_units;
		this->units << class_of_units;
		this->setValue(this->encode(data));
		return;
	}

	if ((units == Definition::Imperial) && (class_of_units == Definition::Metric))
	{
		boost::tuples::get<valueOffset>(data) *= 25.4;
		boost::tuples::get<unitsOffset>(data) = class_of_units;
		this->units << class_of_units;
		this->setValue(this->encode(data));
		return;
	}

	this->units << class_of_units;
	this->setValue(this->encode(data));
}


double Settings::Length::Minimum() const
{
	Option *option = this->getOption(QString::fromAscii("minimum"));
	if (option)
	{
		bool status;
		double value = option->label.toDouble(&status);
		if (status)
		{
			return(value);
		}
		else
		{
			return(0.0);
		}
	}
	else
	{
		return(0.0);
	}
}

void Settings::Length::Minimum(const double value)
{
	Option *option = this->getOption(QString::fromAscii("minimum"));
	if (option)
	{
		std::ostringstream ossValue;
		ossValue << value;
		option->label = QString::fromStdString(ossValue.str());
	}
}

double Settings::Length::Maximum() const
{
	Option *option = this->getOption(QString::fromAscii("maximum"));
	if (option)
	{
		bool status;
		double value = option->label.toDouble(&status);
		if (status)
		{
			return(value);
		}
		else
		{
			return(0.0);
		}
	}
	else
	{
		return(0.0);
	}
}
void Settings::Length::Maximum(const double value)
{
	Option *option = this->getOption(QString::fromAscii("maximum"));
	if (option)
	{
		std::ostringstream ossValue;
		ossValue << value;
		option->label = QString::fromStdString(ossValue.str());
	}
}

bool Length::AddToPythonDictionary(PyObject *pDictionary, const QString requested_units, const QString prefix) const
{
	// Replace any spaces within the variable name with underbars so that the
	// name can be used as a Python variable name.
	std::string processed_name = PythonName(prefix);

	// Name=Value
	bool status = false;
	
	double value = this->get( this->getUnits() );
	QString metric, imperial;
	metric << Definition::Metric;
	imperial << Definition::Imperial;

	if ((this->getUnits() == Definition::Metric) && (requested_units == imperial))
	{
		// We have mm but the setting being changed uses inches.  Convert now.
		value /= 25.4;
	}
	else if ((this->getUnits() == Definition::Imperial) && (requested_units == metric))
	{
		// We're using inches but the setting being changed uses mm. Convert now.
		value *= 25.4;
	}

	PyObject *pName = PyString_FromString(processed_name.c_str());
	PyObject *pValue = PyFloat_FromDouble(value);

	if ((pName != NULL) && (pValue != NULL))
	{
		PyDict_SetItem(pDictionary, pName, pValue);
		status = true;
	}

	Py_XDECREF(pName); pName=NULL;
	Py_XDECREF(pValue); pValue=NULL;

	return(status);
}

#ifdef WIN32
#pragma endregion "Settings::Length"
#endif


// ----------------Settings::Enumeration------------------------------
#ifdef WIN32
#pragma region "Settings::Enumeration"
#endif

bool Settings::Enumeration::set(const int id)
{
	Map_t data = this->Values();
	if (data.find(id) == data.end())
	{
		return(false);
	}
	else
	{
		// The id has been found in this object's options and must, therefore, be valid.
		// Encode it as a string in the 'value' for this setting.
		std::ostringstream ossValue;
		ossValue << id;
		this->setValue(QString::fromStdString(ossValue.str()));
		return(true);
	}
}


bool Enumeration::AddToPythonDictionary(PyObject *pDictionary, const QString requested_units, const QString prefix) const
{
	return(false);
}

std::map<int, QString> Settings::Enumeration::Values() const
{
	std::map<int, QString> data;

	for (QList<Option *>::const_iterator itOption = this->options.begin(); itOption != this->options.end(); itOption++)
	{
		int id = (*itOption)->id.toInt();
		data.insert( std::make_pair( id, (*itOption)->label ) );
	}

	return(data);
}


void Settings::Enumeration::Add(const int id, const QString label)
{
	std::ostringstream ossId;
	ossId << id;

	Settings::Option *existing_option = this->getOption(QString::fromStdString(ossId.str()));
	if (existing_option == NULL)
	{
		Settings::Option *option = new Settings::Option(QString::fromStdString(ossId.str()), label);
		options.push_back( option );
	}
}


Settings::Enumeration::Pair_t Settings::Enumeration::get() const
{
	// The 'value' stored in the TPGFeature::PropTPGSettings map is the integer portion of the
	// enumerated type.
	int id = this->getValue().toInt();
	Map_t data = this->Values();
	if (data.find(id) == data.end())
	{
		return(Pair_t(-1, QString::null));
	}
	else
	{
		return(*(data.find(id)));
	}
}

#ifdef WIN32
#pragma endregion "Settings::Enumeration"
#endif



// ----------------Settings::Color------------------------------
#ifdef WIN32
#pragma region "Settings::Color"
#endif

QString Settings::Color::encode(const Settings::Color::Encode_t data) const
{
	using boost::property_tree::ptree;
	ptree pt;

	pt.put("color.red",  boost::tuples::get<redOffset>(data));
	pt.put("color.green", boost::tuples::get<greenOffset>(data));
	pt.put("color.blue", boost::tuples::get<blueOffset>(data));
	pt.put("color.alpha", boost::tuples::get<alphaOffset>(data));

	std::ostringstream encoded_value;

	write_json(encoded_value, pt);
	return(QString::fromStdString(encoded_value.str()));
}

Settings::Color::Encode_t Settings::Color::decode() const
{
	std::list<QString> values;
	values.push_back( this->getValue() );
	values.push_back( this->defaultvalue );
	for (std::list<QString>::const_iterator itValue = values.begin(); itValue != values.end(); itValue++)
	{
		if (itValue->isNull() == false)
		{
			try
			{
				using boost::property_tree::ptree;
				ptree pt;

				std::stringstream encoded_value;
				encoded_value << itValue->toAscii().constData();
				
				read_json(encoded_value, pt);

				Encode_t data;
				boost::tuples::get<redOffset>(data) = pt.get<int>("color.red");
				boost::tuples::get<greenOffset>(data) = pt.get<int>("color.green");
				boost::tuples::get<blueOffset>(data) = pt.get<int>("color.blue");
				boost::tuples::get<alphaOffset>(data) = pt.get<int>("color.alpha");

				return(data);
			}
			catch(boost::property_tree::ptree_error const & error)
			{
				qWarning("%s\n", error.what());
			}
		}
	}

	Encode_t data;
	boost::tuples::get<redOffset>(data) = 0;
	boost::tuples::get<greenOffset>(data) = 0;
	boost::tuples::get<blueOffset>(data) = 0;
	boost::tuples::get<alphaOffset>(data) = 255;

	return(data);
}

/**
	The color's value is encoded in an INI document describing
	all the various properties required to define a QColor object.  This
	XML document forms the 'value' part within the PropTPGSetting
	map.  We use the TPGColorSettingDefinition class to handle
	the conversion between the INI string used in the TPGFeature's 
	property and the QColor value we use in the code.
 */
bool Settings::Color::get(int &red, int &green, int &blue, int &alpha) const
{
	Encode_t data = this->decode();

	std::list<QString> values;
	values.push_back( this->getValue() );
	values.push_back( this->defaultvalue );
	for (std::list<QString>::const_iterator itValue = values.begin(); itValue != values.end(); itValue++)
	{
		if (itValue->isNull() == false)
		{
			try
			{
				using boost::property_tree::ptree;
				ptree pt;

				std::stringstream encoded_value;
				encoded_value << itValue->toAscii().constData();
				
				read_json(encoded_value, pt);

				red   = pt.get<int>("color.red");
				green = pt.get<int>("color.green");
				blue  = pt.get<int>("color.blue");
				alpha = pt.get<int>("color.alpha");

				return(true);	// success.
			}
			catch(boost::property_tree::ptree_error const & error)
			{
				qWarning("%s\n", error.what());
			}
		}
	}

	red = 0;
	green = 255;
	blue = 0;
	alpha = 255;

	return(false);	// failure.
}

void Settings::Color::set(const int red, const int green, const int blue, const int alpha)
{
	Encode_t data = this->decode();
	boost::tuples::get<redOffset>(data) = red;
	boost::tuples::get<greenOffset>(data) = green;
	boost::tuples::get<blueOffset>(data) = blue;
	boost::tuples::get<alphaOffset>(data) = alpha;
	
	this->setValue(this->encode(data));
}


bool Color::AddToPythonDictionary(PyObject *pDictionary, const QString requested_units, const QString prefix) const
{
	// Replace any spaces within the variable name with underbars so that the
	// name can be used as a Python variable name.
	std::string processed_name = PythonName(prefix);

	// Name_red=int
	// Name_green=int
	// Name_blue=int
	// Name_alpha=int

	bool status = false;

	Color *colour = (Color *) this;
	int red, green, blue, alpha;
	colour->get(red, green, blue, alpha);

	std::set< std::pair< std::string, int > > values;
	values.insert( std::make_pair( std::string( processed_name + "_red" ), red ) );
	values.insert( std::make_pair( std::string( processed_name + "_green" ), green ) );
	values.insert( std::make_pair( std::string( processed_name + "_blue" ), blue ) );
	values.insert( std::make_pair( std::string( processed_name + "_alpha" ), alpha ) );

	for (std::set< std::pair< std::string, int > >::iterator itValue = values.begin(); itValue != values.end(); itValue++)
	{
		PyObject *pName = PyString_FromString(itValue->first.c_str());
		PyObject *pValue = PyInt_FromLong(itValue->second);

		if ((pName != NULL) && (pValue != NULL))
		{
			PyDict_SetItem(pDictionary, pName, pValue);
			status = true;
		}

		Py_XDECREF(pName); pName=NULL;
		Py_XDECREF(pValue); pValue=NULL;
	}

	return(status);
}

#ifdef WIN32
#pragma endregion "Settings::Color"
#endif


// ----------------Settings::Radio------------------------------

#ifdef WIN32
#pragma region "Settings::Radio"
#endif

bool Radio::AddToPythonDictionary(PyObject *pDictionary, const QString requested_units, const QString prefix) const
{
	return(false);
}


Settings::Radio	*Settings::TPGSettings::asRadio(const QString action, const QString name) const
{
	Settings::Definition *definition = this->getDefinition(action, name);
	if ((definition != NULL) && (definition->type == Settings::Definition::SettingType_Radio)) return((Settings::Radio *) definition);
	return(NULL);
}

void Settings::Radio::Add(const char *value)
{
	this->addOption(value, value);	// Same value for both positions.
}


#ifdef WIN32
#pragma endregion "Settings::Radio"
#endif



} // end namespace Settings
} // end namespace Cam