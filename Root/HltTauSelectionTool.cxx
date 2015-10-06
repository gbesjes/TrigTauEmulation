// vim: ts=2 sw=2
#include "TrigTauEmulation/HltTauSelectionTool.h"
#include "TrigTauEmulation/Utils.h"

//Default constructor
HltTauSelectionTool::HltTauSelectionTool(const std::string& name) : asg::AsgTool(name)

{

  declareProperty("PreselPt", m_pt=25000., "Preselection Pt cut");
  declareProperty("CentFracStrategy", m_centfrac_strategy="pt_dependent", "CentFrac cut strategy");
  declareProperty("CentFracCut", m_centfrac=0.9, "CentFrac cut");
  declareProperty("UsePresel", m_use_presel=true, "Turn on/off preselection");
  declareProperty("UseCaloPresel", m_use_calo_presel=true, "Turn on/off preselection centfrac cut");

  declareProperty("UseFastTracking", m_use_fasttracking=true, "Turn on/off fast tracking");
  declareProperty("FastTrackSelectionTool", m_ftf_tool);
 
  // --> not needed anymore (central tool now) / will remove soooon
  // declareProperty("TrackD0", m_d0=2.0, "d0 cut");
  // declareProperty("TrackZ0", m_z0=150.0, "z0 cut");
  // declareProperty("DeltaZ0", m_delta_z0=2.0, "delta z0 cut");
  // declareProperty("CoreConeDr", m_core=0.2, "Dr of the cone");
  // declareProperty("IsoConeDr", m_iso=0.4, "Dr of the isolation");
  // declareProperty("Ncore", m_ncore_bound=4, "Upper bound on Ncore");
  // declareProperty("Niso", m_niso_bound=2, "Upper bound on Niso");

  declareProperty("IdLevel", m_id_level="medium", "loose/medium/tight");
  declareProperty("UseTauID", m_use_tauid=true, "Turn on/off the tau id selection");

  declareProperty("RecalculateBDTscore", m_recalculateBDTscore=false, "Recalculate tau BDT scores");
  declareProperty("TauDiscriminantToolName", m_TauDiscriminantToolName="TauIDTool");

  m_calopresel = new HltTauCaloPresel();

  // --> not needed anymore (central tool now) / will remove soooon
  // m_ftk = new FastTrackSelection();
  m_tauid = new HltTauID();

}

// Copy constructor
HltTauSelectionTool::HltTauSelectionTool(const HltTauSelectionTool& other) : asg::AsgTool(other.name() + "_copy")
{}


HltTauSelectionTool::~HltTauSelectionTool() {
  delete m_calopresel;
  delete m_tauid;
  // --> not needed anymore (central tool now) / will remove soooon
  // delete m_ftk;
}

StatusCode HltTauSelectionTool::initialize()

{
  // #ifdef ASGTOOL_STANDALONE
  //   if (m_recalculateBDTscore) {
  //     m_tauIDTool = new ToolHandle<TauDiscriminantTool>(m_TauDiscriminantToolName);
  //     m_tauIDTool->operator->()->initialize().ignore();
  //   }
  // #endif

  m_calopresel->SetUsePresel(m_use_presel);
  m_calopresel->SetUseCaloPresel(m_use_calo_presel);
  m_calopresel->SetPreselPt(m_pt);
  m_calopresel->SetPreselCentFracCut(m_centfrac);
  m_calopresel->SetPreselCentFracStrategy(m_centfrac_strategy);


  ATH_CHECK(m_ftf_tool.retrieve());

  // --> not needed anymore (central tool now) / will remove soooon
  // m_ftk->SetUseFastTracking(m_use_fasttracking);
  // m_ftk->SetTrackD0(m_d0);
  // m_ftk->SetTrackZ0(m_z0);
  // m_ftk->SetDeltaZ0(m_delta_z0);
  // m_ftk->SetCoreConeDr(m_core);
  // m_ftk->SetIsoConeDr(m_iso);
  // m_ftk->SetNcore(m_ncore_bound);
  // m_ftk->SetNiso(m_niso_bound);

  m_tauid->SetIdLevel(m_id_level);
  m_tauid->SetUseTauID(m_use_tauid);

  m_accept.addCut("HltTau", "HltTau");

  return StatusCode::SUCCESS;
}

const Root::TAccept& HltTauSelectionTool::accept(const DecoratedHltTau& hlttau) const {
  return accept(hlttau.getHltTau(), hlttau.getPreselTracksIso(), hlttau.getPreselTracksCore()); 
}

