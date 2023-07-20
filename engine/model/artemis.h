  
#pragma once
 

#include "engine/enginemodel.h"

class artemisEngine : public EngineModel
{
    SK_EXTEND_CLASS(artemisEngine, EngineModel)
        static bool attach();  
public:
    artemisEngine()
    {
        name = "Embedartemis";
        
        encoding = Utf8Encoding;
        matchFilesSTD = { L"*.pfs" };
        attachFunction = &Self::attach; 
    }
};

// EOF
