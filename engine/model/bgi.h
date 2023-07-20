#pragma once

// bgi.h
// 5/11/2014 jichi

#include "engine/enginemodel.h"
#include<string>
class BGIEngine : public EngineModel
{
  SK_EXTEND_CLASS(BGIEngine, EngineModel)
  static bool attach(); 
  static std::wstring rubyRemove(const std::wstring&text);
public:
  BGIEngine()
  {
    name = "EmbedBGI";
    enableDynamicEncoding = true;
    enableDynamicFont = true; // CreateFontIndirect only invoked once
    matchFilesSTD = { L"BGI.*|BHVC.exe|sysgrp.arc" };
    //newLineString = "\n";
    attachFunction = &Self::attach; 
    rubyRemoveFunctionSTD = &Self::rubyRemove;
  }
};

// EOF
