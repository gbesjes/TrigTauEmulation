// Dear emacs, this is -*- c++ -*-
// $Id$
// System include(s):
#include <memory>
#include <set>
#include <string>
#include <algorithm>

// ROOT include(s):
#include <TFile.h>
#include <TChain.h>
#include <TError.h>
#include <TTree.h>
#include <TSystem.h>
#include <TH1F.h>

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

// Local include(s):
#ifdef ROOTCORE
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/TStore.h"
#include "xAODRootAccess/tools/ReturnCheck.h"
#include "xAODRootAccess/tools/Message.h"
#endif

#include "AsgTools/ToolHandle.h"

#include "TrigTauEmulation/Utils.h"

// Trigger Decision Tool
#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

using namespace TrigConf;
using namespace Trig;


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

int main(int argc, char** argv) {


  TH1F h("h_ntracks", "h_ntracks", 10, 0, 10);
  // Get the name of the application:
  const char* APP_NAME = "tautriggeremulationtool_hlt_quentin";

  if (argc != 1) {
    ::Error(APP_NAME, XAOD_MESSAGE("Wrong number of arguments"));
    return 1;
  }
  std::string chain_to_test = "HLT_tau25_idperf_tracktwo";

  // Initialise the environment:
  RETURN_CHECK( APP_NAME, xAOD::Init( APP_NAME ) );

   // Create the TEvent object
   xAOD::TEvent event(xAOD::TEvent::kClassAccess);
   // CHECK( event.readFrom(ifile.get()) );

   xAOD::TStore store;


   ::TChain chain1("CollectionTree");
   chain1.Add("/afs/cern.ch/work/q/qbuat/public/mc15_13TeV/mc15_13TeV.361108.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Ztautau.recon.AOD.e3601_s2576_s2132_r6630/*root.1");
   // chain1.Add("/tmp/qbuat/mc15_13TeV.361108.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Ztautau.recon.AOD.e3601_s2576_s2132_r6630_tid05367726_00/*root.1");
   // chain1.Add("/tmp/qbuat/user.dzanzi.data15_13TeV.00267638.physics_EnhancedBias.merge.RAW_EXT0.31882626/user.dzanzi.5729275.EXT0._000004.AOD.root");
   // chain1.Add("/tmp/qbuat/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.merge.AOD.e3698_s2608_s2183_r6630_r6264_tid05382618_00/*root*");

   RETURN_CHECK(APP_NAME, event.readFrom(&chain1));
  //Set up TDT for testing
  //add config tool
  // xAODConfigTool configTool("TrigConf::xAODConfigTool");
  auto* trigConfigTool = new TrigConf::xAODConfigTool("xAODConfigTool");
  ToolHandle<TrigConf::ITrigConfigTool> configHandle(trigConfigTool);
  configHandle->initialize();

  // The decision tool
  Trig::TrigDecisionTool *trigDecTool = new Trig::TrigDecisionTool("TrigDecTool");
  trigDecTool->setProperty("ConfigTool", configHandle);
  //  trigDecTool.setProperty("OutputLevel", MSG::VERBOSE);
  trigDecTool->setProperty("TrigDecisionKey", "xTrigDecision");
  trigDecTool->initialize();



   Long64_t entries = event.getEntries();
   for (Long64_t entry = 0; entry < entries; entry++) {

     ::Info(APP_NAME, "Start processing event %d", (int)entry);

     event.getEntry(entry);

     // retrieve the EDM objects
     const xAOD::EventInfo * ei = 0;
     CHECK(event.retrieve(ei, "EventInfo"));

     const xAOD::EmTauRoIContainer *l1taus = 0;
     CHECK(event.retrieve(l1taus, "LVL1EmTauRoIs"));

     const xAOD::JetRoIContainer *l1jets = 0;
     CHECK(event.retrieve(l1jets, "LVL1JetRoIs"));

     const xAOD::MuonRoIContainer* l1muons = 0;
     CHECK(event.retrieve(l1muons, "LVL1MuonRoIs"));

     const xAOD::EnergySumRoI* l1xe = 0;
     CHECK(event.retrieve(l1xe, "LVL1EnergySumRoI"));


     if (not trigDecTool->isPassed(chain_to_test)) 
       continue;

     // std::cout << "features !" << std::endl;
     auto cg = trigDecTool->getChainGroup(chain_to_test);
     auto features = cg->features();


     auto tauHltFeatures = features.containerFeature<xAOD::TauJetContainer>("TrigTauRecMerged");
     auto preselTracksIsoFeatures  = features.containerFeature<xAOD::TrackParticleContainer>("InDetTrigTrackingxAODCnv_TauIso_FTF");
     auto preselTracksCoreFeatures = features.containerFeature<xAOD::TrackParticleContainer>("InDetTrigTrackingxAODCnv_TauCore_FTF");
     ::Info(APP_NAME, "Container size: HLT = %d, TauCore = %d, TauIso = %d", (int)tauHltFeatures.size(), (int)preselTracksCoreFeatures.size(), (int)preselTracksIsoFeatures.size());
     if (tauHltFeatures.size() != preselTracksIsoFeatures.size())
       ::Fatal(APP_NAME, "HLT and TauIso features have different sizes");
     if (tauHltFeatures.size() != preselTracksCoreFeatures.size())
       ::Fatal(APP_NAME, "HLT and TauCore features have different sizes");
     if (preselTracksCoreFeatures.size() != preselTracksIsoFeatures.size())
       ::Fatal(APP_NAME, "TauIso and TauCore features have different sizes");


     for (unsigned int i=0; i < tauHltFeatures.size(); i++) {
       auto taus  = tauHltFeatures.at(i).cptr();
       auto cores = preselTracksCoreFeatures.at(i).cptr();
       auto isos = preselTracksIsoFeatures.at(i).cptr();
       if (not taus or not cores or not isos)
	 continue;
       if (taus->size() != 1) 
	 ::Fatal(APP_NAME, "More than 1 tau for a single tau chain, does not make any sense...");
       ::Info(APP_NAME, "Feature %d: taus = %d, cores = %d, isos = %d", (int)i, (int)taus->size(), (int)cores->size(), (int)isos->size());
       auto *tau = taus->at(0);
       // std::vector<std::string> lines_to_print;
       for (unsigned int il = 0; il < std::max(cores->size(), isos->size()); il++) {
	 std::ostringstream line; 
	 if (il < 1) 
	   line << "| (" 
		<< std::setw(5) << (tau->pt())
		<< ", " 
		<< std::setw(5) << (tau->eta())
		<< ", " 
		<< std::setw(5) << (tau->phi()) 
		<< ") |";
	 else
	   line << "| " << std::setw(30) << std::setfill('-') << " | ";
	  
	 if (il < cores->size())
	   line << "(" << il << ": dR = " << Utils::DeltaR(cores->at(il)->eta(), cores->at(il)->phi(), tau->eta(), tau->phi()) << ") | ";
	 else
	   line << "------------------- | ";

	 if (il < isos->size())
	   line << "(" << il << ": dR = "<< Utils::DeltaR(isos->at(il)->eta(), isos->at(il)->phi(), tau->eta(), tau->phi()) << ") |";
	 else
	   line << "------------------- |";
	 ::Info(APP_NAME, line.str().c_str());
       }

     }



   }

   return 0;
}

