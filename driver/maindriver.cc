// maindriver.cc
// 2/1/2013 jichi
#include "config.h"
#include "driver/maindriver.h"
#include "driver/maindriver_p.h"
#include "driver/rpcclient.h"
#include "driver/settings.h"
#include "embed/embeddriver.h"
#include "hijack/hijackdriver.h"
#include "hijack/hijackdriver.h" 

#define DEBUG "maindriver"
#include "sakurakit/skdebug.h"

/** Public class */

MainDriver::MainDriver(void*parent)
  :   d_(new D(this))
{
    
}

MainDriver::~MainDriver() { delete d_; }

void MainDriver::requestDeleteLater() {   }

void MainDriver::quit() { d_->quit(); }

/** Private class */

MainDriverPrivate::MainDriverPrivate(void *parent)
  :   settings(nullptr), rpc(nullptr), hijack(nullptr), win(nullptr), eng(nullptr)
{
  DOUT("enter");
  settings = new Settings(this);
  {
      settings->loadFinished = [this]() {onLoadFinished(); };
  }

  rpc = new RpcClient( );
  {
   

    rpc->disconnected = std::bind(&MainDriverPrivate::onDisconnected, this);
    rpc->detachRequested = std::bind(&MainDriverPrivate::unload, this);
    rpc->settingsReceived = std::bind(&Settings::load, settings, std::placeholders::_1);
    rpc->disableRequested = std::bind(&Settings::disable, settings );
  }
  DOUT("leave");
}


void MainDriverPrivate::quit()
{
  if (eng)
    eng->quit();
  rpc->quit();
}

void MainDriverPrivate::createHijackDriver()
{
  if (hijack)
    return;
  hijack = new HijackDriver(this);
  hijack->setEncoding(settings->gameEncoding());
  hijack->setFontFamily(settings->embeddedFontFamily());
  hijack->setFontCharSet(settings->embeddedFontCharSet());
  hijack->setFontCharSetEnabled(settings->isEmbeddedFontCharSetEnabled());
  hijack->setFontScale(settings->embeddedFontScale());
  hijack->setFontWeight(settings->embeddedFontWeight());

  settings->gameEncodingChanged = std::bind(&HijackDriver::setEncoding, hijack, std::placeholders::_1);;
  settings->embeddedFontFamilyChanged = std::bind(&HijackDriver::setFontFamily, hijack, std::placeholders::_1);;
  settings->embeddedFontCharSetChanged = std::bind(&HijackDriver::setFontCharSet, hijack, std::placeholders::_1);;
  settings->embeddedFontCharSetEnabledChanged = std::bind(&HijackDriver::setFontCharSetEnabled, hijack, std::placeholders::_1);;
  settings->embeddedFontScaleChanged = std::bind(&HijackDriver::setFontScale, hijack, std::placeholders::_1);;
  settings->embeddedFontWeightChanged = std::bind(&HijackDriver::setFontWeight, hijack, std::placeholders::_1);;

  if (eng) {
    hijack->setDeviceContextFontEnabled(eng->isDeviceContextFontEnabled());
    hijack->setLocaleEmulationEnabled(eng->isLocaleEmulationEnabled());
  }
}
 
