  
#pragma once
 

#include "engine/enginemodel.h"

class horkeye : public EngineModel
{
    SK_EXTEND_CLASS(horkeye, EngineModel)
        static bool attach(); static bool match();
public:
    horkeye()
    {
        name = "HorkEye";
        
        encoding = Utf8Encoding;
        attachFunction = &Self::attach;
        matchFunction = &Self::match;
    }
};

// EOF
