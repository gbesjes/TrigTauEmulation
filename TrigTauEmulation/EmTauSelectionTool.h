#ifndef EMTAUSELECTIONTOOL_H
#define EMTAUSELECTIONTOOL_H

// Framework includes
#include "AsgTools/AsgTool.h"
/* #include "PATCore/IAsgSelectionTool.h" */


// Local includes
#include "TrigTauEmulation/IEmTauSelectionTool.h"

class EmTauSelectionTool : public virtual IEmTauSelectionTool, public asg::AsgTool

{

  ASG_TOOL_CLASS(EmTauSelectionTool, IEmTauSelectionTool)


 public:

  // Default Constructor 
  EmTauSelectionTool(const std::string& name);

  // Copy Constructor 
  EmTauSelectionTool(const EmTauSelectionTool& other);

  // Destructor
  virtual ~EmTauSelectionTool() {};

  // Tool initialization
  virtual StatusCode initialize();

  // Get the decision for a specific EmTauRoI
  virtual const Root::TAccept& accept(const xAOD::EmTauRoI& l1tau) const;


 private:

  mutable Root::TAccept m_accept;
  
  double m_roi_pt_cut;
  double m_roi_eta_cut;

  double m_iso_slope;
  double m_iso_offset;
  double m_iso_thresh;
  double m_iso_min;

  double m_had_leak_slope;
  double m_had_leak_offset;
  double m_had_leak_thresh;
  double m_had_leak_min;

  bool m_use_had_core;
  bool m_use_emclus;

 protected:

  bool pass_isolation(const xAOD::EmTauRoI& l1tau) const;
  bool pass_hadronic_leakage(const xAOD::EmTauRoI& l1tau) const;

};
#endif
