// rpcclient.cc
// 2/1/2013 jichi
#include "config.h"
#include "driver/rpcclient.h"
#include "driver/rpcclientprivate.h"
#include "qtsocketsvc/socketpack.h" 
 
# include "qtsocketsvc/localsocketclient.h" 

#define DEBUG "rpcclient"
#include "sakurakit/skdebug.h"
#include<vector>
#include<string>
#include<Windows.h>
/** Private class */

RpcClientPrivate::RpcClientPrivate(Q *q)
  :   q_(q), appQuit(false), client(new RpcSocketClient( )) 
{ 

    client->dataReceived = std::bind(&RpcClientPrivate::onDataReceived, this, std::placeholders::_1);
     
}
 
#include<thread>
void RpcClientPrivate::start()
{
  //if (client->isConnected())
  //  return true;
  //client->stop();
    std::thread([this]() {
        while (true) {
            client->wait();
            client->start();

            onConnected(); 
        }
         }).detach();
}
 
void RpcClientPrivate::onConnected()
{
  DOUT("connected"); 
  pingServer();
}

void RpcClientPrivate::pingServer()
{ 
  callServerSTD(L"agent.ping",std::to_wstring(GetCurrentProcessId()));
}
void RpcClientPrivate::callServerSTD(const std::vector<std::wstring>& args) {
    if (client->isConnected()) {
        for (auto s : args) {
            DOUT(s);
        } 
        sendData(SocketService::packStringListSTD(args, CP_UTF8));
    }
}
 
 

void RpcClientPrivate::sendData(const std::string &data)
{ client->sendData(data, WaitInterval); }
 
void RpcClientPrivate::onDataReceived(const std::string&data)
{
  auto l = SocketService::unpackStringListSTD(data);
  /*for (auto _l : l) { 
      MessageBoxA(0, _l.toUtf8().data(), "", 0);
  }*/
  if (!l.empty())
    onCall(l);
}

void RpcClientPrivate::onCall(const std::vector<std::wstring> &args)
{
  if (args.empty()) return;
  //DOUT("cmd:" << args.first());
  auto arg = args[0];
  if (arg == L"ping") {}
  else if (arg == L"detach") q_->detachRequested(); 
  //else if (arg == "enable") q_->emit enableRequested(true);
  else if (arg == L"disable") q_->disableRequested();
  else if (arg == L"settings")
  {
    if (args.size() == 2) q_->settingsReceived(args[1]);
  }
  //else if (arg == "window.clear") q_->emit clearWindowTranslationRequested();
  //else if (arg == "window.enable") q_->emit enableWindowTranslationRequested(true);
  //else if (arg == "window.disable") q_->emit enableWindowTranslationRequested(false);
  else if (arg == L"windows.text")
  {
    if (args.size() == 2) q_->windowTranslationReceived(args[1]);
  }
  //else if (arg == "engine.clear") q_->emit clearEngineRequested();
  //else if (arg == "engine.enable") q_->emit enableEngineRequested(true);
  //else if (arg == "engine.disable") q_->emit enableEngineRequested(false);
   
  else {} //growl::debug(QString("Unknown command: %s").arg(cmd)); 
}

static RpcClient* instance_;
RpcClient* RpcClient::instance() { return ::instance_; }

RpcClient::RpcClient( )
    :  d_(new D(this))
{
    d_->start();
    ::instance_ = this;
}
RpcClient::~RpcClient() { ::instance_ = nullptr; }

bool RpcClient::isActive() const { return d_->client->isConnected(); }

void RpcClient::quit() { d_->appQuit = true; }

// - Requests -

void RpcClient::requestWindowTranslation(const std::wstring &json) { d_->sendWindowTexts(json); }

void RpcClient::sendEngineName(const std::wstring &name)
{ d_->sendEngineName(name); }


void RpcClient::sendEngineTextSTD(const std::wstring& text, UINT64 hash, long signature, int role, bool needsTranslation)
{
    d_->sendEngineTextSTD(text, hash, signature, role, needsTranslation);
}

void RpcClient::directSendEngineTextSTD(const std::wstring& text, UINT64 hash, long signature, int role, bool needsTranslation)
{
    d_->sendEngineTextSTD(text, hash, signature, role, needsTranslation);
}
//void RpcClient::sendEngineTextLater(const QString &text, UINT64 hash, int role, bool needsTranslation)
//{ d_->sendEngineTextLater(text, hash, role, needsTranslation); }
  

// EOF
