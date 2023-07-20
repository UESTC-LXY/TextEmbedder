#pragma once

// fvp.h
// 7/28/2015 jichi

#include "engine/enginemodel.h"

class FVPEngine : public EngineModel
{
  SK_EXTEND_CLASS(FVPEngine, EngineModel)
  static bool attach();
  static std::wstring rubyCreate(const std::wstring &rb, const std::wstring &rt);
  static std::wstring rubyRemove(const std::wstring &text);

public:
  FVPEngine()
  {
    name = "EmbedFVP";
    matchFilesSTD = { L"*.hcb" };
    enableDynamicEncoding = true;
    enableDynamicFont = true;
    newLineStringW = nullptr;
    attachFunction = &Self::attach;
    //rubyCreateFunction = &Self::rubyCreate;
    rubyRemoveFunctionSTD = &Self::rubyRemove;
  }
};

// EOF
