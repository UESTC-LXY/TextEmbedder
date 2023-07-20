#include"stringutil.h"
#include<string>
#include<Windows.h>
#include<vector>

std::wstring  StringToWideString(const std::string& text, UINT encoding)
{
    std::vector<wchar_t> buffer(text.size() + 1);
    if (int length = MultiByteToWideChar(encoding, 0, text.c_str(), text.size() + 1, buffer.data(), buffer.size()))
        return std::wstring(buffer.data(), length - 1);
    return L"";
}
std::string WideStringToString(const std::wstring& text, UINT encoding)
{
    std::vector<char> buffer((text.size() + 1) * 4);
    WideCharToMultiByte(encoding, 0, text.c_str(), -1, buffer.data(), buffer.size(), nullptr, nullptr);
    return buffer.data();
}
std::string toLower(std::string  s) {

    // ת��ΪСд
    for (auto& c : s) {
        c = std::tolower(c);
    }
    return s;
}

std::vector<std::wstring> strSplit(const std::wstring& s, const std::wstring  delim)
{
    std::wstring item;
    std::vector<std::wstring> tokens;

    // Copy the input string so that we can modify it
    std::wstring str = s;

    size_t pos = 0;
    while ((pos = str.find(delim)) != std::wstring::npos) {
        item = str.substr(0, pos);
        tokens.push_back(item);
        str.erase(0, pos + delim.length());
    }
    tokens.push_back(str);

    return tokens;
}

bool endWith(std::wstring& s, std::wstring s2) {
    if ((s.size() > s2.size()) && (s.substr(s.size() - s2.size(), s2.size()) == s2)) {
        return true;
    }
    return false;
}
void strReplace(std::wstring& str, const std::wstring& oldStr, const std::wstring& newStr)
{
    size_t pos = 0;
    while ((pos = str.find(oldStr, pos)) != std::wstring::npos) {
        str.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
}
std::wstring strReplace(const std::wstring& str, const wchar_t* oldStr, const wchar_t* newStr) {
    std::wstring sold = oldStr;
    std::wstring  news = newStr;
    std::wstring _str = str;
    strReplace(_str, sold, news);
    return _str;
}

void strReplace(std::wstring& str, const wchar_t* oldStr, const wchar_t* newStr)
{
    std::wstring sold = oldStr;
    std::wstring  news = newStr;
    strReplace(str, sold, news);
}

void strReplace(std::string& str, const std::string& oldStr, const std::string& newStr)
{
    size_t pos = 0;
    while ((pos = str.find(oldStr, pos)) != std::string::npos) {
        str.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
}
void strReplace(std::string& str, const char* oldStr, const char* newStr)
{
    std::string sold = oldStr;
    std::string  news = newStr;
    strReplace(str, sold, news);
}

