// maindriver_p.h
// 2/1/2013 jichi
#include "sakurakit/skglobal.h" 

class AbstractEngine;
class EmbedDriver;
class HijackDriver;
class RpcClient;
class Settings;
class WindowDriver;
class MainDriverPrivate  
{  
  SK_EXTEND_CLASS(MainDriverPrivate, void)

  Settings *settings;
  RpcClient *rpc;
  HijackDriver *hijack;
  EmbedDriver *eng;
  WindowDriver *win;
public:
  explicit MainDriverPrivate(void *parent);
  void quit();
    

private  :
  void onDisconnected();
  void unload();

  void onLoadFinished();

private:
  void createHijackDriver();
  void createEmbedDriver();
  void createWindowDriver();
};

// EOF
