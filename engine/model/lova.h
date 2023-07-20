#pragma once

// lova.h
// 7/19/2015 jichi

#include "engine/enginemodel.h"
#include"stringutil.h"
class LovaEngine : public EngineModel
{
  SK_EXTEND_CLASS(LovaEngine, EngineModel)
  static bool attach();
public:
  LovaEngine()
  {
    name = "EmbedLova";
    encoding = Utf8Encoding;
    matchFilesSTD = { L"awesomium_process.exe" , L"UE3ShaderCompileWorker.exe" };

    //newLineString = "<br>"; // <br> is only for scenario
    newLineStringW = nullptr; // <br> does not work for other texts
    textSeparatorsSTD = { L"<br>" }; // mark <br> as text separator instead

    attachFunction = &Self::attach;
  }
};

// EOF
