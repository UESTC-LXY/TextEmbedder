#pragma once

// textutil.h
// 5/7/2014 jichi
#include<string>

namespace Util {

    ///  Assume the text is Japanese, and return if it is not understandable for non-Japanese speakers.

    bool needsTranslationSTD(const std::wstring& text);
    bool allHangul(const wchar_t* s);
    std::wstring thin2wideSTD(const std::wstring& s);


    inline bool allAscii(const char* s)
    {
        while (*s)
            if ((signed char)*s++ < 0)
                return false;
        return true;
    }

    inline bool allAscii(const wchar_t* s)
    {
        while (*s)
            if (*s++ >= 128)
                return false;
        return true;
    }

    inline bool allAscii(const char* s, size_t size)
    {
        for (size_t i = 0; s[i] && i < size; i++)
            if ((signed char)*s++ < 0)
                return false;
        return true;
    }

    inline bool allAscii(const wchar_t* s, size_t size)
    {
        for (size_t i = 0; s[i] && i < size; i++)
            if (*s++ >= 128)
                return false;
        return true;
    }

    inline bool allSpace(const char* s)
    {
        while (*s)
            if (!std::isspace(*s++))
                return false;
        return true;
    }
    //inline bool allSpace(const wchar_t *s)
    //{
    //  while (*s)
    //    if (!::iswspace(*s++))
    //      return false;
    //  return true;
    //}


    //  Return the number of thin characters
    inline size_t measureTextSize(const wchar_t* s)
    {
        size_t ret = 0;
        while (*s)
            ret += (127 >= *s++) ? 1 : 2;
        return ret;
    }

    inline size_t measureTextSize(const wchar_t* begin, const wchar_t* end)
    {
        size_t ret = 0;
        while (*begin && begin != end)
            ret += (*begin++ <= 127) ? 1 : 2;
        return ret;
    }
}

// EOF
