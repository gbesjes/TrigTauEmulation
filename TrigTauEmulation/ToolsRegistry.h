// vim: ts=2 sw=2
#ifndef TOOLSREGISTRY_TOOLSREGISTRY_H
#define TOOLSREGISTRY_TOOLSREGISTRY_H

# include <unordered_set>

// Framework includes
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/ToolHandleArray.h"

#include "TrigTauEmulation/IToolsRegistry.h"

#include "TrigTauEmulation/IEmTauSelectionTool.h"
#include "TrigTauEmulation/IEnergySumSelectionTool.h"
#include "TrigTauEmulation/IJetRoISelectionTool.h"
#include "TrigTauEmulation/IMuonRoISelectionTool.h"

#include "TrigTauEmulation/EmTauSelectionTool.h"
#include "TrigTauEmulation/EnergySumSelectionTool.h"
#include "TrigTauEmulation/JetRoISelectionTool.h"
#include "TrigTauEmulation/MuonRoISelectionTool.h"

#include "TrigTauEmulation/IHltTauSelectionTool.h"
#include "TrigTauEmulation/HltTauSelectionTool.h"

#include "TrigTauEmulation/FastTrackSelectionTool.h"

#include <type_traits>

class ToolsRegistry : virtual public IToolsRegistry, virtual public asg::AsgTool
{
  ASG_TOOL_CLASS(ToolsRegistry, IToolsRegistry)

  //typedef StatusCode (ToolsRegistry::*ToolInitializeFunction)(void);

  using ToolInitializeFunction = StatusCode (ToolsRegistry::*)(void); // New introduced syntax

  public:

    ToolsRegistry(const std::string& name);
    ToolsRegistry(const ToolsRegistry& other);

    virtual ~ToolsRegistry();

    /// Initialize the tool
    virtual StatusCode initialize();

    ToolHandleArray<IEmTauSelectionTool> GetL1TauTools() { return m_l1tau_tools; }
    ToolHandleArray<IEnergySumSelectionTool> GetL1XeTools() { return m_l1xe_tools; }
    ToolHandleArray<IJetRoISelectionTool> GetL1JetTools() { return m_l1jet_tools; }
    ToolHandleArray<IMuonRoISelectionTool> GetL1MuonTools() { return m_l1muon_tools; }
    ToolHandleArray<IHltTauSelectionTool> GetHltTauTools() { return m_hlttau_tools; }

    StatusCode initializeTool(const std::string &name);

  private:

    ToolHandleArray<IEmTauSelectionTool> m_l1tau_tools;
    ToolHandleArray<IEnergySumSelectionTool> m_l1xe_tools;
    ToolHandleArray<IJetRoISelectionTool> m_l1jet_tools;
    ToolHandleArray<IMuonRoISelectionTool> m_l1muon_tools;
    ToolHandleArray<IHltTauSelectionTool> m_hlttau_tools;

    bool m_recalculateBDTscore;

    std::map<std::string, ToolInitializeFunction> m_initializeFunctions;
    std::unordered_set<std::string> m_initializedToolNames;
    //std::vector<ILevel1SelectionTool*> m_initializedLevel1SelectionTools;
    //std::vector<IHltTauSelectionTool*> m_initializedHltTauSelectionTools;

    // FTF Ttool
    FastTrackSelectionTool* m_ftf_tool;

    // L1 TAUS
    StatusCode L1_J12();
    StatusCode L1_J20();
    StatusCode L1_J25();
    StatusCode L1_TAU8();
    StatusCode L1_TAU12();
    StatusCode L1_TAU15();
    StatusCode L1_TAU20();
    StatusCode L1_TAU25();
    StatusCode L1_TAU30();
    StatusCode L1_TAU40();
    StatusCode L1_TAU60();
    StatusCode L1_TAU12IL();
    StatusCode L1_TAU12IM();
    StatusCode L1_TAU12IT();
    StatusCode L1_TAU20IL();
    StatusCode L1_TAU20IM();
    StatusCode L1_TAU20IT();
    StatusCode L1_TAU25IT();
    StatusCode L1_EM15();
    StatusCode L1_EM15HI();
    StatusCode L1_XE35();
    StatusCode L1_XE40();
    StatusCode L1_XE45();
    StatusCode L1_XE50();
    StatusCode L1_MU10();
    StatusCode L1_MU20();

    // HLT TAUS
    StatusCode HLT_tau25_perf_ptonly();
    StatusCode HLT_tau25_perf_calo();
    StatusCode HLT_tau25_perf_tracktwo();
    StatusCode HLT_tau25_idperf_tracktwo();
    StatusCode HLT_tau25_loose1_ptonly();
    StatusCode HLT_tau25_loose1_calo();
    StatusCode HLT_tau25_loose1_tracktwo();
    StatusCode HLT_tau25_medium1_ptonly();
    StatusCode HLT_tau25_medium1_calo();
    StatusCode HLT_tau25_medium1_tracktwo();
    StatusCode HLT_tau25_medium1_mvonly();
    StatusCode HLT_tau25_tight1_ptonly();
    StatusCode HLT_tau25_tight1_calo();
    StatusCode HLT_tau25_tight1_tracktwo();
    StatusCode HLT_tau35_loose1_tracktwo();
    StatusCode HLT_tau35_loose1_ptonly();
    StatusCode HLT_tau35_medium1_tracktwo();
    StatusCode HLT_tau35_medium1_ptonly();
    StatusCode HLT_tau35_medium1_calo();
    StatusCode HLT_tau35_tight1_tracktwo();
    StatusCode HLT_tau35_tight1_ptonly();
    StatusCode HLT_tau35_perf_tracktwo();
    StatusCode HLT_tau35_perf_ptonly();
    StatusCode HLT_tau80_medium1_calo();
    StatusCode HLT_tau80_medium1_tracktwo();
    StatusCode HLT_tau50_medium1_tracktwo();
    StatusCode HLT_tau125_medium1_tracktwo();
    StatusCode HLT_tau125_medium1_calo();
    StatusCode HLT_tau125_perf_tracktwo();
    StatusCode HLT_tau125_perf_ptonly();
    StatusCode HLT_tau160_medium1_tracktwo();
    StatusCode HLT_tau5_perf_ptonly();
    StatusCode HLT_tau0_perf_ptonly();

