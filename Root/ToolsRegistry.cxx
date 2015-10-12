// vim: ts=2 sw=2

#include "TrigTauEmulation/ToolsRegistry.h"

StatusCode ToolsRegistry::L1_TAU12IM() {
  auto m_l1tau_tool_12IM = new EmTauSelectionTool("TAU12IM");
  m_l1tau_tools.push_back(ToolHandle<IEmTauSelectionTool>(m_l1tau_tool_12IM));
  
  ATH_CHECK(m_l1tau_tool_12IM->setProperty("ClusterPt", 12000.));
  ATH_CHECK(m_l1tau_tool_12IM->setProperty("IsolationOffset", 2000.));

  //m_initializedLevel1SelectionTools.push_back(m_l1tau_tool_12IM);

  return StatusCode::SUCCESS;
}

StatusCode ToolsRegistry::L1_TAU12() {
  auto m_l1tau_tool_12 = new EmTauSelectionTool("TAU12");
  m_l1tau_tools.push_back(ToolHandle<IEmTauSelectionTool>(m_l1tau_tool_12));
  
  ATH_CHECK(m_l1tau_tool_12->setProperty("ClusterPt", 12000.));
  ATH_CHECK(m_l1tau_tool_12->setProperty("IsolationThresh", -9999.));

  //m_initializedTools.push_back(m_l1tau_tool_12);

  return StatusCode::SUCCESS;
}

StatusCode ToolsRegistry::HLT_tau25_perf_tracktwo() {
  auto m_hlttau_tool_25_perf_tracktwo = new HltTauSelectionTool("tau25_perf_tracktwo");
  m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_25_perf_tracktwo));
  
  ToolHandle<FastTrackSelectionTool> ftf_handle(m_ftf_tool);
  ATH_CHECK(m_hlttau_tool_25_perf_tracktwo->setProperty("PreselPt", 25000.));
  ATH_CHECK(m_hlttau_tool_25_perf_tracktwo->setProperty("UsePresel", false));
  ATH_CHECK(m_hlttau_tool_25_perf_tracktwo->setProperty("UseCaloPresel", false));
  ATH_CHECK(m_hlttau_tool_25_perf_tracktwo->setProperty("UseFastTracking", true));
  ATH_CHECK(m_hlttau_tool_25_perf_tracktwo->setProperty("FastTrackSelectionTool", ftf_handle));
  ATH_CHECK(m_hlttau_tool_25_perf_tracktwo->setProperty("UseTauID", false));
  ATH_CHECK(m_hlttau_tool_25_perf_tracktwo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //m_initializedTools.push_back(m_hlttau_tool_25_perf_tracktwo);

  return StatusCode::SUCCESS;
}

