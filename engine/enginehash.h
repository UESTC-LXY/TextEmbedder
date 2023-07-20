#pragma once

// enginehash.h
// 4/26/2014 jichi
// Hashing algorithms are so important that I put them into a different file.
#include<Windows.h>
#include "sakurakit/skhash.h" 

#ifdef _MSC_VER
# pragma warning (disable:4334)   // C4334: 32-bit shift implicit converted to 64 bits
#endif // _MSC_VER

namespace Engine {

// Cast quint64 to UINT64
     

inline UINT64 hashByteArraySTD(const std::string& b, UINT64 h = Sk::djb2_hash0)
{
    return Sk::djb2_n2(b.c_str(), b.size(), h);
}

inline UINT64 hashString(const std::string &s, UINT64 h = Sk::djb2_hash0)
{ return Sk::djb2_n(reinterpret_cast<const UINT8 *>(s.c_str()), s.size() * 2, h); }

inline UINT64 hashCharArray(const void *lp)
{ return Sk::djb2(reinterpret_cast<const UINT8 *>(lp)); }

inline UINT64 hashCharArray(const void *lp, size_t len, UINT64 h = Sk::djb2_hash0)
{ return Sk::djb2_n(reinterpret_cast<const UINT8 *>(lp), len, h); }

inline UINT64 hashWCharArray(const wchar_t *lp, size_t len = -1, UINT64 h = Sk::djb2_hash0)
{
  if (len == -1)
    len = ::wcslen(lp);
  return Sk::djb2_n(reinterpret_cast<const UINT8 *>(lp), 2 * len, h);
}

inline UINT64 hashWStringSTD(const std::wstring& s, UINT64 h = Sk::djb2_hash0)
{
    return hashWCharArray(s.c_str(), s.size(), h);
} 

inline UINT64 hashTextKey(UINT64 hash, unsigned role) { return hash + (1 << role); }

// This is consistent with textthread_p in texthook
//inline qint32 hashThreadSignature(ULONG returnAddress, ULONG split = 0)
//{
//  return (returnAddress & 0xffff)  // context
//       | (split & 0xffff) << 16;   // subcontext
//}
inline UINT32 hashThreadSignature(ULONG role, ULONG split = 1)
{
  return (split & 0xffff) // context
       | (role << 16);    // subcontext
}

} // namespace Engine

// EOF
