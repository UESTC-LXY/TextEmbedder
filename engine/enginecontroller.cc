// enginecontroller.cc
// 4/20/2014 jichi
#include"stringutil.h"
#include "config.h"
#include "engine/enginecontroller.h"
#include "engine/enginehash.h"
#include "engine/enginemodel.h"
#include "engine/engineutil.h"
//#include "engine/enginememory.h"
#include "engine/enginesettings.h"
#include "embed/embedmanager.h"
#include "hijack/hijackfuns.h"
#include "util/codepage.h"
#include "util/dyncodec.h"
#include "util/i18n.h"
#include "util/textutil.h"
//#include "windbg/util.h"
#include "winhook/hookcode.h" 
//#include "mhook/mhook.h" // must after windows.h  
#include <unordered_set>

#define DEBUG "enginecontroller"
#include "sakurakit/skdebug.h"

/** Private class */

class EngineControllerPrivate
{
  typedef EngineController Q;
   

public:
  static Q *globalInstance;

  enum { ExchangeInterval = 10 };

  EngineSettings settings;

  EngineModel*model;

  UINT codePage;
  //Q::RequiredAttributes attributes;

  UINT encoder,
             decoder, spaceCodec;

  bool dynamicEncodingEnabled;
  DynamicCodec *dynamicCodec;

  bool finalized;

  int scenarioLineCapacity, // current maximum number bytes in a line for scenario thread, always increase and never decrease
      otherLineCapacity;
  std::unordered_set<UINT64> textHashes_; // hashes of rendered text

  EngineControllerPrivate(EngineModel*model)
    : model(model)
    , codePage(0)
    , encoder(0), decoder(0 )
    , dynamicEncodingEnabled(true)
    , dynamicCodec(nullptr)
    , finalized(false)
    , scenarioLineCapacity(0)
    , otherLineCapacity(0)
  {}

  ~EngineControllerPrivate()
  {
    if (dynamicCodec)
      delete dynamicCodec;
  }

  void finalize()
  {
    if (!finalized) {
      finalizeCodecs();
      scenarioLineCapacity = model->scenarioLineCapacity;
      otherLineCapacity = model->otherLineCapacity;
      finalized = true;
    }
  }

  // Property helpers

  //bool testAttribute(Q::RequiredAttribute v) const { return attributes & v; }
  //bool testAttributes(Q::RequiredAttributes v) const { return attributes & v; }

  // Encoding
   
#include"stringutil.h"
  std::string encode(const std::wstring&text) const
  { return encoder ? WideStringToString(text,encoder): WideStringToString(text,CP_ACP); }
   

  std::wstring decode(const std::string &data) const
  { return decoder ? StringToWideString(data,decoder): StringToWideString(data, CP_ACP); }

  bool containsTextHash(UINT64 hash) const
  { return textHashes_.find(hash) != textHashes_.end(); }

  void addTextHash(UINT64 hash)
  { textHashes_.insert(hash); }


  void finalizeCodecs()
  { 
    decoder = codePage;
     
    encoder = GetACP();

    if (model->enableDynamicEncoding && dynamicEncodingEnabled) {
      dynamicCodec = new DynamicCodec;
      dynamicCodec->setMinimumByte(model->dynamicEncodingMinimumByte);
    }

    DOUT("encoding =" << engineEncoding  << ", system =" << systemEncoding);
  }
   
  static std::wstring alwaysInsertSpacesSTD(const std::wstring& text)
  {
      std::wstring ret;
      for(auto c: text) {
          ret.push_back(c);
          if (c  >= 32) // ignore non-printable characters
              ret.push_back(L' '); // or insert \u3000 if needed
      }
      return ret;
  }
  static std::wstring insertSpacesAfterUnencodableSTD(const std::wstring& text, UINT codepage)
  {
      std::wstring ret;
      for(const wchar_t & c: text) {
          ret.push_back(c);
          if (!Util::charEncodableSTD(c, codepage))
              ret.push_back(L' ');
      }
      return ret;
  }
   
  static size_t getLineCapacity(const wchar_t *s)
  {
    enum : wchar_t { br = '\n' };
    if (!::wcschr(s, br))
      return Util::measureTextSize(s);
    size_t ret = 0;
    for (auto p = s; *s; s++)
      if (*s == br) {
        ret = max(ret, Util::measureTextSize(p, s));
        p = s + 1;
      }
    return ret;
  }

   
  static int getTextDistanceSTD(const std::wstring& text, int startPos, int stopPos, bool skipBBCode)
  {
      if (!skipBBCode)
          return stopPos - startPos;
      int ret = 0;
      for (int i = startPos; i <= stopPos; i++) {
          if (text[i] == L'[') {
              int closePos = text.find(L']', i + 1);
              if (closePos != std::wstring::npos)
                  i = closePos;
          }
          else
              ret++;
      }
      return ret;
  }

  //QString renderRuby(const QString &text) const
  //{
  //  if (text.isEmpty() || !rubyParser.containsRuby(text))
  //    return text;
  //  else if (model->rubyCreateFunction)
  //    return rubyParser.renderRuby(text, model->rubyCreateFunction);
  //  else
  //    return rubyParser.removeRuby(text);
  //}

