// i18n.cc
// 6/29/2015 jichi
#include "util/i18n.h" 
 
 
bool starts_with(const std::wstring& input, const std::wstring match) {
    if (match.size() > input.size()) {
        return false;
    }
    return std::equal(match.begin(), match.end(), input.begin());
}
bool Util::languageNeedsWordWrapSTD(const std::wstring& lang)
{
	return lang != L"ja" && !starts_with( lang,(L"zh"));
}
// EOF
