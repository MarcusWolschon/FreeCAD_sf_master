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

#ifndef CAM_TPGSETTINGS_H
#define CAM_TPGSETTINGS_H

#include <PreCompiled.h>	// we need the __declspec(dllexport) macros for the Windows build

#include <App/PropertyLinks.h>
#include <App/PropertyStandard.h>

namespace Cam {
	namespace Settings {
	class CamExport Option;
	class CamExport Definition;
	class CamExport TPGSettings;

	// Forward declarations of the wrapper classes.
	class CamExport Text;
	class CamExport Radio;
	class CamExport Color;
	class CamExport ObjectNamesForType;
	class CamExport SingleObjectNameForType;
	class CamExport Enumeration;
	class CamExport Length;
	class CamExport Filename;
	class CamExport Directory;
	class CamExport Integer;
	class CamExport Double;
	class CamExport Rate;
	}
}

#include "../Features/TPGFeature.h"

#include <vector>
#include <QString>
#include <QList>
#include <QColor>

#include <App/PropertyStandard.h>

#include <boost/tuple/tuple.hpp>

namespace Cam
{
	namespace Settings 
	{
class CamExport TPGFeature; // Forward declaration so that the TPGFeature and the TPGSettings classes can both refer to each other.


/**
	Setting definitions such as 'Enumeration' require options so that the user interface can
	present the appropriate information.  Other setting definitions need extra pieces of information
	to make the setting work for its particular data type.  Such values are stored in a list
	of Option objects within the Definition class.  These options are effectively 'reference' data
	for the setting.  i.e. these values are NOT written to the data file and are, therefore, not
	restored when a new data file is openned.
 */
class CamExport Option
{
public:
    QString id;
    QString label;

    Option(QString id, QString label)
    {
        this->id = id;
        this->label = label;
    }

    Option(const char *id, const char *label)
    {
        this->id = QString::fromUtf8(id);
        this->label = QString::fromUtf8(label);
    }

	Option( const Option & rhs )
	{
		*this = rhs;	// Call the assignment operator.
	}

	Option & operator= ( const Option & rhs )
	{
		if (this != &rhs)
		{
			this->id = rhs.id;
			this->label = rhs.label;
		}

		return(*this);
	}

	friend QString & operator<< ( QString & qs, const Option & option )
	{
		std::ostringstream xml;
		xml << "<OPTION>\n"
			<< "\t<ID>" << option.id.toStdString() << "</ID>\n"
			<< "\t<LABEL>" << option.label.toStdString() << "</LABEL>\n"
			<< "</OPTION>\n";
		qs += QString::fromStdString(xml.str());
		return(qs);
	}
};

/**
	Each setting is defined here.  This class contains mostly reference information (i.e. information
	that remains the same throughout all instances of FreeCAD).  This reference information is not
	stored away with the data file.  There are a couple of pieces of information that are stored
	away with the data file.  Such pieces of information MUST be encoded into the 'value' which is
	stored in the TPGFeature::PropTPGSettings map.  This Definition class and its owning TPGSettings
	class have references back to the TPGFeature to which they belong.  The TPGFeature class
	inherits from the DocumentObject class which means its direct contents are written to the data
	file and restored when the file is subsequently reopenned.

	Many of the setting types have a single value which can easily be stored in the PropTPGSettings
	map.  Other setting types require more than one 'value' to be saved/restored in order to function
	correctly.  eg: A Length setting only makes sense when we know both the 'value' and the 'units'.
	Such values must encode all such information into a single string representation for storage
	in the PropTPGSettings map (and thus into the data file).  We define a series of 'wrapper classes'
	that perform such acts in a manner specific to their setting type.  By using these wrapper classes
	we can get/set all the various values required for their setting type and such values are
	written to the data file and re-instated when that data file is re-openned.  To that end, adding
	settings by instantiating this class directly is to be avoided.  Indeed the constructor is
	'protected' to avoid such.  It is safer to use the wrapper classes so that each piece of data
	is encoded into the Definition object in a consistent manner.
 */
class CamExport Definition
{
public:
	typedef enum
	{
		Metric = 0,
		Imperial
	} Units_t;

public:
	/**
		Define an enumeration for the types of settings we support.

		NOTE: If any values are added/removed from this enumeration then
		their corresponding ASCII equivalents (used via the Python interface)
		must also be updated.  Such values are defined within the
		PyDefinition_init() method of the PyTPGSettings.cpp file.
	 */
	typedef enum
	{
		SettingType_Text = 0,	// Values of this type are stored in TPGFeature::PropTPGSettings
		SettingType_Radio,		// Values of this type are stored in TPGFeature::PropTPGSettings
		SettingType_ObjectNamesForType,	// Object names whose types are included in the list of options.
		SettingType_SingleObjectNameForType,	// Object name whose type is defined within the list of options.
		SettingType_Enumeration,	// Produces a combo-box whose values include the verbose forms of the enumerated type.
		SettingType_Length,			// MUST have units of 'mm' or 'inch' for this to make sense.
		SettingType_Filename,
		SettingType_Directory,
		SettingType_Color,
		SettingType_Integer,
		SettingType_Double,
		SettingType_Rate
	} SettingType;

