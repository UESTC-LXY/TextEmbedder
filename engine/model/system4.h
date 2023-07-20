#pragma once

// system4.h
// 5/25/2015 jichi

#include "engine/enginemodel.h"

// Single thread
class System4Engine : public EngineModel
{
  SK_EXTEND_CLASS(System4Engine, EngineModel)
  bool attach();

  static bool attachSystem43(ULONG startAddress, ULONG stopAddress);
  static bool attachSystem44(ULONG startAddress, ULONG stopAddress);
public:
  System4Engine()
  {
    name = "EmbedSystem4";
    //enableDynamicEncoding = true;
    matchFilesSTD = { L"AliceStart.ini" };
    newLineStringW = nullptr;
    attachFunction = std::bind(&Self::attach, this);
    textSeparatorsSTD = { L"|" }; // only for other texts
  }
};

// EOF
