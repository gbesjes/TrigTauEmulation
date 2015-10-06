// Dear emacs, this is -*- c++ -*-
// vim: ts=2 sw=2
// $Id$
// System include(s):
#include <memory>
#include <set>
#include <string>

#include <csignal>

// ROOT include(s):
#include <TFile.h>
#include <TChain.h>
#include <TError.h>
#include <TTree.h>
#include <TSystem.h>

// Core EDM include(s):
#include "AthContainers/AuxElement.h"
#include "AthContainers/DataVector.h"

// EDM includes
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauJetAuxContainer.h"
#include "xAODTrigger/JetRoIContainer.h"
#include "xAODTrigger/EmTauRoIContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"
#include "xAODTrigger/EnergySumRoI.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticleAuxContainer.h"

#include "TrigNavStructure/TrigNavStructure.h"

// Local include(s):
#include "AsgTools/ToolHandle.h"

#ifdef ROOTCORE
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/TStore.h"
#include "xAODRootAccess/tools/ReturnCheck.h"
#include "xAODRootAccess/tools/Message.h"
#endif

#include "TrigTauEmulation/ToolsRegistry.h"
#include "TrigTauEmulation/Level1EmulationTool.h"
#include "TrigTauEmulation/HltEmulationTool.h"

#include "TrigTauEmulation/DecoratedHltTau.h"
#include "TrigTauEmulation/Utils.h"

// Trigger Decision Tool
#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TrigDecisionTool/Conditions.h"

// #ifdef ASGTOOL_STANDALONE
//  #include "TauDiscriminant/TauPi0BDT.h"
//  #include "TauDiscriminant/TauJetBDT.h"
//  #include "TauDiscriminant/TauDiscriminantTool.h"
// #endif

using namespace TrigConf;
using namespace Trig;

volatile sig_atomic_t exitFlag = 0;
std::vector<std::string> all_l1_chains() ;

// Error checking macro
#define CHECK( ARG )\
  do {                                                                  \
    const bool result = ARG;\
    if(!result) {\
      ::Error(APP_NAME, "Failed to execute: \"%s\"",\
#ARG );\
      return 1;\
    }\
  } while( false )

/* The logic is following
   The truth eta phi are provided and the chain
   The feature of type Feaure1 is looked for if matching is found dR < 0.1 then the other feature  of typ e Feature2 is looked for in the same RoI.
 */
template<class Feature1, class Feature2>
  std::pair<Trig::AsgFeature<Feature1>,  Trig::AsgFeature<Feature2> > passing(const Trig::ChainGroup* cg, double truthEta, double truthPhi) {
    const unsigned condition = TrigDefs::Physics;
    auto roisf1 = cg->features(condition ).containerFeature<Feature1>("", condition);
    auto roisf2 = cg->features(condition).containerFeature<Feature2>("", condition);
    for ( auto roif1:  roisf1 ) {
      for ( auto f1 : *(roif1.cptr()) ) {
      const double deltaR = Utils::dR(truthEta, truthPhi, f1->eta(), f1->phi());
      if ( deltaR < 0.1  ) {
        for ( auto roif2: roisf2 ) { // that should be doable w/o the loop
          if ( HLT::TrigNavStructure::haveCommonRoI(roif1.te(), roif2.te()))
            return std::make_pair(roif1, roif2);
        }
      }
    }
  }
  return std::make_pair(Trig::AsgFeature<Feature1>(), Trig::AsgFeature<Feature2>()); // empty
}


//exit flag setter
void catchSighup(int sig){
  exitFlag = 1;
}

// helper to clear xAOD containers
template <typename T> void clearContainer(T &c){
  for(auto it: *c){
    it->clearDecorations();
    if(it->usingPrivateStore()){
      it->releasePrivateStore();
    }
  }
}

// trim from start
static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

static inline std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}