	friend QString & operator<< ( QString & qs, const SettingType &type )
	{
		switch (type)
		{
		case SettingType_Text:	qs += QString::fromAscii("TEXT");
			break;

		case SettingType_Radio:	qs += QString::fromAscii("RADIO");
			break;

		case SettingType_ObjectNamesForType:	qs += QString::fromAscii("OBJECT_NAMES_FOR_TYPE");
			break;

		case SettingType_SingleObjectNameForType:	qs += QString::fromAscii("SINGLE_OBJECT_NAME_FOR_TYPE");
			break;

		case SettingType_Enumeration:	qs += QString::fromAscii("ENUMERATION");
			break;

		case SettingType_Length:	qs += QString::fromAscii("LENGTH");
			break;

		case SettingType_Filename:	qs += QString::fromAscii("FILENAME");
			break;

		case SettingType_Directory:	qs += QString::fromAscii("DIRECTORY");
			break;

		case SettingType_Color:	qs += QString::fromAscii("COLOR");
			break;

		case SettingType_Integer:	qs += QString::fromAscii("INTEGER");
			break;

		case SettingType_Double:	qs += QString::fromAscii("DOUBLE");
			break;

		case SettingType_Rate:	qs += QString::fromAscii("RATE");
			break;

		default:
			break;
		}

		return(qs);
	}

	/**
		These mimic the QValidator state enumeration values.  We don't use them directly in this class
		because the QValidator class relies on the QT user interface libraries while the Definition class
		may be used in a non-user interface manner.  There is a routine that converts from one value
		to the other within the CamGUI project.
	 */
	typedef enum {
        Invalid,
        Intermediate,
        Acceptable
	} ValidationState;

protected:

	/// reference counter
    int refcnt;

    /// reference to the settings object that this setting belongs too
    TPGSettings *parent;

    /// the action that this setting belongs to
    QString action;

public:
    friend class TPGSettings;

	bool operator== ( const Definition & rhs ) const;
	

	//(<name>, <label>, <type>, <defaultvalue>, <units>, <helptext>)
	QString name;
	QString label;
	SettingType type;
	QString defaultvalue;
	QString units;
	QString helptext;
	bool	visible;		// The visible flag is used to display/hide the widgets that represent this setting in the settings editor dialog.
							// If its value is changed as part of an onChanged() method call, the settings editor dialog is updated to
							// reflect the new value.
	
	QList<Option*> options;

//	QString value; // deprecated: now uses FreeCAD data structure which is contained in TPGSettings.tpgFeature

	// Making the constructors protected to force the use of the wrapper classes rather than using this class directly.
public:
	Definition(const char *name, const char *label, const SettingType  type, const char *defaultvalue, const char *units, const char *helptext);
	Definition(QString name, QString label, SettingType type, QString defaultvalue, QString units, QString helptext);
	Definition();

public:
	Definition( Definition & rhs );
	Definition & operator= ( Definition & rhs );