  //QString removeRuby(const QString &text) const
  //{
  //  if (text.isEmpty() || !rubyParser.containsRuby(text))
  //    return text;
  //  return rubyParser.removeRuby(text);
  //}

public: 
  std::wstring mergeLinesSTD(const std::wstring& text)
  {
      const wchar_t br = L'\n';
      if (text.find(br) == std::wstring::npos)
          return text;

      std::wstring ret;
      bool br_found = false;
      for (auto ch : text)
      {
          if (ch == br)
          {
              br_found = true;
          }
          else if (!br_found || !std::isspace(ch))
          {
              br_found = false;
              ret.push_back(ch);
          }
      }

      return ret;
  }
  /**
   *  @param  text
   *  @param  limit
   *  @param  wordWrap
   *  @param  skipBBCode  skip bbcode such as [ruby]
   */
  static std::wstring limitTextWidthSTD(const std::wstring& text, int limit, bool wordWrap = true, bool skipBBCode = true)
  {
      if (limit <= 0 || text.size() <= limit / 2)
          return text;

      if (skipBBCode)
          skipBBCode = text.find(L'[') != std::wstring::npos;

      const wchar_t br = L'\n';
      int maximumWordSize = limit / 3 + 1;

      std::wstring ret;
      int width = 0;
      int spacePos = -1,
          brPos = -1;
      for (int pos = 0; pos < text.size(); pos++) {
          const wchar_t w = text[pos];
          if (skipBBCode && w == L'[') {
              int closePos = text.find(L']', pos + 1);
              if (closePos != std::wstring::npos) {
                  ret.append(text.substr(pos, closePos - pos + 1));
                  pos = closePos;
                  continue;
              }
          }
          ret.push_back(w);
          if (wordWrap && std::isspace(w)) {
              spacePos = pos;
              if (w == br)
                  brPos = pos;
          }
          width += (w <= 127) ? 1 : 2;
          if (width >= limit) {
              width = 0;
              if (w != br) {
                  if (spacePos > brPos && getTextDistanceSTD(text, spacePos, pos, skipBBCode) < maximumWordSize) {
                      ret[ret.size() - 1 - (pos - spacePos)] = br;
                      brPos = spacePos;
                  }
                  else {
                      ret.push_back(br);
                      brPos = pos;
                  }
              }
          }
          //else if (w == br)
          //  ret[ret.size() - 1] = ' '; // replace '\n' by ' '
      }
      return ret;
  }
   
  std::wstring adjustSpacesSTD(const std::wstring& text,UINT codepage) const
  {
      if (settings.alwaysInsertsSpaces)
          return alwaysInsertSpacesSTD(text);
      if (settings.smartInsertsSpaces) {
          return insertSpacesAfterUnencodableSTD(text, codepage);
      }
      return text;
  } 
   
  static bool isCharTrimmableSTD(const wchar_t& ch) { return ch <= 32 || std::isspace(ch); }

  static std::wstring trimTextSTD(const std::wstring& text, std::wstring* prefix = nullptr, std::wstring* suffix = nullptr)
  {
      if (text.empty() ||
          !isCharTrimmableSTD(text[0]) && !isCharTrimmableSTD(text[text.size() - 1]))
          return text;
      std::wstring ret = text;
      if (isCharTrimmableSTD(ret[0])) {
          size_t pos = 1;
          for (; pos < ret.size() && isCharTrimmableSTD(ret[pos]); pos++);
          if (prefix)
              *prefix = ret.substr(0, pos);
          ret = ret.substr(pos);
      }
      if (!ret.empty() && isCharTrimmableSTD(ret[ret.size() - 1])) {
          size_t pos = ret.size() - 2;
          for (; pos >= 0 && isCharTrimmableSTD(ret[pos]); pos--);
          if (suffix)
              *suffix = ret.substr(pos + 1);
          ret = ret.substr(0, pos + 1);
      }
      return ret;
  }

  std::wstring filterTextSTD(const std::wstring& text, int role) const
  {
      std::wstring ret = text;
      if (model->newLineStringW && ::wcscmp(model->newLineStringW, L"\n"))
          strReplace(ret, model->newLineStringW, L"\n");
      if (model->rubyRemoveFunctionSTD)
          ret = model->rubyRemoveFunctionSTD(ret);
      if (model->textFilterFunctionSTD)
          ret = model->textFilterFunctionSTD(ret, role);
      if (!model->textSeparatorsSTD.empty()) {
          wchar_t s[1];
          for (int i = 0; i < model->textSeparatorsSTD.size(); i++) {
              s[0] = i + 1;
              strReplace(ret, model->textSeparatorsSTD[i], s);
          }
      }
      return ret;
  }
   
  std::wstring filterTranslationSTD(const std::wstring& text, int role) const
  {
      if (text.empty())
          return text;
      std::wstring ret = text;

      
      if (!model->textSeparatorsSTD.empty()) {
          wchar_t s[1];
          for (int i = 0; i < model->textSeparatorsSTD.size(); i++) {
              s[0] = i + 1;
              strReplace(ret, s, model->textSeparatorsSTD[i]);
          }
      }
      // LCScriptEngine requires newLineString must be after textSeparators
      if (model->newLineStringW && ::wcscmp(model->newLineStringW, L"\n") && ret.find(L'\n') == ret.npos)
          strReplace(ret, L"\n", model->newLineStringW);
      if (model->translationFilterFunctionSTD)
          ret = model->translationFilterFunctionSTD(ret, role);
      if (model->enableThin2WideCharacters)
          ret = Util::thin2wideSTD(ret);
      return ret;
  } 
};

EngineController *EngineControllerPrivate::globalInstance;

/** Public class */

// - Detection -

EngineController *EngineController::instance() { return D::globalInstance; }

