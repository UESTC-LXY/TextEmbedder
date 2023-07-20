#pragma once

// leaf.h
// 6/24/2015 jichi

#include "engine/enginemodel.h"

class LeafEngine : public EngineModel
{
  SK_EXTEND_CLASS(LeafEngine, EngineModel)
  static bool attach();
  static std::wstring textFilter(const std::wstring &text, int role);
  static std::wstring rubyCreate(const std::wstring &rb, const std::wstring &rt);
  static std::wstring rubyRemove(const std::wstring &text);
public:
  LeafEngine()
  {
    name = "EmbedLeaf";
    //enableDynamicEncoding = true;
    scenarioLineCapacity = 40; // 26 wide characters in White Album 2
    matchFilesSTD = { L"bak.pak" };
    newLineStringW = L"\\n";
    attachFunction = &Self::attach;
    textFilterFunctionSTD = &Self::textFilter; 
    rubyRemoveFunctionSTD = &Self::rubyRemove;
  }
};

// EOF
