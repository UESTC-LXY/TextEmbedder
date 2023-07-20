#pragma once

// rio.h
// 7/10/2015 jichi

#include "engine/enginemodel.h"

class ShinaRioEngine : public EngineModel
{
  SK_EXTEND_CLASS(ShinaRioEngine, EngineModel)
  static bool attach();
  static std::wstring textFilter(const std::wstring &text, int role);
  //static std::wstring rubyCreate(const std::wstring &rb, const std::wstring &rt);
  static std::wstring rubyRemove(const std::wstring &text);

public:
  ShinaRioEngine()
  {
    name = "EmbedRio";
    enableDynamicEncoding = true;
    newLineStringW = L"_r";
    matchFilesSTD = { L"*.war" };
    scenarioLineCapacity =
    otherLineCapacity = 40; // 60 in 3rd games (30 wide characters)
    attachFunction = &Self::attach;
    //rubyCreateFunction = &Self::rubyCreate; // FIXME: does not work
    rubyRemoveFunctionSTD = &Self::rubyRemove;
  }
};

// EOF