std::vector<std::string> getInputFiles(){

  // file paths found with rucio list-file-replicas
  // -> DON'T FORGET TO UPDATE THIS COMMENT WHEN CHANGING DS

  std::vector<std::string> files;
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/f6/5f/AOD.05763094._000001.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/6e/b4/AOD.05763094._000002.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/28/90/AOD.05763094._000003.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/17/e4/AOD.05763094._000004.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/46/9c/AOD.05763094._000005.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/1b/7d/AOD.05763094._000006.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/4d/09/AOD.05763094._000007.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/51/9e/AOD.05763094._000009.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/55/74/AOD.05763094._000010.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/95/e2/AOD.05763094._000011.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/a6/61/AOD.05763094._000012.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/9d/b7/AOD.05763094._000013.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/d6/b1/AOD.05763094._000014.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/57/9a/AOD.05763094._000015.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/e5/95/AOD.05763094._000016.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/e6/07/AOD.05763094._000017.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/58/d8/AOD.05763094._000018.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/a0/c8/AOD.05763094._000019.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/cd/36/AOD.05763094._000020.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/9f/c5/AOD.05763094._000021.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/c5/ac/AOD.05763094._000022.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/92/12/AOD.05763094._000023.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/8c/0e/AOD.05763094._000024.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/78/d4/AOD.05763094._000025.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/7d/4b/AOD.05763094._000026.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/b4/c0/AOD.05763094._000027.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/03/42/AOD.05763094._000028.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/ea/f2/AOD.05763094._000029.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/43/02/AOD.05763094._000030.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/8e/ce/AOD.05763094._000031.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/42/a3/AOD.05763094._000032.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/a7/22/AOD.05763094._000033.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/da/4c/AOD.05763094._000034.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/53/8c/AOD.05763094._000035.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/bc/f7/AOD.05763094._000036.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/29/35/AOD.05763094._000037.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/1a/80/AOD.05763094._000038.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/f4/30/AOD.05763094._000039.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/da/fd/AOD.05763094._000040.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/05/b2/AOD.05763094._000041.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/74/05/AOD.05763094._000042.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/cd/57/AOD.05763094._000043.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/88/f5/AOD.05763094._000044.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/df/72/AOD.05763094._000045.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/1c/44/AOD.05763094._000046.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/06/d8/AOD.05763094._000047.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/8b/4d/AOD.05763094._000048.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/79/60/AOD.05763094._000049.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/89/22/AOD.05763094._000050.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/44/3c/AOD.05763094._000051.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/21/fe/AOD.05763094._000052.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/fa/09/AOD.05763094._000053.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/e2/2f/AOD.05763094._000054.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/16/9e/AOD.05763094._000055.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/b2/32/AOD.05763094._000056.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/53/92/AOD.05763094._000057.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/dd/d1/AOD.05763094._000058.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/49/f6/AOD.05763094._000059.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/59/29/AOD.05763094._000060.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/44/bb/AOD.05763094._000061.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/84/05/AOD.05763094._000062.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/5f/e0/AOD.05763094._000063.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/82/c1/AOD.05763094._000064.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/d7/09/AOD.05763094._000065.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/40/9d/AOD.05763094._000066.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/91/bb/AOD.05763094._000067.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/fe/7c/AOD.05763094._000068.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/53/a9/AOD.05763094._000069.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/9d/3b/AOD.05763094._000070.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/7a/7f/AOD.05763094._000071.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/28/8c/AOD.05763094._000072.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/45/4c/AOD.05763094._000073.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/45/aa/AOD.05763094._000074.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/28/83/AOD.05763094._000075.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/85/6b/AOD.05763094._000076.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/24/40/AOD.05763094._000077.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/5b/e9/AOD.05763094._000078.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/ff/36/AOD.05763094._000079.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/5d/f3/AOD.05763094._000080.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/27/42/AOD.05763094._000081.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/97/17/AOD.05763094._000082.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/ad/12/AOD.05763094._000083.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/3d/ea/AOD.05763094._000084.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/80/97/AOD.05763094._000085.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/e9/50/AOD.05763094._000086.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/28/d7/AOD.05763094._000087.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/1b/7a/AOD.05763094._000088.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/c9/a3/AOD.05763094._000089.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/ca/f9/AOD.05763094._000090.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/bd/e8/AOD.05763094._000091.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/f1/08/AOD.05763094._000092.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/d7/40/AOD.05763094._000093.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/17/47/AOD.05763094._000094.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/71/7d/AOD.05763094._000095.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/85/cd/AOD.05763094._000096.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/89/b1/AOD.05763094._000097.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/67/b8/AOD.05763094._000098.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/9a/ef/AOD.05763094._000099.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/9d/6d/AOD.05763094._000100.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/e2/ed/AOD.05763094._000101.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/06/10/AOD.05763094._000102.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/0f/ed/AOD.05763094._000103.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/08/6e/AOD.05763094._000104.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/be/dd/AOD.05763094._000105.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/7c/76/AOD.05763094._000106.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/12/0d/AOD.05763094._000107.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/cf/2e/AOD.05763094._000108.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/9f/4d/AOD.05763094._000109.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/69/66/AOD.05763094._000110.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/fe/52/AOD.05763094._000111.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/37/9d/AOD.05763094._000112.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/f1/e8/AOD.05763094._000113.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/bb/8c/AOD.05763094._000114.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/63/56/AOD.05763094._000115.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/05/b8/AOD.05763094._000116.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/d3/8d/AOD.05763094._000117.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/15/d6/AOD.05763094._000118.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/30/cf/AOD.05763094._000119.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/8f/97/AOD.05763094._000120.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/4d/85/AOD.05763094._000121.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/a1/28/AOD.05763094._000122.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/c3/bd/AOD.05763094._000123.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/39/8d/AOD.05763094._000124.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/5a/f8/AOD.05763094._000125.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/ce/f3/AOD.05763094._000126.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/a6/c4/AOD.05763094._000127.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/d7/17/AOD.05763094._000128.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/f8/d2/AOD.05763094._000129.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/71/9d/AOD.05763094._000130.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/2f/9d/AOD.05763094._000131.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/a2/41/AOD.05763094._000132.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/34/c6/AOD.05763094._000133.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/27/0f/AOD.05763094._000134.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/49/75/AOD.05763094._000135.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/fa/60/AOD.05763094._000136.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/66/8d/AOD.05763094._000137.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/03/0f/AOD.05763094._000138.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/1d/b3/AOD.05763094._000139.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/43/11/AOD.05763094._000140.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/2b/df/AOD.05763094._000141.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/fb/11/AOD.05763094._000142.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/73/2c/AOD.05763094._000143.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/d8/6f/AOD.05763094._000144.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/16/13/AOD.05763094._000145.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/e4/65/AOD.05763094._000146.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/3d/06/AOD.05763094._000147.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/7a/b4/AOD.05763094._000148.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/d9/65/AOD.05763094._000149.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/f0/39/AOD.05763094._000150.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/9c/54/AOD.05763094._000151.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/c1/d4/AOD.05763094._000152.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/85/35/AOD.05763094._000153.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/b1/8e/AOD.05763094._000154.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/8c/77/AOD.05763094._000155.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/9c/2d/AOD.05763094._000156.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/2a/17/AOD.05763094._000157.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/7e/b4/AOD.05763094._000158.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/00/52/AOD.05763094._000159.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/19/75/AOD.05763094._000160.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/97/6f/AOD.05763094._000161.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/84/b0/AOD.05763094._000162.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/76/c8/AOD.05763094._000163.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/53/12/AOD.05763094._000164.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/5d/46/AOD.05763094._000165.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/f8/48/AOD.05763094._000166.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/c9/6b/AOD.05763094._000167.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/f1/e9/AOD.05763094._000168.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/78/56/AOD.05763094._000169.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/8d/c2/AOD.05763094._000170.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/73/ee/AOD.05763094._000171.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/3a/1b/AOD.05763094._000172.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/ff/90/AOD.05763094._000173.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/9d/13/AOD.05763094._000174.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/88/9e/AOD.05763094._000175.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/df/cc/AOD.05763094._000176.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/76/0a/AOD.05763094._000177.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/9d/1c/AOD.05763094._000178.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/3a/2f/AOD.05763094._000179.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/48/03/AOD.05763094._000180.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/b2/41/AOD.05763094._000181.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/93/b5/AOD.05763094._000182.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/e1/de/AOD.05763094._000183.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/dd/b1/AOD.05763094._000184.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/3d/c0/AOD.05763094._000185.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/77/cc/AOD.05763094._000186.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/91/52/AOD.05763094._000187.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/bd/da/AOD.05763094._000188.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/09/2e/AOD.05763094._000189.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/19/f3/AOD.05763094._000190.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/f1/07/AOD.05763094._000191.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/7a/b0/AOD.05763094._000192.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/be/17/AOD.05763094._000193.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/ce/70/AOD.05763094._000194.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/86/ed/AOD.05763094._000195.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/f2/3f/AOD.05763094._000196.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/5d/db/AOD.05763094._000197.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/ae/ae/AOD.05763094._000198.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/af/ad/AOD.05763094._000199.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/48/bb/AOD.05763094._000200.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/6b/ed/AOD.05763094._000201.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/c5/d5/AOD.05763094._000202.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/a2/5c/AOD.05763094._000203.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/d3/31/AOD.05763094._000204.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/7d/67/AOD.05763094._000205.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/09/db/AOD.05763094._000206.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/00/76/AOD.05763094._000207.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/ba/00/AOD.05763094._000208.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/9a/c5/AOD.05763094._000209.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/ed/be/AOD.05763094._000210.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/56/75/AOD.05763094._000211.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/1c/4b/AOD.05763094._000212.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/01/e5/AOD.05763094._000213.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/00/40/AOD.05763094._000214.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/b0/12/AOD.05763094._000215.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/93/65/AOD.05763094._000216.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/f4/9c/AOD.05763094._000217.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/17/1a/AOD.05763094._000218.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/a2/0e/AOD.05763094._000220.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/eb/06/AOD.05763094._000221.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/22/54/AOD.05763094._000222.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/b6/68/AOD.05763094._000223.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/0e/88/AOD.05763094._000224.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/89/a3/AOD.05763094._000225.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/c9/bd/AOD.05763094._000226.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/3a/ff/AOD.05763094._000227.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/9d/16/AOD.05763094._000228.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/28/8a/AOD.05763094._000230.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/96/d7/AOD.05763094._000231.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/38/5f/AOD.05763094._000232.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/aa/d5/AOD.05763094._000233.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/91/69/AOD.05763094._000234.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/65/69/AOD.05763094._000235.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/66/eb/AOD.05763094._000236.pool.root.1");
  files.push_back("root://eosatlas//eos/atlas/atlasdatadisk/rucio/data15_13TeV/77/9c/AOD.05763094._000237.pool.root.1");

  return files;
}

