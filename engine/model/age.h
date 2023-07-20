#pragma once

// age.h
// 6/1/2014 jichi

#include "engine/enginemodel.h"

class ARCGameEngine : public EngineModel
{
  SK_EXTEND_CLASS(ARCGameEngine, EngineModel)
  static bool attach();
  static std::wstring textFilter(const std::wstring&text, int role);
public:
  ARCGameEngine()
  {
    name = "EmbedARCGameEngine";
    enableDynamicEncoding = true;
    enableNonDecodableCharacters = true; // there could be illegal characters even in scenario
    matchFilesSTD = { L"AGERC.DLL" }; // the process name is AGE.EXE.
    newLineStringW = nullptr;
    attachFunction = &Self::attach;
    textFilterFunctionSTD = &Self::textFilter;
  }
};

// EOF