void MainDriverPrivate::createEmbedDriver()
{
  if (eng)
    return;

  eng = new EmbedDriver(this);
   
  eng->engineNameChanged = std::bind(&RpcClient::sendEngineName, rpc, std::placeholders::_1);
   
   
  settings->embeddedScenarioVisibleChanged = std::bind(&EmbedDriver::setScenarioVisible, eng, std::placeholders::_1);;
  settings->embeddedScenarioTextVisibleChanged = std::bind(&EmbedDriver::setScenarioTextVisible, eng, std::placeholders::_1);;
  settings->embeddedScenarioTranscodingEnabledChanged = std::bind(&EmbedDriver::setScenarioTranscodingEnabled, eng, std::placeholders::_1);;
  settings->embeddedScenarioTranslationEnabledChanged = std::bind(&EmbedDriver::setScenarioTranslationEnabled, eng, std::placeholders::_1);;
  settings->embeddedNameVisibleChanged = std::bind(&EmbedDriver::setNameVisible, eng, std::placeholders::_1);;
  settings->embeddedNameTextVisibleChanged = std::bind(&EmbedDriver::setNameTextVisible, eng, std::placeholders::_1);;
  settings->embeddedNameTranscodingEnabledChanged = std::bind(&EmbedDriver::setNameTranscodingEnabled, eng, std::placeholders::_1);;
  settings->embeddedNameTranslationEnabledChanged = std::bind(&EmbedDriver::setNameTranslationEnabled, eng, std::placeholders::_1);;
  settings->embeddedOtherVisibleChanged = std::bind(&EmbedDriver::setOtherVisible, eng, std::placeholders::_1);;
  settings->embeddedOtherTextVisibleChanged = std::bind(&EmbedDriver::setOtherTextVisible, eng, std::placeholders::_1);;
  settings->embeddedOtherTranscodingEnabledChanged = std::bind(&EmbedDriver::setOtherTranscodingEnabled, eng, std::placeholders::_1);;
  settings->embeddedOtherTranslationEnabledChanged = std::bind(&EmbedDriver::setOtherTranslationEnabled, eng, std::placeholders::_1);;

  settings->embeddedTextEnabledChanged = std::bind(&EmbedDriver::setEnabled, eng, std::placeholders::_1);;

  settings->embeddedScenarioWidthChanged = std::bind(&EmbedDriver::setScenarioWidth, eng, std::placeholders::_1);;

  settings->embeddedSpaceAlwaysInsertedChanged = std::bind(&EmbedDriver::setAlwaysInsertsSpaces, eng, std::placeholders::_1);;
  settings->embeddedSpaceSmartInsertedChanged = std::bind(&EmbedDriver::setSmartInsertsSpaces, eng, std::placeholders::_1);;
  settings->embeddedSpacePolicyEncodingChanged = std::bind(&EmbedDriver::setSpacePolicyEncoding, eng, std::placeholders::_1);;


  settings->embeddedAllTextsExtractedChanged = std::bind(&EmbedDriver::setExtractsAllTexts, eng, std::placeholders::_1);;

  settings->scenarioSignatureChanged = std::bind(&EmbedDriver::setScenarioSignature, eng, std::placeholders::_1);;
  settings->nameSignatureChanged = std::bind(&EmbedDriver::setNameSignature, eng, std::placeholders::_1);;

  settings->embeddedTranslationWaitTimeChanged = std::bind(&EmbedDriver::setTranslationWaitTime, eng, std::placeholders::_1);;


  settings->gameEncodingChanged = std::bind(&EmbedDriver::setEncoding, eng, std::placeholders::_1);;

  if (eng->load()) {
    eng->setTranslationWaitTime(settings->embeddedTranslationWaitTime());
    eng->setEncoding(settings->gameEncoding());

    eng->setScenarioVisible(settings->isEmbeddedScenarioVisible());
    eng->setScenarioTextVisible(settings->isEmbeddedScenarioTextVisible());
    eng->setScenarioTranscodingEnabled(settings->isEmbeddedScenarioTranscodingEnabled());
    eng->setScenarioTranslationEnabled(settings->isEmbeddedScenarioTranslationEnabled());
    eng->setNameVisible(settings->isEmbeddedNameVisible());
    eng->setNameTextVisible(settings->isEmbeddedNameTextVisible());
    eng->setNameTranscodingEnabled(settings->isEmbeddedNameTranscodingEnabled());
    eng->setNameTranslationEnabled(settings->isEmbeddedNameTranslationEnabled());
    eng->setOtherVisible(settings->isEmbeddedOtherVisible());
    eng->setOtherTextVisible(settings->isEmbeddedOtherTextVisible());
    eng->setOtherTranscodingEnabled(settings->isEmbeddedOtherTranscodingEnabled());
    eng->setOtherTranslationEnabled(settings->isEmbeddedOtherTranslationEnabled());

    eng->setAlwaysInsertsSpaces(settings->isEmbeddedSpaceAlwaysInserted());
    eng->setSmartInsertsSpaces(settings->isEmbeddedSpaceSmartInserted());
    eng->setSpacePolicyEncoding(settings->embeddedSpacePolicyEncoding());

    eng->setScenarioWidth(settings->embeddedScenarioWidth());

    //eng->setDetectsControl(settings->isEmbeddedTextCancellableByControl());
    eng->setExtractsAllTexts(settings->isEmbeddedAllTextsExtracted());

    eng->setScenarioSignature(settings->scenarioSignature());
    eng->setNameSignature(settings->nameSignature());

    // Always enable text extraction
    eng->setScenarioExtractionEnabled(true);
    eng->setNameExtractionEnabled(true);
  }
  //eng->setEnabled(settings->isEmbedDriverNeeded()); // enable it at last
  eng->setEnabled(settings->isEmbeddedTextEnabled());
}

void MainDriverPrivate::onDisconnected()
{
  
  unload();
}

void MainDriverPrivate::unload()
{
#ifdef VNRAGENT_ENABLE_UNLOAD
  // Add contents to qDebug will crash the application while unload.
  //DOUT("enter");
  if (hijack) hijack->unload();
  if (eng) eng->unload();
  //DOUT("leave");
  WinDbg::unloadCurrentModule();
#endif // VNRAGENT_ENABLE_UNLOAD
}

void MainDriverPrivate::onLoadFinished()
{
  if (eng)
    ; //eng->setEnabled(settings->isEmbedDriverNeeded());
  else if (settings->isEmbedDriverNeeded()){
    createEmbedDriver();
    createHijackDriver();
  }
   
}

// EOF
