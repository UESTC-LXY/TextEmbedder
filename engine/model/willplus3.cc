  
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "winhook/hookcode.h"
#include"winhook/hookfun.h"
#include "memdbg/memsearch.h"
#include"util/dyncodec.h"
#include"ithsys/ithsys.h"
#include "winasm/winasmdef.h"
#include"hijack/hijackfuns.h"
#include<unordered_set>
#include "stringutil.h"
#include"engine/model/willplus3.h" 
#include "sakurakit/skdebug.h"   
#include<regex>
 /*
wchar_t* __userpurge sub_4392B0@<eax>(
    int a1@<ecx>,
    int a2@<ebp>,
    int a3@<edi>,
    HDC a4,
    _DWORD* a5,
    int a6,
    wchar_t* a7,    //***********
    int a8,
    float a9,
    int a10)
int __userpurge sub_43A150@<eax>(  
        int a1@<ecx>,
        int a2@<ebp>,
        int a3,
        HDC a4,
        _DWORD *a5,
        _DWORD *a6, 
        int a7,
        wchar_t *a8,  //*******
        void *a9,
        int a10,
        float a11,
        _OWORD *a12,
        int a13,
        int a14,
        _OWORD *a15,
        int a16,
        int a17,
        int a18,
        int a19,
        int a20,
        void *a21,
        int a22,
        int a23)
    ¡­¡­¡­¡­
    sub_4392B0(a1, (int)v32, a1, a4, a6, a7, a8, a10, a11, a18);
    */
namespace { // unnamed
    class ScenarioHook {

    public:
        explicit ScenarioHook(int role) : role_(role) {}

        typedef ScenarioHook Self;
        bool attach()
        {
            ULONG startAddress, stopAddress;
            //getMemoryRange(&startAddress, &stopAddress);
            Engine::getProcessMemoryRange(&startAddress, &stopAddress);

            const BYTE bytes[] = {
                   0xc7,0x45,0xfc,0x00,0x00,0x00,0x00,
                   0x33,0xc9,
                   0xc7,0x47,0x78,0x00,0x00,0x00,0x00
            };
            ULONG range = min(stopAddress - startAddress, 0x00300000);
            ULONG addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, startAddress + range);
            
            if (addr == 0)return false;
             
            addr = MemDbg::findEnclosingFunctionBeforeDword(0x83dc8b53,addr, MemDbg::MaximumFunctionSize,1);
             
            if (addr == 0)return false; 
             
            return   winhook::hook_before(addr,
                    std::bind(&Self::hookBefore, this, std::placeholders::_1));

        }

      
        std::unordered_set<LPCSTR> translatedTexts_;
        bool hookBefore(winhook::hook_stack* s)
        {  
           // DOUT(QString::fromUtf16((LPWSTR)s->stack[6]));//"MS UI Gothic"
            //DOUT(QString::fromUtf16((LPWSTR)s->stack[7]));//"¡ºÃÀ¤·¤¤ÈË¤Ë¤Ê¤ê¤¿¤¤¡»%K%P"
              auto text = (LPWSTR)s->stack[7]; // text in arg1
              
              if (!text || !*text)
                  return true;
              auto split = s->stack[0]; // retaddr
              auto sig = Engine::hashThreadSignature(role_, split);
               
              
              std::wstring str =((LPWSTR)s->stack[7] );
              int kp = 0;
              if (endWith(str,L"%K%P")){
                  kp = 1;
                  str = str.substr(0, str.size() - 4);
              }
               

              std::wregex reg1(L"\\{(.*?):(.*?)\\}");
              str = std::regex_replace(str, reg1, L"$1");

              std::wregex reg11(L"\\{(.*?);(.*?)\\}");
              str = std::regex_replace(str, reg11, L"$1");

              strReplace(str, L"\\n", L"");  

              auto data_ = EngineController::instance()->dispatchTextWSTD(str, role_, sig);
              if (kp) {
                  data_.append(L"%K%P");
              } 
              s->stack[7] = (ULONG)(data_.c_str());
            return true;
        }

        
        int role_;

    }; // namespace ScenarioHook
} // unnamed namespace
 
bool willplus3Engine::attach()
{ 
    static auto h = new ScenarioHook(Engine::ScenarioRole); // never deleted
    if (!h->attach())
        return false;

    HijackManager::instance()->attachFunction((ULONG)::GetGlyphOutlineW);
    HijackManager::instance()->attachFunction((ULONG)::TextOutW);  
    return true;
}
