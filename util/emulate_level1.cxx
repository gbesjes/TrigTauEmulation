// Dear emacs, this is -*- c++ -*-
// vim: ts=2 sw=2
// $Id$

// ROOT include(s):
#include <TChain.h>
#include <TFile.h>
#include <TError.h>
#include <TH1.h>

// EDM includes
#include "xAODEventInfo/EventInfo.h"
#include "xAODTrigger/JetRoIContainer.h"
#include "xAODTrigger/EmTauRoIContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"
#include "xAODTrigger/EnergySumRoI.h"

#ifdef ROOTCORE
// Local include(s):
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/TStore.h"
#include "xAODRootAccess/tools/ReturnCheck.h"
#include "xAODRootAccess/tools/Message.h"
#endif

#include "TrigTauEmulation/Utils.h"
#include "TrigTauEmulation/ToolsRegistry.h"
#include "TrigTauEmulation/Level1EmulationTool.h"
#include "TrigTauEmulation/TriggerValidation.h"

// Trigger Decision Tool
#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

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

std::vector<std::string> all_l1_chains();

int main(int argc, char **argv) {


  // Get the name of the application:
  const char* APP_NAME = "emulate_level1";

  // Initialise the environment:
  RETURN_CHECK( APP_NAME, xAOD::Init( APP_NAME ) );

  //
  ToolsRegistry registry("ToolsRegistry"); 
  CHECK(registry.initialize());

  // Initialize the tool
  TrigTauEmul::Level1EmulationTool emulationTool("TauTriggerEmulator");
  auto chains_to_test = all_l1_chains();

  CHECK(emulationTool.setProperty("l1_chains", chains_to_test));
  CHECK(emulationTool.setProperty("JetTools", registry.GetL1JetTools()));
  CHECK(emulationTool.setProperty("EmTauTools", registry.GetL1TauTools()));
  CHECK(emulationTool.setProperty("XeTools", registry.GetL1XeTools()));
  CHECK(emulationTool.setProperty("MuonTools", registry.GetL1MuonTools()));
  //emulationTool.msg().setLevel(MSG::DEBUG);
  CHECK(emulationTool.initialize());

  static const char* FNAME = "/afs/cern.ch/user/q/qbuat/work/public/"
    "mc15_13TeV/mc15_13TeV.361108.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Ztautau"
    ".recon.AOD.e3601_s2576_s2132_r6630/AOD.05358802._002522.pool.root.1";

  std::vector<std::string> filenames;
  if(argc < 2){
    filenames.push_back(std::string(FNAME));
  } else {
    filenames = Utils::splitNames(argv[1]);
  }

  // Create the TEvent object
  xAOD::TEvent event(xAOD::TEvent::kClassAccess);
  xAOD::TStore store;

  ::TChain chain1("CollectionTree");
  for(auto fname : filenames){
    chain1.Add(fname.c_str());
  }

  RETURN_CHECK(APP_NAME, event.readFrom(&chain1));

  //Set up TDT for testing
  //add config tool
  TrigConf::xAODConfigTool configTool("TrigConf::xAODConfigTool");
  ToolHandle<TrigConf::ITrigConfigTool> configHandle(&configTool);
  configHandle->initialize();

  // The decision tool
  Trig::TrigDecisionTool trigDecTool("TrigDecTool");
  trigDecTool.setProperty("ConfigTool",configHandle);
  //  trigDecTool.setProperty("OutputLevel", MSG::VERBOSE);
  trigDecTool.setProperty("TrigDecisionKey","xTrigDecision");
  trigDecTool.initialize();

  TH1D* h_TDT_EMU_diff = new TH1D("h_TDT_Emulation_differences", "TDT_Emulation_differences", chains_to_test.size(), 0, chains_to_test.size());
  TH1D* h_TDT_fires = new TH1D("h_TDT_fires", "TDT_fires_total_number", chains_to_test.size(), 0, chains_to_test.size());
  TH1D* h_EMU_fires = new TH1D("h_EMU_fires", "EMU_fires_total_number", chains_to_test.size(), 0, chains_to_test.size());
  for (unsigned int ich = 0; ich < chains_to_test.size(); ich++) {
    auto chain = chains_to_test[ich];
    h_TDT_EMU_diff->GetXaxis()->SetBinLabel(ich + 1, chain.c_str());
    h_TDT_fires->GetXaxis()->SetBinLabel(ich + 1, chain.c_str());
    h_EMU_fires->GetXaxis()->SetBinLabel(ich + 1, chain.c_str());
  }

  TriggerValidation Validator("TAU12");


  Long64_t entries = event.getEntries();
  for (Long64_t entry = 0; entry < entries; entry++) {
    if ((entry % 200) == 0)
      ::Info(APP_NAME, "Start processing event %d / %d", (int)entry, (int)entries);
    
    if (entry > 100000)
      break;

    event.getEntry(entry);

    // retrieve the EDM objects
    const xAOD::EventInfo * ei = 0;
    CHECK(event.retrieve(ei, "EventInfo"));

    // if ((int)ei->eventNumber() != 299856417) 
    //   continue;

    const xAOD::EmTauRoIContainer *l1taus = 0;
    CHECK(event.retrieve(l1taus, "LVL1EmTauRoIs"));

    // if (l1taus->size() != 0)
    //   continue;

    const xAOD::JetRoIContainer *l1jets = 0;
    CHECK(event.retrieve(l1jets, "LVL1JetRoIs"));

    const xAOD::MuonRoIContainer* l1muons = 0;
    CHECK(event.retrieve(l1muons, "LVL1MuonRoIs"));

    const xAOD::EnergySumRoI* l1xe = 0;
    CHECK(event.retrieve(l1xe, "LVL1EnergySumRoI"));


    CHECK(emulationTool.calculate(l1taus, l1jets, l1muons, l1xe));

    for (auto it: chains_to_test) {
      // emulation decision
      bool emul_passes_event = emulationTool.decision(it);

      // TDT decision
      auto chain_group = trigDecTool.getChainGroup(it);
      bool cg_passes_event = chain_group->isPassedBits() & TrigDefs::L1_isPassedBeforePrescale;  
      // bool cg_passes_event = chain_group->isPassed(TrigDefs::L1_isPassedBeforePrescale);  
      
      if(cg_passes_event) 
	h_TDT_fires->Fill(it.c_str(), 1);

      if (emul_passes_event)
	h_EMU_fires->Fill(it.c_str(), 1);

      // ::Warning(APP_NAME, "CHAIN %s: TDT: %d -- EMULATION: %d", it.c_str(), (int)cg_passes_event, (int)emul_passes_event);
      if (emul_passes_event != cg_passes_event){
        ::Warning(APP_NAME, "CHAIN %s: event %d -- event number %d -- lumi block %d", it.c_str(), (int)entry, (int)ei->eventNumber(), (int) ei->lumiBlock());
	// ::Warning(APP_NAME, "CHAIN %s: TDT: %d -- EMULATION: %d", it.c_str(), (int)cg_passes_event, (int)emul_passes_event);
	// emulationTool.PrintReport(it, l1taus, l1jets, l1muons, l1xe);
        h_TDT_EMU_diff->Fill(it.c_str(), 1);
	if (it == "L1_TAU12")
	  Validator.fill_histograms(ei, l1taus, "TAU12");
      }
    }


    // clear the decorations
    l1taus->clearDecorations();
    l1jets->clearDecorations();
    l1muons->clearDecorations();
    l1xe->clearDecorations();
    store.clear();



  }
  CHECK(emulationTool.finalize());

  // -> print out from the filled histograms
  ::Info(APP_NAME, "+--------------------------------------------------+------------+------------+--------------+");
  ::Info(APP_NAME, "|                                            Chain | TDT        | Emulation  |  differences |");
  ::Info(APP_NAME, "+--------------------------------------------------+------------+------------+--------------+");
  for (int ibin=0; ibin < h_TDT_fires->GetNbinsX(); ibin++) {
    std::ostringstream tdt_diff;
    std::string trig_name = h_TDT_fires->GetXaxis()->GetBinLabel(ibin + 1);
    int TDT_counts = h_TDT_fires->GetBinContent(ibin + 1);
    int EMU_counts = h_EMU_fires->GetBinContent(ibin + 1);
    int difference = h_TDT_EMU_diff->GetBinContent(ibin + 1);
    tdt_diff << "| " << std::setfill(' ') << std::setw(48) << trig_name
	     << " | "<< std::setw(10) << TDT_counts
	     << " | "<< std::setw(10) << EMU_counts
	     << " | "<< std::setw(12) << difference 
	     << " |";
    ::Info(APP_NAME, tdt_diff.str().c_str());
  }
  ::Info(APP_NAME, "+--------------------------------------------------+------------+------------+--------------+");

  TFile fout("level1_validation.root", "RECREATE");
  fout.cd();
  h_TDT_fires->Write();
  h_EMU_fires->Write();
  h_TDT_EMU_diff->Write();
  for (auto it: Validator.Histograms())
    it.second->Write();
  for (auto it: Validator.Maps())
    it.second->Write();


  fout.Close();

  return 0;
}

