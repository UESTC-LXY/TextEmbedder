#pragma once
// hijackhelper.h
// 5/7/2014 jichi
#include<Windows.h>
#include "sakurakit/skglobal.h" 
#include<string>
class HijackSettings;
class HijackHelperPrivate;
// Root object for all qobject
class HijackHelper 
{ 
  SK_EXTEND_CLASS(HijackHelper, void)
  SK_DECLARE_PRIVATE(HijackHelperPrivate)
public:
  static Self *instance(); // needed by hijack functions
  explicit HijackHelper(void*parent = nullptr);
  ~HijackHelper();

  HijackSettings *settings() const;

  void setEncoding(const std::wstring &v);
  void setEncodingEnabled(bool t);

  bool isTranscodingNeeded() const;

  UINT8 systemCharSet() const;
};


// EOF
