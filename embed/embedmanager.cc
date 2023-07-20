// embedmanager.cc
// 4/26/2014 jichi
#include"stringutil.h"
#include "config.h"
#include "embed/embedmanager.h"
#include "embed/embedmemory.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "driver/rpcclient.h"
//#include "QxtCore/QxtJSON"
//#include "qtjson/qtjson.h"
#include "winevent/winevent.h"
#include "winmutex/winmutex.h"    
//#include "debug.h"
#include<string>
#include<vector>
#include<map>
//#define ENGINE_SLEEP_EVENT "vnragent_engine_sleep"
#define D_LOCK win_mutex_lock<D::mutex_type> d_lock(d_->mutex)

// TODO: Use read-write lock instead
//#define D_LOCK (void)0 // locking is temporarily disabled to avoid hanging

#define DEBUG "embedmanager"
#include "sakurakit/skdebug.h"

/** Private class */

class EmbedManagerPrivate
{
public:
  
  struct TextEntrySTD
  {
      std::wstring text,
          language;
      TextEntrySTD() {}
      TextEntrySTD(const std::wstring& text, const std::wstring& language) : text(text), language(language) {}
      explicit TextEntrySTD(const std::wstring& text) : text(text) {}
  }; 
  std::map<UINT64, TextEntrySTD> translations_std;   // cached, {key:text}

  typedef win_mutex<CRITICAL_SECTION> mutex_type;
  mutex_type mutex; // mutex to lock translations

  int waitTime;
   
  EmbedMemory_MMAP* memory_win;
  EmbedManagerPrivate(void*parent)
    : waitTime(1000) // sleep for 1 second by default
    
  {
      memory_win = new EmbedMemory_MMAP; 
  }
   
  static inline std::wstring createEventNameSTD(UINT64 hash, int role)
  { 
      wchar_t buff[1024];
      swprintf_s(buff, VNRAGENT_MEMORY_EVENT_W, std::to_wstring(GetCurrentProcessId()).c_str(), std::to_wstring(role).c_str(), std::to_wstring(hash).c_str());
      return buff;
  }
  //static void sleep(int msecs) { ::Sleep(msecs); }
  //void notify() {}
};

//  // - Event -
//  // Sleep 10*100 = 1 second
//  enum { SleepTimeout = 100 };
//  enum { SleepCount = 10 };
//  void sleep(int interval = SleepTimeout, int count = SleepCount)
//  {
//    sleepEvent.signal(false);
//    for (int i = 0; !sleepEvent.wait(interval) && i < count; i++);
//    //sleepEvent.signal(false);
//  }
//
//  void notify() { sleepEvent.signal(true); }

//  mutex_type sleepMutex;
//  typedef win_mutex_cond<CONDITION_VARIABLE> cond_type;
//  cond_type sleepCond;
//  void sleep()
//  {
//    sleepMutex.lock();
//    sleepCond.wait_for(sleepMutex, SleepTimeout * SleepCount);
//  }
//  void notify() { sleepCond.notify_all(); }

/** Public class */

// - Construction -

static EmbedManager *instance_;
EmbedManager *EmbedManager::instance() { return instance_; }

EmbedManager::EmbedManager(void*parent)
  :  d_(new D(this))
{ ::instance_ = this; }

EmbedManager::~EmbedManager()
{
  ::instance_ = nullptr;
  delete d_;
}

void EmbedManager::setTranslationWaitTime(int msecs)
{ d_->waitTime = msecs; }

//bool EmbedManager::tryLock() { return d_->mutex.try_lock(); }
//void EmbedManager::lock() { d_->mutex.lock(); }
//void EmbedManager::unlock() { d_->mutex.unlock(); }


// - Actions -

void EmbedManager::quit()
{
 /* D_LOCK;
  if (d_->memory->isAttached())
    d_->memory->detach();*/
}
 