class BrokenEventInfo {

  public:
    BrokenEventInfo(int _entry, int _eventNumber, int _LB, std::string _t) : entry(_entry), eventNumber(_eventNumber), lumiBlock(_LB), trigger(_t){}

    int getEntry() const { return entry; }
    int getEventNumber() const { return eventNumber; }
    int getLumiBlock() const { return lumiBlock; }
    std::string getTrigger() const { return trigger; }

  private:
    int entry;
    int eventNumber;
    int lumiBlock;
    std::string trigger;

};

std::ostream& operator<< (std::ostream &out, const BrokenEventInfo &b) {
  out << "\t" << std::setw(10) << b.getEntry() << "\t event " << std::setw(12) << b.getEventNumber() << "\t LB " << std::setw(6) << b.getLumiBlock() << "\t trigger \t" << b.getTrigger();
  return out;
}

int main(int argc, char** argv) {

  signal(SIGINT, catchSighup);

  // Get the name of the application:
  const char* APP_NAME = "tautriggeremulationtool_hlt";

  if (argc < 3) {
    ::Error(APP_NAME, XAOD_MESSAGE("Wrong number of arguments, use\n\n%s <reference_chain> <chains_to_test>"), argv[0] );
    return 1;
  }
  std::string reference_chain = argv[1];
  std::vector<std::string> argv_chains_to_test;

  for(int i=2; i < argc; ++i){
    argv_chains_to_test.push_back(argv[i]);
  }

  std::cout << "Evaluate the following chains based on " << reference_chain << ":" << std::endl;
  for(auto s: argv_chains_to_test){
    std::cout << "\t" << s << std::endl;
  }

  // Initialise the environment:
  RETURN_CHECK( APP_NAME, xAOD::Init( APP_NAME ) );

  // Create the TEvent object
  xAOD::TEvent event(xAOD::TEvent::kClassAccess);
  xAOD::TStore store;

  ::TChain chain1("CollectionTree");
  //chain1.Add("/afs/cern.ch/work/q/qbuat/public/mc15_13TeV/mc15_13TeV.361108.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Ztautau.recon.AOD.e3601_s2576_s2132_r6630/*root.1");

  //std::vector<std::string> filenames = getInputFiles();
  std::vector<std::string> filenames;
  //filenames.push_back("root://eosatlas//eos/atlas/user/q/qbuat/EB_REPRO/271421/data15_13TeV.00271421.physics_EnhancedBias.merge.AOD.r6913_p2346/AOD.06135404._000026.pool.root.1");

  //filenames.push_back("/afs/cern.ch/user/q/qbuat/work/public/mc15_13TeV/mc15_13TeV.361108.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Ztautau.recon.AOD.e3601_s2576_s2132_r6630/AOD.05358802._002522.pool.root.1");
  filenames.push_back("/afs/cern.ch/user/q/qbuat/work/public/mc15_13TeV/test/mc15_13TeV.341124.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_tautauhh.merge.DAOD_HIGG4D3.e3935_s2608_s2183_r6765_r6282_p2401/DAOD_HIGG4D3.06241784._000001.pool.root.1");

  int i = 0;
  int N = 1; //change to -1 to take all files 
  int skip = 0; //skip first N files? 
  for(auto s: filenames){
    if(--skip > 0){
      ::Info(APP_NAME, "skipping file %s", s.c_str()); continue; 
    }
    chain1.Add(s.c_str());
    ::Info(APP_NAME, "adding file %s", s.c_str());
    if (N > 0 && ++i > N-1) { break; }
  }

  RETURN_CHECK(APP_NAME, event.readFrom(&chain1));
  //Set up TDT for testing
  //add config tool
  xAODConfigTool configTool("TrigConf::xAODConfigTool");
  ToolHandle<TrigConf::ITrigConfigTool> configHandle(&configTool);
  configHandle->initialize();

  // The decision tool
  Trig::TrigDecisionTool *trigDecTool = new Trig::TrigDecisionTool("TrigDecTool");
  trigDecTool->setProperty("ConfigTool", configHandle);
  //  trigDecTool.setProperty("OutputLevel", MSG::VERBOSE);
  trigDecTool->setProperty("TrigDecisionKey", "xTrigDecision");
  trigDecTool->initialize();

// #ifdef ASGTOOL_STANDALONE
//   // tau BDT
//   ToolHandleArray<ITauDiscriToolBase> tools;

//   auto taupi0BDT = new TauPi0BDT("TauPi0BDT");
//   taupi0BDT->setProperty("pi0BDTPrimary", "pi0Primary.BDT.bin").ignore();
//   taupi0BDT->setProperty("pi0BDTSecondary", "pi0Secondary.BDT.bin").ignore();
//   taupi0BDT->msg().setLevel( MSG::WARNING );
//   tools.push_back(taupi0BDT);

//   auto taujetBDT = new TauJetBDT("TauJetBDT");
//   taujetBDT->setProperty("jetBDT", "jet.BDT.bin").ignore();
//   taujetBDT->setProperty("jetSigTrans", "sig.trans.jet.BDT.root").ignore();
//   taujetBDT->setProperty("jetBkgTrans", "").ignore();
//   taujetBDT->setProperty("jetSigBits", "sig.bits.jet.BDT.txt").ignore();
//   taujetBDT->setProperty("jetBkgBits", "bkg.bits.jet.BDT.txt").ignore();
//   taujetBDT->msg().setLevel( MSG::WARNING );
//   tools.push_back(taujetBDT);

//   auto m_tauIDTool = std::make_shared<TauDiscriminantTool>("TauIDTool");
//   m_tauIDTool->setProperty("tools", tools).ignore();
//   m_tauIDTool->msg().setLevel( MSG::WARNING );
//   m_tauIDTool->initialize().ignore();
// #endif

  // chains to test
  std::vector<std::string> chains_to_test(argv_chains_to_test);

  std::map<std::string, int> fire_tdt;
  std::map<std::string, int> fire_emul;
  std::map<std::string, int> fire_difference_TDT;
  std::map<std::string, int> fire_difference_emu;
  for (auto c: chains_to_test) {
    fire_tdt[c] = 0;
    fire_emul[c] = 0;
    fire_difference_TDT[c] = 0;
    fire_difference_emu[c] = 0;
  }

  ToolsRegistry registry("ToolsRegistry");
  CHECK(registry.setProperty("RecalculateBDTscore", false));
  CHECK(registry.initialize());

  auto l1_chains = all_l1_chains();
  // Initialize the tool
  TrigTauEmul::Level1EmulationTool l1_emulationTool("Level1TauTriggerEmulator");
  CHECK(l1_emulationTool.setProperty("l1_chains", l1_chains));
  CHECK(l1_emulationTool.setProperty("JetTools", registry.GetL1JetTools()));
  CHECK(l1_emulationTool.setProperty("EmTauTools", registry.GetL1TauTools()));
  CHECK(l1_emulationTool.setProperty("XeTools", registry.GetL1XeTools()));
  CHECK(l1_emulationTool.setProperty("MuonTools", registry.GetL1MuonTools()));
  //l1_emulationTool.msg().setLevel(MSG::DEBUG);

  ToolHandle<TrigTauEmul::ILevel1EmulationTool> handle(&l1_emulationTool);

  TrigTauEmul::HltEmulationTool emulationTool("TauTriggerEmulator");
  CHECK(emulationTool.setProperty("hlt_chains", chains_to_test));
  CHECK(emulationTool.setProperty("PerformL1Emulation", true));
  CHECK(emulationTool.setProperty("Level1EmulationTool", handle));
  CHECK(emulationTool.setProperty("HltTauTools", registry.GetHltTauTools()));
  CHECK(emulationTool.setProperty("TrigDecTool", "TrigDecTool"));

  //const unsigned int triggerCondition = TrigDefs::Physics;
  const unsigned int triggerCondition = TrigDefs::Physics | TrigDefs::allowResurrectedDecision;
  CHECK(emulationTool.setProperty("L1TriggerCondition", triggerCondition));
  CHECK(emulationTool.setProperty("HLTTriggerCondition", triggerCondition));
  
  //emulationTool.msg().setLevel(MSG::DEBUG);
  CHECK(emulationTool.initialize());

  if(triggerCondition == TrigDefs::Physics){
    ::Info(APP_NAME, "Using trigger condition TrigDefs::Physics");
  } else if (triggerCondition == (TrigDefs::Physics | TrigDefs::allowResurrectedDecision) ) {
    ::Info(APP_NAME, "Using trigger condition TrigDefs::Physics | TrigDefs::allowResurrectedDecision");
  } else {
    // this shouldn't really happen
    ::Info(APP_NAME, "Using unknown trigger condition?!");
  }
  
  //const int maxEntries  = 1000;
  //const int skipEntries = 668200;
  //const int pickEntry = 668568;

  const int maxEntries = 1500;
  const int pickEntry = -1;
  const int skipEntries = -1;

  Long64_t entries = event.getEntries();
  unsigned long processedEntries = 0;
  std::list< BrokenEventInfo > brokenEvents;
  for (Long64_t entry = skipEntries; entry < entries && exitFlag == 0; entry++) {
   
    if(entry < 0) { continue; } //sanity check

    if(pickEntry > -1 && entry != pickEntry ) { continue; }
    if(maxEntries > -1 && processedEntries > maxEntries ) { break; }

    ++processedEntries;

    //if ((entry%50)==0)
      //::Info(APP_NAME, "Start processing event %d", (int)entry);

    ::Info(APP_NAME, "Start processing event %d", (int)entry);
    event.getEntry(entry);

    // retrieve the EDM objects
    const xAOD::EventInfo * ei = 0;
    CHECK(event.retrieve(ei, "EventInfo"));

    const xAOD::EmTauRoIContainer *l1taus = 0;
    CHECK(event.retrieve(l1taus, "LVL1EmTauRoIs"));

    // for (auto it:  *l1taus) {
    //   std::cout << "pT: \t" << it->tauClus() << "\n" 
    // 		 << "eta: \t" << it->eta() << "\n"
    // 		 << "phi: \t " << it->phi() << std::endl;
    // } 

    const xAOD::JetRoIContainer *l1jets = 0;
    CHECK(event.retrieve(l1jets, "LVL1JetRoIs"));

    const xAOD::MuonRoIContainer* l1muons = 0;
    CHECK(event.retrieve(l1muons, "LVL1MuonRoIs"));

    const xAOD::EnergySumRoI* l1xe = 0;
    CHECK(event.retrieve(l1xe, "LVL1EnergySumRoI"));

    // const xAOD::TauJetContainer* hlt_taus;
    // CHECK(event.retrieve(hlt_taus, "HLT_xAOD__TauJetContainer_TrigTauRecMerged"));

    if (not trigDecTool->isPassed(reference_chain, triggerCondition)) {
      continue;
    }

    const DataVector<xAOD::TrackParticle>* fast_tracks;
    CHECK(event.retrieve(fast_tracks, "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_TauIso_FTF"));

    // std::cout << "features !" << std::endl;
    auto cg = trigDecTool->getChainGroup(reference_chain);
    auto features = cg->features(triggerCondition);

    xAOD::TauJetContainer* presel_taus = new xAOD::TauJetContainer();
    xAOD::TauJetAuxContainer* presel_taus_aux = new xAOD::TauJetAuxContainer();
    presel_taus->setStore(presel_taus_aux);
    auto tauPreselFeatures = features.containerFeature<xAOD::TauJetContainer>("TrigTauRecPreselection");
    for (auto &tauContainer: tauPreselFeatures) {
      if (!tauContainer.cptr()) { continue; }
      for (auto tau: *tauContainer.cptr()) {
        xAOD::TauJet *new_tau = new xAOD::TauJet();
        new_tau->makePrivateStore(tau);
        presel_taus->push_back(new_tau);
      }
    }
    
    xAOD::TauJetContainer* hlt_taus = new xAOD::TauJetContainer();
    xAOD::TauJetAuxContainer* hlt_taus_aux = new xAOD::TauJetAuxContainer();
    hlt_taus->setStore(hlt_taus_aux);

    //std::string tauContainerName = "TrigTauRecMerged";
    std::string tauContainerName = "TrigTauRecCaloOnly";
    std::cout << "Create the HLT tau container from " << tauContainerName << std::endl;
    auto tauHltFeatures = features.containerFeature<xAOD::TauJetContainer>(tauContainerName);
    for (auto &tauContainer: tauHltFeatures) {
      if (!tauContainer.cptr()) { continue; }
      
      for (auto tau: *tauContainer.cptr()) {
        xAOD::TauJet *new_tau = new xAOD::TauJet();
        new_tau->makePrivateStore(tau);
        hlt_taus->push_back(new_tau);
        //std::cout << "tau " << tau->index() << ": Pt/Eta/Phi/Discri/IsLoose = " 
               //<< tau->pt() << " / "
               //<< tau->eta() << " / "
               //<< tau->phi() << " / "
               //<< tau->discriminant(xAOD::TauJetParameters::BDTJetScore) << " / "
               //<< tau->isTau(xAOD::TauJetParameters::JetBDTSigLoose) 
               //<< std::endl;
      }
    }

    //get the tracking info
    // NOTE: we should fix this in two ways
    // 1) only ask for this when needed (an idperf chain is the reference)
    // 2) ask it directly for the reference chain
    std::cout << "Creating track containers" << std::endl;
    auto preselTracksIsoFeatures  = features.containerFeature<xAOD::TrackParticleContainer>("InDetTrigTrackingxAODCnv_TauIso_FTF");
    auto preselTracksCoreFeatures = features.containerFeature<xAOD::TrackParticleContainer>("InDetTrigTrackingxAODCnv_TauCore_FTF");

    xAOD::TrackParticleContainer *preselTracksIso  = new xAOD::TrackParticleContainer();
    xAOD::TrackParticleContainer *preselTracksCore = new xAOD::TrackParticleContainer();
    xAOD::TrackParticleAuxContainer *preselTracksIsoAux  = new xAOD::TrackParticleAuxContainer();
    xAOD::TrackParticleAuxContainer *preselTracksCoreAux = new xAOD::TrackParticleAuxContainer();

    preselTracksIso->setStore(preselTracksIsoAux);
    preselTracksCore->setStore(preselTracksCoreAux);

    // make a bunch of decorated HLT taus
    std::vector<DecoratedHltTau> decoratedTaus;
    for (auto &tauContainer: tauHltFeatures) {
      if (!tauContainer.cptr()) { continue; }
      
      for(auto tau: *tauContainer.cptr()){
        xAOD::TauJet *new_tau = new xAOD::TauJet();
        new_tau->makePrivateStore(tau);
        DecoratedHltTau d(new_tau);

        //find the iso and core tracks for this guy
        for(auto &trackContainer: preselTracksIsoFeatures) {
          if( HLT::TrigNavStructure::haveCommonRoI(tauContainer.te(), trackContainer.te()) ){
            std::cout << "GOT AN Iso MATCH" << std::endl;
          }
        }
        
        for(auto &trackContainer: preselTracksCoreFeatures) {
          if( HLT::TrigNavStructure::haveCommonRoI(tauContainer.te(), trackContainer.te()) ){
            std::cout << "GOT A Core MATCH" << std::endl;
          }
        }
      
        decoratedTaus.push_back(d);
      }
    }

    // iso tracks
    for(auto &trackContainer: preselTracksIsoFeatures) {
      if(!trackContainer.cptr()) { continue; }
      
      for(auto track: *trackContainer.cptr()) {
        xAOD::TrackParticle *new_track = new xAOD::TrackParticle();
        new_track->makePrivateStore(track);
        preselTracksIso->push_back(new_track);
      }
    }
    
    // core tracks
    for(auto &trackContainer: preselTracksCoreFeatures) {
      if(!trackContainer.cptr()) { continue; }
      
      for(auto track: *trackContainer.cptr()) {
        xAOD::TrackParticle *new_track = new xAOD::TrackParticle();
        new_track->makePrivateStore(track);
        preselTracksCore->push_back(new_track);
      }
    }

    CHECK(emulationTool.execute(l1taus, l1jets, l1muons, l1xe, hlt_taus, preselTracksIso, preselTracksCore));
    //CHECK(emulationTool.execute(l1taus, l1jets, l1muons, l1xe, hlt_taus, presel_taus));
    //CHECK(emulationTool.execute(l1taus, l1jets, l1muons, l1xe, hlt_taus, fast_tracks));

    // Print the decision for all the tested chains and the TDT decision
    for (auto it: chains_to_test) {
      bool emulation_decision = emulationTool.decision(it);
      
      // std::cout << it << " emulation : " << emulation_decision << std::endl;
      
      auto chain_group = trigDecTool->getChainGroup(trim(it));
      bool cg_passes_event = chain_group->isPassed(triggerCondition);  
      
      fire_emul[it] += (int) emulation_decision;
      fire_tdt[it]  += (int) cg_passes_event;
      // std::cout << it << " TDT : " <<  cg_passes_event << std::endl;
      if (emulation_decision != cg_passes_event){
        if(emulation_decision) { 
          ++fire_difference_emu[it]; 
        } else { 
          ++fire_difference_TDT[it]; 
        }
        
        BrokenEventInfo eventInfo(entry, ei->eventNumber(), ei->lumiBlock(), it);
        brokenEvents.push_back( eventInfo );

        ::Info(APP_NAME, "event %d processed -- event number %d -- lumi block %d", (int)entry, (int)ei->eventNumber(), (int) ei->lumiBlock() );
        std::cout << "\n" << std::endl;
        ::Warning(APP_NAME, "TDT AND EMULATION DECISION DIFFERENT. TDT: %d -- EMULATION: %d", (int)cg_passes_event, (int)emulation_decision);
        std::cout << "\t\t Scanning construction of chain " << it << " for event " << (int)entry << " #" << (int)ei->eventNumber() << std::endl;
        auto chain = HltParsing::chains()[it];
 
        std::cout << "\t\t" << std::setw(65) << std::setfill('-') << "" << std::endl;
        std::cout << "\t\t| L1 TAUS" << std::endl;
        std::ostringstream header;
        //header << "\t\t Index |\t\t Pt |\t\t Eta |\t\t Phi |\t\t iso |\t\t type |";
        header << "\t\t|" << std::setw(6) << "Idx" << " | " << std::setw(8) << "pT" << " | " << std::setw(5) << "eta" << " | " << std::setw(9) << "phi" << " | " << std::setw(4) << "iso" << " | " << std::setw(4) << "Type" << " | ";
        for (auto &item: chain.items()) {
          if (item.isTau()) {
            header << std::setw(8) << item.seed() << " |" ;
          }
        }
        std::cout << "\t\t" << std::setw(65) << std::setfill('-') << "" << std::endl;
        std::cout << header.str() << std::endl;
        std::cout << "\t\t" << std::setw(65) << std::setfill('-') << "" << std::endl;
        for(auto l1tau: *l1taus) {
          std::ostringstream l1_line;
          l1_line << "\t\t|" << std::setw(6);
          l1_line << (l1tau->index()) ;
          l1_line << " | " << std::setw(8);
          l1_line << (l1tau->tauClus()) ;
          l1_line << " | " << std::setw(5);
          l1_line << (l1tau->eta()); 
          l1_line << " | " << std::setw(9);
          l1_line << (l1tau->phi()); 
          l1_line << " | " << std::setw(4);
          l1_line << (l1tau->emIsol()); 
          l1_line << " | " << std::setw(4);
          l1_line << (l1tau->roiType()); 
          l1_line << " | ";
          for (auto &item: chain.items()) {
            if (item.isTau()) {
              l1_line << std::setw(8) << (l1tau->auxdataConst<bool>(item.seed()));
              l1_line << " | ";
            }
          }
          std::cout << l1_line.str() << std::endl;
        }
        std::cout << "\t\t" << std::setw(65) << std::setfill('-') << "" << std::endl;
        std::cout << "\n" << std::endl;

        // preselected taus

        std::cout << "\t\t" << std::setw(68) << std::setfill('-') << "" << std::endl;
        std::cout << "\t\t| PRESEL TAUS" << std::endl;
        std::ostringstream presel_header;
        presel_header << "\t\t|" << std::setw(6) << "Idx" << " | " << std::setw(10) << "pT" << " | " << std::setw(8) << "eta" << " | " << std::setw(10) << "phi" << " | " << std::setw(8) << "N_tr" << " | " << std::setw(8) << "N_widetr" << " | ";
        std::cout << "\t\t" << std::setw(68) << std::setfill('-') << "" << std::endl;
        std::cout << presel_header.str() << std::endl;
        std::cout << "\t\t" << std::setw(68) << std::setfill('-') << "" << std::endl;

        //std::cout << "\t\t PRESEL TAUS" << std::endl;
        //std::string presel_header = "\t\t Index |\t\t Pt |\t\t Eta |\t\t Phi |\t\t Ntracks |\t\t Nwidetracks |";
        //std::cout << presel_header << std::endl;
        for(auto tau: *presel_taus) {
          std::ostringstream presel_line;
          presel_line << "\t\t|" << std::setw(6);
          presel_line << (tau->index()) ;
          presel_line << " | " << std::setw(10);
          presel_line << (tau->pt()) ;
          presel_line << " | " << std::setw(8);
          presel_line << (tau->eta()); 
          presel_line << " | " << std::setw(10);
          presel_line << (tau->phi()); 
          presel_line << " | " << std::setw(8);
          presel_line << (tau->nTracks()); 
          presel_line << " | " << std::setw(8);
          presel_line << (tau->nWideTracks()); 
          presel_line << " |";
          std::cout << presel_line.str() << std::endl;
        }
        std::cout << "\t\t" << std::setw(68) << std::setfill('-') << "" << std::endl;
        std::cout << "\n" << std::endl;
        
        // hlt taus

        std::cout << "\t\t" << std::setw(108) << std::setfill('-') << "" << std::endl;
        std::cout << "\t\t| HLT TAUS" << std::endl;
        std::ostringstream hlt_header;
        //header << "\t\t Index |\t\t Pt |\t\t Eta |\t\t Phi |\t\t iso |\t\t type |";
        hlt_header << "\t\t|" << std::setw(6) << "Idx" << " | " << std::setw(8) << "pT" << " | " << std::setw(8) << "eta" << " | " << std::setw(10) << "phi" << " | " << std::setw(10) << "l1_index";
        for (auto &item: chain.items()) {
          if (item.isTau()) {
            hlt_header << " | " << std::setw(48) << item.name();
          }
        }
        hlt_header << " |";

        std::cout << "\t\t" << std::setw(108) << std::setfill('-') << "" << std::endl;
        std::cout << hlt_header.str() << std::endl;
        std::cout << "\t\t" << std::setw(108) << std::setfill('-') << "" << std::endl;
        for(auto tau: *hlt_taus) {
          std::ostringstream hlt_line;
          hlt_line << "\t\t|" << std::setw(6);
          hlt_line << (tau->index()) ;
          hlt_line << " | " << std::setw(8);
          hlt_line << (tau->pt()) ;
          hlt_line << " | " << std::setw(8);
          hlt_line << (tau->eta()); 
          hlt_line << " | " << std::setw(10);
          hlt_line << (tau->phi()); 
          hlt_line << " | " << std::setw(10);
          hlt_line << (tau->auxdataConst<int>("l1_index")); 
          for (auto &item: chain.items()) {
            if (item.isTau()) {
              hlt_line << " | " << std::setw(48);
              hlt_line << (tau->auxdataConst<bool>(item.name()));
            }
          }
          hlt_line << " |";
          std::cout << hlt_line.str() << std::endl;
        }
        std::cout << "\t\t" << std::setw(108) << std::setfill('-') << "" << std::endl;
        std::cout << std::setfill(' ') << "\n" << std::endl;

        //return 0;
      }
    }

    // clear presel_taus and hlt_taus
    clearContainer(presel_taus);
    clearContainer(hlt_taus);
    clearContainer(preselTracksIso);
    clearContainer(preselTracksCore);

    //for(auto it: *presel_taus){
      //it->clearDecorations();
      //if(it->usingPrivateStore()){
        //it->releasePrivateStore();
      //}
    //}
    //for(auto it: *hlt_taus){
      //it->clearDecorations();
      //if(it->usingPrivateStore()){
        //it->releasePrivateStore();
      //}
    //}

    delete hlt_taus;
    delete presel_taus;

    delete preselTracksIso;
    delete preselTracksCore;

    if(hlt_taus_aux) { delete hlt_taus_aux; }
    if(presel_taus_aux) { delete presel_taus_aux; }
    if(preselTracksIsoAux) { delete preselTracksIsoAux; }
    if(preselTracksCoreAux) { delete preselTracksCoreAux; }

    store.clear();

  }
  // CHECK(emulationTool.finalize());
  std::cout << "------------------------------" << std::endl;
  for (auto ch: chains_to_test) { 
    std::cout << "| " << std::setfill(' ') << std::setw(48) << ch << " | "<< std::setw(10) << fire_tdt[ch] << " | " << std::setw(10) << fire_emul[ch] << " | only TDT: " << std::setw(10) << fire_difference_TDT[ch] << " | only emul: " << std::setw(10) << fire_difference_emu[ch] << std::endl;
  }

  
  if (brokenEvents.size() > 0 ){
    std::cout << "------------------------------" << std::endl;
    std::cout << "Problematic events: " << std::endl;
    for(auto i: brokenEvents){
      std::cout << i << std::endl;
      //std::cout << "\t" << std::setw(10) << i["entry"] << "\t event " << std::setw(12) << i["eventNumber"] << "\t LB " << std::setw(6) << i["lumiBlock"] << "\t trigger " << std::setw(48) << i["trigger"] << std::endl;
    }
    std::cout << "------------------------------" << std::endl;
  } 
  
  return 0;
}