	~Definition();

public:
	/**
		The wrapper classes may overload this method to provide their own type-specific validation.  If no special
		validation is required then this class's implementation simply returns 'ValidationState::Acceptable'.
	 */	
	virtual ValidationState validate(QString & input, int & position) const;

	/**
	 * Perform a deep copy of this class
	 */
	Definition* clone();

    void print() const;

    /**
     * Increases reference count
     * Note: it returns a pointer to this for convenience.
     */
    Definition *grab();

    /**
     * Decreases reference count and deletes self if no other references
     */
    void release();

    /// add an option for the value of this setting
    void addOption(QString id, QString label);
    /// add an option for the value of this setting
    void addOption(const char *id, const char *label);

    /// get the value associated with this setting
    QString getValue() const;

    /// set the value associated with this setting
    bool setValue(QString value);

    /// get the namespaced name <action>::<name>
    QString getFullname() const;

	/// Used by the TPGSetting::EvaluateWithPython() method.
	virtual bool AddToPythonDictionary(PyObject *dictionary, const QString requested_units, const QString prefix) const;

	/// Search for an option by the ID alone.  This method ignores case.
	Option *getOption(const QString id) const;

	/**
		This method is used within the AddToPythonDictionary() implementations.  It's included here so that each
		implementation produces a consistent variable name.
	 */	
	std::string PythonName(const QString prefix) const;

	friend QString & operator<< ( QString & qs, const Definition & definition )
	{
		std::ostringstream xml;
		xml << "<DEFINITION>\n"
			<< "\t<refcnt>" << definition.refcnt << "</refcnt>\n"
			<< "\t<action>" << definition.action.toStdString() << "</action>\n"
			<< "\t<name>" << definition.name.toStdString() << "</name>\n"
			<< "\t<label>" << definition.label.toStdString() << "</label>\n"
			<< "\t<type>"; { QString val; val << definition.type; xml << val.toStdString() << "</type>\n"; }
		xml << "\t<defaultvalue>" << definition.defaultvalue.toStdString() << "</defaultvalue>\n"
			<< "\t<helptext>" << definition.helptext.toStdString() << "</helptext>\n"
			<< "\t<visible>" << (char *) (definition.visible?"TRUE":"FALSE") << "</visible>\n"
			<< "\t<options NUM=\"" << definition.options.size() << "\">\n";
		
			for (QList<Option *>::const_iterator itOption = definition.options.begin(); itOption != definition.options.end(); itOption++)
			{
				QString option_description;
				option_description << **itOption;

				xml << option_description.toAscii().constData();
			}
		xml << "\t</options>\n"
			<< "</DEFINITION>\n";
		qs += QString::fromStdString(xml.str());
		return(qs);
	}
};

/** 
	Class stores hash of settings for managing each independant TPG

	This class mostly just holds a map of Definition objects.
	The Definition objects hold data 'about' the setting but
	the value itself is stored in one of the member variables contained
	within the owning TPGFeature object.
 */
class CamExport TPGSettings
{
public:
    TPGSettings();
    ~TPGSettings();

    void initialise() {};
    void loadSettings() {};

	/**
		called when any one of the settings contained within the TPGFeature::PropTPGSettings 
		member variable changes.  These are called from the TPGFeature::onBeforeChange() and
		TPGFeature::onChanged() methods respectively.
	 */
	void onBeforePropTPGSettingsChange(const App::PropertyMap* prop);
	void onPropTPGSettingsChanged(const App::PropertyMap* prop);

	/**
	 * Perform a deep copy of this class
	 */
    TPGSettings* clone();

    /**
     * Add a setting to this Settings collection
     */
    Definition* addSettingDefinition(QString action, Definition* setting);

    /**
     * Get the currently selected action.
     */
    const QString getAction() const;

    /**
     * Change the currently selected action.
     */
    bool setAction(QString action);

