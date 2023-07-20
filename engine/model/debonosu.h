#pragma once

// debonosu.h
// 6/18/2015 jichi

#include "engine/enginemodel.h"

class DebonosuEngine : public EngineModel
{
  SK_EXTEND_CLASS(DebonosuEngine, EngineModel)
  static bool attach();
  static std::wstring rubyCreate(const std::wstring &rb, const std::wstring &rt);
  static std::wstring rubyRemove(const std::wstring &text);
public:
  DebonosuEngine()
  {
    name = "EmbedDebonosu";
    enableDynamicEncoding = true;
    enableLocaleEmulation = true; // fix thread codepage in MultiByteToWideChar
    matchFilesSTD = { L"bmp.pak" , L"dsetup.dll" };
    //newLineString = "\n";
    attachFunction = &Self::attach;
  //  rubyCreateFunction = &Self::rubyCreate;
    rubyRemoveFunctionSTD = &Self::rubyRemove;
  }
};

// EOF
