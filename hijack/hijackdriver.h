#pragma once
// hijackdriver.h
// 5/1/2013 jichi

#include "sakurakit/skglobal.h" 
#include<string>
class HijackDriverPrivate;
// Root object for all qobject
class HijackDriver 
{ 
  SK_EXTEND_CLASS(HijackDriver, void)
  SK_DECLARE_PRIVATE(HijackDriverPrivate)
public:
  explicit HijackDriver(void*parent = nullptr);
  ~HijackDriver();

  void unload();

public:
  void setEncoding(const std::wstring &v);
  void setFontFamily(const std::wstring&v);
  void setFontScale(float v);
  void setFontWeight(int v);
  void setFontCharSet(int v);
  void setFontCharSetEnabled(bool t);

  void setDeviceContextFontEnabled(bool t);
  void setLocaleEmulationEnabled(bool t);

private :
  void refresh();
};

// EOF