    //// Declaration of the tools themselves
    //// --> L1 JETS
    //JetRoISelectionTool* m_l1jet_tool_12;
    //JetRoISelectionTool* m_l1jet_tool_20;
    //JetRoISelectionTool* m_l1jet_tool_25;

    //// --> L1 TAU / ELE
    //EmTauSelectionTool* m_l1tau_tool_8;
    //EmTauSelectionTool* m_l1tau_tool_12;
    //EmTauSelectionTool* m_l1tau_tool_15;
    //EmTauSelectionTool* m_l1tau_tool_20;
    //EmTauSelectionTool* m_l1tau_tool_25;
    //EmTauSelectionTool* m_l1tau_tool_30;
    //EmTauSelectionTool* m_l1tau_tool_40;
    //EmTauSelectionTool* m_l1tau_tool_60;

    //EmTauSelectionTool* m_l1tau_tool_12IL;
    //EmTauSelectionTool* m_l1tau_tool_12IM;
    //EmTauSelectionTool* m_l1tau_tool_12IT;

    //EmTauSelectionTool* m_l1tau_tool_20IL;
    //EmTauSelectionTool* m_l1tau_tool_20IM;
    //EmTauSelectionTool* m_l1tau_tool_20IT;

    //EmTauSelectionTool* m_l1tau_tool_25IT;

    //EmTauSelectionTool* m_l1ele_tool_15;
    //EmTauSelectionTool* m_l1ele_tool_15HI;

    //// --> L1 MET
    //EnergySumSelectionTool* m_l1xe_tool_35;
    //EnergySumSelectionTool* m_l1xe_tool_40;
    //EnergySumSelectionTool* m_l1xe_tool_45;
    //EnergySumSelectionTool* m_l1xe_tool_50;

    //// --> L1 MUONS
    //MuonRoISelectionTool* m_l1muon_tool_10;
    //MuonRoISelectionTool* m_l1muon_tool_20;

    //// --> HLT TAUS
    //HltTauSelectionTool* m_hlttau_tool_25_perf_ptonly;
    //HltTauSelectionTool* m_hlttau_tool_25_perf_calo;
    //HltTauSelectionTool* m_hlttau_tool_25_perf_tracktwo;

    //HltTauSelectionTool* m_hlttau_tool_25_idperf_tracktwo;

    //HltTauSelectionTool* m_hlttau_tool_25_loose1_ptonly;
    //HltTauSelectionTool* m_hlttau_tool_25_loose1_calo;
    //HltTauSelectionTool* m_hlttau_tool_25_loose1_tracktwo;

    //HltTauSelectionTool* m_hlttau_tool_25_medium1_ptonly;
    //HltTauSelectionTool* m_hlttau_tool_25_medium1_calo;
    //HltTauSelectionTool* m_hlttau_tool_25_medium1_tracktwo;
    //HltTauSelectionTool* m_hlttau_tool_25_medium1_mvonly;

    //HltTauSelectionTool* m_hlttau_tool_25_tight1_ptonly;
    //HltTauSelectionTool* m_hlttau_tool_25_tight1_calo;
    //HltTauSelectionTool* m_hlttau_tool_25_tight1_tracktwo;

    //HltTauSelectionTool* m_hlttau_tool_35_loose1_tracktwo;
    //HltTauSelectionTool* m_hlttau_tool_35_loose1_ptonly;

    //HltTauSelectionTool* m_hlttau_tool_35_medium1_tracktwo;
    //HltTauSelectionTool* m_hlttau_tool_35_medium1_ptonly;
    //HltTauSelectionTool* m_hlttau_tool_35_medium1_calo;

    //HltTauSelectionTool* m_hlttau_tool_35_tight1_tracktwo;
    //HltTauSelectionTool* m_hlttau_tool_35_tight1_ptonly;

    //HltTauSelectionTool* m_hlttau_tool_35_perf_tracktwo;
    //HltTauSelectionTool* m_hlttau_tool_35_perf_ptonly;

    //HltTauSelectionTool* m_hlttau_tool_80_medium1_calo;
    //HltTauSelectionTool* m_hlttau_tool_80_medium1_tracktwo;

    //HltTauSelectionTool* m_hlttau_tool_50_medium1_tracktwo;

    //HltTauSelectionTool* m_hlttau_tool_125_medium1_tracktwo;
    //HltTauSelectionTool* m_hlttau_tool_125_medium1_calo;
    //HltTauSelectionTool* m_hlttau_tool_125_perf_tracktwo;
    //HltTauSelectionTool* m_hlttau_tool_125_perf_ptonly;

    //HltTauSelectionTool* m_hlttau_tool_160_medium1_tracktwo;

    //HltTauSelectionTool* m_hlttau_tool_5_perf_ptonly;
    //HltTauSelectionTool* m_hlttau_tool_0_perf_ptonly;

};


#endif