std::vector<std::string> all_l1_chains() 

{
  std::vector<std::string> chains_to_test;

  chains_to_test.push_back("L1_TAU12");
  chains_to_test.push_back("L1_TAU12IL");
  chains_to_test.push_back("L1_TAU12IM");
  chains_to_test.push_back("L1_TAU12IT");
  chains_to_test.push_back("L1_TAU20");
  chains_to_test.push_back("L1_TAU20IL");
  chains_to_test.push_back("L1_TAU20IM");
  chains_to_test.push_back("L1_TAU20IT");
  chains_to_test.push_back("L1_TAU30");
  chains_to_test.push_back("L1_TAU40");
  chains_to_test.push_back("L1_TAU60");
  chains_to_test.push_back("L1_TAU8");
  chains_to_test.push_back("L1_J12");
  chains_to_test.push_back("L1_TAU20IM_2TAU12IM");
  chains_to_test.push_back("L1_TAU20_2TAU12");
  chains_to_test.push_back("L1_EM15HI_2TAU12");
  chains_to_test.push_back("L1_EM15HI_2TAU12IM");
  chains_to_test.push_back("L1_EM15HI_2TAU12IM_J25_3J12");
  chains_to_test.push_back("L1_EM15HI_2TAU12_J25_3J12");
  chains_to_test.push_back("L1_EM15HI_TAU40_2TAU15");
  chains_to_test.push_back("L1_J25_3J12_EM15-TAU12I");
  chains_to_test.push_back("L1_MU10_TAU12");
  chains_to_test.push_back("L1_MU10_TAU12IM");
  chains_to_test.push_back("L1_MU10_TAU12IM_J25_2J12");
  chains_to_test.push_back("L1_MU10_TAU12IL_J25_2J12");
  chains_to_test.push_back("L1_MU10_TAU12_J25_2J12");
  chains_to_test.push_back("L1_J25_2J12_DR-MU10TAU12I");
  chains_to_test.push_back("L1_MU10_TAU20");
  chains_to_test.push_back("L1_MU10_TAU20IM");
  chains_to_test.push_back("L1_TAU25IT_2TAU12IT_2J25_3J12");
  chains_to_test.push_back("L1_TAU20IL_2TAU12IL_J25_2J20_3J12");
  chains_to_test.push_back("L1_TAU20IM_2TAU12IM_J25_2J20_3J12");
  chains_to_test.push_back("L1_TAU20_2TAU12_J25_2J20_3J12");
  chains_to_test.push_back("L1_J25_2J20_3J12_BOX-TAU20ITAU12I");
  chains_to_test.push_back("L1_J25_2J20_3J12_DR-TAU20ITAU12I");
  chains_to_test.push_back("L1_DR-MU10TAU12I_TAU12I-J25");
  chains_to_test.push_back("L1_MU10_TAU12I-J25");
  chains_to_test.push_back("L1_TAU20IM_2J20_XE45");
  chains_to_test.push_back("L1_TAU20_2J20_XE45");
  chains_to_test.push_back("L1_TAU25_2J20_XE45");
  chains_to_test.push_back("L1_TAU20IM_2J20_XE50");
  chains_to_test.push_back("L1_XE45_TAU20-J20");
  chains_to_test.push_back("L1_EM15HI_2TAU12IM_XE35");
  chains_to_test.push_back("L1_EM15HI_2TAU12IL_XE35");
  chains_to_test.push_back("L1_EM15HI_2TAU12_XE35");
  chains_to_test.push_back("L1_XE35_EM15-TAU12I");
  chains_to_test.push_back("L1_XE40_EM15-TAU12I");
  chains_to_test.push_back("L1_MU10_TAU12_XE35");
  chains_to_test.push_back("L1_MU10_TAU12IM_XE35");
  chains_to_test.push_back("L1_MU10_TAU12IL_XE35");
  chains_to_test.push_back("L1_MU10_TAU12IT_XE35");
  chains_to_test.push_back("L1_MU10_TAU12IM_XE40");
  chains_to_test.push_back("L1_TAU20IM_2TAU12IM_XE35");
  chains_to_test.push_back("L1_TAU20IL_2TAU12IL_XE35");
  chains_to_test.push_back("L1_TAU20IT_2TAU12IT_XE35");
  chains_to_test.push_back("L1_TAU20_2TAU12_XE35");
  chains_to_test.push_back("L1_TAU20IM_2TAU12IM_XE40");
  chains_to_test.push_back("L1_DR-MU10TAU12I");
  chains_to_test.push_back("L1_TAU12I-J25");
  chains_to_test.push_back("L1_EM15-TAU40");
  chains_to_test.push_back("L1_TAU20-J20");
  chains_to_test.push_back("L1_EM15-TAU12I");
  chains_to_test.push_back("L1_EM15TAU12I-J25");
  chains_to_test.push_back("L1_DR-EM15TAU12I-J25");
  chains_to_test.push_back("L1_TAU20ITAU12I-J25");
  chains_to_test.push_back("L1_DR-TAU20ITAU12I");
  chains_to_test.push_back("L1_BOX-TAU20ITAU12I");
  chains_to_test.push_back("L1_DR-TAU20ITAU12I-J25");
  chains_to_test.push_back("L1_EM15");
  chains_to_test.push_back("L1_EM15HI");
  return chains_to_test;
}

