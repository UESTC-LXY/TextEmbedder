// enginefactory.cc
// 4/26/2014 jichi
// List of all engines
#include "engine/enginecontroller.h"
#include "engine/enginefactory.h"
#include "windbg/util.h" 
#include"engine/model/kirikiriz.h"
#include"engine/model/kirikiri2.h"
#include "engine/model/age.h"
#include "engine/model/aoi.h"
#include "engine/model/bgi.h"
#include "engine/model/circus.h"
#include "engine/model/cmvs.h"
#include "engine/model/cotopha.h"
#include "engine/model/cs2.h"
#include "engine/model/debonosu.h"
#include "engine/model/elf.h"
#include "engine/model/escude.h"
#include "engine/model/fvp.h"
#include "engine/model/gxp.h"
#include "engine/model/lcse.h"
#include "engine/model/leaf.h"
#include "engine/model/lucifen.h"
#include "engine/model/luna.h"
#include "engine/model/malie.h"
#include "engine/model/minori.h"
#include "engine/model/mono.h"
#include "engine/model/nexton.h"
#include "engine/model/nitro.h"
#include "engine/model/pal.h"
#include "engine/model/pensil.h"
#include "engine/model/qlie.h"
#include "engine/model/retouch.h"
#include "engine/model/rgss.h"
#include "engine/model/rio.h"
#include "engine/model/siglus.h"
#include "engine/model/silkys.h"
#include "engine/model/system4.h"
#include "engine/model/taskforce.h"
#include "engine/model/unicorn.h"
#include "engine/model/waffle.h"
#include "engine/model/will.h"
#include "engine/model/wolf.h"
#include "engine/model/aos.h"
#include "engine/model/lova.h" 
#include"engine/model/willplus3.h"
#include"engine/model/artemis.h"
#include"engine/model/horkeye.h"
#include"engine/model/EntisGLS.h"
#define DEBUG "enginefactory"
#include "sakurakit/skdebug.h"

// TODO: Use boost_pp or boost::mpl to reduce runtime polymorphic
// http://stackoverflow.com/questions/1883862/c-oop-list-of-classes-class-types-and-creating-instances-of-them
EngineController *EngineFactory::createEngine()
{
  static EngineModel *models[] = { // sort reversely
    
      new LovaEngine, new QLiEEngine, new CotophaEngine,

    new LunaSoftEngine,new horkeye, new ShinaRioEngine, new PalEngine , new RGSSEngine,
      new LCScriptEngine, new CircusEngine,
      new CMVSEngine, new EscudeEngine , new GXPEngine, new SilkysEngine, new ElfEngine
    , new LeafEngine, new RetouchEngine
     ,new willplus3Engine , new MalieEngine, new CatSystemEngine, new SiglusEngine
      ,new aosEngine , new ARCGameEngine
      ,new BGIEngine,new artemisEngine, new WolfRPGEngine, new WillPlusEngine, new WaffleEngine
    , new TaskforceEngine, new SystemAoiEngine
       , new LucifenEngine, new DebonosuEngine
    , new MonoEngine , new NitroplusEngine
    , new MinoriEngine, new FVPEngine, new PensilEngine
    , new UnicornEngine , new NextonEngine, new System4Engine,new EntisGLS,new kirikiriz,new kirikiri2
  };
  for each (EngineModel *m in models) {
    auto p = new EngineController(m);
    if (p->match()) {
      DOUT("matched, engine =" << p->name());
      WinDbg::ThreadsSuspender suspendedThreads; // lock all threads
      if (p->load()) {
        DOUT("ret = true, engine =" << p->name());
        return p;
      }
      DOUT("load failed, engine =" << p->name());
    }
    delete p;
  }
  DOUT("ret = false, none of the engines matches");
  return nullptr;
}

// EOF