    /**
     * Get the value of a given setting (by name)
     */
    const QString getValue(QString name) const;

    /**
     * Get the value of a given setting (by name)
     */
    const QString getValue(QString action, QString name) const;

    /**
     * Print the settings to stdout
     */
    void print() const;

    std::vector<Definition*> getSettings() const;

    /**
     * Sets the default value for every setting in Settings collection
     */
    void addDefaults();

    /**
     * Get the action names defined
     */
    QStringList getActions() const;

    /**
     * Set the value for the named setting
     */
    bool setValue(QString action, QString name, QString value);

    /**
     * Set the value for the named setting
     */
    bool setValue(QString name, QString value);

    /**
     * Sets the TPGFeature that the value will be saved-to/read-from.
     */
    void setTPGFeature(Cam::TPGFeature *tpgFeature);

    /**
     * Increases reference count
     * Note: it returns a pointer to this for convenience.
     */
    TPGSettings *grab();

    /**
     * Decreases reference count and deletes self if no other references
     */
    void release();

	/**
		These Python methods appear here so that each value entered by the operator may refer
		to variables that represent the values of other settings in this class.  eg: the
		operator may enter 'Diameter / 2' when defining a 'Peck Depth' setting's value.  These
		methods allow such constructs.
	 */
	bool EvaluateLength( const Definition *definition, const char *entered_value, double *pResult ) const;
	bool EvaluateWithPython( const Definition *definition, QString value, QString & evaluated_version ) const;
	bool AddToPythonDictionary(PyObject *dictionary, const QString requested_units, const QString prefix) const;

	Settings::Definition *getDefinition(const QString action, const QString name) const;

	// These methods find a setting by name and return a pointer to that object's wrapper
	// class based on the setting's type.  If the name doesn't match the type requested then
	// a NULL pointer is returned.
	Settings::Text		*asText(const QString action, const QString name) const;
	Settings::Radio		*asRadio(const QString action, const QString name) const;
	Settings::Color		*asColor(const QString action, const QString name) const;
	Settings::ObjectNamesForType	*asObjectNamesForType(const QString action, const QString name) const;
	Settings::SingleObjectNameForType	*asSingleObjectNameForType(const QString action, const QString name) const;
	Settings::Enumeration *asEnumeration(const QString action, const QString name) const;
	Settings::Length	*asLength(const QString action, const QString name) const;
	Settings::Filename	*asFilename(const QString action, const QString name) const;
	Settings::Directory	*asDirectory(const QString action, const QString name) const;
	Settings::Integer	*asInteger(const QString action, const QString name) const;
	Settings::Double	*asDouble(const QString action, const QString name) const;
	Settings::Rate		*asRate(const QString action, const QString name) const;

protected:

    /// the action that is selected (i.e. algorithm and group of settings to use)
    QString action;
    /// the settings in the order that the TPG developer wants them to display
    std::vector<Definition*> settingDefs;
    /// the same settings but in map data structure for random access (key: <action>::<name>)
    std::map<QString, Definition*> settingDefsMap;
    /// a map to store which settings are in which action
    std::map<QString, std::vector<Definition*> > settingDefsActionMap;
    /// reference counter
    int refcnt;
    /// the tpgFeature to which these settings belong
	Cam::TPGFeature *tpgFeature;

