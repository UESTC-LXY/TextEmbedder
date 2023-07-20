  
#pragma once
 

#include "engine/enginemodel.h"

class aosEngine : public EngineModel
{
    SK_EXTEND_CLASS(aosEngine, EngineModel)
        static bool attach();  
public:
    aosEngine()
    {
        name = "AOSengine";
        enableDynamicEncoding = true;
        //encoding = Utf8Encoding;
        matchFilesSTD = { L"*.aos" };
        attachFunction = &Self::attach; 
    }
};

// EOF
