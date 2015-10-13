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

#include "TrigTauEmulation/ISelectionTool.h"
#include "TrigTauEmulation/extension.h"

using namespace bitpowder::lib;

class ToolsRegistry : public ExtensionContainer<SelectionTool*>, virtual public IToolsRegistry, virtual public asg::AsgTool
{
  ASG_TOOL_CLASS(ToolsRegistry, IToolsRegistry)
  using ToolInitializeFunction = StatusCode (ToolsRegistry::*)(void); 

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

    const FastTrackSelectionTool* getFTFTool() { return m_ftf_tool; }

  private:

    ToolHandleArray<IEmTauSelectionTool> m_l1tau_tools;
    ToolHandleArray<IEnergySumSelectionTool> m_l1xe_tools;
    ToolHandleArray<IJetRoISelectionTool> m_l1jet_tools;
    ToolHandleArray<IMuonRoISelectionTool> m_l1muon_tools;
    ToolHandleArray<IHltTauSelectionTool> m_hlttau_tools;

    bool m_recalculateBDTscore;

    std::map<std::string, ToolInitializeFunction> m_initializeFunctions;
    std::unordered_set<std::string> m_initializedToolNames;

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
    StatusCode HLT_tau0_perf_ptonly();
    StatusCode HLT_tau5_perf_ptonly();
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
    StatusCode HLT_tau50_medium1_tracktwo();
    StatusCode HLT_tau80_medium1_calo();
    StatusCode HLT_tau80_medium1_tracktwo();
    StatusCode HLT_tau125_medium1_tracktwo();
    StatusCode HLT_tau125_medium1_calo();
    StatusCode HLT_tau125_perf_tracktwo();
    StatusCode HLT_tau125_perf_ptonly();
    StatusCode HLT_tau160_medium1_tracktwo();
};


#endif
