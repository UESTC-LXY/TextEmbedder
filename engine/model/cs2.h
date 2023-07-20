#pragma once

// cs2.h
// 6/21/2015 jichi

#include "engine/enginemodel.h"

class CatSystemEngine : public EngineModel
{
  SK_EXTEND_CLASS(CatSystemEngine, EngineModel)
  bool attach();
  static std::wstring rubyCreate(const std::wstring &rb, const std::wstring &rt);
  static std::wstring rubyRemove(const std::wstring &text);
  static std::wstring translationFilter(const std::wstring &text, int role);
public:
  CatSystemEngine()
  {
    name = "EmbedCatSystem2";
    //enableDynamicEncoding = true;
    enableDynamicFont = true;
    matchFilesSTD = { L"*.int" };
    textSeparatorsSTD = { L"\\pc" , L"\\@" };
    newLineStringW = L"\\n";
    attachFunction = std::bind(&Self::attach, this);
    //rubyCreateFunction = &Self::rubyCreate;
    rubyRemoveFunctionSTD = &Self::rubyRemove;
    translationFilterFunctionSTD = &Self::translationFilter;
  }
};

// EOF