    /// make a namespaced name (from <action>::<name>)
    QString makeName(QString action, QString name) const;

private:
	// NOTE: ONLY used to determine which properties changed in a single update.  i.e. this
	// value is quite transient.  It only makes sense when comparing the values written
	// between the onBeforePropTPGSettingsChange() and onPropTPGSettingsChanged() method
	// calls.
	std::map<std::string,std::string>	previous_tpg_properties_version;

public:
	friend QString & operator<< ( QString & qs, const TPGSettings & settings )
	{
		std::ostringstream xml;
		xml << "<SETTINGS>\n"
			<< "\t<DEFINITIONS>\n";

	    std::vector<Definition*> definitions = settings.getSettings();
		for (std::vector<Definition *>::const_iterator itDefinition = definitions.begin(); itDefinition != definitions.end(); itDefinition++)
		{
			QString def;
			def << **itDefinition;
			xml << def.toStdString();
		}
		xml << "\t</DEFINITIONS>\n"
			<< "</SETTINGS>\n";
		qs += QString::fromStdString(xml.str());
		return(qs);
	}
};



// Declare some classes used to get/set Definition values depending on the
// type of Definition object used.  Consider these 'wrapper classes' that
// will allow a Cam::Setting::Definition to be correctly set and interpreted
// when the various types of settings are defined.

/**
	The Color setting requires four integers to be retained within the data
	file.  To this end it uses the boost::property_tree class to encode
	all such values into a single string which is stored in the
	PropTPGSettings map of the owning TPGFeature object.  This wrapper class
	supports the encoding/decoding mechanisms required for this to occur.

	No validation method is provided as the QColorPicker class ensures
	the values are always valid.  i.e. the user doesn't get a chance to
	change the values to somethat that can't be used.  This means we end up
	using the Definition::validate() method which always returns Acceptable.
 */
class CamExport Color : public Definition
{
public:
	typedef int Red_t;
	typedef int Green_t;
	typedef int Blue_t;
	typedef int Alpha_t;

	// NOTE: These offset values MUST align with the Encode_t definition.
	static const ::size_t redOffset = 0;
	static const ::size_t greenOffset = 1;
	static const ::size_t blueOffset = 2;
	static const ::size_t alphaOffset = 3;

	typedef boost::tuple<Red_t, Green_t, Blue_t, Alpha_t> Encode_t;

	QString encode(const Encode_t data) const;
	Encode_t decode() const;

public:
	Color(const char *name, const char *label, const char *helptext ):
	  Definition(name, label, SettingType_Color, "", "", helptext)
	{
	}

	bool get(int &red, int &green, int &blue, int &alpha) const;
	void set(const int red, const int green, const int blue, const int alpha);
	virtual bool AddToPythonDictionary(PyObject *dictionary, const QString requested_units, const QString prefix) const;
};


/**
	The Length setting requires both the 'double' value and the 'units'
	to be retained within the datafile.  To this end it uses the
	boost::property_tree class to encode
	all such values into a single string which is stored in the
	PropTPGSettings map of the owning TPGFeature object.  This wrapper class
	supports the encoding/decoding mechanisms required for this to occur.

	This class allows the value to be interpreted as a Python script so that
	its value may be based on the values of other settings.  It also means
	the operator may enter keywords describing the units so that any conversions
	may occur as necessary.  eg: The operator may enter '1/8 inch' when
	this setting's units are 'mm'.  In this case the value 3.175 will be
	used.
 */
class CamExport Length : public Definition
{
public:
	typedef double Value_t;
	// Definition::Units_t
	
	// NOTE: These offset values MUST align with the Encode_t definition.
	static const ::size_t valueOffset = 0;
	static const ::size_t unitsOffset = 1;
	
	typedef boost::tuple<Value_t, Definition::Units_t> Encode_t;

	QString encode(const Encode_t data) const;
	Encode_t decode() const;

public:
	Length(	const char *name, 
			const char *label, 
			const char *helptext,
			const double default_value,
			const double minimum, 
			const double maximum, 
			const Definition::Units_t units );

	Length(	const char *name, 
			const char *label, 
			const char *helptext,
			const double default_value,
			const Definition::Units_t units );

	bool Evaluate( const char *entered_value, double *pResult ) const;
	virtual ValidationState validate(QString & input,int & position) const;

	double Minimum() const;
	void Minimum(const double value);

	double Maximum() const;
	void Maximum(const double value);
	virtual bool AddToPythonDictionary(PyObject *dictionary, const QString requested_units, const QString prefix) const;

	Definition::Units_t getUnits() const;
	void setUnits(const Definition::Units_t class_of_units);

