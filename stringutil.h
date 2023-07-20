#pragma once
#ifndef __STRINGUTIL_HPP
#define __STRINGUTIL_HPP
#include<string>
#include<Windows.h>
#include<vector>

std::wstring  StringToWideString(const std::string& text, UINT encoding);
std::string WideStringToString(const std::wstring& text, UINT encoding);
void strReplace(std::wstring& str, const std::wstring& oldStr, const std::wstring& newStr);
void strReplace(std::wstring& str, const wchar_t* oldStr, const wchar_t* newStr);
std::wstring strReplace(const std::wstring& str, const wchar_t* oldStr, const wchar_t* newStr);
void strReplace(std::string& str, const std::string& oldStr, const std::string& newStr);
void strReplace(std::string& str, const char* oldStr, const char* newStr);
bool endWith(std::wstring& s, std::wstring);
std::vector<std::wstring> strSplit(const std::wstring& s, const std::wstring  delim);

std::string toLower(std::string);

template <class T>
std::wstring strjoin(T& val, std::wstring delim)
{
    std::wstring str;
    typename T::iterator it;
    const typename T::iterator itlast = val.end() - 1;
    for (it = val.begin(); it != val.end(); it++)
    {
        str += *it;
        if (it != itlast)
        {
            str += delim;
        }
    }
    return str;
}
#endif // !__STRINGUTIL_HPP

