// taskforce.cc
// 8/4/2015 jichi
#include "engine/model/kirikiri2.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "winhook/hookcode.h"
#include "memdbg/memsearch.h" 
#include <cstdint>

#include"ithsys/ithsys.h"
#define DEBUG "model/kirikiri2"
#include "sakurakit/skdebug.h"
#include<regex>
namespace { // unnamed
namespace ScenarioHook {
namespace Private {

bool all_ascii(const wchar_t *s, int maxsize )
{
  if (s)
    for (int i = 0; i < maxsize && *s; i++, s++)
      if (*s > 127) // unsigned char
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
     //シロガネオトメ
    auto text = (LPWSTR)s->esi; 
    if (  !text || !*text)
      return true;
    if (all_ascii(text,wcslen(text)))return true;
    std::wstring wstext=text;
    //[「]ぱ、ぱんつなんてどうしてそんなに気になるの。ゆきちゃんだってはいてるでしょ[」][plc]     ->对话
    //[x]彼女は言葉通りに、お風呂上がりにパンツを穿き忘れてそのまま一日過ごしかけたりすることがあった。ボクはそれをまじめに心配していたのだ（開き直り）。[plc]    ->旁白
    /*
    //算了，改人名容易出问题
    //[name name="？／翼"]    ->人名
    //[name name="翼"]
    auto checkisname=std::regex_replace(wstext, std::wregex(L"\\[name name=\"(.*?)\"\\]"), L"");
    if(wstext!=L"" && checkisname==L""){
      auto name=std::regex_replace(wstext, std::wregex(L"\\[name name=\"(.*?)\"\\]"), L"$1");
      
      auto _idx=name.find(L'\uff0f');
      std::wstring end=L"";
      if(_idx!=name.npos){
        name=name.substr(0,_idx);
        end=name.substr(_idx);
      }
      name = EngineController::instance()->dispatchTextWSTD(name, Engine::NameRole, 0);
      name+=end;
      name=L"[name name=\""+name+L"\"]";
      wcscpy(text,name.c_str()); 
      return true;
    }
    */
    if(wstext.size()<5||(wstext.substr(wstext.size()-5)!=L"[plc]"))return true;
    
    int type=0;
    if(wstext.substr(0,3)==L"[x]"){
      type=1;
      wstext=wstext.substr(3);
    }
    else if (wstext.substr(0,3)==L"[\u300c]"){  //「 」
      type=2;
      wstext=std::regex_replace(wstext, std::wregex(L"\\[\u300c\\]"), L"\u300c");
      wstext=std::regex_replace(wstext, std::wregex(L"\\[\u300d\\]"), L"\u300d");
    }
    if(type==0)return true;//未知类型
    std::wstring saveend=L"";
    auto innner=wstext.substr(0,wstext.size()-5);
    innner=std::regex_replace(innner, std::wregex(L"\\[eruby text=(.*?) str=(.*?)\\]"), L"$2");
    if(innner[innner.size()-1]==L']'){
      //「ボクの身体をあれだけ好き勝手しておいて、いまさらカマトトぶっても遅いよ。ほら、正直になりなよ」[waitsd layer=&CHAR6]
      for(int i=innner.size();i>0;i--){
        if(innner[i]=='['){
          saveend=innner.substr(i);
          innner=innner.substr(0,i);
          break;
        }
      }
    }
    auto newText = EngineController::instance()->dispatchTextWSTD(innner, Engine::ScenarioRole, 0);
    newText=newText+L"[plc]";
    if(type==2){
      newText=L"[x]"+newText;
    }
    else if(type==1){
      newText=std::regex_replace(newText, std::wregex(L"\u300c"), L"\\[\u300c\\]");
      newText=std::regex_replace(newText, std::wregex(L"\u300d"), L"\\[\u300d\\]");
    } 
    newText+=saveend;
    wcscpy(text,newText.c_str()); 
    return true;
  }
} // namespace Private
 
bool attach(ULONG startAddress, ULONG stopAddress)
{
   //シロガネオトメ
//    .text:005D288D 66 8B 06                      mov     ax, [esi]
// .text:005D2890 66 83 F8 3B                   cmp     ax, 3Bh ; ';'
// .text:005D2894 0F 84 AA 06 00 00             jz      loc_5D2F44
// .text:005D2894
// .text:005D289A 66 83 F8 2A                   cmp     ax, 2Ah ; '*'
// .text:005D289E 0F 85 DF 02 00 00             jnz     loc_5D2B83

//修改v3的值
// v3 = *(const wchar_t **)(*(_DWORD *)(a1 + 100) + 8 * *(_DWORD *)(a1 + 116));
//     if ( *v3 != 59 )
//     {
//       if ( *v3 == 42 )
     const uint8_t bytes[] = {
    0x66,0x8B,0x06,0x66,0x83,0xF8,0x3B,0x0F,XX,XX4,0x66,0x83,0xF8,0x2A,0x0F
  };
  ULONG addr = MemDbg::matchBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)  return false;
  return   winhook::hook_before(addr, Private::hookBefore); 
     
}

} // namespace ScenarioHook
} // unnamed namespace

bool kirikiri2::attach()
{
  ULONG startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;
  if (!ScenarioHook::attach(startAddress, stopAddress))
    return false;
  HijackManager::instance()->attachFunction((ULONG)::CreateFontIndirectA);
  HijackManager::instance()->attachFunction((ULONG)::ExtTextOutA);
  HijackManager::instance()->attachFunction((ULONG)::GetGlyphOutlineA);
  HijackManager::instance()->attachFunction((ULONG)::TextOutA);
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
bool kirikiri2::match()
{
  return CheckFile(L"*.xp3") || SearchResourceString(L"TVP(KIRIKIRI) 2");
}
// EOF