// - Construction -
#include"stringutil.h"
EngineController::EngineController(EngineModel*model)
  : d_(new D(model))
{ 
  switch (model->encoding) {
  case EngineModel::Utf16Encoding: setEncoding(StringToWideString(ENC_UTF16,CP_ACP)); break;
  case EngineModel::Utf8Encoding: setEncoding(StringToWideString(ENC_UTF8,CP_ACP)); break;
  default: setEncoding(StringToWideString(ENC_SJIS,CP_ACP));
  }
  
}

EngineController::~EngineController() { delete d_; }

EngineSettings *EngineController::settings() const { return &d_->settings; }
EngineModel*EngineController::model() const { return d_->model; }

const std::wstring EngineController::name() const { std::string _ = d_->model->name; return StringToWideString(_,CP_UTF8).c_str(); }

const char *EngineController::encoding() const
{ return Util::encodingForCodePage(d_->codePage); }

bool EngineController::isDynamicEncodingEnabled() const
{ return d_->dynamicEncodingEnabled; }

void EngineController::setDynamicEncodingEnabled(bool t)
{
  if (d_->dynamicEncodingEnabled != t) {
    d_->dynamicEncodingEnabled = t;
    if (t && !d_->dynamicCodec && d_->finalized && d_->model->enableDynamicEncoding)
      d_->dynamicCodec = new DynamicCodec;
  }
}

void EngineController::setCodePage(UINT v)
{
  if (v != d_->codePage) {
    d_->codePage = v;

    if (d_->finalized) { 
      d_->decoder =v;
    }
  }
}

void EngineController::setEncoding(const std::wstring&v)
{ setCodePage(Util::codePageForEncoding(v)); }

//bool EngineController::isTranscodingNeeded() const
//{ return d_->encoder != d_->decoder; }

void EngineController::setSpacePolicyEncoding(const std::wstring &v)
{
    d_->spaceCodec = v.empty() ? 0 : Util::codePageForEncoding(v);
}

std::wstring EngineController::decode(const std::string&v) const { return d_->decode(v); }
std::string EngineController::encode(const std::wstring&v) const { return d_->encode(v); }
 

// - Attach -

bool EngineController::attach()
{
  if (d_->model->attachFunction)
    return d_->model->attachFunction();
  return false;

  //if (!d_->model->searchFunction)
  //  return false;
  //ulong startAddress,
  //      stopAddress;
  //if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
  //  return false;
  //ulong addr = d_->model->searchFunction(startAddress, stopAddress);
  ////ulong addr = startAddress + 0x31850; // 世界と世界の真ん中 体験版
  ////ulong addr = 0x41af90; // レミニセンス function address
  //if (addr) {
  //  DOUT("attached, engine =" << name() << ", absaddr =" << QString::number(addr, 16) << "reladdr =" << QString::number(addr - startAddress, 16));
  //  auto d = d_;
  //  auto callback = [addr, d](winhook::hook_stack *s) -> bool {
  //    if (d->globalDispatchFun)
  //      d->globalDispatchFun((EngineModel::HookStack *)s);
  //    return true;
  //  };
  //  return winhook::hook_before(addr, callback);

  //  //WinDbg::ThreadsSuspender suspendedThreads; // lock all threads to prevent crashing
  //  //d_->oldHookFun = Engine::replaceFunction<Engine::address_type>(addr, ::newHookFun);
  //  return true;
  //}
  //return false;
}

bool EngineController::load()
{
  bool ok = attach();
  if (ok) {
    d_->finalize();
    D::globalInstance = this;
  }
  return ok;
}

bool EngineController::unload() { return false; }

// - Exists -

bool EngineController::match()
{
  /*return d_->model->matchFunction ?
      d_->model->matchFunction() :
      matchFiles(d_->model->matchFiles);*/
  return d_->model->matchFunction?
      d_->model->matchFunction() :
      matchFiles(d_->model->matchFilesSTD);
}
#include"stringutil.h"
bool EngineController::matchFiles(const std::vector<std::wstring>&relpaths)
{
  if (relpaths.empty())
    return false;
  for (const auto &path: relpaths)
    if (path.find(L'|')!=path.npos) {
      bool found = false;
      for(const auto &it: strSplit(path,L"|")) {
        if (Engine::matchFiles(it)) {
          found = true;
          break;
        }
      }
      if (!found)
        return false;
    } else if (!Engine::matchFiles(path))
      return false;

  DOUT("ret = true, relpaths =" << relpaths);
  return true;
}


