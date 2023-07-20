#pragma once

// pensil.h
// 8/1/2015 jichi

#include "engine/enginemodel.h"

class PensilEngine : public EngineModel
{
  SK_EXTEND_CLASS(PensilEngine, EngineModel)
  static bool attach();
  static std::wstring rubyCreate(const std::wstring &rb, const std::wstring &rt);
  static std::wstring rubyRemove(const std::wstring &text);
public:
  PensilEngine()
  {
    name = "EmbedPensil";
    enableDynamicEncoding = true;
    enableDynamicFont = true;
    //newLineString = "\n";
    matchFilesSTD = { L"PSetup.exe|MovieTexture.dll|PENCIL.*" }; // || Util::SearchResourceString(L"2XT -") || Util::SearchResourceString(L"2RM -")
    attachFunction = &Self::attach;
  //  rubyCreateFunctionSTD = &Self::rubyCreate; // ruby only works for double-width characters
    rubyRemoveFunctionSTD = &Self::rubyRemove;
  }
};

// EOF
