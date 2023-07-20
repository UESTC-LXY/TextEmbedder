#pragma once

// rpcclient_p.h
// 2/1/2013 jichi

#include "config.h"
#include "sakurakit/skglobal.h"  
#include<vector>
#include<string>
#include <sakurakit/skdebug.h> 

class LocalSocketClient;
typedef LocalSocketClient RpcSocketClient;

class RpcClient;
class RpcClientPrivate 
{  
  SK_DECLARE_PUBLIC(RpcClient) 
       
  //enum { BufferInterval = 20 }; // wait for next buffered data
  enum { WaitInterval = 5000 }; // wait for data sent
public:
  explicit RpcClientPrivate(Q *q); 

  bool appQuit; // the application has quit

  RpcSocketClient *client; 
   

  void start(); // This method is only called once on the startup
private  : 
  void onDataReceived(const std::string&data);

private:
  void onConnected();
  void pingServer();

  void onCall(const std::vector<std::wstring>&args); // called from server
   
   
  void callServerSTD(const std::vector<std::wstring>& args); // call server
  void sendData(const std::string &data);
   

  void callServerSTD(const std::wstring& arg0, const std::wstring& arg1)
  {
      callServerSTD({ arg0 , arg1 });
  }


public:
     

  void sendWindowTexts(const std::wstring &json) { callServerSTD(L"agent.window.text", json); }

  void sendEngineName(const std::wstring &name) { callServerSTD(L"agent.engine.name", name); }

   
  void sendEngineTextSTD(const std::wstring& text, UINT64 hash, long signature, int role, bool needsTranslation)
  {
      std::vector<std::wstring> _ = { L"agent.engine.text",
           text,
            std::to_wstring(hash),
            std::to_wstring(signature),
          std::to_wstring(role),
            std::to_wstring(needsTranslation) };
      callServerSTD(_);
  }
};

// EOF

/*
class RpcRouter : public MetaCallRouter
{
public:
  int convertSendMethodId(int value) override
  {
    switch (value) {
    case 6: return 16; // pingServer
    case 8: return 10; // pingClient
    case 10: return 14; // updateServerString
    default: return value;
    }
  }

  int convertReceiveMethodId(int value) override
  {
    switch (value) {
    case 12: return 12; // updateClientString
    default: return value;
    }
  }
};

*/
