// taskforce.cc
// 8/4/2015 jichi
#include "engine/model/kirikiriz.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "winhook/hookcode.h"
#include "memdbg/memsearch.h" 
#include <cstdint>

#include"ithsys/ithsys.h"
#define DEBUG "model/kirikiriz"
#include "sakurakit/skdebug.h"

namespace { // unnamed
namespace ScenarioHook {
namespace Private {
  bool all_ascii(const char* s, int maxsize)
{
    if (s)
        for (int i = 0; i < maxsize && *s; i++, s++)
            if ((BYTE)*s > 127) // unsigned char
                return false;
    return true;
}
void strReplace(std::string& str, const std::string& oldStr, const std::string& newStr)
{
    size_t pos = 0;
    while ((pos = str.find(oldStr, pos)) != std::string::npos) {
        str.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
}
std::wstring StringToWideString(const std::string& text, UINT encoding=65001)
{
    std::vector<wchar_t> buffer(text.size() + 1);
    int length = MultiByteToWideChar(encoding, 0, text.c_str(), text.size() + 1, buffer.data(), buffer.size());
    return std::wstring(buffer.data(), length - 1);

}

std::string WideStringToString(const std::wstring& text, UINT encoding = CP_UTF8)
{
    std::vector<char> buffer((text.size() + 1) * 4);
    WideCharToMultiByte(encoding, 0, text.c_str(), -1, buffer.data(), buffer.size(), nullptr, nullptr);
    return buffer.data();
} 
std::wstring ConvertToFullWidth(const std::wstring& str) {
    std::wstring fullWidthStr;
    for (wchar_t c : str) {
        if (c >= 32 && c <= 126) {
            fullWidthStr += static_cast<wchar_t>(c + 65248);
        } else {
            fullWidthStr += c;
        }
    }
    return fullWidthStr;
}

  bool hookBefore(winhook::hook_stack *s)
  {
    static std::string res; 
    auto text = (LPCSTR)s->ecx; 
    if (  !text || !*text)
      return true;
    if (all_ascii(text,strlen(text)))return true;
    //"。」』？―！、"
    auto chatflags={"\xe3\x80\x82", "\xe3\x80\x8d","\xe3\x80\x8f","\xef\xbc\x9f","\xe2\x80\x95","\xef\xbc\x81","\xe3\x80\x81"};
    bool ok=false;
    for (auto f:chatflags){
      if(strstr(text,f))ok=true;
    }
    if(ok==false)return true;
    auto role =  Engine::ScenarioRole ;
    //auto split = s->edx;
    //auto sig = Engine::hashThreadSignature(role, split);
    enum { sig = 0 }; // split not used
    std::string utf8save=text;
    strReplace(utf8save, "%51;", "\\-");
    strReplace(utf8save, "%164;", "\\+\\+");
    strReplace(utf8save, "%123;", "\\+");
    strReplace(utf8save, "%205;", "\\+\\+\\+");
    strReplace(utf8save, "#000033ff;", "\\#0033FF");
    strReplace(utf8save, "#;", "\\#FFFFFF");
    strReplace(utf8save, "#00ff0000;", "\\#FF0000");
    strReplace(utf8save, "%p-1;%f\xef\xbc\xad\xef\xbc\xb3 \xe3\x82\xb4\xe3\x82\xb7\xe3\x83\x83\xe3\x82\xaf;", ""); //"%p-1;%fＭＳ ゴシック;"
    strReplace(utf8save, "%p;%fuser;", "");
    res = EngineController::instance()->dispatchTextASTD(utf8save, role, Engine::hashThreadSignature(role, s->stack[0]) ,0,true,nullptr,true,true);
     strReplace(res, "\\-", "%51;");
        strReplace(res, "\\+\\+", "%164;");
        strReplace(res, "\\+", "%123;");
        strReplace(res, "\\+\\+\\+", "%205;");
        strReplace(res, "\\#0033FF", "#000033ff;");
        strReplace(res, "\\#FFFFFF", "#;");
        strReplace(res, "\\#FF0000", "#00ff0000;");
      res=WideStringToString(ConvertToFullWidth((StringToWideString(res))));
    auto cs = new char[res.size() + 1];
        strcpy(cs, res.c_str());
        s->ecx = (DWORD)cs;
    return true;
  }
} // namespace Private
 
bool attach(ULONG startAddress, ULONG stopAddress)
{
  //mashiro_fhd
      BYTE sig0[]={0x8B,0xF1};//mov esi,ecx
      BYTE sig01[]={0x8A,0x06};//mov     al, [esi]

      BYTE sig1[]={0x3C,0x80,XX};//0x73//0x0f
			BYTE sig2[]={0x3C,0xC2,XX};
			BYTE sig3[]={0x3C,0xE0,XX};
			BYTE sig4[]={0x3C,0xF0,XX};
			BYTE sig5[]={0x3C,0xF8,XX};
			BYTE sig6[]={0x3C,0xFC,XX};
			BYTE sig7[]={0x3C,0xFE,XX};
    
  ULONG addr =startAddress;
  wchar_t xx[1000];
  while(addr){
    addr=MemDbg::findBytes(sig0, sizeof(sig0), addr+sizeof(sig0), stopAddress);
    // wsprintf(xx,L"%x",(DWORD)addr-(DWORD)startAddress);
    // MessageBox(0,xx,L"",0);
    if(addr==0)continue;
    auto funcstart=MemDbg::findEnclosingAlignedFunction(addr);
    // wsprintf(xx,L"%x",(DWORD)funcstart-(DWORD)startAddress);
    // MessageBox(0,xx,L"1",0);
    if(funcstart==0)continue;
    auto check=MemDbg::findBytes(sig01, sizeof(sig01), addr, addr+0x10);
    if(check==0)continue;
    
    bool ok=true;
    int i=0;
    for(auto p:std::vector<std::pair<BYTE*,int>>{{sig1,sizeof(sig1)},{sig2,sizeof(sig2)},{sig3,sizeof(sig3)},{sig4,sizeof(sig4)},{sig5,sizeof(sig5)},{sig6,sizeof(sig6)},{sig7,sizeof(sig7)}}){
      i+=0;
      check=MemDbg::matchBytes(p.first, p.second, check, check+0x1000);
      if(check==0){
        ok=false;break;
      }
    }
    if(ok){
      return   winhook::hook_before(funcstart, Private::hookBefore);
    }
    
  }
    
  return false;
}

} // namespace ScenarioHook
} // unnamed namespace

bool kirikiriz::attach()
{
  ULONG startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;
  if (!ScenarioHook::attach(startAddress, stopAddress))
    return false;
  HijackManager::instance()->attachFunction((ULONG)::CreateFontIndirectW);
  return true;
}
namespace{
  bool CheckFile(LPCWSTR name)
{
	WIN32_FIND_DATAW unused;
	HANDLE file = FindFirstFileW(name, &unused);
	if (file != INVALID_HANDLE_VALUE)
	{
		FindClose(file);
		return true;
	}
	wchar_t path[MAX_PATH * 2];
	wchar_t* end = path + GetModuleFileNameW(nullptr, path, MAX_PATH);
	while (*(--end) != L'\\');
	wcscpy_s(end + 1, MAX_PATH, name);
	file = FindFirstFileW(path, &unused);
	if (file != INVALID_HANDLE_VALUE)
	{
		FindClose(file);
		return true;
	}
	return false;
}
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
bool kirikiriz::match()
{
  return CheckFile(L"*.xp3") || SearchResourceString(L"TVP(KIRIKIRI) Z");
}
// EOF
