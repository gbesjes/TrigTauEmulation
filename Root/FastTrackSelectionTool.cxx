// vim: ts=2 sw=2
// #include "TrigTauEmulation/FastTrackSelection.h"
#include "TrigTauEmulation/FastTrackSelectionTool.h"
#include "TrigTauEmulation/Utils.h"

// Default constructor
FastTrackSelectionTool::FastTrackSelectionTool(const std::string& name) : asg::AsgTool(name)
{

  declareProperty("d0cut", m_d0=2.0);
  declareProperty("z0cut", m_z0=150.0);
  declareProperty("DeltaZ0", m_delta_z0=2.0);
  declareProperty("CoreCone", m_core=0.2);
  declareProperty("IsoCone", m_iso=0.4);
  declareProperty("nCoreBound", m_ncore_bound=4);
  declareProperty("nIsoBound", m_niso_bound=2);
}

// Copy constructor
FastTrackSelectionTool::FastTrackSelectionTool(const FastTrackSelectionTool& other) : asg::AsgTool(other.name() + "_copy")
{}


StatusCode FastTrackSelectionTool::initialize()
{

  m_accept.addCut("FastTrackSel", "FastTrackSel");
  return StatusCode::SUCCESS;
}

// method to find leading track closest to the tau passes
inline const xAOD::TrackParticle* FastTrackSelectionTool::findLeadingTrack(const xAOD::TauJet *hlt_tau, const DataVector<xAOD::TrackParticle>* tracks) const {
  //  Lead Track Finding                                                                                                              
  const xAOD::TrackParticle* lead_trk(0);

  for (DataVector<xAOD::TrackParticle>::const_iterator tr = tracks->begin(); tr != tracks->end(); ++tr){
    const double lead_deta = Utils::DeltaEta((*tr)->eta(), hlt_tau->etaDetectorAxis());
    const double lead_dphi = Utils::DeltaPhi((*tr)->phi(), hlt_tau->phiDetectorAxis());

    // const double lead_dR = Utils::DeltaR((*tr)->eta(), (*tr)->phi(), hlt_tau->etaDetectorAxis(), hlt_tau->phiDetectorAxis());
    //const double lead_dR = Utils::DeltaR((*tr)->eta(), (*tr)->phi(), hlt_tau->eta(), hlt_tau->phi() );
    if (not (lead_deta < 0.1 and lead_dphi < 0.1)) {
    // if (!(lead_dR < 0.1)) {
      ATH_MSG_DEBUG("findLeadingTrack(): skipping track with pT=" << (*tr)->pt() 
		    << " eta=" << (*tr)->eta() 
		    << " phi=" << (*tr)->phi() 
		    << " for dEta*DPhi > 0.1");
      continue;
    }
    
    if (not lead_trk) {
      lead_trk = (*tr);
    }
    
    if (lead_trk->pt() < (*tr)->pt()) {
      lead_trk = (*tr);
    }
  }

  return lead_trk;
}

inline const xAOD::TrackParticle* FastTrackSelectionTool::findCoreLeadingTrack(const DataVector<xAOD::TrackParticle>* tracks) const {
  //  Lead Track Finding                                                                                                              
  const xAOD::TrackParticle* lead_trk(0);

  for (DataVector<xAOD::TrackParticle>::const_iterator tr = tracks->begin(); tr != tracks->end(); ++tr){

    if (not lead_trk) {
      lead_trk = (*tr);
    }
    
    if (lead_trk->pt() < (*tr)->pt()) {
      lead_trk = (*tr);
    }
  }

  return lead_trk;
}

inline const xAOD::TrackParticle* FastTrackSelectionTool::findIsoLeadingTrack(const xAOD::TauJet* tau, const DataVector<xAOD::TrackParticle>* tracks) const {
  //  Lead Track Finding                                                                                                              
  const xAOD::TrackParticle* lead_trk(0);
  // protection agains NULL core track pointer
  if (not tau)
    return lead_trk;

  for (DataVector<xAOD::TrackParticle>::const_iterator tr = tracks->begin(); tr != tracks->end(); ++tr){

    const double lead_dR = Utils::DeltaR((*tr)->eta(), (*tr)->phi(), tau->etaDetectorAxis(), tau->phiDetectorAxis());
    if (!(lead_dR < 0.2)) {
      ATH_MSG_DEBUG("findIsoLeadingTrack(): reference tau with pT=" << (tau)->pt() 
		    << " eta=" << (tau)->eta() 
		    << " phi=" << (tau)->phi()); 
      ATH_MSG_DEBUG("findIsoLeadingTrack(): skipping track with pT=" << (*tr)->pt() 
		    << " eta=" << (*tr)->eta() 
		    << " phi=" << (*tr)->phi() 
		    << " for dR > 0.2: dR = " << lead_dR);
      continue;
    }
    
    if (not lead_trk) {
      lead_trk = (*tr);
    }
    
    if (lead_trk->pt() < (*tr)->pt()) {
      lead_trk = (*tr);
    }
  }

  return lead_trk;
}