StatusCode ToolsRegistry::HLT_tau25_idperf_tracktwo() {
  auto m_hlttau_tool_25_idperf_tracktwo = new HltTauSelectionTool("tau25_idperf_tracktwo");
  m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_25_idperf_tracktwo));
  
  ATH_CHECK(m_hlttau_tool_25_idperf_tracktwo->setProperty("PreselPt", 25000.));
  ATH_CHECK(m_hlttau_tool_25_idperf_tracktwo->setProperty("UsePresel", false));
  ATH_CHECK(m_hlttau_tool_25_idperf_tracktwo->setProperty("UseFastTracking", false));
  ATH_CHECK(m_hlttau_tool_25_idperf_tracktwo->setProperty("UseTauID", false));
  ATH_CHECK(m_hlttau_tool_25_idperf_tracktwo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  return StatusCode::SUCCESS;
}

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
  
  m_initializeFunctions["TAU12"] = &ToolsRegistry::L1_TAU12; 
  m_initializeFunctions["TAU12IM"] = &ToolsRegistry::L1_TAU12IM; 
  m_initializeFunctions["tau25_idperf_tracktwo"] = &ToolsRegistry::HLT_tau25_idperf_tracktwo; 
  m_initializeFunctions["tau25_perf_tracktwo"] = &ToolsRegistry::HLT_tau25_perf_tracktwo; 

  //// Declaration of the all the tools we might need in the run
  //// This block of code could be significantly improved by only making
  //// instances of the tools we actually need.
  //m_l1jet_tool_12 = new JetRoISelectionTool("J12");
  //m_l1jet_tool_20 = new JetRoISelectionTool("J20");
  //m_l1jet_tool_25 = new JetRoISelectionTool("J25");

  //m_l1jet_tools.push_back(ToolHandle<IJetRoISelectionTool>(m_l1jet_tool_12));
  //m_l1jet_tools.push_back(ToolHandle<IJetRoISelectionTool>(m_l1jet_tool_20));
  //m_l1jet_tools.push_back(ToolHandle<IJetRoISelectionTool>(m_l1jet_tool_25));


  //// L1 TAUS / ELE
  //m_l1tau_tool_8  = new EmTauSelectionTool("TAU8");
  //m_l1tau_tool_12 = new EmTauSelectionTool("TAU12");
  //m_l1tau_tool_15 = new EmTauSelectionTool("TAU15");
  //m_l1tau_tool_20 = new EmTauSelectionTool("TAU20");
  //m_l1tau_tool_25 = new EmTauSelectionTool("TAU25");
  //m_l1tau_tool_30 = new EmTauSelectionTool("TAU30");
  //m_l1tau_tool_40 = new EmTauSelectionTool("TAU40");
  //m_l1tau_tool_60 = new EmTauSelectionTool("TAU60");

  //m_l1tau_tool_12IL = new EmTauSelectionTool("TAU12IL");
  //m_l1tau_tool_12IM = new EmTauSelectionTool("TAU12IM");
  //m_l1tau_tool_12IT = new EmTauSelectionTool("TAU12IT");
  //m_l1tau_tool_20IL = new EmTauSelectionTool("TAU20IL");
  //m_l1tau_tool_20IM = new EmTauSelectionTool("TAU20IM");
  //m_l1tau_tool_20IT = new EmTauSelectionTool("TAU20IT");
  //m_l1tau_tool_25IT = new EmTauSelectionTool("TAU25IT");

  //m_l1ele_tool_15 = new EmTauSelectionTool("EM15");
  //m_l1ele_tool_15HI = new EmTauSelectionTool("EM15HI");


  //m_l1tau_tools.push_back(ToolHandle<IEmTauSelectionTool>(m_l1tau_tool_8));
  //m_l1tau_tools.push_back(ToolHandle<IEmTauSelectionTool>(m_l1tau_tool_12));
  //m_l1tau_tools.push_back(ToolHandle<IEmTauSelectionTool>(m_l1tau_tool_15));
  //m_l1tau_tools.push_back(ToolHandle<IEmTauSelectionTool>(m_l1tau_tool_20));
  //m_l1tau_tools.push_back(ToolHandle<IEmTauSelectionTool>(m_l1tau_tool_25));
  //m_l1tau_tools.push_back(ToolHandle<IEmTauSelectionTool>(m_l1tau_tool_30));
  //m_l1tau_tools.push_back(ToolHandle<IEmTauSelectionTool>(m_l1tau_tool_40));
  //m_l1tau_tools.push_back(ToolHandle<IEmTauSelectionTool>(m_l1tau_tool_60));

  //m_l1tau_tools.push_back(ToolHandle<IEmTauSelectionTool>(m_l1tau_tool_12IL));
  //m_l1tau_tools.push_back(ToolHandle<IEmTauSelectionTool>(m_l1tau_tool_12IM));
  //m_l1tau_tools.push_back(ToolHandle<IEmTauSelectionTool>(m_l1tau_tool_12IT));
  //m_l1tau_tools.push_back(ToolHandle<IEmTauSelectionTool>(m_l1tau_tool_20IL));
  //m_l1tau_tools.push_back(ToolHandle<IEmTauSelectionTool>(m_l1tau_tool_20IM));
  //m_l1tau_tools.push_back(ToolHandle<IEmTauSelectionTool>(m_l1tau_tool_20IT));
  //m_l1tau_tools.push_back(ToolHandle<IEmTauSelectionTool>(m_l1tau_tool_25IT));

  //m_l1tau_tools.push_back(ToolHandle<IEmTauSelectionTool>(m_l1ele_tool_15));
  //m_l1tau_tools.push_back(ToolHandle<IEmTauSelectionTool>(m_l1ele_tool_15HI));


  //m_l1xe_tool_35 = new EnergySumSelectionTool("XE35");
  //m_l1xe_tool_40 = new EnergySumSelectionTool("XE40");
  //m_l1xe_tool_45 = new EnergySumSelectionTool("XE45");
  //m_l1xe_tool_50 = new EnergySumSelectionTool("XE50");

  //m_l1xe_tools.push_back(ToolHandle<IEnergySumSelectionTool>(m_l1xe_tool_35));
  //m_l1xe_tools.push_back(ToolHandle<IEnergySumSelectionTool>(m_l1xe_tool_40));
  //m_l1xe_tools.push_back(ToolHandle<IEnergySumSelectionTool>(m_l1xe_tool_45));
  //m_l1xe_tools.push_back(ToolHandle<IEnergySumSelectionTool>(m_l1xe_tool_50));

  //m_l1muon_tool_10 = new MuonRoISelectionTool("MU10");
  //m_l1muon_tool_20 = new MuonRoISelectionTool("MU20");

  //m_l1muon_tools.push_back(ToolHandle<IMuonRoISelectionTool>(m_l1muon_tool_10));
  //m_l1muon_tools.push_back(ToolHandle<IMuonRoISelectionTool>(m_l1muon_tool_20));

  //// HLT Taus
  //m_hlttau_tool_25_perf_ptonly          = new HltTauSelectionTool("tau25_perf_ptonly");
  //m_hlttau_tool_25_perf_calo            = new HltTauSelectionTool("tau25_perf_calo");
  //m_hlttau_tool_25_perf_tracktwo        = new HltTauSelectionTool("tau25_perf_tracktwo");
  //m_hlttau_tool_25_idperf_tracktwo      = new HltTauSelectionTool("tau25_idperf_tracktwo");
  //m_hlttau_tool_25_loose1_ptonly        = new HltTauSelectionTool("tau25_loose1_ptonly");
  //m_hlttau_tool_25_loose1_calo          = new HltTauSelectionTool("tau25_loose1_calo");
  //m_hlttau_tool_25_loose1_tracktwo      = new HltTauSelectionTool("tau25_loose1_tracktwo");
  //m_hlttau_tool_25_medium1_ptonly       = new HltTauSelectionTool("tau25_medium1_ptonly");
  //m_hlttau_tool_25_medium1_calo         = new HltTauSelectionTool("tau25_medium1_calo");
  //m_hlttau_tool_25_medium1_tracktwo     = new HltTauSelectionTool("tau25_medium1_tracktwo");
  //m_hlttau_tool_25_medium1_mvonly       = new HltTauSelectionTool("tau25_medium1_mvonly");
  //m_hlttau_tool_25_tight1_ptonly        = new HltTauSelectionTool("tau25_tight1_ptonly");
  //m_hlttau_tool_25_tight1_calo          = new HltTauSelectionTool("tau25_tight1_calo");
  //m_hlttau_tool_25_tight1_tracktwo      = new HltTauSelectionTool("tau25_tight1_tracktwo");
  //m_hlttau_tool_35_loose1_tracktwo      = new HltTauSelectionTool("tau35_loose1_tracktwo");    
  //m_hlttau_tool_35_loose1_ptonly        = new HltTauSelectionTool("tau35_loose1_ptonly");	    
  //m_hlttau_tool_35_medium1_tracktwo     = new HltTauSelectionTool("tau35_medium1_tracktwo");   
  //m_hlttau_tool_35_medium1_ptonly       = new HltTauSelectionTool("tau35_medium1_ptonly");	    
  //m_hlttau_tool_35_medium1_calo         = new HltTauSelectionTool("tau35_medium1_calo");	    
  //m_hlttau_tool_35_tight1_tracktwo      = new HltTauSelectionTool("tau35_tight1_tracktwo");    
  //m_hlttau_tool_35_tight1_ptonly        = new HltTauSelectionTool("tau35_tight1_ptonly");	    
  //m_hlttau_tool_35_perf_tracktwo        = new HltTauSelectionTool("tau35_perf_tracktwo");	    
  //m_hlttau_tool_35_perf_ptonly          = new HltTauSelectionTool("tau35_perf_ptonly");	    
  //m_hlttau_tool_80_medium1_calo         = new HltTauSelectionTool("tau80_medium1_calo");	    
  //m_hlttau_tool_80_medium1_tracktwo     = new HltTauSelectionTool("tau80_medium1_tracktwo");   
  //m_hlttau_tool_50_medium1_tracktwo     = new HltTauSelectionTool("tau50_medium1_tracktwo");   
  //m_hlttau_tool_125_medium1_tracktwo    = new HltTauSelectionTool("tau125_medium1_tracktwo");  
  //m_hlttau_tool_125_medium1_calo        = new HltTauSelectionTool("tau125_medium1_calo");	    
  //m_hlttau_tool_125_perf_tracktwo       = new HltTauSelectionTool("tau125_perf_tracktwo");	    
  //m_hlttau_tool_125_perf_ptonly         = new HltTauSelectionTool("tau125_perf_ptonly");	    
  //m_hlttau_tool_160_medium1_tracktwo    = new HltTauSelectionTool("tau160_medium1_tracktwo");  
  //m_hlttau_tool_5_perf_ptonly           = new HltTauSelectionTool("tau5_perf_ptonly");	    
  //m_hlttau_tool_0_perf_ptonly           = new HltTauSelectionTool("tau0_perf_ptonly");         


  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_25_perf_ptonly          ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_25_perf_calo            ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_25_perf_tracktwo        ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_25_idperf_tracktwo      ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_25_loose1_ptonly        ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_25_loose1_calo          ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_25_loose1_tracktwo      ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_25_medium1_ptonly       ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_25_medium1_calo         ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_25_medium1_tracktwo     ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_25_medium1_mvonly       ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_25_tight1_ptonly        ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_25_tight1_calo          ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_25_tight1_tracktwo      ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_35_loose1_tracktwo      ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_35_loose1_ptonly        ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_35_medium1_tracktwo     ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_35_medium1_ptonly       ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_35_medium1_calo         ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_35_tight1_tracktwo      ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_35_tight1_ptonly        ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_35_perf_tracktwo        ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_35_perf_ptonly          ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_80_medium1_calo         ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_80_medium1_tracktwo     ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_50_medium1_tracktwo     ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_125_medium1_tracktwo    ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_125_medium1_calo        ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_125_perf_tracktwo       ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_125_perf_ptonly         ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_160_medium1_tracktwo    ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_5_perf_ptonly           ));
  //m_hlttau_tools.push_back(ToolHandle<IHltTauSelectionTool>(m_hlttau_tool_0_perf_ptonly           ));

}

