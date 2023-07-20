#pragma once

// rpcclient.h
// 2/1/2013 jichi
#include<Windows.h>
#include "sakurakit/skglobal.h" 
#include<string>
#include<functional>
class RpcClientPrivate;
class RpcClient {
	SK_DECLARE_PRIVATE(RpcClientPrivate)

public:
	static RpcClient* instance();
	explicit RpcClient( );
	~RpcClient();
	bool isActive() const;
	void quit();

  // - API -
	std::function<void()>detachRequested;
	std::function<void()>disableRequested;
	std::function<void()>disconnected;
	std::function<void()>aborted;
	std::function<void(std::wstring)>settingsReceived;
	std::function<void()>clearWindowTranslationRequested;
	std::function<void(bool)>enableWindowTranslationRequested;
	std::function<void(std::wstring)>windowTranslationReceived;
	std::function<void()>clearEngineRequested;
	std::function<void(bool)>enableEngineRequested;
 
public  :
  void sendEngineName(const std::wstring &name); 
  void requestWindowTranslation(const std::wstring &json); // json: {hash:text}
  

  // Direct IO bypassing Qt, blocking
public:
	void sendEngineTextSTD(const std::wstring& text, UINT64 hash, long signature, int role, bool); 
  void  directSendEngineTextSTD(const std::wstring& text, UINT64 hash, long signature, int role, bool);
};

// EOF
