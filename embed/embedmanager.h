#pragma once

// embedmanager.h
// 4/26/2014 jichi
// Embedded game engine text manager.

#include "sakurakit/skglobal.h"
#include<Windows.h>
class EmbedManagerPrivate;
class EmbedManager  
{ 
  SK_EXTEND_CLASS(EmbedManager, void)
  SK_DECLARE_PRIVATE(EmbedManagerPrivate)

public:
  static Self *instance(); // needed by Engine

  explicit EmbedManager(void*parent = nullptr);
  ~EmbedManager(); 
   
public: // synchronized methods
  //void updateTranslation(const QString &text, UINT64 hash, int role); 
  void quit();

  // Interface to engine
public: // unsynchronized methods
  std::wstring findTranslationSTD(UINT64 hash, int role, std::wstring* language) const;
  void setTranslationWaitTime(int msecs);
  std::wstring waitForTranslationSTD(UINT64 hash, int role, std::wstring* language) const;
   
  void sendTextSTD(const std::wstring& text, UINT64 hash, long signature, int role, bool needsTranslation);

  // Expose the internal mutex
  //bool tryLock();
  //void lock();
  //void unlock();
};

// Helper mutex locker class
//class EmbedManagerLock
//{
//  EmbedManager *mutex_;
//public:
//  explicit EmbedManagerLock(EmbedManager *m) : mutex_(m) { mutex_->lock(); }
//  ~EmbedManagerLock() { mutex_->unlock(); }
//};

// EOF