ToolsRegistry::~ToolsRegistry(){
    //for(auto it: m_initializedToolNames){
    //  TODO: get ptr to tool from toolstore; delete it
      //auto tool = it;
      //delete tool;
    //}

    //delete m_l1jet_tool_12;
    //delete m_l1jet_tool_20;
    //delete m_l1jet_tool_25;
    //delete m_l1tau_tool_8;
    //delete m_l1tau_tool_12;
    //delete m_l1tau_tool_15;
    //delete m_l1tau_tool_20;
    //delete m_l1tau_tool_25;
    //delete m_l1tau_tool_30;
    //delete m_l1tau_tool_40;
    //delete m_l1tau_tool_60;
    //delete m_l1tau_tool_12IL;
    //delete m_l1tau_tool_12IM;
    //delete m_l1tau_tool_12IT;
    //delete m_l1tau_tool_20IL;
    //delete m_l1tau_tool_20IM;
    //delete m_l1tau_tool_20IT;
    //delete m_l1tau_tool_25IT;
    //delete m_l1ele_tool_15;
    //delete m_l1ele_tool_15HI;
    //delete m_l1xe_tool_35;
    //delete m_l1xe_tool_40;
    //delete m_l1xe_tool_45;
    //delete m_l1xe_tool_50;
    //delete m_l1muon_tool_10;
    //delete m_l1muon_tool_20;
    //delete m_hlttau_tool_25_perf_ptonly;
    //delete m_hlttau_tool_25_perf_calo;
    //delete m_hlttau_tool_25_perf_tracktwo;
    //delete m_hlttau_tool_25_idperf_tracktwo;
    //delete m_hlttau_tool_25_loose1_ptonly;
    //delete m_hlttau_tool_25_loose1_calo;
    //delete m_hlttau_tool_25_loose1_tracktwo;
    //delete m_hlttau_tool_25_medium1_ptonly;
    //delete m_hlttau_tool_25_medium1_calo;
    //delete m_hlttau_tool_25_medium1_tracktwo;
    //delete m_hlttau_tool_25_medium1_mvonly;
    //delete m_hlttau_tool_25_tight1_ptonly;
    //delete m_hlttau_tool_25_tight1_calo;
    //delete m_hlttau_tool_25_tight1_tracktwo;
    //delete m_hlttau_tool_35_loose1_tracktwo;
    //delete m_hlttau_tool_35_loose1_ptonly;
    //delete m_hlttau_tool_35_medium1_tracktwo;
    //delete m_hlttau_tool_35_medium1_ptonly;
    //delete m_hlttau_tool_35_medium1_calo;
    //delete m_hlttau_tool_35_tight1_tracktwo;
    //delete m_hlttau_tool_35_tight1_ptonly;
    //delete m_hlttau_tool_35_perf_tracktwo;
    //delete m_hlttau_tool_35_perf_ptonly;
    //delete m_hlttau_tool_80_medium1_calo;
    //delete m_hlttau_tool_80_medium1_tracktwo;
    //delete m_hlttau_tool_50_medium1_tracktwo;
    //delete m_hlttau_tool_125_medium1_tracktwo;
    //delete m_hlttau_tool_125_medium1_calo;
    //delete m_hlttau_tool_125_perf_tracktwo;
    //delete m_hlttau_tool_125_perf_ptonly;
    //delete m_hlttau_tool_160_medium1_tracktwo;
    //delete m_hlttau_tool_5_perf_ptonly;
    //delete m_hlttau_tool_0_perf_ptonly;
}