	double get(const Definition::Units_t requested_units) const;
	void   set(const double value);
	void   set(const double value, const Settings::Definition::Units_t units);
};


/**
	The Rate setting requires both the 'double' value and the 'units'
	to be retained within the datafile.  To this end it uses the
	boost::property_tree class to encode
	all such values into a single string which is stored in the
	PropTPGSettings map of the owning TPGFeature object.  This wrapper class
	supports the encoding/decoding mechanisms required for this to occur.

	This class allows the value to be interpreted as a Python script so that
	its value may be based on the values of other settings.
 */
class CamExport Rate : public Definition
{
public:
	typedef double Value_t;
	// Definition::Units_t

	// NOTE: These offset values MUST align with the Encode_t definition.
	static const ::size_t valueOffset = 0;
	static const ::size_t unitsOffset = 1;

	typedef boost::tuple<Value_t, Definition::Units_t> Encode_t;

	QString encode(const Encode_t data) const;
	Encode_t decode() const;

public:
	Rate(	const char *name, 
			const char *label, 
			const char *helptext,
			const double default_value,
			const double minimum, 
			const double maximum, 
			const Definition::Units_t units );
	Rate(	const char *name, 
			const char *label, 
			const char *helptext,
			const double default_value,
			const Definition::Units_t units );

	bool Evaluate( const char *entered_value, double *pResult ) const;
	virtual ValidationState validate(QString & input,int & position) const;

	double Minimum() const;
	void Minimum(const double value);

	double Maximum() const;
	void Maximum(const double value);
	virtual bool AddToPythonDictionary(PyObject *dictionary, const QString requested_units, const QString prefix) const;

	Definition::Units_t getUnits() const;
	void setUnits(const Definition::Units_t class_of_units);

	double get(const Definition::Units_t requested_units) const;
	void   set(const double value);
	void   set(const double value, const Settings::Definition::Units_t units);
};

/**
	The Double setting is similar to the Length setting except that
	the 'units' field is free-format.

	This class allows the value to be interpreted as a Python script so that
	its value may be based on the values of other settings.
 */
class CamExport Double : public Definition
{
public:
	Double(	const char *name, 
								const char *label, 
								const char *helptext,
								const double default_value,
								const double minimum, 
								const double maximum, 
								const char *units );
	Double(	const char *name, 
								const char *label, 
								const char *helptext,
								const double default_value,
								const char *units );

	bool Evaluate( const char *entered_value, double *pResult ) const;
	virtual ValidationState validate(QString & input,int & position) const;

	double Minimum() const;
	void Minimum(const double value);

	double Maximum() const;
	void Maximum(const double value);
	virtual bool AddToPythonDictionary(PyObject *dictionary, const QString requested_units, const QString prefix) const;

	double get() const;
	void   set(const double value);
};

/**
	NOTE: The SettingType_ObjectNamesForType setting type assumes that
	the Options list has been populated with the following entries;
		- id = "Delimiters" (as a keyword) and label = <all characters that delimit separate values in the setting>
		- id = "TypeId" (the keyword) and label = <a single class type ID as would be used within the Base::Type::fromName() method>

	There must be only one option with an option.id="Delimiters" but there may be many options with an id="TypeId".
 */
class CamExport ObjectNamesForType : public Definition
{
public:
	ObjectNamesForType(	const char *name, 
								const char *label, 
								const char *helptext,
								const char *delimiters,
								const char *object_type );

	void Add(const char * object_type);
	void SetDelimiters(const char * object_type);
	virtual ValidationState validate(QString & input,int & position) const;

	QStringList GetTypes() const;
	QStringList GetNames() const;
	virtual bool AddToPythonDictionary(PyObject *dictionary, const QString requested_units, const QString prefix) const;
};

class CamExport SingleObjectNameForType : public Definition
{
public:
	SingleObjectNameForType(	const char *name, 
								const char *label, 
								const char *helptext,
								const char *object_type );

	void Add(const char * object_type);
	virtual ValidationState validate(QString & input,int & position) const;