//void EmbedManager::updateTranslation(const QString &text, UINT64 hash, int role)
//{
//  D_LOCK;
//  UINT64 key = Engine::hashTextKey(hash, role);
//  d_->translations[key] = text;
//  //d_->notify();
//}

//void EmbedManager::abortTranslation()
//{ d_->unblock(); }
 
void EmbedManager::sendTextSTD(const std::wstring& text, UINT64 hash, long signature, int role, bool needsTranslation)
{
#ifdef VNRAGENT_ENABLE_NATIVE_PIPE
    if (needsTranslation)
        RpcClient::instance()->directSendEngineTextSTD(text, hash, signature, role, needsTranslation);
     
#endif  
} 
std::wstring EmbedManager::findTranslationSTD(UINT64 hash, int role, std::wstring* language) const
{
    D_LOCK;
    if (role <= 1) {
        UINT64 key = Engine::hashTextKey(hash, role);
        auto p = d_->translations_std.find(key);
        if (p != d_->translations_std.end()) {
            const auto& v = p->second;
            if (language)
                *language = v.language;
            return v.text;
        }
    }
    else {
        std::wstring ret;
        for (int i = role; i > 0; i--) {
            UINT64 key = Engine::hashTextKey(hash, i);
            auto p = d_->translations_std.find(key);
            if (p != d_->translations_std.end()) {
                const auto& v = p->second;
                if (language)
                    *language = v.language;
                return v.text;
            }
        }
    }
    return L"";
} 
std::wstring EmbedManager::waitForTranslationSTD(UINT64 hash, int role, std::wstring* language) const
{ 
    UINT64 key = Engine::hashTextKey(hash, role);
    {
        D_LOCK;
        auto p = d_->translations_std.find(key);
        if (p != d_->translations_std.end()) {
            const auto& v = p->second;
            if (language)
                *language = v.language;
            return v.text;
        }
    } 
    std::wstring ret;
    auto m = d_->memory_win;
    
    if (ret.empty() && m->isAttached()) {
        std::wstring eventName = D::createEventNameSTD(hash, role); 
        win_event event(WideStringToString(eventName,CP_UTF8).c_str());
        enum { WaitInterval = VNRAGENT_MEMORY_TIMEOUT };
        //int WaitInterval = d_->waitTime;
        int waitCount = max(1, d_->waitTime / WaitInterval);

        for (int count = 0;
            count <= waitCount && m->isAttached() && !Engine::isPauseKeyPressed();
            count++) { // repeat twice
            for (int i = 0; i < m->cellCount(); i++) {
                /*FILE* f = fopen(R"(C:\Users\wcy\Documents\GitHub\TextEmbedder\Release\1.txt)", "wb");
                fwrite(m->sharedcell, sizeof(Cell), 1, f);
                int _ = m->dataRole(i);
                fwrite(&_, 4, 1, f);
                fwrite(&role, 4, 1, f);
                UINT64 _h = m->dataHash(i);
                fwrite(&_h, 8, 1, f);
                fwrite(&hash, 8, 1, f);
                fclose(f);*/
                /*MessageBox(0, (std::to_wstring(m->dataHash(0)) + L" " + std::to_wstring(m->dataRole(0))).c_str(), L"", 0);*/
                if (m->dataRole(i) == role && m->dataHash(i) == hash) {
                    if (m->isDataCanceled(i))
                        return ret;
                    if (m->isDataReady(i)) {
                        std::wstring lang = m->dataLanguage(i);
                        if (language)
                            *language = lang;
                        ret = m->dataText(i);
                        if (!ret.empty()) {
                            D_LOCK;
                            d_->translations_std[key] = D::TextEntrySTD(ret, lang);
                        }
                        return ret;
                    }
                }
            }
            if (count != waitCount) {
                if (event.wait(WaitInterval)) // if wake up by VNR, no longer wait anymore
                    count = waitCount - 1;
            }
        }
    }
    return ret;
}
// EOF
