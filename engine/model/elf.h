#pragma once

// elf.h
// 5/31/2014 jichi

#include "engine/enginemodel.h"

class ElfEngine : public EngineModel
{
  SK_EXTEND_CLASS(ElfEngine, EngineModel)
  static bool attach();
  static std::wstring textFilter(const std::wstring&text, int role);
public:
  ElfEngine()
  {
    name = "EmbedElf";
    enableDynamicEncoding = true;
    newLineStringW = nullptr;
    // "Silkys.exe" or AI6WIN.exe might not exist
    // All arc files in 愛姉妹4 are: data, effect, layer, mes, music
    // mes.arc is the scenario
    matchFilesSTD = { L"data.arc" ,L"effect.arc" ,L"mes.arc" };
    attachFunction = &Self::attach;
    textFilterFunctionSTD = &Self::textFilter;
  }
};

// EOF
