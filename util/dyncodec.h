#pragma once

// dyncodec.h
// 6/3/2015 jichi

#include "sakurakit/skglobal.h" 
class DynamicCodecPrivate;
class DynamicCodec
{
  SK_CLASS(DynamicCodec)
  SK_DECLARE_PRIVATE(DynamicCodecPrivate)
  SK_DISABLE_COPY(DynamicCodec)
public:
  static Self *instance();
  DynamicCodec();
  ~DynamicCodec();

  void setMinimumByte(int v);
  std::string  encodeSTD(const std::wstring& text, bool* dynamic = nullptr) const;

  std::string encode(const std::wstring&text, bool *dynamic = nullptr) const;
  std::wstring decode(const std::string&data, bool *dynamic = nullptr) const;
  UINT decodeChar(UINT ch, bool *dynamic = nullptr) const;
};

// EOF
