// vim: ts=2 sw=2
#ifndef TrigTauEmulation_ToolsRegistry_H
#define TrigTauEmulation_ToolsRegistry_H

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

#include <EventLoop/Algorithm.h>

class ToolsRegistry : public EL::Algorithm
{
  // put your configuration variables here as public variables.
  // that way they can be set directly from CINT and python.
private:
  // float cutValue;

  // variables that don't get filled at submission time should be
  // protected from being send from the submission node to the worker
  // node (done by the //!)
  
  std::string name;

  ToolHandleArray<IEmTauSelectionTool> m_l1tau_tools; //!
  ToolHandleArray<IEnergySumSelectionTool> m_l1xe_tools; //!
  ToolHandleArray<IJetRoISelectionTool> m_l1jet_tools; //!
  ToolHandleArray<IMuonRoISelectionTool> m_l1muon_tools; //!
  ToolHandleArray<IHltTauSelectionTool> m_hlttau_tools; //!

  bool m_recalculateBDTscore;
  FastTrackSelectionTool *m_ftf_tool; //!
  EmTauSelectionTool *m_l1tau_tool_12IM; //!

public:
  // Tree *myTree; //!
  // TH1 *myHist; //!

  ToolHandleArray<IEmTauSelectionTool> GetL1TauTools() {return m_l1tau_tools;}
  ToolHandleArray<IEnergySumSelectionTool> GetL1XeTools() {return m_l1xe_tools;}
  ToolHandleArray<IJetRoISelectionTool> GetL1JetTools() {return m_l1jet_tools;}
  ToolHandleArray<IMuonRoISelectionTool> GetL1MuonTools() {return m_l1muon_tools;}
  ToolHandleArray<IHltTauSelectionTool> GetHltTauTools() {return m_hlttau_tools;}

  // this is a standard constructor
  ToolsRegistry ();
  ToolsRegistry(const std::string& name);

  // these are the functions inherited from Algorithm
  virtual EL::StatusCode setupJob (EL::Job& job);
  virtual EL::StatusCode fileExecute ();
  virtual EL::StatusCode histInitialize ();
  virtual EL::StatusCode changeInput (bool firstFile);
  virtual EL::StatusCode initialize ();
  virtual EL::StatusCode execute ();
  virtual EL::StatusCode postExecute ();
  virtual EL::StatusCode finalize ();
  virtual EL::StatusCode histFinalize ();

  // this is needed to distribute the algorithm to the workers
  ClassDef(ToolsRegistry, 1);
};

#endif