bool FastTrackSelectionTool::trackSelection(const xAOD::TauJet *hlt_tau, const xAOD::TrackParticle *leadingTrack, const DataVector<xAOD::TrackParticle> *tracks) const {
  //NB: we could call the leading track finder oursevles, but then we don't know whether we're using TauCore or TauIso tracks
 
  float lead_trk_z0, lead_trk_phi, lead_trk_eta;
  bool usePileupSuppCut;

  if(leadingTrack) {
    lead_trk_z0 =  leadingTrack->z0();
    lead_trk_phi = leadingTrack->phi();
    lead_trk_eta = leadingTrack->eta();
    usePileupSuppCut = true;
    ATH_MSG_DEBUG("leading track pT = " << leadingTrack->pt());
  } else {
    lead_trk_z0 = 0.0;
    lead_trk_eta = hlt_tau->etaDetectorAxis();
    lead_trk_phi = hlt_tau->phiDetectorAxis();
    usePileupSuppCut = false;
  }
 
  int nCore = 0;
  int nIso = 0;
  for(auto t: *tracks){
      
    const float deltaz0 = fabs(t->z0() - lead_trk_z0);
    if (usePileupSuppCut && deltaz0 >= m_delta_z0) { 
      continue;
    }
    
    const double dR = Utils::DeltaR_Square(t->eta(), t->phi(), lead_trk_eta, lead_trk_phi);
    
    if (dR < m_core * m_core) {
      ATH_MSG_DEBUG("TauIso: nCore++: dR = " << dR << " for track " << t << " with pT=" << t->pt() << " eta=" << t->eta()  << " phi=" << t->phi());
      ++nCore;
    } else if (dR < m_iso * m_iso) { 
      ++nIso;
      ATH_MSG_DEBUG("TauIso: nIso++: dR = " << sqrt(dR) << " for track " << t << " with pT=" << t->pt() << " eta=" << t->eta()  << " phi=" << t->phi());
    }
  }
 
  if (/*nCore > 0 &&*/ nCore < m_ncore_bound && nIso < m_niso_bound) {
    ATH_MSG_DEBUG("ACCEPT nCore = " << nCore << " nIso = " << nIso); 
    return true;
  } 

  ATH_MSG_DEBUG("REJECT nCore = " << nCore << " nIso = " << nIso); 
  return false;
}

// Accept method for EDM tau and TauIso and TauCore tracks from FTF
const Root::TAccept& FastTrackSelectionTool::accept(const xAOD::TauJet *hlt_tau, const DataVector<xAOD::TrackParticle> *preselTracksIso, const DataVector<xAOD::TrackParticle> *preselTracksCore) const
{
  // NOTE: this methods wants to be passed a CaloOnly tau
  m_accept.clear();
  m_accept.setCutResult("FastTrackSel", false);


  ATH_MSG_DEBUG("Checking tracks for HLT tau with pT=" << hlt_tau->pt() << " eta=" << hlt_tau->eta() << " phi=" << hlt_tau->phi());
  if (preselTracksCore->size() == 0)
    return m_accept;

  const xAOD::TrackParticle* coreLeadingTrack = findCoreLeadingTrack(preselTracksCore);
  
  ATH_MSG_DEBUG("Size of TauCore = " << preselTracksCore->size());
  ATH_MSG_DEBUG("Size of TauIso = " << preselTracksIso->size());

  // const xAOD::TrackParticle* isoLeadingTrack = findLeadingTrack(hlt_tau, preselTracksIso);
  const xAOD::TrackParticle* isoLeadingTrack = findIsoLeadingTrack(hlt_tau, preselTracksIso);
  if(isoLeadingTrack){
    ATH_MSG_DEBUG("iso leading track pT = " << isoLeadingTrack->pt());
  } else {
    ATH_MSG_DEBUG("no iso leading track");
  }

  bool passIsoCut = trackSelection(hlt_tau, isoLeadingTrack, preselTracksIso);
  if(passIsoCut){
    m_accept.setCutResult("FastTrackSel", true);
    ATH_MSG_DEBUG("passed trackSelection() for core & iso");
  } else {
    ATH_MSG_DEBUG("did not pass trackSelection() for iso");
  }

  return m_accept;
}


const Root::TAccept& FastTrackSelectionTool::accept(const xAOD::TauJet * presel_tau) const {
  m_accept.clear();
  m_accept.setCutResult("FastTrackSel", false);

  // Track Counting
  int Ncore = presel_tau->nTracks();   
  int Niso  = presel_tau->nWideTracks();
  
  //std::cout << "\t \t \t \t Presel tau " << presel_tau->index() 
        //<< ": pt / eta / phi / Ncore / Niso = " 
        //<< presel_tau->pt() << " / " << presel_tau->eta() << " / "
        //<< presel_tau->phi() << " / " << Ncore << " / " << Niso << std::endl;

  if (Ncore > 0 && Ncore < m_ncore_bound && Niso < m_niso_bound) {
    //std::cout << "accepting nCore = " << Ncore << " and nIso = " << Niso << " (bounds " << m_ncore_bound << " and " << m_niso_bound << ")" << std::endl;
    m_accept.setCutResult("FastTrackSel", true);
  } else {
    //std::cout << "NOT accepting nCore = " << Ncore << " and nIso = " << Niso << " (bounds " << m_ncore_bound << " and " << m_niso_bound << ")" << std::endl;
  }

  return m_accept;
}



