// settings.cc
// 5/1/2014 jichi

#include "driver/settings.h"
#include<string>
//#define DEBUG "settings"
#include "sakurakit/skdebug.h"
#include<string>
#include<Windows.h>
/** Private class */

class SettingsPrivate
{
public:
	bool windowTranslationEnabled
		, windowTranscodingEnabled
		, windowTextVisible
		, embeddedScenarioVisible
		, embeddedScenarioTextVisible
		, embeddedScenarioTranslationEnabled
		, embeddedScenarioTranscodingEnabled
		, embeddedNameVisible
		, embeddedNameTextVisible
		, embeddedNameTranslationEnabled
		, embeddedNameTranscodingEnabled
		, embeddedOtherVisible
		, embeddedOtherTextVisible
		, embeddedOtherTranslationEnabled
		, embeddedOtherTranscodingEnabled
		, embeddedSpaceAlwaysInserted
		, embeddedSpaceSmartInserted
		, embeddedTextEnabled
		//, embeddedTextCancellableByControl
		, embeddedAllTextsExtracted
		;

	bool embeddedFontCharSetEnabled;
	int embeddedFontCharSet;

	int embeddedFontWeight;
	float embeddedFontScale;

	std::wstring embeddedFontFamily;
	std::wstring embeddedSpacePolicyEncoding;
	std::wstring gameEncoding;

	int embeddedTranslationWaitTime;

	int embeddedScenarioWidth;

	long scenarioSignature,
		nameSignature;

	SettingsPrivate()
		: windowTranslationEnabled(false)
		, windowTranscodingEnabled(false)
		, windowTextVisible(false)
		, embeddedScenarioVisible(false)
		, embeddedScenarioTextVisible(false)
		, embeddedScenarioTranslationEnabled(false)
		, embeddedScenarioTranscodingEnabled(false)
		, embeddedNameVisible(false)
		, embeddedNameTextVisible(false)
		, embeddedNameTranslationEnabled(false)
		, embeddedNameTranscodingEnabled(false)
		, embeddedOtherVisible(false)
		, embeddedOtherTextVisible(false)
		, embeddedOtherTranslationEnabled(false)
		, embeddedOtherTranscodingEnabled(false)
		, embeddedSpaceAlwaysInserted(false)
		, embeddedSpaceSmartInserted(false)
		, embeddedTextEnabled(false)
		//, embeddedTextCancellableByControl(false)
		, embeddedAllTextsExtracted(false)
		, embeddedFontCharSetEnabled(false)
		, embeddedFontCharSet(0)
		, embeddedFontWeight(0)
		, embeddedFontScale(0)
		, gameEncoding(L"shift-jis")
		, embeddedTranslationWaitTime(1000) // 1 second
		, embeddedScenarioWidth(0)
		, scenarioSignature(0)
		, nameSignature(0)
	{}
};

/** Public class */

static Settings* instance_;
Settings* Settings::instance() { return ::instance_; }

Settings::Settings(void* parent)
	:   d_(new D)
{
	instance_ = this;
}

Settings::~Settings()
{
	instance_ = nullptr;
	delete d_;
}

