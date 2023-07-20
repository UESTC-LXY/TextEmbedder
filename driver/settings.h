#pragma once

// settings.h
// 4/1/2014 jichi

#include "sakurakit/skglobal.h" 
#include<functional>
#include<string>
class SettingsPrivate;
// Root object for all qobject
class Settings  
{ 
  SK_EXTEND_CLASS(Settings, void)
  SK_DECLARE_PRIVATE(SettingsPrivate)
public:
  static Self *instance();

  explicit Settings(void *parent = nullptr);
  ~Settings();

public:
  bool isEmbedDriverNeeded() const;
  bool isWindowDriverNeeded() const;

  bool isEmbeddedTextNeeded() const; // Whether send embedded text; placeholder that always returns true

  bool isWindowTranslationEnabled() const; // Whether translate text
  bool isWindowTranscodingEnabled() const; // Whether fix text encoding
  bool isWindowTextVisible() const; // Whether display text after the translation
  bool isEmbeddedScenarioVisible() const; // Whether disable scenario text
  bool isEmbeddedScenarioTextVisible() const; // Whether display both scenario text and translation
  bool isEmbeddedScenarioTranslationEnabled() const; // Whether translate scenario text
  bool isEmbeddedScenarioTranscodingEnabled() const; // Whether translate scenario text
  bool isEmbeddedNameVisible() const;
  bool isEmbeddedNameTextVisible() const;
  bool isEmbeddedNameTranslationEnabled() const;
  bool isEmbeddedNameTranscodingEnabled() const;
  bool isEmbeddedOtherVisible() const;
  bool isEmbeddedOtherTextVisible() const;
  bool isEmbeddedOtherTranslationEnabled() const;
  bool isEmbeddedOtherTranscodingEnabled() const;

  bool isEmbeddedSpaceAlwaysInserted() const;
  bool isEmbeddedSpaceSmartInserted() const;
  std::wstring embeddedSpacePolicyEncoding() const;

  bool isEmbeddedAllTextsExtracted() const;
  bool isEmbeddedTextEnabled() const;
  //bool isEmbeddedTextCancellableByControl() const;
  int embeddedTranslationWaitTime() const;

  std::wstring embeddedFontFamily() const;
  float embeddedFontScale() const;
  int embeddedFontWeight() const;
  int embeddedFontCharSet() const;
  bool isEmbeddedFontCharSetEnabled() const;

  std::wstring gameEncoding() const;

  int embeddedScenarioWidth() const;

  long scenarioSignature() const;
  long nameSignature() const;

public :
  void load(const std::wstring&json);
  void disable();

  void setWindowTranslationEnabled(bool t);
  void setWindowTranscodingEnabled(bool t);
  void setWindowTextVisible(bool t);
  void setEmbeddedScenarioVisible(bool t);
  void setEmbeddedScenarioTextVisible(bool t);
  void setEmbeddedScenarioTranslationEnabled(bool t);
  void setEmbeddedScenarioTranscodingEnabled(bool t);
  void setEmbeddedNameVisible(bool t);
  void setEmbeddedNameTextVisible(bool t);
  void setEmbeddedNameTranslationEnabled(bool t);
  void setEmbeddedNameTranscodingEnabled(bool t);
  void setEmbeddedOtherVisible(bool t);
  void setEmbeddedOtherTextVisible(bool t);
  void setEmbeddedOtherTranslationEnabled(bool t);
  void setEmbeddedOtherTranscodingEnabled(bool t);

  void setEmbeddedSpaceAlwaysInserted(bool t);
  void setEmbeddedSpaceSmartInserted(bool t);
  void setEmbeddedSpacePolicyEncoding(const std::wstring&v);

  void setEmbeddedAllTextsExtracted(bool t);
  void setEmbeddedTextEnabled(bool t);
  //void setEmbeddedTextCancellableByControl(bool t);
  void setEmbeddedTranslationWaitTime(int v);

  void setEmbeddedScenarioWidth(int v);

  void setEmbeddedFontFamily(const std::wstring&v);
  void setEmbeddedFontCharSetEnabled(bool t);
  void setEmbeddedFontCharSet(int v);
  void setEmbeddedFontWeight(int v);
  void setEmbeddedFontScale(float v);

  void setGameEncoding(const std::wstring&v);

  void setScenarioSignature(long v);
  void setNameSignature(long v);

  std::function<void(bool)>windowTranslationEnabledChanged;
  std::function<void(bool)>windowTranscodingEnabledChanged;
  std::function<void(bool)>windowTextVisibleChanged;
  std::function<void(bool)>embeddedScenarioVisibleChanged;
  std::function<void(bool)>embeddedScenarioTextVisibleChanged;
  std::function<void(bool)>embeddedScenarioTranslationEnabledChanged;
  std::function<void(bool)>embeddedScenarioTranscodingEnabledChanged;
  std::function<void(bool)>embeddedNameVisibleChanged;
  std::function<void(bool)>embeddedNameTextVisibleChanged;
  std::function<void(bool)>embeddedNameTranslationEnabledChanged;
  std::function<void(bool)>embeddedNameTranscodingEnabledChanged;
  std::function<void(bool)>embeddedOtherVisibleChanged;
  std::function<void(bool)>embeddedOtherTextVisibleChanged;
  std::function<void(bool)>embeddedOtherTranslationEnabledChanged;
  std::function<void(bool)>embeddedOtherTranscodingEnabledChanged;

  std::function<void(bool)>embeddedSpaceAlwaysInsertedChanged;
  std::function<void(bool)>embeddedSpaceSmartInsertedChanged;

  std::function<void(bool)>embeddedAllTextsExtractedChanged;
  std::function<void(bool)>embeddedTextEnabledChanged;

  std::function<void(bool)>embeddedFontCharSetEnabledChanged;

  std::function<void(int)>embeddedScenarioWidthChanged;

  std::function<void(std::wstring)>embeddedSpacePolicyEncodingChanged;

  std::function<void(int)>embeddedTranslationWaitTimeChanged;

  std::function<void(std::wstring)>embeddedFontFamilyChanged;
  std::function<void(int)>embeddedFontCharSetChanged;
  std::function<void(int)>embeddedFontWeightChanged;
  std::function<void(float)>embeddedFontScaleChanged;

  std::function<void(std::wstring)>gameEncodingChanged;
  std::function<void(long)>scenarioSignatureChanged;
  std::function<void(long)>nameSignatureChanged;
  std::function<void()>loadFinished;
 
};

// EOF
