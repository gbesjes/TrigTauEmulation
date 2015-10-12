#ifndef IMUONROISELECTIONTOOL_MUONROISELECTIONTOOL_H
#define IMUONROISELECTIONTOOL_MUONROISELECTIONTOOL_H

#include "AsgTools/IAsgTool.h"
#include "PATCore/TAccept.h"

#include "TrigTauEmulation/ILevel1SelectionTool.h"

//EDM include
#include "xAODTrigger/MuonRoI.h"

class IMuonRoISelectionTool : public virtual ILevel1SelectionTool
{
   ASG_TOOL_INTERFACE(IMuonRoISelectionTool)
    
 public:

  virtual const Root::TAccept& accept(const xAOD::MuonRoI& l1jet) const = 0;
  virtual ~IMuonRoISelectionTool() {};


};

#endif
