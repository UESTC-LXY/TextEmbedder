#pragma once

// qlie.h
// 7/5/2015 jichi

#include "engine/enginemodel.h"

class QLiEEngine : public EngineModel
{
  SK_EXTEND_CLASS(QLiEEngine, EngineModel)
  static bool attach();
  static std::wstring textFilter(const std::wstring &text, int role);
  static std::wstring translationFilter(const std::wstring &text, int role);
  static std::wstring rubyCreate(const std::wstring &rb, const std::wstring &rt);
  static std::wstring rubyRemove(const std::wstring &text);

public:
  QLiEEngine()
  {
    name = "EmbedQLiE";
    enableDynamicEncoding = true;
    newLineStringW = L"[n]";
    matchFilesSTD = { L"GameData/data*.pack" }; // mostly data0.pack, data1.pack, ...
    attachFunction = &Self::attach;
    textFilterFunctionSTD = &Self::textFilter;
    translationFilterFunctionSTD = &Self::translationFilter;
    //rubyCreateFunction = &Self::rubyCreate;
    rubyRemoveFunctionSTD = &Self::rubyRemove;
  }
};

// EOF
