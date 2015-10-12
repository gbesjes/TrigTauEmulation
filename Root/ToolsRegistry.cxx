// vim: ts=2 sw=2

#include "TrigTauEmulation/ToolsRegistry.h"


StatusCode ToolsRegistry::initializeTool(const std::string &name) {
  auto it = m_initializeFunctions.find(name);
  if(it == m_initializeFunctions.end()) {
    std::stringstream e;
    e << "Chain " << name << " not known in ToolsRegistry";
    throw std::invalid_argument(e.str());
  }

  auto func = it->second;
  ATH_CHECK( (this->*func)() );
  m_initializedToolNames.insert(name);

  return StatusCode::SUCCESS;
}

ToolsRegistry::ToolsRegistry(const std::string & name)
  : asg::AsgTool(name),
    m_l1tau_tools(),
    m_l1xe_tools(),
    m_l1jet_tools(),
    m_l1muon_tools()
{

  declareProperty("RecalculateBDTscore", m_recalculateBDTscore=false, "Recalculate tau BDT scores");

  // FTF TOOL
  m_ftf_tool = new FastTrackSelectionTool("FastTrackSelector");

  m_initializeFunctions["J12"]     = &ToolsRegistry::L1_J12;
  m_initializeFunctions["J20"]     = &ToolsRegistry::L1_J20;
  m_initializeFunctions["J25"]     = &ToolsRegistry::L1_J25;

  m_initializeFunctions["TAU8"]    = &ToolsRegistry::L1_TAU8;
  m_initializeFunctions["TAU12"]   = &ToolsRegistry::L1_TAU12;
  m_initializeFunctions["TAU15"]   = &ToolsRegistry::L1_TAU15;
  m_initializeFunctions["TAU20"]   = &ToolsRegistry::L1_TAU20;
  m_initializeFunctions["TAU25"]   = &ToolsRegistry::L1_TAU25;
  m_initializeFunctions["TAU30"]   = &ToolsRegistry::L1_TAU30;
  m_initializeFunctions["TAU40"]   = &ToolsRegistry::L1_TAU40;
  m_initializeFunctions["TAU60"]   = &ToolsRegistry::L1_TAU60;
  
  m_initializeFunctions["TAU12IL"] = &ToolsRegistry::L1_TAU12IL;
  m_initializeFunctions["TAU12IM"] = &ToolsRegistry::L1_TAU12IM;
  m_initializeFunctions["TAU12IT"] = &ToolsRegistry::L1_TAU12IT;
  m_initializeFunctions["TAU20IL"] = &ToolsRegistry::L1_TAU20IL;
  m_initializeFunctions["TAU20IM"] = &ToolsRegistry::L1_TAU20IM;
  m_initializeFunctions["TAU20IT"] = &ToolsRegistry::L1_TAU20IT;
  m_initializeFunctions["TAU25IT"] = &ToolsRegistry::L1_TAU25IT;
  
  m_initializeFunctions["EM15"]    = &ToolsRegistry::L1_EM15;
  m_initializeFunctions["EM15HI"]  = &ToolsRegistry::L1_EM15HI;
  
  m_initializeFunctions["XE35"]    = &ToolsRegistry::L1_XE35;
  m_initializeFunctions["XE40"]    = &ToolsRegistry::L1_XE40;
  m_initializeFunctions["XE45"]    = &ToolsRegistry::L1_XE45;
  m_initializeFunctions["XE50"]    = &ToolsRegistry::L1_XE50;
  m_initializeFunctions["MU10"]    = &ToolsRegistry::L1_MU10;
  m_initializeFunctions["MU20"]    = &ToolsRegistry::L1_MU20;

  m_initializeFunctions["tau0_perf_ptonly"]        = &ToolsRegistry::HLT_tau0_perf_ptonly;
  m_initializeFunctions["tau5_perf_ptonly"]        = &ToolsRegistry::HLT_tau5_perf_ptonly;
  
  m_initializeFunctions["tau25_perf_ptonly"]       = &ToolsRegistry::HLT_tau25_perf_ptonly;
  m_initializeFunctions["tau25_perf_calo"]         = &ToolsRegistry::HLT_tau25_perf_calo;
  m_initializeFunctions["tau25_perf_tracktwo"]     = &ToolsRegistry::HLT_tau25_perf_tracktwo;
  m_initializeFunctions["tau25_idperf_tracktwo"]   = &ToolsRegistry::HLT_tau25_idperf_tracktwo;
  m_initializeFunctions["tau25_loose1_ptonly"]     = &ToolsRegistry::HLT_tau25_loose1_ptonly;
  m_initializeFunctions["tau25_loose1_calo"]       = &ToolsRegistry::HLT_tau25_loose1_calo;
  m_initializeFunctions["tau25_loose1_tracktwo"]   = &ToolsRegistry::HLT_tau25_loose1_tracktwo;
  m_initializeFunctions["tau25_medium1_ptonly"]    = &ToolsRegistry::HLT_tau25_medium1_ptonly;
  m_initializeFunctions["tau25_medium1_calo"]      = &ToolsRegistry::HLT_tau25_medium1_calo;
  m_initializeFunctions["tau25_medium1_tracktwo"]  = &ToolsRegistry::HLT_tau25_medium1_tracktwo;
  m_initializeFunctions["tau25_medium1_mvonly"]    = &ToolsRegistry::HLT_tau25_medium1_mvonly;
  m_initializeFunctions["tau25_tight1_ptonly"]     = &ToolsRegistry::HLT_tau25_tight1_ptonly;
  m_initializeFunctions["tau25_tight1_calo"]       = &ToolsRegistry::HLT_tau25_tight1_calo;
  m_initializeFunctions["tau25_tight1_tracktwo"]   = &ToolsRegistry::HLT_tau25_tight1_tracktwo;
  
  m_initializeFunctions["tau35_loose1_tracktwo"]   = &ToolsRegistry::HLT_tau35_loose1_tracktwo;
  m_initializeFunctions["tau35_loose1_ptonly"]     = &ToolsRegistry::HLT_tau35_loose1_ptonly;
  m_initializeFunctions["tau35_medium1_tracktwo"]  = &ToolsRegistry::HLT_tau35_medium1_tracktwo;
  m_initializeFunctions["tau35_medium1_ptonly"]    = &ToolsRegistry::HLT_tau35_medium1_ptonly;
  m_initializeFunctions["tau35_medium1_calo"]      = &ToolsRegistry::HLT_tau35_medium1_calo;
  m_initializeFunctions["tau35_tight1_tracktwo"]   = &ToolsRegistry::HLT_tau35_tight1_tracktwo;
  m_initializeFunctions["tau35_tight1_ptonly"]     = &ToolsRegistry::HLT_tau35_tight1_ptonly;
  m_initializeFunctions["tau35_perf_tracktwo"]     = &ToolsRegistry::HLT_tau35_perf_tracktwo;
  m_initializeFunctions["tau35_perf_ptonly"]       = &ToolsRegistry::HLT_tau35_perf_ptonly;
  
  m_initializeFunctions["tau50_medium1_tracktwo"]  = &ToolsRegistry::HLT_tau50_medium1_tracktwo;
  
  m_initializeFunctions["tau80_medium1_calo"]      = &ToolsRegistry::HLT_tau80_medium1_calo;
  m_initializeFunctions["tau80_medium1_tracktwo"]  = &ToolsRegistry::HLT_tau80_medium1_tracktwo;
  
  m_initializeFunctions["tau125_medium1_tracktwo"] = &ToolsRegistry::HLT_tau125_medium1_tracktwo;
  m_initializeFunctions["tau125_medium1_calo"]     = &ToolsRegistry::HLT_tau125_medium1_calo;
  m_initializeFunctions["tau125_perf_tracktwo"]    = &ToolsRegistry::HLT_tau125_perf_tracktwo;
  m_initializeFunctions["tau125_perf_ptonly"]      = &ToolsRegistry::HLT_tau125_perf_ptonly;
  
  m_initializeFunctions["tau160_medium1_tracktwo"] = &ToolsRegistry::HLT_tau160_medium1_tracktwo;

}

ToolsRegistry::~ToolsRegistry(){
    //for(auto it: m_initializedToolNames){
    //  TODO: get ptr to tool from toolstore; delete it
      //auto tool = it;
      //delete tool;
    //}

}

ToolsRegistry::ToolsRegistry(const ToolsRegistry& other): asg::AsgTool(other.name() + "_copy")
{}

StatusCode ToolsRegistry::initialize()
{
  m_ftf_tool->msg().setLevel(this->msg().level());
  ATH_CHECK(m_ftf_tool->initialize());

  return StatusCode::SUCCESS;
}