#define DEFINE_PROPERTY(property, getter, setter, rettype, argtype) \
  rettype Settings::getter() const \
  { return d_->property; } \
  void Settings::setter(argtype value)  \
  { if (d_->property != value) { d_->property = value; try{property##Changed(value);} catch (std::bad_function_call) {}} }

#define DEFINE_BOOL_PROPERTY(property, getter, setter)      DEFINE_PROPERTY(property, getter, setter, bool, bool)
#define DEFINE_INT_PROPERTY(property, getter, setter)       DEFINE_PROPERTY(property, getter, setter, int, int)
#define DEFINE_LONG_PROPERTY(property, getter, setter)      DEFINE_PROPERTY(property, getter, setter, long, long)
#define DEFINE_FLOAT_PROPERTY(property, getter, setter)     DEFINE_PROPERTY(property, getter, setter, float, float)
#define DEFINE_STRING_PROPERTY(property, getter, setter)    DEFINE_PROPERTY(property, getter, setter, std::wstring , const std::wstring &)

DEFINE_BOOL_PROPERTY(windowTranslationEnabled, isWindowTranslationEnabled, setWindowTranslationEnabled)
DEFINE_BOOL_PROPERTY(windowTranscodingEnabled, isWindowTranscodingEnabled, setWindowTranscodingEnabled)
DEFINE_BOOL_PROPERTY(windowTextVisible, isWindowTextVisible, setWindowTextVisible)
DEFINE_BOOL_PROPERTY(embeddedScenarioVisible, isEmbeddedScenarioVisible, setEmbeddedScenarioVisible)
DEFINE_BOOL_PROPERTY(embeddedScenarioTextVisible, isEmbeddedScenarioTextVisible, setEmbeddedScenarioTextVisible)
DEFINE_BOOL_PROPERTY(embeddedScenarioTranslationEnabled, isEmbeddedScenarioTranslationEnabled, setEmbeddedScenarioTranslationEnabled)
DEFINE_BOOL_PROPERTY(embeddedScenarioTranscodingEnabled, isEmbeddedScenarioTranscodingEnabled, setEmbeddedScenarioTranscodingEnabled)
DEFINE_BOOL_PROPERTY(embeddedNameVisible, isEmbeddedNameVisible, setEmbeddedNameVisible)
DEFINE_BOOL_PROPERTY(embeddedNameTextVisible, isEmbeddedNameTextVisible, setEmbeddedNameTextVisible)
DEFINE_BOOL_PROPERTY(embeddedNameTranslationEnabled, isEmbeddedNameTranslationEnabled, setEmbeddedNameTranslationEnabled)
DEFINE_BOOL_PROPERTY(embeddedNameTranscodingEnabled, isEmbeddedNameTranscodingEnabled, setEmbeddedNameTranscodingEnabled)
DEFINE_BOOL_PROPERTY(embeddedOtherVisible, isEmbeddedOtherVisible, setEmbeddedOtherVisible)
DEFINE_BOOL_PROPERTY(embeddedOtherTextVisible, isEmbeddedOtherTextVisible, setEmbeddedOtherTextVisible)
DEFINE_BOOL_PROPERTY(embeddedOtherTranslationEnabled, isEmbeddedOtherTranslationEnabled, setEmbeddedOtherTranslationEnabled)
DEFINE_BOOL_PROPERTY(embeddedOtherTranscodingEnabled, isEmbeddedOtherTranscodingEnabled, setEmbeddedOtherTranscodingEnabled)

DEFINE_BOOL_PROPERTY(embeddedSpaceAlwaysInserted, isEmbeddedSpaceAlwaysInserted, setEmbeddedSpaceAlwaysInserted)
DEFINE_BOOL_PROPERTY(embeddedSpaceSmartInserted, isEmbeddedSpaceSmartInserted, setEmbeddedSpaceSmartInserted)
DEFINE_STRING_PROPERTY(embeddedSpacePolicyEncoding, embeddedSpacePolicyEncoding, setEmbeddedSpacePolicyEncoding)

DEFINE_BOOL_PROPERTY(embeddedTextEnabled, isEmbeddedTextEnabled, setEmbeddedTextEnabled)
//DEFINE_BOOL_PROPERTY(embeddedTextCancellableByControl, isEmbeddedTextCancellableByControl, setEmbeddedTextCancellableByControl)
DEFINE_BOOL_PROPERTY(embeddedAllTextsExtracted, isEmbeddedAllTextsExtracted, setEmbeddedAllTextsExtracted)

DEFINE_INT_PROPERTY(embeddedTranslationWaitTime, embeddedTranslationWaitTime, setEmbeddedTranslationWaitTime)

DEFINE_INT_PROPERTY(embeddedScenarioWidth, embeddedScenarioWidth, setEmbeddedScenarioWidth)

DEFINE_STRING_PROPERTY(embeddedFontFamily, embeddedFontFamily, setEmbeddedFontFamily)
DEFINE_BOOL_PROPERTY(embeddedFontCharSetEnabled, isEmbeddedFontCharSetEnabled, setEmbeddedFontCharSetEnabled)
DEFINE_INT_PROPERTY(embeddedFontCharSet, embeddedFontCharSet, setEmbeddedFontCharSet)

DEFINE_INT_PROPERTY(embeddedFontWeight, embeddedFontWeight, setEmbeddedFontWeight)
DEFINE_FLOAT_PROPERTY(embeddedFontScale, embeddedFontScale, setEmbeddedFontScale)

DEFINE_STRING_PROPERTY(gameEncoding, gameEncoding, setGameEncoding)

DEFINE_LONG_PROPERTY(scenarioSignature, scenarioSignature, setScenarioSignature)
DEFINE_LONG_PROPERTY(nameSignature, nameSignature, setNameSignature)

// Groupped settings

void Settings::disable()
{
	setWindowTranslationEnabled(false);
	setWindowTextVisible(false);

	setEmbeddedTextEnabled(false);
}

bool Settings::isWindowDriverNeeded() const
{
	return isWindowTranslationEnabled() || isWindowTranscodingEnabled();
}

bool Settings::isEmbeddedTextNeeded() const { return true; } // placeholder

bool Settings::isEmbedDriverNeeded() const
{
	return true; // always embedding text
	//return isEmbeddedTextNeeded()
	//    || isEmbeddedScenarioTranslationEnabled() || !isEmbeddedScenarioVisible()
	//    || isEmbeddedNameTranslationEnabled() || !isEmbeddedNameVisible()
	//    || isEmbeddedOtherTranslationEnabled() || !isEmbeddedOtherVisible();
}

// Marshal

//todo change this  
#include"nlohmann/json.hpp"
#include<Windows.h>
#include"stringutil.h"
void Settings::load(const std::wstring& json)
{
	enum {
		H_debug = 6994359 // "debug"
		, H_gameEncoding = 156622791
		, H_embeddedFontFamily = 112965145
		, H_embeddedFontCharSet = 235985668
		, H_embeddedFontCharSetEnabled = 173862964
		, H_embeddedFontScale = 25063557
		, H_embeddedFontWeight = 128219092
		, H_embeddedScenarioWidth = 28091752
		, H_embeddedTranslationWaitTime = 245002357
		, H_embeddedTextEnabled = 261153908
		//, H_embeddedTextCancellableByControl = 96153884
		, H_embeddedAllTextsExtracted = 227821172
		, H_scenarioSignature = 246832709
		, H_nameSignature = 122678949

		, H_windowTranslationEnabled = 79059828
		, H_windowTranscodingEnabled = 219567700
		, H_windowTextVisibleChange = 23360709
		, H_embeddedScenarioVisible = 207043173
		, H_embeddedScenarioTextVisible = 241097605
		, H_embeddedScenarioTranslationEnabled = 132391348
		, H_embeddedScenarioTranscodingEnabled = 105135476
		, H_embeddedNameVisible = 180590501
		, H_embeddedNameTextVisible = 60027589
		, H_embeddedNameTranslationEnabled = 239147220
		, H_embeddedNameTranscodingEnabled = 266409492
		, H_embeddedOtherVisible = 32685349
		, H_embeddedOtherTextVisible = 151359621
		, H_embeddedOtherTranslationEnabled = 9290068
		, H_embeddedOtherTranscodingEnabled = 19782804
		, H_embeddedSpaceAlwaysInserted = 241397364
		, H_embeddedSpaceSmartInserted = 187266164
		, H_embeddedSpacePolicyEncoding = 213320263
	};
	
	std::string js = WideStringToString(json, CP_UTF8);
	nlohmann::json map = nlohmann::json::parse(js);
	 
	for (auto it = map.begin(); it != map.end(); ++it) {
		auto  value = it.value(); 
		auto key = it.key(); 
		
		if (key == "windowTranslationEnabled") setWindowTranslationEnabled(value);
		else if (key == "windowTranscodingEnabled") setWindowTranscodingEnabled(value);
		else if (key == "windowTextVisibleChange") setWindowTextVisible(value);
		else if (key == "embeddedScenarioVisible") setEmbeddedScenarioVisible(value);
		else if (key == "embeddedScenarioTextVisible") setEmbeddedScenarioTextVisible(value);
		else if (key == "embeddedScenarioTranslationEnabled") setEmbeddedScenarioTranslationEnabled(value);
		else if (key == "embeddedScenarioTranscodingEnabled") setEmbeddedScenarioTranscodingEnabled(value);
		else if (key == "embeddedNameVisible") setEmbeddedNameVisible(value);
		else if (key == "embeddedNameTextVisible") setEmbeddedNameTextVisible(value);
		else if (key == "embeddedNameTranslationEnabled") setEmbeddedNameTranslationEnabled(value);
		else if (key == "embeddedNameTranscodingEnabled") setEmbeddedNameTranscodingEnabled(value);
		else if (key == "embeddedOtherVisible") setEmbeddedOtherVisible(value);
		else if (key == "embeddedOtherTextVisible") setEmbeddedOtherTextVisible(value);
		else if (key == "embeddedOtherTranslationEnabled") setEmbeddedOtherTranslationEnabled(value);
		else if (key == "embeddedOtherTranscodingEnabled") setEmbeddedOtherTranscodingEnabled(value);

		else if (key == "embeddedSpaceAlwaysInserted") setEmbeddedSpaceAlwaysInserted(value);
		else if (key == "embeddedSpaceSmartInserted") setEmbeddedSpaceSmartInserted(value);
		else if (key == "embeddedSpacePolicyEncoding") setEmbeddedSpacePolicyEncoding(StringToWideString(value,CP_UTF8));

		else if (key == "embeddedTextEnabled") setEmbeddedTextEnabled(value);
		//else if(key == "embeddedTextCancellableByControl") setEmbeddedTextCancellableByControl(value);
		else if (key == "embeddedAllTextsExtracted") setEmbeddedAllTextsExtracted(value);

		else if (key == "embeddedScenarioWidth") setEmbeddedScenarioWidth(value );
		else if (key == "embeddedTranslationWaitTime") setEmbeddedTranslationWaitTime(value );

		else if (key == "scenarioSignature") setScenarioSignature(value );
		else if (key == "nameSignature") setNameSignature(value );

		else if (key == "embeddedFontFamily") setEmbeddedFontFamily(StringToWideString(value, CP_UTF8));
		else if (key == "embeddedFontCharSet") setEmbeddedFontCharSet(value );
		else if (key == "embeddedFontCharSetEnabled") setEmbeddedFontCharSetEnabled(value);
		else if (key == "embeddedFontWeight") setEmbeddedFontWeight(value);
		else if (key == "embeddedFontScale") setEmbeddedFontScale(value );
		else if (key == "gameEncoding") setGameEncoding((StringToWideString(value, CP_UTF8)));
		 
		else DOUT("warning: unknown key:" << it.key());
	}
	loadFinished();
}

// EOF
