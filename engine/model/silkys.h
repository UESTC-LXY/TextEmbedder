#pragma once

// silkys.h
// 6/17/2015 jichi
// For games made by Silky's+, which forks out of Elf

#include "engine/enginemodel.h"

class SilkysEngine : public EngineModel
{
  SK_EXTEND_CLASS(SilkysEngine, EngineModel)
  static bool attach();
public:
  SilkysEngine()
  {
    name = "EmbedSilkysPlus";
    enableDynamicEncoding = true;
    matchFilesSTD = { L"data.arc" , L"effect.arc" , L"Script.arc" };
    newLineStringW = nullptr;
    attachFunction = &Self::attach;
  }
};

// EOF
