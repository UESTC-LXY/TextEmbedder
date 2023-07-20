#pragma once

// pal.h
// 7/18/2015 jichi

#include "engine/enginemodel.h"

class PalEngine : public EngineModel
{
  SK_EXTEND_CLASS(PalEngine, EngineModel)
  static bool attach();
  static std::wstring textFilter(const std::wstring &text, int role);
  static std::wstring rubyCreate(const std::wstring &rb, const std::wstring &rt);
  static std::wstring rubyRemove(const std::wstring &text);

public:
  PalEngine()
  {
    name = "EmbedPal";
    //enableDynamicEncoding = true;
    newLineStringW = nullptr;
    matchFilesSTD = { L"dll/Pal.dll" };
    attachFunction = &Self::attach;
    textFilterFunctionSTD = &Self::textFilter;
  //  rubyCreateFunction = &Self::rubyCreate;
    rubyRemoveFunctionSTD = &Self::rubyRemove;
  }
};

// EOF
