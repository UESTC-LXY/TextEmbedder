// siglus.cc
// 5/25/2014 jichi
//
// Hooking thiscall: http://tresp4sser.wordpress.com/2012/10/06/how-to-hook-thiscall-functions/
#include "engine/model/EntisGLS.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/enginesettings.h"
#include "engine/engineutil.h"
#include "engine/util/textunion.h"
#include "hijack/hijackmanager.h"
#include "winhook/hookcode.h"
#include "winhook/hookfun.h" 
#include <cstdint>

#define DEBUG "model/siglus"
#include "sakurakit/skdebug.h"

//それは舞い散る桜のように-完全版-
//int __thiscall sub_4BB5D0(_BYTE *this, LPCWCH lpWideCharStr)
namespace{
  bool hookfun(winhook::hook_stack* s)
  {
      auto text = (wchar_t*)s->stack[1];
      if (text == 0)return false;
      //if (wcscmp(L"「…………」", text) == 0)return true;
      auto newText = EngineController::instance()->dispatchTextWSTD(text, Engine::ScenarioRole, 0);
         
      auto _ = new wchar_t[newText.size() + 1];
      wcscpy(_, newText.c_str());
      s->stack[1] = (ULONG)_;
      return true;
  }
  bool attach(ULONG startAddress, ULONG stopAddress) // attach scenario
  {
    
    const uint8_t bytes1[]={
       0x66,0x83,0xF9,0x41 ,
       0x72,0x06,
       0x66,0x83,0xF9,0x5a ,
       0x76,0x0C,
       0x66,0x83,0xF9,0x61 ,
       0x72,0x12,
       0x66,0x83,0xF9,0x7a ,
       0x77,0x0c

    };
    ULONG addr = MemDbg::findBytes(bytes1, sizeof(bytes1), startAddress, stopAddress);
     
    if (!addr) return false;
    addr=MemDbg::findEnclosingAlignedFunction(addr);
    if (!addr) return false;
    //ULONG addr=(0x4BB5D0);
    return winhook::hook_before(addr, hookfun);
  }
}
bool EntisGLS::attach()
{
  ULONG startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;
  if (!::attach(startAddress, stopAddress))
    return false;
 
  // Allow change font
  HijackManager::instance()->attachFunction((ULONG)::CreateFontIndirectA);
  return true;
}

// EOF
