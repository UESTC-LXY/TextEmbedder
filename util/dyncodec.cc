// dyncodec.h
// 6/3/2015 jichi

#include "util/codepage.h"
#include "util/dyncodec.h"
#include "qtdyncodec/qtdynsjis.h"

class DynamicCodecPrivate
{
public:
  DynamicShiftJISCodec *codec;

  DynamicCodecPrivate()
    : codec(new DynamicShiftJISCodec(Util::codePageForEncodingA(ENC_SJIS)))
  {}

  ~DynamicCodecPrivate() { delete codec; }
};

static DynamicCodec *instance_;
DynamicCodec *DynamicCodec::instance() { return ::instance_; }

DynamicCodec::DynamicCodec()
  : d_(new D)
{ ::instance_= this; }

DynamicCodec::~DynamicCodec()
{
  ::instance_ = nullptr;
  delete d_;
}

void DynamicCodec::setMinimumByte(int v)
{ return d_->codec->setMinimumSecondByte(v); }

std::string DynamicCodec::encodeSTD(const std::wstring & text, bool* dynamic) const
{
    return d_->codec->encodeSTD(text, dynamic);
}

std::wstring DynamicCodec::decode(const std::string&data, bool *dynamic) const
{ return d_->codec->decode(data, dynamic); }

UINT DynamicCodec::decodeChar(UINT ch, bool *dynamic) const
{
  if (dynamic)
    *dynamic = false;
  if (ch > 0xff) {
    bool t;
    char data[3] = {(ch>>8)&0xff, ch&0xff, 0};
    auto text = d_->codec->decode(data, &t);
    if (t && text.size() == 1) {
      if (dynamic)
        *dynamic= true;
      return text[0] ;
    }
  }
  return ch;
}

// EOF
