#pragma once

// enginecontroller.h
// 4/20/2014 jichi

#include "sakurakit/skglobal.h" 
#include<string>
#include<vector>
#include<Windows.h>
class EngineModel;
class EngineSettings;
class EngineControllerPrivate;


class EngineController
{
  //Q_OBJECT
  SK_CLASS(EngineController)
  SK_DECLARE_PRIVATE(EngineControllerPrivate)
  SK_DISABLE_COPY(EngineController)

public:
  // Successful engine controller
  // TODO: get rid of global instance
  // Need to write my own ASM bytes to achieve C partial functions
  static Self *instance();

  ///  Model cannot be null. Engine does NOT take the ownership of the model.
  explicit EngineController(EngineModel*model);
  ~EngineController();

  EngineSettings *settings() const;
  EngineModel*model() const;

  const std::wstring name() const;

  const char *encoding() const;
  void setEncoding(const std::wstring &v);
  std::string encode(const std::wstring&text) const;
  std::wstring decode(const std::string&text) const;

  //bool encodable(const QString &text) const; 
   

  // Encoding for inserting spaces
  void setSpacePolicyEncoding(const std::wstring &v);

  //bool isTranscodingNeeded() const;

  bool match(); // match files
  bool load();
  bool unload();

  //static bool isEnabled();
  //static void setEnabled(bool t);

protected:
  UINT codePage() const;
  void setCodePage(UINT cp);

  bool isDynamicEncodingEnabled() const;
  void setDynamicEncodingEnabled(bool t);

  bool attach(); ///< Invoked by load
  //bool detach(); ///< Invoked by unload

  // Hook

  ///  Return whether relpaths exist
  static bool matchFiles(const std::vector<std::wstring>&relpaths);

  // Interface to descendant classes, supposed to be protected
public:
  ///  Send LPCSTR text to VNR, timeout == true iff translation timeout. MaxSize is the maximum data size excluding \0
  
  std::string dispatchTextASTD(const std::string& data, int role, long signature = 0, int maxSize = 0, bool sendAllowed = true, bool* timeout = nullptr, bool fromutf8 = false, bool toutf8 = false, bool waitfortrans = true);
   
  std::wstring dispatchTextWSTD(const std::wstring& text, int role, long signature = 0, int maxSize = 0, bool sendAllowed = true, bool* timeout = nullptr);
  // This function is not thread-safe
  //const char *exchangeTextA(const char *data, long signature, int role = 0);
};

// EOF
