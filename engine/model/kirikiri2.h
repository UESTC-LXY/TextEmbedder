#pragma once

// taskforce.h
// 8/4/2015 jichi

#include "engine/enginemodel.h"

class kirikiri2 : public EngineModel
{
  SK_EXTEND_CLASS(kirikiri2, EngineModel)
  static bool attach();
  static bool match();
public:
  kirikiri2()
  {
    name = "kirikiri2";
    enableDynamicFont = true;
    //newLineString = "\n";
    encoding=Utf16Encoding;
    matchFunction =&Self::match;
    attachFunction = &Self::attach;
  }
};

// EOF