// - Dispatch -
std::string EngineController::dispatchTextASTD(const std::string& data_, int role, long signature, int maxSize, bool sendAllowed, bool* timeout, bool fromutf8, bool toutf8, bool waitfortrans)
{
    if (timeout)
        *timeout = false;
      if (data_.empty())
        return data_;
      if (!d_->settings.enabled || Engine::isPauseKeyPressed())
        return data_;
      if (!signature)
        signature = Engine::hashThreadSignature(role);
      if (!role)
        role = d_->settings.textRoleOf(signature);
       
      if (role == Engine::OtherRole
          && d_->containsTextHash(Engine::hashByteArraySTD(data_)))
        return data_;
      std::wstring text_;
      if (fromutf8)
          text_ = StringToWideString(data_,CP_UTF8);
       else
          text_ = StringToWideString(data_, d_->codePage);//应该是可变的编码里面，若为qbytearray，会setCodecForLocale。这里暂且用932.
      
      if (text_.empty())
        return data_; 
    if (!d_->model->enableNonDecodableCharacters && (text_.find(L'\xfffd')!= text_.npos))
        return data_; 
            
    std::wstring prefix_,
            suffix_,
            trimmedText_ = D::trimTextSTD(text_, &prefix_, &suffix_);
             
   trimmedText_ = d_->filterTextSTD(trimmedText_, role);   

      if (role == Engine::ScenarioRole && d_->scenarioLineCapacity ||
      role == Engine::OtherRole && d_->otherLineCapacity)
          trimmedText_ = d_->mergeLinesSTD(trimmedText_);
  
  if (trimmedText_.empty()) {
    if (!d_->settings.textVisible[role])
      return "";
    return data_;
  } 
  UINT64 hash = Engine::hashWStringSTD(trimmedText_);

  std::wstring language_, repl_;

  auto p = EmbedManager::instance(); 
  if (role == Engine::OtherRole) { // skip sending text
    if (!d_->settings.textVisible[role])
      return "";
    if (!d_->settings.translationEnabled[role] || !Util::needsTranslationSTD(trimmedText_))
      return data_;
    repl_ = p->findTranslationSTD(hash, role, &language_);
  }
  
  bool sent = false;
  bool needsTranslation = false;
  if (sendAllowed
      && !d_->settings.translationEnabled[role]
      && (d_->settings.extractionEnabled[role] || d_->settings.extractsAllTexts)
      && (role != Engine::OtherRole || repl_.empty())) {
    p->sendTextSTD(trimmedText_, hash, signature, role, needsTranslation);
    sent = true;
  }
    if (!d_->settings.textVisible[role])
    return "";
  if (!d_->settings.translationEnabled[role])
    return d_->settings.transcodingEnabled[role] ?   WideStringToString(text_, d_->codePage) : data_;
  
  if (repl_.empty())
    repl_ = p->findTranslationSTD(hash, role, &language_);
   
  if (sendAllowed && !sent) {
    needsTranslation = repl_.empty();
    if (role != Engine::OtherRole || needsTranslation) {
      p->sendTextSTD(trimmedText_, hash, signature, role, needsTranslation);
      sent = true;
    }
  } 
  if (!waitfortrans)return "";
  if (sent && needsTranslation)
    repl_ = p->waitForTranslationSTD(hash, role, &language_); 
  /// 
    if (repl_.empty()) {
    if (timeout)
      *timeout = true;
    repl_ = trimmedText_;
  } else if (repl_ != trimmedText_) {
    if (!repl_.empty() && d_->model->newLineStringW) { 
      bool wordWrap = Util::languageNeedsWordWrapSTD(language_);
      bool containsBBCode = role == Engine::ScenarioRole && d_->model->rubyRemoveFunctionSTD;
      if (role == Engine::ScenarioRole) { 
        if (d_->scenarioLineCapacity) { 
          int capacity = D::getLineCapacity(d_->filterTextSTD(text_, role).c_str());
          if (d_->scenarioLineCapacity < capacity)
            d_->scenarioLineCapacity = capacity;
          repl_ = d_->limitTextWidthSTD(repl_, d_->scenarioLineCapacity, wordWrap, containsBBCode);
        } else if (d_->settings.scenarioWidth)
            repl_ = d_->limitTextWidthSTD(repl_, d_->settings.scenarioWidth, wordWrap, containsBBCode);
      } else if (role == Engine::OtherRole && d_->otherLineCapacity) { 
        int capacity = D::getLineCapacity(d_->filterTextSTD(text_, role).c_str());
        if (d_->otherLineCapacity < capacity)
          d_->otherLineCapacity = capacity;
        repl_ = d_->limitTextWidthSTD(repl_, d_->otherLineCapacity, wordWrap, containsBBCode);
      }
    } 
    repl_ = d_->filterTranslationSTD(repl_, role);
    switch (role) {
    case Engine::ScenarioRole:
      if (d_->settings.scenarioTextVisible)
          repl_.append(d_->model->newLineStringW ? d_->model->newLineStringW : L" ")
            .append(trimmedText_);
      break;
    case Engine::NameRole:
      if (d_->settings.nameTextVisible)
          repl_.append(d_->model->separatorStringW ? d_->model->separatorStringW : L" ")
            .append(trimmedText_);
      break;
    case Engine::OtherRole:
      if (d_->settings.otherTextVisible)
          repl_.append(d_->model->separatorStringW ? d_->model->separatorStringW : L" ")
            .append(trimmedText_);
      break;
    }
  }
   
  repl_ = d_->adjustSpacesSTD(repl_, d_->spaceCodec);

    std::string ret;
  if (maxSize > 0) {
    std::string prefixData,
               suffixData;
    if (!prefix_.empty())
      prefixData = WideStringToString(prefix_, d_->codePage);             /////////////////////////////
    if (!suffix_.empty())
      suffixData = WideStringToString(suffix_, d_->codePage);
    if (fromutf8)
        ret = WideStringToString(repl_, CP_UTF8);
    else
         ret = (d_->dynamicEncodingEnabled && d_->dynamicCodec) ? d_->dynamicCodec->encodeSTD(repl_)
        : WideStringToString(suffix_, GetACP()); 
    int capacity = maxSize - prefixData.size() - suffixData.size(); // excluding trailing \0
    if (capacity < ret.size()) {
      if (capacity >= 2) {
        const char *end = Hijack::oldCharPrevA(ret.c_str(), ret.c_str() + capacity);
        ret = ret.substr(0,end - data_.c_str());
      } else
        ret = ret.substr(0,capacity);
    }



    if (!prefixData.empty())
      ret.insert(0,prefixData);
    if (!suffixData.empty())
      ret.append(suffixData);
  } else {
    if (!prefix_.empty())
      repl_.insert(0, prefix_);
    if (!suffix_.empty())
      repl_.append(suffix_);
    if ( toutf8)
        ret = WideStringToString(repl_, CP_UTF8); 
    else
        ret = (d_->dynamicEncodingEnabled && d_->dynamicCodec) ? d_->dynamicCodec->encodeSTD(repl_)
        : WideStringToString(suffix_, GetACP());
     
  } 
  if (role == Engine::OtherRole)
    d_->addTextHash(Engine::hashByteArraySTD(ret)); 
   
  return ret;
  
}
//QByteArray EngineController::dispatchTextA(const QByteArray& data, int role, long signature, int maxSize, bool sendAllowed, bool* timeout, bool fromutf8, bool toutf8, bool waitfortrans)
//{
//    if (timeout)
//        *timeout = false;
//    if (data.isEmpty())
//        return data;
//    if (!d_->settings.enabled || Engine::isPauseKeyPressed())
//        return data;
//    if (!signature)
//        signature = Engine::hashThreadSignature(role);
//    if (!role)
//        role = d_->settings.textRoleOf(signature);
//
//    if (role == Engine::OtherRole
//        && d_->containsTextHash(Engine::hashByteArray(data)))
//        return data;
//    QString text;
//    if (fromutf8)
//        text = QString::fromUtf8(data);
//    else
//        text = d_->decode(data);
//    if (text.isEmpty())
//        return data;
//    if (!d_->model->enableNonDecodableCharacters && text.contains(L'\xfffd'))
//        return data;
//    QString prefix,
//        suffix,
//        trimmedText = D::trimText(text, &prefix, &suffix);
//     
//    trimmedText = d_->filterText(trimmedText, role);
//
//    if (role == Engine::ScenarioRole && d_->scenarioLineCapacity ||
//        role == Engine::OtherRole && d_->otherLineCapacity)
//        trimmedText = d_->mergeLines(trimmedText);
//
//    if (trimmedText.isEmpty()) {
//        if (!d_->settings.textVisible[role])
//            return QByteArray();
//        return data;
//    }
//
//    UINT64 hash = Engine::hashWString(trimmedText);
//
//    QString language;
//
//    auto p = EmbedManager::instance();
//    QString repl;
//    if (role == Engine::OtherRole) { // skip sending text
//        if (!d_->settings.textVisible[role])
//            return QByteArray();
//        if (!d_->settings.translationEnabled[role] || !Util::needsTranslation(trimmedText))
//            return data;
//        repl = p->findTranslation(hash, role, &language);
//    }
//
//    bool sent = false;
//    bool needsTranslation = false;
//    if (sendAllowed
//        && !d_->settings.translationEnabled[role]
//        && (d_->settings.extractionEnabled[role] || d_->settings.extractsAllTexts)
//        && (role != Engine::OtherRole || repl.isEmpty())) {
//        p->sendText(trimmedText, hash, signature, role, needsTranslation);
//        sent = true;
//    }
//
//    if (!d_->settings.textVisible[role])
//        return QByteArray();
//    if (!d_->settings.translationEnabled[role])
//        return d_->settings.transcodingEnabled[role] ? d_->encode(text) : data;
//
//    if (repl.isEmpty())
//        repl = p->findTranslation(hash, role, &language);
//
//    if (sendAllowed && !sent) {
//        needsTranslation = repl.isEmpty();
//        if (role != Engine::OtherRole || needsTranslation) {
//            p->sendText(trimmedText, hash, signature, role, needsTranslation);
//            sent = true;
//        }
//    }
//    if (!waitfortrans)return "";
//    if (sent && needsTranslation)
//        repl = p->waitForTranslation(hash, role, &language);
//
//    if (repl.isEmpty()) {
//        if (timeout)
//            *timeout = true;
//        repl = trimmedText;
//    }
//    else if (repl != trimmedText) {
//        if (!repl.isEmpty() && d_->model->newLineString) {
//            bool wordWrap = Util::languageNeedsWordWrap(language);
//            bool containsBBCode = role == Engine::ScenarioRole && d_->model->rubyCreateFunction;
//            if (role == Engine::ScenarioRole) {
//                if (d_->scenarioLineCapacity) {
//                    int capacity = D::getLineCapacity(d_->filterText(text, role));
//                    if (d_->scenarioLineCapacity < capacity)
//                        d_->scenarioLineCapacity = capacity;
//                    repl = d_->limitTextWidth(repl, d_->scenarioLineCapacity, wordWrap, containsBBCode);
//                }
//                else if (d_->settings.scenarioWidth)
//                    repl = d_->limitTextWidth(repl, d_->settings.scenarioWidth, wordWrap, containsBBCode);
//            }
//            else if (role == Engine::OtherRole && d_->otherLineCapacity) {
//                int capacity = D::getLineCapacity(d_->filterText(text, role));
//                if (d_->otherLineCapacity < capacity)
//                    d_->otherLineCapacity = capacity;
//                repl = d_->limitTextWidth(repl, d_->otherLineCapacity, wordWrap, containsBBCode);
//            }
//        }
//        repl = d_->filterTranslation(repl, role);
//        switch (role) {
//        case Engine::ScenarioRole:
//            if (d_->settings.scenarioTextVisible)
//                repl.append(d_->model->newLineString ? d_->model->newLineString : " ")
//                .append(trimmedText);
//            break;
//        case Engine::NameRole:
//            if (d_->settings.nameTextVisible)
//                repl.append(d_->model->separatorString ? d_->model->separatorString : " ")
//                .append(trimmedText);
//            break;
//        case Engine::OtherRole:
//            if (d_->settings.otherTextVisible)
//                repl.append(d_->model->separatorString ? d_->model->separatorString : " ")
//                .append(trimmedText);
//            break;
//        }
//    }
//
//    repl = d_->adjustSpaces(repl);
//
//    QByteArray ret;
//    if (maxSize > 0) {
//        QByteArray prefixData,
//            suffixData;
//        if (!prefix.isEmpty())
//            prefixData = d_->encode(prefix);
//        if (!suffix.isEmpty())
//            suffixData = d_->encode(suffix);
//        if (fromutf8)
//            ret = repl.toUtf8();
//        else
//            ret = (d_->dynamicEncodingEnabled && d_->dynamicCodec) ? d_->dynamicCodec->encode(repl)
//            : d_->encode(repl);
//        int capacity = maxSize - prefixData.size() - suffixData.size(); // excluding trailing \0
//        if (capacity < ret.size()) {
//            if (capacity >= 2) {
//                const char* end = Hijack::oldCharPrevA(ret.constData(), ret.constData() + capacity);
//                ret = ret.left(end - data.constData());
//            }
//            else
//                ret = ret.left(capacity);
//        }
//
//        if (!prefixData.isEmpty())
//            ret.prepend(prefixData);
//        if (!suffixData.isEmpty())
//            ret.append(suffixData);
//    }
//    else {
//        if (!prefix.isEmpty())
//            repl.prepend(prefix);
//        if (!suffix.isEmpty())
//            repl.append(suffix);
//        if (toutf8)
//            ret = repl.toUtf8();
//        else
//            ret = (d_->dynamicEncodingEnabled && d_->dynamicCodec) ? d_->dynamicCodec->encode(repl)
//            : d_->encode(repl);
//    }
//
//    if (role == Engine::OtherRole)
//        d_->addTextHash(Engine::hashByteArray(ret));
//    return ret;
//}
//QString EngineController::dispatchTextW(const QString &text, int role, long signature, int maxSize, bool sendAllowed, bool *timeout)
//{
//  if (timeout)
//    *timeout = false;
//  if (text.isEmpty())
//    return text;
//
//  // Canceled
//  if (!d_->settings.enabled || Engine::isPauseKeyPressed())
//    return text;
//
//  if (!signature)
//    signature = Engine::hashThreadSignature(role);
//  if (!role)
//    role = d_->settings.textRoleOf(signature);
//
//  if (role == Engine::OtherRole
//      && d_->containsTextHash(Engine::hashWString(text)))
//    return text;
//
//  QString prefix,
//          suffix,
//          trimmedText = D::trimText(text, &prefix, &suffix);
//  trimmedText = d_->filterText(trimmedText, role);
//  if (role == Engine::ScenarioRole && d_->scenarioLineCapacity ||
//      role == Engine::OtherRole && d_->otherLineCapacity)
//    trimmedText = d_->mergeLines(trimmedText);
//
//  if (trimmedText.isEmpty()) {
//    if (!d_->settings.textVisible[role])
//      return QString();
//    return text;
//  }
//
//  UINT64 hash = Engine::hashWString(trimmedText);
//
//  QString language;
//
//  auto p = EmbedManager::instance();
//  QString repl;
//  if (role == Engine::OtherRole) { // skip sending text
//    if (!d_->settings.textVisible[role])
//      return QString();
//    if (!d_->settings.translationEnabled[role] || !Util::needsTranslation(trimmedText))
//      return text;
//    repl = p->findTranslation(hash, role, &language);
//  }
//
//  bool sent = false;
//  bool needsTranslation = false;
//  if (sendAllowed
//      && !d_->settings.translationEnabled[role]
//      && (d_->settings.extractionEnabled[role] || d_->settings.extractsAllTexts)
//      && (role != Engine::OtherRole || repl.isEmpty())) {
//    p->sendText(trimmedText, hash, signature, role, needsTranslation);
//    sent = true;
//  }
//
//  if (!d_->settings.textVisible[role])
//    return QString();
//  if (!d_->settings.translationEnabled[role])
//    return text;
//
//  if (repl.isEmpty())
//    repl = p->findTranslation(hash, role, &language);
//
//  if (sendAllowed && !sent) {
//    needsTranslation = repl.isEmpty();
//    if (role != Engine::OtherRole || needsTranslation) {
//      p->sendText(trimmedText, hash, signature, role, needsTranslation);
//      sent = true;
//    }
//  }
//  if (sent && needsTranslation)
//    repl = p->waitForTranslation(hash, role, &language);
//
//  if (repl.isEmpty()) {
//    if (timeout)
//      *timeout = true;
//    repl = trimmedText; // prevent from deleting text
//  } else if (repl != trimmedText) {
//    if (!repl.isEmpty() && d_->model->newLineString) {
//      bool wordWrap = Util::languageNeedsWordWrap(language);
//      bool containsBBCode = role == Engine::ScenarioRole && d_->model->rubyCreateFunction;
//      if (role == Engine::ScenarioRole) {
//        if (d_->scenarioLineCapacity) {
//          int capacity = D::getLineCapacity(d_->filterText(text, role));
//          if (d_->scenarioLineCapacity < capacity)
//            d_->scenarioLineCapacity = capacity;
//          repl = d_->limitTextWidth(repl, d_->scenarioLineCapacity, wordWrap, containsBBCode);
//        } else if (d_->settings.scenarioWidth)
//          repl = d_->limitTextWidth(repl, d_->settings.scenarioWidth, wordWrap, containsBBCode);
//      } else if (role == Engine::OtherRole && d_->otherLineCapacity) {
//        int capacity = D::getLineCapacity(d_->filterText(text, role));
//        if (d_->otherLineCapacity < capacity)
//          d_->otherLineCapacity = capacity;
//        repl = d_->limitTextWidth(repl, d_->otherLineCapacity, wordWrap, containsBBCode);
//      }
//    }
//    repl = d_->filterTranslation(repl, role);
//    switch (role) {
//    case Engine::ScenarioRole:
//      if (d_->settings.scenarioTextVisible)
//        repl.append(d_->model->newLineString ? d_->model->newLineString : " ")
//            .append(trimmedText);
//      break;
//    case Engine::NameRole:
//      if (d_->settings.nameTextVisible)
//        repl.append(d_->model->separatorString ? d_->model->separatorString : " ")
//            .append(trimmedText);
//      break;
//    case Engine::OtherRole:
//      if (d_->settings.otherTextVisible)
//        repl.append(d_->model->separatorString ? d_->model->separatorString : " ")
//            .append(trimmedText);
//        //repl = QString("%1 / %2").arg(repl, trimmedText);
//      break;
//    }
//  }
//
//  repl = d_->adjustSpaces(repl);
//
//  if (maxSize > 0 && maxSize < repl.size() + prefix.size() + suffix.size())
//    repl = repl.left(maxSize - prefix.size() - suffix.size());
//
//  if (!prefix.isEmpty())
//    repl.prepend(prefix);
//  if (!suffix.isEmpty())
//    repl.append(suffix);
//
//  if (role == Engine::OtherRole)
//    d_->addTextHash(Engine::hashWString(repl));
//  return repl;
//}
std::wstring EngineController::dispatchTextWSTD(const std::wstring& data_, int role, long signature, int maxSize, bool sendAllowed, bool* timeout)
{
    if (timeout)
        *timeout = false;
    if (data_.empty())
        return data_;
    if (!d_->settings.enabled || Engine::isPauseKeyPressed())
        return data_;
    if (!signature)
        signature = Engine::hashThreadSignature(role);
    if (!role)
        role = d_->settings.textRoleOf(signature);


    if (role == Engine::OtherRole
        && d_->containsTextHash(Engine::hashWStringSTD(data_)))
        return data_;

    auto text_ = data_;
    std::wstring prefix_,
        suffix_,
        trimmedText_ = D::trimTextSTD(text_, &prefix_, &suffix_);
    trimmedText_ = d_->filterTextSTD(trimmedText_, role);
    if (role == Engine::ScenarioRole && d_->scenarioLineCapacity ||
        role == Engine::OtherRole && d_->otherLineCapacity)
        trimmedText_ = d_->mergeLinesSTD(trimmedText_);

    if (trimmedText_.empty()) {
        if (!d_->settings.textVisible[role])
            return L"";
        return data_;
    }

    UINT64 hash = Engine::hashWStringSTD(trimmedText_);

    std::wstring language_, repl_;

    auto p = EmbedManager::instance();
    if (role == Engine::OtherRole) { // skip sending text
        if (!d_->settings.textVisible[role])
            return L"";
        if (!d_->settings.translationEnabled[role] || !Util::needsTranslationSTD(trimmedText_))
            return data_;
        repl_ = p->findTranslationSTD(hash, role, &language_);
    }

    bool sent = false;
    bool needsTranslation = false;
    if (sendAllowed
        && !d_->settings.translationEnabled[role]
        && (d_->settings.extractionEnabled[role] || d_->settings.extractsAllTexts)
        && (role != Engine::OtherRole || repl_.empty())) {
        p->sendTextSTD(trimmedText_, hash, signature, role, needsTranslation);
        sent = true;
    }

    if (!d_->settings.textVisible[role])
        return L"";
    if (!d_->settings.translationEnabled[role])
        return text_;

    if (repl_.empty())
        repl_ = p->findTranslationSTD(hash, role, &language_);

    if (sendAllowed && !sent) {
        needsTranslation = repl_.empty();
        if (role != Engine::OtherRole || needsTranslation) {
            p->sendTextSTD(trimmedText_, hash, signature, role, needsTranslation);
            sent = true;
        }
    } 
    if (sent && needsTranslation)
        repl_ = p->waitForTranslationSTD(hash, role, &language_);

    if (repl_.empty()) {
        if (timeout)
            *timeout = true;
        repl_ = trimmedText_;
    }
    else if (repl_ != trimmedText_) {
        if (!repl_.empty() && d_->model->newLineStringW) {
            bool wordWrap = Util::languageNeedsWordWrapSTD(language_);
            bool containsBBCode = role == Engine::ScenarioRole && d_->model->rubyRemoveFunctionSTD;
            if (role == Engine::ScenarioRole) {
                if (d_->scenarioLineCapacity) {
                    int capacity = D::getLineCapacity(d_->filterTextSTD(text_, role).c_str());
                    if (d_->scenarioLineCapacity < capacity)
                        d_->scenarioLineCapacity = capacity;
                    repl_ = d_->limitTextWidthSTD(repl_, d_->scenarioLineCapacity, wordWrap, containsBBCode);
                }
                else if (d_->settings.scenarioWidth)
                    repl_ = d_->limitTextWidthSTD(repl_, d_->settings.scenarioWidth, wordWrap, containsBBCode);
            }
            else if (role == Engine::OtherRole && d_->otherLineCapacity) {
                int capacity = D::getLineCapacity(d_->filterTextSTD(text_, role).c_str());
                if (d_->otherLineCapacity < capacity)
                    d_->otherLineCapacity = capacity;
                repl_ = d_->limitTextWidthSTD(repl_, d_->otherLineCapacity, wordWrap, containsBBCode);
            }
        }
        repl_ = d_->filterTranslationSTD(repl_, role);
        switch (role) {
        case Engine::ScenarioRole:
            if (d_->settings.scenarioTextVisible)
                repl_.append(d_->model->newLineStringW ? d_->model->newLineStringW : L" ")
                .append(trimmedText_);
            break;
        case Engine::NameRole:
            if (d_->settings.nameTextVisible)
                repl_.append(d_->model->separatorStringW ? d_->model->separatorStringW : L" ")
                .append(trimmedText_);
            break;
        case Engine::OtherRole:
            if (d_->settings.otherTextVisible)
                repl_.append(d_->model->separatorStringW ? d_->model->separatorStringW : L" ")
                .append(trimmedText_);
            break;
        }
    }

    repl_ = d_->adjustSpacesSTD(repl_, d_->spaceCodec);

    if (maxSize > 0 && maxSize < repl_.size() + prefix_.size() + suffix_.size())
        repl_ = repl_.substr(0,maxSize - prefix_.size() - suffix_.size());

    if (!prefix_.empty())
        repl_.insert(0,prefix_);
    if (!suffix_.empty())
        repl_.insert(0, suffix_);

    if (role == Engine::OtherRole)
        d_->addTextHash(Engine::hashWStringSTD(repl_));
    return repl_;
}
// EOF

