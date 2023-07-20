#pragma once

// minori.h
// 8/29/2015 jichi

#include "engine/enginemodel.h"

class MinoriEngine : public EngineModel
{
  SK_EXTEND_CLASS(MinoriEngine, EngineModel)
  static bool attach(); 
  static std::wstring rubyRemove(const std::wstring&text);
public:
  MinoriEngine()
  {
    name = "EmbedMinori";
    enableDynamicEncoding = true;
    //enableDynamicFont = true;
    //newLineString = "\n";
    matchFilesSTD = { L"*.paz" };
    textSeparatorsSTD = { L"\\N" , L"\\a" , L"\\v" }; //<< "#";
    attachFunction = &Self::attach; 
    rubyRemoveFunctionSTD = &Self::rubyRemove;
  }
};

// EOF
