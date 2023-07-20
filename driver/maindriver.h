#pragma once

// maindriver.h
// 2/1/2013 jichi

#include "sakurakit/skglobal.h" 

class MainDriverPrivate;
// Root object for all qobject
class MainDriver  
{ 
  SK_EXTEND_CLASS(MainDriver, void)
  SK_DECLARE_PRIVATE(MainDriverPrivate)
public:
  explicit MainDriver(void*parent = nullptr);
  ~MainDriver();

  void quit();
  // Connected with deleteLater through queued signal
  void requestDeleteLater(); 
};

// EOF