/*
// - Exchange -

// Qt is not allowed to appear in this function
const char *EngineController::exchangeTextA(const char *data, long signature, int role)
{
  auto d_mem = d_->exchangeMemory;
  if (!d_mem || !data)
    return data;

  ulong key = ::GetTickCount();
  d_mem->setRequestStatus(EngineSharedMemory::BusyStatus);
  d_mem->setRequestKey(key);
  d_mem->setRequestSignature(signature);
  d_mem->setRequestRole(role);
  d_mem->setRequestText(data);
  d_mem->setRequestStatus(EngineSharedMemory::ReadyStatus);

  // Spin lock
  while (d_mem->responseKey() != key ||
         d_mem->responseStatus() != EngineSharedMemory::ReadyStatus) {
    if (d_mem->responseStatus() == EngineSharedMemory::CancelStatus)
      return data;
    ::Sleep(D::ExchangeInterval);
  }
  return d_mem->responseText();
}

void EngineControllerPrivate::exchange()
{
  if (!exchangeMemory)
    return;
  if (exchangeMemory->requestStatus() == EngineSharedMemory::ReadyStatus) {
    exchangeMemory->setRequestStatus(EngineSharedMemory::EmptyStatus);
    if (auto req = exchangeMemory->requestText()) {
      auto key = exchangeMemory->requestKey();
      auto role = exchangeMemory->requestRole();
      auto sig = exchangeMemory->requestSignature();
      QByteArray resp = q_->dispatchTextA(req, role, sig);
      exchangeMemory->setResponseStatus(EngineSharedMemory::BusyStatus);
      exchangeMemory->setResponseText(resp);
      exchangeMemory->setResponseRole(role);
      exchangeMemory->setResponseKey(key);
      exchangeMemory->setResponseStatus(EngineSharedMemory::ReadyStatus);
    }
  }
}



typedef int (WINAPI *MultiByteToWideCharFun)(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);
typedef int (WINAPI *WideCharToMultiByteFun)(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar);
MultiByteToWideCharFun oldMultiByteToWideChar;
WideCharToMultiByteFun oldWideCharToMultiByte;
int WINAPI newMultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
{
  if (CodePage == 932)
    CodePage = 936;
  return ::oldMultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
}

int WINAPI newWideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar)
{
  if (CodePage == 932)
    CodePage = 936;
  return ::oldWideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
}
    ::oldMultiByteToWideChar = Engine::replaceFunction<MultiByteToWideCharFun>(addr, ::newMultiByteToWideChar);
    ::oldWideCharToMultiByte = Engine::replaceFunction<WideCharToMultiByteFun>(addr, ::newWideCharToMultiByte);
*/

/**
 *  The stack must be consistent with struct HookStack
 *
 *  Note for detours
 *  - It simply replaces the code with jmp and int3. Jmp to newHookFun
 *  - oldHookFun is the address to a code segment that jmp back to the original function
 */
/*
__declspec(naked) static int newHookFun()
{
  // The push order must be consistent with struct HookStack in enginemodel.h
  //static DWORD lastArg2;
  __asm // consistent with struct HookStack
  {
    //pushfd      // 5/25/2015: pushfd twice according to ith, not sure if it is really needed
    pushad      // increase esp by 0x20 = 4 * 8, push ecx for thiscall is enough, though
    pushfd      // eflags
    push esp    // arg1
    call EngineControllerPrivate::globalDispatchFun
    //add esp,4   // pop esp
    popfd
    popad
    //popfd
    // TODO: instead of jmp, allow modify the stack after calling the function
    jmp EngineControllerPrivate::globalOldHookFun
  }
}
*/
