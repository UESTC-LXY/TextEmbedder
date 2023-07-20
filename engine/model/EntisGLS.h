#pragma once

// siglus.h
// 5/25/2014 jichi

#include "engine/enginemodel.h"

// Single thread
class EntisGLS : public EngineModel
{
  SK_EXTEND_CLASS(EntisGLS, EngineModel)
  static bool attach();
public:
  EntisGLS()
  {
    name = "EmbedEntisGLS";
    encoding = Utf16Encoding;
    enableDynamicFont = true;
    matchFilesSTD = { L"Data\\*.dat" };
    //newLineString = "\n";
    attachFunction = &Self::attach;
  }
};

// EOF
