#pragma once

// embeddriver.h
// 4/26/2014 jichi

#include "sakurakit/skglobal.h" 
#include<string>
#include<functional>
class EmbedDriverPrivate;
class EmbedDriver  
{ 
  SK_EXTEND_CLASS(EmbedDriver, void)
  SK_DECLARE_PRIVATE(EmbedDriverPrivate)

public:
  explicit EmbedDriver(void *parent = nullptr);
  ~EmbedDriver();

  bool isDeviceContextFontEnabled() const;
  bool isLocaleEmulationEnabled() const;
  std::function<void(std::wstring)>engineNameChanged;
public  :
  void sendEngineName();
  //void updateTranslation(const QString &text, UINT64 hash, int role);


  void setTranslationWaitTime(int v);

  void setEnabled(bool t);
  void setEncoding(const std::wstring&v);
  //void setDetectsControl(bool t);
  void setExtractsAllTexts(bool t);
  void setScenarioVisible(bool t);
  void setScenarioTextVisible(bool t);
  void setScenarioExtractionEnabled(bool t);
  void setScenarioTranscodingEnabled(bool t);
  void setScenarioTranslationEnabled(bool t);
  void setNameVisible(bool t);
  void setNameTextVisible(bool t);
  void setNameExtractionEnabled(bool t);
  void setNameTranscodingEnabled(bool t);
  void setNameTranslationEnabled(bool t);
  void setOtherVisible(bool t);
  void setOtherTextVisible(bool t);
  void setOtherExtractionEnabled(bool t);
  void setOtherTranscodingEnabled(bool t);
  void setOtherTranslationEnabled(bool t);

  void setAlwaysInsertsSpaces(bool t);
  void setSmartInsertsSpaces(bool t);
  void setSpacePolicyEncoding(const std::wstring&v);

  void setScenarioWidth(int v);

  void setScenarioSignature(long v);
  void setNameSignature(long v);

  // Called by engine
public:
  void quit();
  bool load();
  void unload();
  std::wstring engineName() const;
};

// EOF
