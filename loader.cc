// loader.cc
// 1/27/2013

#include "config.h"
#include "loader.h"
#include "driver/maindriver.h"
#include "windbg/inject.h"
#include "windbg/util.h"
#include "hijack/hijackmodule.h" 

#ifdef VNRAGENT_ENABLE_APPRUNNER
#include "qtembedapp/applicationrunner.h"
#endif // VNRAGENT_ENABLE_APPRUNNER

#ifdef VNRAGENT_DEBUG
# include "util/msghandler.h"
#endif // VNRAGENT_DEBUG

#define DEBUG "loader"
#include "sakurakit/skdebug.h"

// Global variables

namespace { // unnamed
     

// Persistent data
MainDriver *driver_;

#ifdef VNRAGENT_ENABLE_APPRUNNER
QtEmbedApp::ApplicationRunner *appRunner_;
#endif // VNRAGENT_ENABLE_APPRUNNER

} // unnamed namespace

// Loader

void Loader::initWithInstance(HINSTANCE hInstance)
{ 

#ifdef VNRAGENT_DEBUG
  Util::installDebugMsgHandler();
#endif // VNRAGENT_DEBUG
   
  ::driver_ = new MainDriver;

  // Hijack UI threads
  {
    WinDbg::ThreadsSuspender suspendedThreads; // lock all threads
    Hijack::overrideModules();
  }
   
}

void Loader::destroy()
{
  if (::driver_) {
    ::driver_->quit();
 
  }
   
}

// EOF
