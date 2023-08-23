#pragma once

// taskforce.h
// 8/4/2015 jichi

#include "engine/enginemodel.h"

class kirikiriz : public EngineModel
{
  SK_EXTEND_CLASS(kirikiriz, EngineModel)
  static bool attach();
  static bool match();
public:
  kirikiriz()
  {
    name = "kirikiriz";
    enableDynamicFont = true;
    //newLineString = "\n";
    
    encoding = Utf8Encoding;
    matchFunction =&Self::match;
    attachFunction = &Self::attach;
  }
};

// EOF
