#pragma once

// will.h
// 7/3/2015 jichi

#include "engine/enginemodel.h"

class WillPlusEngine : public EngineModel
{
  SK_EXTEND_CLASS(WillPlusEngine, EngineModel)
  bool attach(); 
  static std::wstring rubyRemove(const std::wstring&text);

public:
  WillPlusEngine()
  {
    name = "EmbedWillPlus";
    //enableDynamicEncoding = true; // dynamically update this value
    newLineStringW = L"\\n";
    matchFilesSTD = { L"Rio.arc" , L"Chip*.arc" };
    attachFunction = std::bind(&Self::attach, this); 
    rubyRemoveFunctionSTD = &Self::rubyRemove;
  }
};

// EOF