ToolsRegistry::ToolsRegistry(const ToolsRegistry& other): asg::AsgTool(other.name() + "_copy")
{}

StatusCode ToolsRegistry::initialize()

{
  m_ftf_tool->msg().setLevel(this->msg().level());
  ATH_CHECK(m_ftf_tool->initialize());

  //ATH_CHECK(m_l1jet_tool_12->setProperty("ClusterPt", 12000.));
  //ATH_CHECK(m_l1jet_tool_20->setProperty("ClusterPt", 20000.));
  //ATH_CHECK(m_l1jet_tool_25->setProperty("ClusterPt", 25000.));

  //ATH_CHECK(m_l1tau_tool_8->setProperty("ClusterPt", 8000.));
  //ATH_CHECK(m_l1tau_tool_8->setProperty("IsolationThresh", -9999.));

  //ATH_CHECK(m_l1tau_tool_12->setProperty("ClusterPt", 12000.));
  //ATH_CHECK(m_l1tau_tool_12->setProperty("IsolationThresh", -9999.));

  //ATH_CHECK(m_l1tau_tool_15->setProperty("ClusterPt", 15000.));
  //ATH_CHECK(m_l1tau_tool_15->setProperty("IsolationThresh", -9999.));

  //ATH_CHECK(m_l1tau_tool_20->setProperty("ClusterPt", 20000.));
  //ATH_CHECK(m_l1tau_tool_20->setProperty("IsolationThresh", -9999.));
 
  //ATH_CHECK(m_l1tau_tool_25->setProperty("ClusterPt", 25000.));
  //ATH_CHECK(m_l1tau_tool_25->setProperty("IsolationThresh", -9999.));

  //ATH_CHECK(m_l1tau_tool_30->setProperty("ClusterPt", 30000.));
  //ATH_CHECK(m_l1tau_tool_30->setProperty("IsolationThresh", -9999.));

  //ATH_CHECK(m_l1tau_tool_40->setProperty("ClusterPt", 40000.));
  //ATH_CHECK(m_l1tau_tool_40->setProperty("IsolationThresh", -9999.));
 
  //ATH_CHECK(m_l1tau_tool_60->setProperty("ClusterPt", 60000.));
  //ATH_CHECK(m_l1tau_tool_60->setProperty("IsolationThresh", -9999.));


  //ATH_CHECK(m_l1tau_tool_12IL->setProperty("ClusterPt", 12000.));
  //ATH_CHECK(m_l1tau_tool_12IL->setProperty("IsolationOffset", 3000.));

  //ATH_CHECK(m_l1tau_tool_12IM->setProperty("ClusterPt", 12000.));
  //ATH_CHECK(m_l1tau_tool_12IM->setProperty("IsolationOffset", 2000.));

  //ATH_CHECK(m_l1tau_tool_12IT->setProperty("ClusterPt", 12000.));
  //ATH_CHECK(m_l1tau_tool_12IT->setProperty("IsolationOffset", 1500.));

  //ATH_CHECK(m_l1tau_tool_20IL->setProperty("ClusterPt", 20000.));
  //ATH_CHECK(m_l1tau_tool_20IL->setProperty("IsolationOffset", 3000.));

  //ATH_CHECK(m_l1tau_tool_20IM->setProperty("ClusterPt", 20000.));
  //ATH_CHECK(m_l1tau_tool_20IM->setProperty("IsolationOffset", 2000.));

  //ATH_CHECK(m_l1tau_tool_20IT->setProperty("ClusterPt", 20000.));
  //ATH_CHECK(m_l1tau_tool_20IT->setProperty("IsolationOffset", 1500.));

  //ATH_CHECK(m_l1tau_tool_25IT->setProperty("ClusterPt", 25000.));
  //ATH_CHECK(m_l1tau_tool_25IT->setProperty("IsolationOffset", 1500.));


  //ATH_CHECK(m_l1ele_tool_15->setProperty("ClusterPt", 15000.));
  //ATH_CHECK(m_l1ele_tool_15->setProperty("UseEmClus", true));
  //ATH_CHECK(m_l1ele_tool_15->setProperty("IsolationThresh", -9999.));

  //ATH_CHECK(m_l1ele_tool_15HI->setProperty("ClusterPt", 15000.));
  //ATH_CHECK(m_l1ele_tool_15HI->setProperty("UseEmClus", true));
  //ATH_CHECK(m_l1ele_tool_15HI->setProperty("UseHadCore", true));
  //ATH_CHECK(m_l1ele_tool_15HI->setProperty("IsolationThresh", 50000.));
  //ATH_CHECK(m_l1ele_tool_15HI->setProperty("IsolationMinimum", 2000.));
  //ATH_CHECK(m_l1ele_tool_15HI->setProperty("IsolationOffset", -1800.));
  //ATH_CHECK(m_l1ele_tool_15HI->setProperty("IsolationSlope", 1./8.));
  //ATH_CHECK(m_l1ele_tool_15HI->setProperty("HadronicLeakageThresh", 50000.));
  //ATH_CHECK(m_l1ele_tool_15HI->setProperty("HadronicLeakageMinimum", 1000.));
  //ATH_CHECK(m_l1ele_tool_15HI->setProperty("HadronicLeakageOffset", -200.));
  //ATH_CHECK(m_l1ele_tool_15HI->setProperty("HadronicLeakageSlope", 1./23.0));


  //ATH_CHECK(m_l1xe_tool_35->setProperty("XE_cut", 35000.));
  //ATH_CHECK(m_l1xe_tool_40->setProperty("XE_cut", 40000.));
  //ATH_CHECK(m_l1xe_tool_45->setProperty("XE_cut", 45000.));
  //ATH_CHECK(m_l1xe_tool_50->setProperty("XE_cut", 50000.));

  //ATH_CHECK(m_l1muon_tool_10->setProperty("ClusterPt", 10000.));
  //ATH_CHECK(m_l1muon_tool_20->setProperty("ClusterPt", 20000.));

  //// HLT TAU TOOLS
  //ATH_CHECK(m_hlttau_tool_25_perf_ptonly->setProperty("PreselPt", 25000.));
  //ATH_CHECK(m_hlttau_tool_25_perf_ptonly->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_25_perf_ptonly->setProperty("UseFastTracking", false));
  //ATH_CHECK(m_hlttau_tool_25_perf_ptonly->setProperty("UseTauID", false));
  //ATH_CHECK(m_hlttau_tool_25_perf_ptonly->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_25_perf_calo->setProperty("PreselPt", 25000.));	    
  //ATH_CHECK(m_hlttau_tool_25_perf_calo->setProperty("CentFracStrategy", "pt_dependent"));	    
  //ATH_CHECK(m_hlttau_tool_25_perf_calo->setProperty("UseFastTracking", false));
  //ATH_CHECK(m_hlttau_tool_25_perf_calo->setProperty("UseTauID", false));
  //ATH_CHECK(m_hlttau_tool_25_perf_calo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_25_perf_tracktwo->setProperty("PreselPt", 25000.));
  //ATH_CHECK(m_hlttau_tool_25_perf_tracktwo->setProperty("UsePresel", false));
  //ATH_CHECK(m_hlttau_tool_25_perf_tracktwo->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_25_perf_tracktwo->setProperty("UseFastTracking", true));
  //ATH_CHECK(m_hlttau_tool_25_perf_tracktwo->setProperty("FastTrackSelectionTool", ftf_handle));
  //ATH_CHECK(m_hlttau_tool_25_perf_tracktwo->setProperty("UseTauID", false));
  //ATH_CHECK(m_hlttau_tool_25_perf_tracktwo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));


  //ATH_CHECK(m_hlttau_tool_25_idperf_tracktwo->setProperty("PreselPt", 25000.));
  //ATH_CHECK(m_hlttau_tool_25_idperf_tracktwo->setProperty("UsePresel", false));
  //ATH_CHECK(m_hlttau_tool_25_idperf_tracktwo->setProperty("UseFastTracking", false));
  //ATH_CHECK(m_hlttau_tool_25_idperf_tracktwo->setProperty("UseTauID", false));
  //ATH_CHECK(m_hlttau_tool_25_idperf_tracktwo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_25_loose1_ptonly->setProperty("PreselPt", 25000.));
  //// ATH_CHECK(m_hlttau_tool_25_loose1_ptonly->setProperty("UsePresel", false));
  //ATH_CHECK(m_hlttau_tool_25_loose1_ptonly->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_25_loose1_ptonly->setProperty("UseFastTracking", false));
  //ATH_CHECK(m_hlttau_tool_25_loose1_ptonly->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_25_loose1_ptonly->setProperty("IdLevel", "loose"));
  //ATH_CHECK(m_hlttau_tool_25_loose1_ptonly->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_25_loose1_calo->setProperty("PreselPt", 25000.));	    
  //ATH_CHECK(m_hlttau_tool_25_loose1_calo->setProperty("CentFracStrategy", "pt_dependent"));	    
  //ATH_CHECK(m_hlttau_tool_25_loose1_calo->setProperty("UseFastTracking", false));
  //ATH_CHECK(m_hlttau_tool_25_loose1_calo->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_25_loose1_calo->setProperty("IdLevel", "loose"));
  //ATH_CHECK(m_hlttau_tool_25_loose1_calo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_25_loose1_tracktwo->setProperty("PreselPt", 25000.));
  //ATH_CHECK(m_hlttau_tool_25_loose1_tracktwo->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_25_loose1_tracktwo->setProperty("UseFastTracking", true));
  //ATH_CHECK(m_hlttau_tool_25_loose1_tracktwo->setProperty("FastTrackSelectionTool", ftf_handle));
  //ATH_CHECK(m_hlttau_tool_25_loose1_tracktwo->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_25_loose1_tracktwo->setProperty("IdLevel", "loose"));
  //ATH_CHECK(m_hlttau_tool_25_loose1_tracktwo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_25_medium1_ptonly->setProperty("PreselPt", 25000.));
  //ATH_CHECK(m_hlttau_tool_25_medium1_ptonly->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_25_medium1_ptonly->setProperty("UseFastTracking", false));
  //ATH_CHECK(m_hlttau_tool_25_medium1_ptonly->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_25_medium1_ptonly->setProperty("IdLevel", "medium"));
  //ATH_CHECK(m_hlttau_tool_25_medium1_ptonly->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_25_medium1_calo->setProperty("PreselPt", 25000.));	    
  //ATH_CHECK(m_hlttau_tool_25_medium1_calo->setProperty("CentFracStrategy", "pt_dependent"));	    
  //ATH_CHECK(m_hlttau_tool_25_medium1_calo->setProperty("UseFastTracking", false));
  //ATH_CHECK(m_hlttau_tool_25_medium1_calo->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_25_medium1_calo->setProperty("IdLevel", "medium"));
  //ATH_CHECK(m_hlttau_tool_25_medium1_calo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_25_medium1_tracktwo->setProperty("PreselPt", 25000.));
  //ATH_CHECK(m_hlttau_tool_25_medium1_tracktwo->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_25_medium1_tracktwo->setProperty("UseFastTracking", true));
  //ATH_CHECK(m_hlttau_tool_25_medium1_tracktwo->setProperty("FastTrackSelectionTool", ftf_handle));
  //ATH_CHECK(m_hlttau_tool_25_medium1_tracktwo->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_25_medium1_tracktwo->setProperty("IdLevel", "medium"));
  //ATH_CHECK(m_hlttau_tool_25_medium1_tracktwo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_25_medium1_mvonly->setProperty("PreselPt", 25000.));
  //ATH_CHECK(m_hlttau_tool_25_medium1_mvonly->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_25_medium1_mvonly->setProperty("UseFastTracking", false));
  //ATH_CHECK(m_hlttau_tool_25_medium1_mvonly->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_25_medium1_tracktwo->setProperty("IdLevel", "medium"));
  //ATH_CHECK(m_hlttau_tool_25_medium1_mvonly->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_25_tight1_ptonly->setProperty("PreselPt", 25000.));
  //ATH_CHECK(m_hlttau_tool_25_tight1_ptonly->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_25_tight1_ptonly->setProperty("UseFastTracking", false));
  //ATH_CHECK(m_hlttau_tool_25_tight1_ptonly->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_25_tight1_ptonly->setProperty("IdLevel", "tight"));
  //ATH_CHECK(m_hlttau_tool_25_tight1_ptonly->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_25_tight1_calo->setProperty("PreselPt", 25000.));	    
  //ATH_CHECK(m_hlttau_tool_25_tight1_calo->setProperty("CentFracStrategy", "pt_dependent"));	    
  //ATH_CHECK(m_hlttau_tool_25_tight1_calo->setProperty("UseFastTracking", false));
  //ATH_CHECK(m_hlttau_tool_25_tight1_calo->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_25_tight1_calo->setProperty("IdLevel", "tight"));
  //ATH_CHECK(m_hlttau_tool_25_tight1_calo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_25_tight1_tracktwo->setProperty("PreselPt", 25000.));
  //ATH_CHECK(m_hlttau_tool_25_tight1_tracktwo->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_25_tight1_tracktwo->setProperty("UseFastTracking", true));
  //ATH_CHECK(m_hlttau_tool_25_tight1_tracktwo->setProperty("FastTrackSelectionTool", ftf_handle));
  //ATH_CHECK(m_hlttau_tool_25_tight1_tracktwo->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_25_tight1_tracktwo->setProperty("IdLevel", "tight"));
  //ATH_CHECK(m_hlttau_tool_25_tight1_tracktwo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_35_loose1_tracktwo->setProperty("PreselPt", 35000.));    
  //ATH_CHECK(m_hlttau_tool_35_loose1_tracktwo->setProperty("UseCaloPresel", false));    
  //ATH_CHECK(m_hlttau_tool_35_loose1_tracktwo->setProperty("UseFastTracking", true));    
  //ATH_CHECK(m_hlttau_tool_35_loose1_tracktwo->setProperty("FastTrackSelectionTool", ftf_handle));
  //ATH_CHECK(m_hlttau_tool_35_loose1_tracktwo->setProperty("UseTauID", true));    
  //ATH_CHECK(m_hlttau_tool_35_loose1_tracktwo->setProperty("IdLevel", "loose"));    
  //ATH_CHECK(m_hlttau_tool_35_loose1_tracktwo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_35_loose1_ptonly->setProperty("PreselPt", 35000.));
  //ATH_CHECK(m_hlttau_tool_35_loose1_ptonly->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_35_loose1_ptonly->setProperty("UseFastTracking", false));
  //ATH_CHECK(m_hlttau_tool_35_loose1_ptonly->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_35_loose1_ptonly->setProperty("IdLevel", "loose"));
  //ATH_CHECK(m_hlttau_tool_35_loose1_ptonly->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_35_medium1_tracktwo->setProperty("PreselPt", 35000.));
  //ATH_CHECK(m_hlttau_tool_35_medium1_tracktwo->setProperty("UseCaloPresel", false)); 
  //ATH_CHECK(m_hlttau_tool_35_medium1_tracktwo->setProperty("UseFastTracking", true));
  //ATH_CHECK(m_hlttau_tool_35_medium1_tracktwo->setProperty("FastTrackSelectionTool", ftf_handle));
  //ATH_CHECK(m_hlttau_tool_35_medium1_tracktwo->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_35_medium1_tracktwo->setProperty("IdLevel", "medium"));
  //ATH_CHECK(m_hlttau_tool_35_medium1_tracktwo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_35_medium1_ptonly->setProperty("PreselPt", 35000.));
  //ATH_CHECK(m_hlttau_tool_35_medium1_ptonly->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_35_medium1_ptonly->setProperty("UseFastTracking", false));
  //ATH_CHECK(m_hlttau_tool_35_medium1_ptonly->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_35_medium1_ptonly->setProperty("IdLevel", "medium"));
  //ATH_CHECK(m_hlttau_tool_35_medium1_ptonly->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_35_medium1_calo->setProperty("PreselPt", 35000.));
  //ATH_CHECK(m_hlttau_tool_35_medium1_calo->setProperty("UseCaloPresel", true));
  //ATH_CHECK(m_hlttau_tool_35_medium1_calo->setProperty("UseFastTracking", false));
  //ATH_CHECK(m_hlttau_tool_35_medium1_calo->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_35_medium1_calo->setProperty("IdLevel", "medium"));
  //ATH_CHECK(m_hlttau_tool_35_medium1_calo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));


  //ATH_CHECK(m_hlttau_tool_35_tight1_tracktwo->setProperty("PreselPt", 35000.));    
  //ATH_CHECK(m_hlttau_tool_35_tight1_tracktwo->setProperty("UseCaloPresel", false)); 
  //ATH_CHECK(m_hlttau_tool_35_tight1_tracktwo->setProperty("UseFastTracking", true));
  //ATH_CHECK(m_hlttau_tool_35_tight1_tracktwo->setProperty("FastTrackSelectionTool", ftf_handle));
  //ATH_CHECK(m_hlttau_tool_35_tight1_tracktwo->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_35_tight1_tracktwo->setProperty("IdLevel", "tight"));
  //ATH_CHECK(m_hlttau_tool_35_tight1_tracktwo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));


  //ATH_CHECK(m_hlttau_tool_35_tight1_ptonly->setProperty("PreselPt", 35000.));
  //ATH_CHECK(m_hlttau_tool_35_tight1_ptonly->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_35_tight1_ptonly->setProperty("UseFastTracking", false));
  //ATH_CHECK(m_hlttau_tool_35_tight1_ptonly->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_35_tight1_ptonly->setProperty("IdLevel", "tight"));
  //ATH_CHECK(m_hlttau_tool_35_tight1_ptonly->setProperty("RecalculateBDTscore", m_recalculateBDTscore));


  //ATH_CHECK(m_hlttau_tool_35_perf_tracktwo->setProperty("PreselPt", 35000.));	    
  //ATH_CHECK(m_hlttau_tool_35_perf_tracktwo->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_35_perf_tracktwo->setProperty("UseFastTracking", true));
  //ATH_CHECK(m_hlttau_tool_35_perf_tracktwo->setProperty("FastTrackSelectionTool", ftf_handle));
  //ATH_CHECK(m_hlttau_tool_35_perf_tracktwo->setProperty("UseTauID", false));
  //ATH_CHECK(m_hlttau_tool_35_perf_tracktwo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_35_perf_ptonly->setProperty("PreselPt", 35000.));	    
  //ATH_CHECK(m_hlttau_tool_35_perf_ptonly->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_35_perf_ptonly->setProperty("UseFastTracking", false));
  //ATH_CHECK(m_hlttau_tool_35_perf_ptonly->setProperty("UseTauID", false));
  //ATH_CHECK(m_hlttau_tool_35_perf_ptonly->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_80_medium1_calo->setProperty("PreselPt", 80000.));
  //ATH_CHECK(m_hlttau_tool_80_medium1_calo->setProperty("UseCaloPresel", true));
  //ATH_CHECK(m_hlttau_tool_80_medium1_calo->setProperty("UseFastTracking", false));
  //ATH_CHECK(m_hlttau_tool_80_medium1_calo->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_80_medium1_calo->setProperty("IdLevel", "medium"));
  //ATH_CHECK(m_hlttau_tool_80_medium1_calo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_80_medium1_tracktwo->setProperty("PreselPt", 80000.));
  //ATH_CHECK(m_hlttau_tool_80_medium1_tracktwo->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_80_medium1_tracktwo->setProperty("UseFastTracking", true));
  //ATH_CHECK(m_hlttau_tool_80_medium1_tracktwo->setProperty("FastTrackSelectionTool", ftf_handle));
  //ATH_CHECK(m_hlttau_tool_80_medium1_tracktwo->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_80_medium1_tracktwo->setProperty("IdLevel", "medium"));
  //ATH_CHECK(m_hlttau_tool_80_medium1_tracktwo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_50_medium1_tracktwo->setProperty("PreselPt", 50000.));
  //ATH_CHECK(m_hlttau_tool_50_medium1_tracktwo->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_50_medium1_tracktwo->setProperty("UseFastTracking", true));
  //ATH_CHECK(m_hlttau_tool_50_medium1_tracktwo->setProperty("FastTrackSelectionTool", ftf_handle));
  //ATH_CHECK(m_hlttau_tool_50_medium1_tracktwo->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_50_medium1_tracktwo->setProperty("IdLevel", "medium"));
  //ATH_CHECK(m_hlttau_tool_50_medium1_tracktwo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_125_medium1_tracktwo->setProperty("PreselPt", 125000.));
  //ATH_CHECK(m_hlttau_tool_125_medium1_tracktwo->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_125_medium1_tracktwo->setProperty("UseFastTracking", true));
  //ATH_CHECK(m_hlttau_tool_125_medium1_tracktwo->setProperty("FastTrackSelectionTool", ftf_handle));
  //ATH_CHECK(m_hlttau_tool_125_medium1_tracktwo->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_125_medium1_tracktwo->setProperty("IdLevel", "medium"));
  //ATH_CHECK(m_hlttau_tool_125_medium1_tracktwo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_125_medium1_calo->setProperty("PreselPt", 125000.));
  //ATH_CHECK(m_hlttau_tool_125_medium1_calo->setProperty("UseCaloPresel", true));
  //ATH_CHECK(m_hlttau_tool_125_medium1_calo->setProperty("UseFastTracking", false));
  //ATH_CHECK(m_hlttau_tool_125_medium1_calo->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_125_medium1_calo->setProperty("IdLevel", "medium"));
  //ATH_CHECK(m_hlttau_tool_125_medium1_calo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_125_perf_tracktwo->setProperty("PreselPt", 125000.));
  //ATH_CHECK(m_hlttau_tool_125_perf_tracktwo->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_125_perf_tracktwo->setProperty("UseFastTracking", true));
  //ATH_CHECK(m_hlttau_tool_125_perf_tracktwo->setProperty("FastTrackSelectionTool", ftf_handle));
  //ATH_CHECK(m_hlttau_tool_125_perf_tracktwo->setProperty("UseTauID", false));
  //ATH_CHECK(m_hlttau_tool_125_perf_tracktwo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_125_perf_ptonly->setProperty("PreselPt", 125000.));
  //ATH_CHECK(m_hlttau_tool_125_perf_ptonly->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_125_perf_ptonly->setProperty("UseFastTracking", false));
  //ATH_CHECK(m_hlttau_tool_125_perf_ptonly->setProperty("UseTauID", false));
  //ATH_CHECK(m_hlttau_tool_125_perf_ptonly->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_160_medium1_tracktwo->setProperty("PreselPt", 160000.));
  //ATH_CHECK(m_hlttau_tool_160_medium1_tracktwo->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_160_medium1_tracktwo->setProperty("UseFastTracking", true));
  //ATH_CHECK(m_hlttau_tool_160_medium1_tracktwo->setProperty("UseTauID", true));
  //ATH_CHECK(m_hlttau_tool_160_medium1_tracktwo->setProperty("IdLevel", "medium"));
  //ATH_CHECK(m_hlttau_tool_160_medium1_tracktwo->setProperty("RecalculateBDTscore", m_recalculateBDTscore));
  //ATH_CHECK(m_hlttau_tool_160_medium1_tracktwo->setProperty("FastTrackSelectionTool", ftf_handle));

  //ATH_CHECK(m_hlttau_tool_5_perf_ptonly->setProperty("PreselPt", 5000.));
  //ATH_CHECK(m_hlttau_tool_5_perf_ptonly->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_5_perf_ptonly->setProperty("UseFastTracking", false));
  //ATH_CHECK(m_hlttau_tool_5_perf_ptonly->setProperty("UseTauID", false));
  //ATH_CHECK(m_hlttau_tool_5_perf_ptonly->setProperty("RecalculateBDTscore", m_recalculateBDTscore));

  //ATH_CHECK(m_hlttau_tool_0_perf_ptonly->setProperty("PreselPt", 0.));
  //ATH_CHECK(m_hlttau_tool_0_perf_ptonly->setProperty("UseCaloPresel", false));
  //ATH_CHECK(m_hlttau_tool_0_perf_ptonly->setProperty("UseFastTracking", false));
  //ATH_CHECK(m_hlttau_tool_0_perf_ptonly->setProperty("UseTauID", false));
  //ATH_CHECK(m_hlttau_tool_0_perf_ptonly->setProperty("RecalculateBDTscore", m_recalculateBDTscore));


  return StatusCode::SUCCESS;
}