	QString GetType() const;
	QString GetName() const;
	virtual bool AddToPythonDictionary(PyObject *dictionary, const QString requested_units, const QString prefix) const;
};

class CamExport Text : public Definition
{
public:
	Text(const char *name, const char *label, const char *defaultvalue, const char *units, const char *helptext) :
			Definition(name, label, SettingType_Text, defaultvalue, units, helptext)
			{
			}
	virtual bool AddToPythonDictionary(PyObject *dictionary, const QString requested_units, const QString prefix) const;
};

class CamExport Radio : public Definition
{
public:
	Radio(const char *name, const char *label, const char *defaultvalue, const char *helptext) :
			Definition(name, label, SettingType_Radio, defaultvalue, "", helptext)
			{
			}
	virtual bool AddToPythonDictionary(PyObject *dictionary, const QString requested_units, const QString prefix) const;

	void Add(const char *value);
};

class CamExport Integer : public Definition
{
public:
	Integer(const char *name, const char *label, const int defaultvalue, const char *units, const char *helptext) :
			Definition(name, label, SettingType_Integer, "", units, helptext)
			{
				std::ostringstream def;
				def << defaultvalue;
				this->defaultvalue = QString::fromStdString(def.str());
			}

	bool Evaluate( const char *entered_value, int *pResult ) const;
	virtual ValidationState validate(QString & input,int & position) const;
	virtual bool AddToPythonDictionary(PyObject *dictionary, const QString requested_units, const QString prefix) const;

	int Minimum() const;
	void Minimum(const int value);

	int Maximum() const;
	void Maximum(const int value);

	int get() const;
	void set(const int value);
};



class CamExport Filename : public Definition
{
public:
	Filename(const char *name, const char *label, const char * defaultvalue, const char *units, const char *helptext) :
			Definition(name, label, SettingType_Filename, defaultvalue, units, helptext)
			{
			}
	virtual bool AddToPythonDictionary(PyObject *dictionary, const QString requested_units, const QString prefix) const;
};


class CamExport Directory : public Definition
{
public:
	Directory(const char *name, const char *label, const char * defaultvalue, const char *units, const char *helptext) :
			Definition(name, label, SettingType_Directory, defaultvalue, units, helptext)
			{
			}
	virtual bool AddToPythonDictionary(PyObject *dictionary, const QString requested_units, const QString prefix) const;
};


/**
	For SettingType_Enumeration, the 'value' is always going to be the integer form.  When it's presented
	in the QComboBox (i.e. the user interface) the verbose (string) form is always used but, once that interaction
	is complete, the value written to the Definition object will always be the integer form.
	To this end, settings of this type must have options whose ID is the integer form and whose LABEL is the string form.
	Since Python is all string-based it might make sense to just use the string form for the value too.  The problem with
	this is that it precludes language changes.  If we use the integer form for the value then the files should carry
	between languages better.
 */
class CamExport Enumeration : public Definition
{
public:
	Enumeration(const char *name, const char *label, const int defaultvalue, const char *units, const char *helptext) :
			Definition(name, label, SettingType_Enumeration, "", units, helptext)
			{
				std::ostringstream ossDefault;
				ossDefault << defaultvalue;
				this->defaultvalue = QString::fromStdString(ossDefault.str());
			}

	typedef std::pair<int, QString> Pair_t;
	typedef std::map<int, QString> Map_t;

	Map_t Values() const;
	void Add(const int id, const QString label);
	virtual bool AddToPythonDictionary(PyObject *dictionary, const QString requested_units, const QString prefix) const;

	Pair_t get() const;
	bool set(const int id);
};

// Convert between the class and the QString version of units.
inline QString & operator<< ( QString & verbose, const Definition::Units_t class_of_units )
{
	switch (class_of_units)
	{
	case Definition::Imperial:
		verbose = QString::fromAscii("inch");
		return(verbose);

	case Definition::Metric:
	default:
		verbose = QString::fromAscii("mm");
		return(verbose);
	}
}


} // namespace Settings
} //namespace Cam


#endif //CAM_TPGSETTINGS_H