std::vector<std::string> all_l1_chains() 
{
  std::vector<std::string> chains;

  // // chains.push_back("L1_TAU8"); // not active
  // chains.push_back("L1_TAU12");
  // chains.push_back("L1_TAU12IL");
  // chains.push_back("L1_TAU12IM");
  // chains.push_back("L1_TAU12IT");
  // chains.push_back("L1_TAU20");
  // chains.push_back("L1_TAU20IL");
  // chains.push_back("L1_TAU20IM");
  // chains.push_back("L1_TAU20IT");
  // // chains.push_back("L1_TAU25IT"); // not active
  // chains.push_back("L1_TAU30");
  // chains.push_back("L1_TAU40");
  // chains.push_back("L1_TAU60");
  // chains.push_back("L1_J12");
  // chains.push_back("L1_J25");
  // chains.push_back("L1_MU10");
  // chains.push_back("L1_XE35");
  // chains.push_back("L1_XE45");
  // chains.push_back("L1_EM15");
  // chains.push_back("L1_EM15HI");
  chains.push_back("L1_TAU20IM_2TAU12IM");
  // chains.push_back("L1_TAU20_2TAU12");
  // // chains.push_back("L1_EM15HI_2TAU12"); // not active
  // chains.push_back("L1_EM15HI_2TAU12IM");
  // chains.push_back("L1_EM15HI_2TAU12IM_J25_3J12");
  // // chains.push_back("L1_EM15HI_2TAU12_J25_3J12"); // not active
  // chains.push_back("L1_EM15HI_TAU40_2TAU15");
  // chains.push_back("L1_J25_3J12_EM15-TAU12I");
  // chains.push_back("L1_MU10_TAU12");
  // chains.push_back("L1_MU10_TAU12IM");
  // chains.push_back("L1_MU10_TAU12IM_J25_2J12");
  // // chains.push_back("L1_MU10_TAU12IL_J25_2J12"); // not active
  // chains.push_back("L1_MU10_TAU12_J25_2J12");
  // // chains.push_back("L1_J25_2J12_DR-MU10TAU12I"); // not active
  // chains.push_back("L1_MU10_TAU20");
  // chains.push_back("L1_MU10_TAU20IM");
  // chains.push_back("L1_TAU25IT_2TAU12IT_2J25_3J12");
  // chains.push_back("L1_TAU20IL_2TAU12IL_J25_2J20_3J12");
  // chains.push_back("L1_TAU20IM_2TAU12IM_J25_2J20_3J12");
  // chains.push_back("L1_TAU20_2TAU12_J25_2J20_3J12");
  // chains.push_back("L1_J25_2J20_3J12_BOX-TAU20ITAU12I");
  // // chains.push_back("L1_J25_2J20_3J12_DR-TAU20ITAU12I"); // not active
  // chains.push_back("L1_DR-MU10TAU12I_TAU12I-J25");
  // chains.push_back("L1_MU10_TAU12I-J25");
  // chains.push_back("L1_TAU20IM_2J20_XE45");
  // chains.push_back("L1_TAU20_2J20_XE45");
  // // chains.push_back("L1_TAU25_2J20_XE45"); // not active
  // chains.push_back("L1_TAU20IM_2J20_XE50");
  // chains.push_back("L1_XE45_TAU20-J20");
  // chains.push_back("L1_EM15HI_2TAU12IM_XE35");
  // // chains.push_back("L1_EM15HI_2TAU12IL_XE35"); // not active
  // chains.push_back("L1_EM15HI_2TAU12_XE35");
  // chains.push_back("L1_XE35_EM15-TAU12I");
  // chains.push_back("L1_XE40_EM15-TAU12I");
  // // chains.push_back("L1_MU10_TAU12_XE35"); // not active
  // chains.push_back("L1_MU10_TAU12IM_XE35");
  // // chains.push_back("L1_MU10_TAU12IL_XE35"); // not active
  // chains.push_back("L1_MU10_TAU12IT_XE35");
  // chains.push_back("L1_MU10_TAU12IM_XE40");
  // chains.push_back("L1_TAU20IM_2TAU12IM_XE35");
  // // chains.push_back("L1_TAU20IL_2TAU12IL_XE35"); // not active
  // // chains.push_back("L1_TAU20IT_2TAU12IT_XE35"); // not active
  // chains.push_back("L1_TAU20_2TAU12_XE35");
  // chains.push_back("L1_TAU20IM_2TAU12IM_XE40");
  // chains.push_back("L1_DR-MU10TAU12I");
  // chains.push_back("L1_TAU12I-J25");
  // chains.push_back("L1_EM15-TAU40");
  // chains.push_back("L1_TAU20-J20");
  // chains.push_back("L1_EM15-TAU12I");
  // chains.push_back("L1_EM15TAU12I-J25");
  // chains.push_back("L1_DR-EM15TAU12I-J25");
  // chains.push_back("L1_TAU20ITAU12I-J25");
  chains.push_back("L1_DR-TAU20ITAU12I");
  // chains.push_back("L1_BOX-TAU20ITAU12I");
  // chains.push_back("L1_DR-TAU20ITAU12I-J25");


  return chains;
}
