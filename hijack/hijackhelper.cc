// hijackhelper.cc
// 5/1/2014 jichi
#include "hijack/hijackhelper.h"
#include "hijack/hijacksettings.h"
#include "util/codepage.h" 

/** Private class */

class HijackHelperPrivate
{
public:
  HijackSettings settings;

  UINT systemCodePage;
  UINT8 systemCharSet;

  std::wstring encoding;
  UINT encodingCodePage;
  bool encodingEnabled;

  HijackHelperPrivate()
    : encodingCodePage(0)
    , encodingEnabled(true)
  {
    systemCodePage = ::GetACP();
    systemCharSet = Util::charSetForCodePage(systemCodePage);
  }
};

/** Public class */

static HijackHelper *instance_;
HijackHelper *HijackHelper::instance() { return ::instance_; }

HijackHelper::HijackHelper(void*parent)
  :   d_(new D)
{ ::instance_ = this; }

HijackHelper::~HijackHelper()
{
  ::instance_ = nullptr;
  delete d_;
}

// - Properties -

HijackSettings *HijackHelper::settings() const { return &d_->settings; }

UINT8 HijackHelper::systemCharSet() const { return d_->systemCharSet; }

void HijackHelper::setEncoding(const std::wstring &v)
{
  if (d_->encoding != v) {
    d_->encoding = v;
    d_->encodingCodePage = Util::codePageForEncoding(v);
  }
}

void HijackHelper::setEncodingEnabled(bool t)
{ d_->encodingEnabled = t; }

// - Encoding -

bool HijackHelper::isTranscodingNeeded() const
{ return d_->encodingEnabled && d_->encodingCodePage != d_->systemCodePage; }

// EOF