// accept based on EDM tau candidate and first step FTF tracks from the TDT
const Root::TAccept& HltTauSelectionTool::accept(const xAOD::TauJet *hlttau, const DataVector<xAOD::TrackParticle> *preselTracksIso, const DataVector<xAOD::TrackParticle> *preselTracksCore) const {

  m_accept.clear();
  m_accept.setCutResult("HltTau", false);

// #ifdef ASGTOOL_STANDALONE
//   if (m_recalculateBDTscore) {
//     ATH_MSG_INFO("Recalculating BDT score for HLT tau " << hlttau);

//     m_tauIDTool->operator->()->applyDiscriminant(*(const_cast<xAOD::TauJet*>(hlttau)) ).ignore();
//     ATH_MSG_INFO("tau BDT score = " << hlttau->discriminant(xAOD::TauJetParameters::BDTJetScore));
//   }
// #endif

  if (not m_calopresel->accept(hlttau)) {
    return m_accept;
  }

  if (m_use_fasttracking) {
    if (not m_ftf_tool->accept(hlttau, preselTracksIso, preselTracksCore)) {
      return m_accept;
    }
  }

  if (not m_tauid->accept(hlttau)) {
    return m_accept;
  }

  m_accept.setCutResult("HltTau", true);
  return m_accept;

}

  // --> This method does not work since we need both core and iso FTF tracks to emulate / to be removed
const Root::TAccept& HltTauSelectionTool::accept(const xAOD::TauJet * hlttau, const DataVector<xAOD::TrackParticle> * tracks) const {

  m_accept.clear();
  m_accept.setCutResult("HltTau", false);

// #ifdef ASGTOOL_STANDALONE
//   if (m_recalculateBDTscore) {
//     ATH_MSG_INFO("Recalculating BDT score for HLT tau " << hlttau);

//     m_tauIDTool->operator->()->applyDiscriminant(*(const_cast<xAOD::TauJet*>(hlttau)) ).ignore();
//     ATH_MSG_INFO("tau BDT score = " << hlttau->discriminant(xAOD::TauJetParameters::BDTJetScore));
//   }
// #endif

  // if (not m_calopresel->accept(hlttau)) {
  //   return m_accept;
  // }

  // if (not m_ftk->accept(hlttau, tracks)) {
  //   //std::cout << "NOT ACCEPTING M_FTK" << std::endl;
  //   return m_accept;
  // }

  // if (not m_tauid->accept(hlttau)) {
  //   return m_accept;
  // }

  // m_accept.setCutResult("HltTau", true);
  return m_accept;

}

const Root::TAccept& HltTauSelectionTool::accept(const xAOD::TauJet * hlttau, const xAOD::TauJetContainer * presel_taus) const {

  m_accept.clear();
  m_accept.setCutResult("HltTau", false);

  // #ifdef ASGTOOL_STANDALONE
  //   if (m_recalculateBDTscore) {
  //     ATH_MSG_INFO("Recalculating BDT score for HLT tau " << hlttau);
  
  //     m_tauIDTool->operator->()->applyDiscriminant(*(const_cast<xAOD::TauJet*>(hlttau)) ).ignore();
  //     ATH_MSG_INFO("tau BDT score = " << hlttau->discriminant(xAOD::TauJetParameters::BDTJetScore));
  //   }
  // #endif

  //ATH_MSG_INFO("\t\tBefore calo cuts");
  if (not m_calopresel->accept(hlttau)) {
    //std::cout << "not accepting calo presel" << std::endl;
    return m_accept;
  }

  const xAOD::TauJet *presel_tau = NULL;
  for (const auto tau: *presel_taus) {
    double dR = Utils::DeltaR(hlttau->eta(), hlttau->phi(), tau->eta(), tau->phi());
    if (dR < 0.01) {
      presel_tau = tau;
      break;
    }
  }

  // ATH_MSG_INFO("\t\tBefore preselection check");
  if (m_use_fasttracking and presel_tau == NULL) { 
    //std::cout << "no presel_tau present" << std::endl;
    return m_accept;
  }

  if (m_use_fasttracking)
    if (not m_ftf_tool->accept(hlttau)) {
      return m_accept;
    }

  // --> not needed anymore (central tool now) / will remove soooon
  // // ATH_MSG_INFO("\t\tBefore FTk cuts");
  // if (not m_ftk->accept(presel_tau)) {
  //   //std::cout << "not accepting tracking cut" << std::endl;
  //   return m_accept;
  // } else {
  //   //std::cout << "accepting tracking cut" << std::endl;
  // }

  // ATH_MSG_INFO("\t\tBefore BDT cuts");
  if (not m_tauid->accept(hlttau)) {
    //std::cout << "not accepting tau ID cut" << std::endl;
    return m_accept;
  } else {
    //std::cout << "accepting tau ID cut" << std::endl;
  }

  // ATH_MSG_INFO("\t\tPass all cuts");
  m_accept.setCutResult("HltTau", true);
  return m_accept;

}
