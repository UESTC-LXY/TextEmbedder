#pragma once

// escude.h
// 7/23/2015 jichi

#include "engine/enginemodel.h"

class EscudeEngine : public EngineModel
{
  SK_EXTEND_CLASS(EscudeEngine, EngineModel)
  static bool attach();
  //static QString rubyCreate(const QString &rb, const QString &rt);
  static std::wstring rubyRemove(const std::wstring&text);
public:
  EscudeEngine()
  {
    name = "EmbedEscude";
    enableDynamicEncoding = true;
    enableDynamicFont = true;
    newLineStringW = L"<r>";
    matchFilesSTD = { L"configure.cfg"  ,L"gfx.bin" };
    attachFunction = &Self::attach; 
    rubyRemoveFunctionSTD = &Self::rubyRemove;
  }
};

// EOF
