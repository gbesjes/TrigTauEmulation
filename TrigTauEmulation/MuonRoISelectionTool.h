// vim: ts=2 sw=2
#ifndef MUONROISELECTIONTOOL_H
#define MUONROISELECTIONTOOL_H

// Framework include
#include "AsgTools/AsgTool.h"
/* #include "PATCore/IAsgSelectionTool.h" */

// Local includes
#include "TrigTauEmulation/IMuonRoISelectionTool.h"

class MuonRoISelectionTool : public virtual IMuonRoISelectionTool, public asg::AsgTool
{
  ASG_TOOL_CLASS(MuonRoISelectionTool, IMuonRoISelectionTool)

  public:

    // Default Constructor 
    MuonRoISelectionTool(const std::string& name);

    // Copy Constructor 
    MuonRoISelectionTool(const MuonRoISelectionTool& other);

    // Destructor
    virtual ~MuonRoISelectionTool() {};

    // Tool initialization
    virtual StatusCode initialize();

    // Get the decision for a specific MuonRoI
    virtual const Root::TAccept& accept(const xAOD::MuonRoI& l1muon) const;

  private:

    mutable Root::TAccept m_accept;

    double m_roi_pt;
    double m_roi_eta;

};
#endif
