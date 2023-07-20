 
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
#include"engine/model/horkeye.h" 

#define DEBUG "model/horkeye"
#include "sakurakit/skdebug.h"   

namespace { // unnamed
    class ScenarioHook {

    public:
        explicit ScenarioHook(int role)  {}

        typedef ScenarioHook Self;
        bool attach()
        {
            ULONG startAddress, stopAddress;
            //getMemoryRange(&startAddress, &stopAddress);
            Engine::getProcessMemoryRange(&startAddress, &stopAddress);

            const BYTE bytes1[] =
            {
                0x83, 0xec, XX, // sub esp,??
                0xa1, XX4, // mov eax,??
                0x8b, 0x0d, XX4, // mov ecx,??
                0x03, 0xc0 // add eax,eax
            };
            const BYTE bytes2[] =
            {
                0x55,
                0x8d,0xac,0x24,XX4,
                0x81,0xec,XX4,
                0x6a,0xff,
                0x68,XX4,
                0x64,0xa1,0x00,0x00,0x00,0x00,
                0x50,
                0x83,0xec,0x38,   //必须是0x38，不能是XX，否则有重的。

//.text:0042E7F0 55                            push    ebp
//.text : 0042E7F1 8D AC 24 24 FF FF FF          lea     ebp,[esp - 0DCh]
//.text : 0042E7F8 81 EC DC 00 00 00             sub     esp, 0DCh
//.text : 0042E7FE 6A FF                         push    0FFFFFFFFh
//.text : 0042E800 68 51 1E 5C 00                push    offset SEH_42E7F0
//.text : 0042E805 64 A1 00 00 00 00             mov     eax, large fs : 0
//.text : 0042E80B 50                            push    eax
//.text : 0042E80C 83 EC 38                      sub     esp, 38h
//.text : 0042E80F A1 24 D0 64 00                mov     eax, ___security_cookie
//.text : 0042E814 33 C5 xor eax, ebp
//.text : 0042E816 89 85 D8 00 00 00             mov[ebp + 0DCh + var_4], eax
            };
             
             
            ULONG range = min((ULONG) (stopAddress - startAddress), (ULONG)0x00300000 );
            ULONG addr = MemDbg::matchBytes(bytes1, sizeof(bytes1), startAddress, startAddress + range);
            if(!addr)
                addr = MemDbg::matchBytes(bytes2, sizeof(bytes2), startAddress, startAddress + range);
            if (!addr) { 
                return false;
            }
            DOUT(startAddress);
            DOUT(stopAddress);
            DOUT(addr);
            return   winhook::hook_before(addr,
                    std::bind(&Self::hookBefore, this, std::placeholders::_1));

        }
         
        bool hookBefore(winhook::hook_stack* s)
        {  
            
              auto text = (char*)s->stack[1]; // text in arg1
               
              if (!text || !*text)
                  return true;
              auto split = s->stack[0]; // retaddr
              auto sig = Engine::hashThreadSignature(Engine::ScenarioRole, split); 

              //if (((unsigned char)text[0] == 129)&& ((unsigned char)text[1] == 121)) {
              //    int end;
              //    for (end = 0; text[end+1]; end += 1) {
              //        if (((unsigned char)text[end] == 129)&&((unsigned char)text[end+1] == 122))break;
              //    }
              //    //auto name = EngineController::instance()->dispatchTextA(QByteArray::fromRawData(text+2,end-2), Engine::ScenarioRole, sig);
              //    //QByteArray result; 
              //    //result.clear();
              //    //result.append(129); result.append(121);//【
              //    //result.append(name); 
              //    //result.append(129); result.append(122);//】
              //    auto result = QByteArray::fromRawData(text, end + 2);
              //    auto trans = EngineController::instance()->dispatchTextA(text+end+2, Engine::ScenarioRole, sig);
              //    
              //    result.append(trans);
              //    //s->stack[1] = (DWORD)result.constData();
              //    //明白为什么崩溃了，前半段是shiftjis，后面接上的是utf8，= =，应该加一个函数把前半段也转成utf8。不过懒得搞了。
              //}
              //else { 
              //    auto data_ = EngineController::instance()->dispatchTextA(text, Engine::ScenarioRole, sig);
              //    //s->stack[1] = (DWORD)data_.constData();
              //}
              auto data_ = EngineController::instance()->dispatchTextASTD(text, Engine::ScenarioRole, sig);
              //s->stack[1] = (DWORD)data_.constData();
            return true;
        }
         

    }; // namespace ScenarioHook
} // unnamed namespace
 
bool horkeye::attach()
{ 
    static auto h = new ScenarioHook(Engine::ScenarioRole); // never deleted
    if (!h->attach())
        return false;
     
    //HijackManager::instance()->attachFunction((ULONG)::GetOutlineTextMetricsA); 
    //HijackManager::instance()->attachFunction((ULONG)::CreateFontA); 
    //HijackManager::instance()->attachFunction((ULONG)::GetGlyphOutlineA);

    return true;
}
namespace {
    bool SearchResourceString(LPCWSTR str)
    {
        DWORD hModule = (DWORD)GetModuleHandleW(nullptr);
        IMAGE_DOS_HEADER* DosHdr;
        IMAGE_NT_HEADERS* NtHdr;
        DosHdr = (IMAGE_DOS_HEADER*)hModule;
        DWORD rsrc, size;
        //__asm int 3
        if (IMAGE_DOS_SIGNATURE == DosHdr->e_magic) {
            NtHdr = (IMAGE_NT_HEADERS*)(hModule + DosHdr->e_lfanew);
            if (IMAGE_NT_SIGNATURE == NtHdr->Signature) {
                rsrc = NtHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
                if (rsrc) {
                    rsrc += hModule;
                    if (IthGetMemoryRange((LPVOID)rsrc, &rsrc, &size) &&
                        SearchPattern(rsrc, size - 4, str, wcslen(str) << 1))
                        return true;
                }
            }
        }
        return false;
    }
}
bool horkeye::match() {
    if ( SearchResourceString(L"HorkEye")) {  
        return true;
    }
    return false;
}