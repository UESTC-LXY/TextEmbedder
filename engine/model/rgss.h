#pragma once

// rgss.h
// 6/7/2015 jichi

#include "engine/enginemodel.h"

class RGSSEngine : public EngineModel
{
  SK_EXTEND_CLASS(RGSSEngine, EngineModel)
  static bool attach();
  static std::wstring textFilter(const std::wstring &text, int role);
public:
  RGSSEngine()
  {
    name = "EmbedRGSS";
    encoding = Utf16Encoding;
    enableDynamicFont = true;
    matchFilesSTD = { L"System/RGSS3*.dll" };
    //newLineString = "\n";
    //matchFiles << "Game.rgss3a";
    attachFunction = &Self::attach;
    textFilterFunctionSTD = &Self::textFilter;
  }
};

// EOF
