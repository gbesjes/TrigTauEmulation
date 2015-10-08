// vim: ts=2 sw=2
// author Quentin Buat <quentin.buat@no.spam.cern.ch>
// author Geert-Jan Besjes <geert-jan.besjes@no.spam.cern.ch>
#ifndef HAVE_HLTITEM_H
#define HAVE_HLTITEM_H

#include <stdexcept>
#include <string>

#include "AsgTools/ToolStore.h"
#include "AsgTools/ToolHandle.h"

#include "TrigTauEmulation/IHltTauSelectionTool.h"
#include "TrigTauEmulation/ILevel1SelectionTool.h"
#include "TrigTauEmulation/EmTauSelectionTool.h"
#include "TrigTauEmulation/EmTauSelectionTool.h"

class HltItem {
  private:

    std::string m_name;
    std::string m_l1_seed;

    ToolHandle<IHltTauSelectionTool> m_HltTauSelectionTool; 
    ToolHandle<ILevel1SelectionTool> m_Level1SelectionTool;

  public:

    HltItem(std::string name, std::string seed_name) {
      m_name = name;
      m_l1_seed = seed_name;
      
      if(asg::ToolStore::contains<ILevel1SelectionTool>(seed_name)){
        m_Level1SelectionTool = asg::ToolStore::get<ILevel1SelectionTool>(seed_name);
        std::cout << "grabbed level1 tool " << seed_name << std::endl;
      } else {
        std::stringstream e;
        e << "No Level1SelectionTool " << seed_name << " found";
        throw std::invalid_argument(e.str());
      }

      // are we actually a tau chain? 
      if(not isTau()) return;

      if(asg::ToolStore::contains<IHltTauSelectionTool>(name)){
        m_HltTauSelectionTool = asg::ToolStore::get<IHltTauSelectionTool>(name);
        std::cout << "grabbed HLT tool " << name << std::endl;
      } else {
        std::stringstream e;
        e << "No HltTauSelectionTool " << name << " found";
        throw std::invalid_argument(e.str()); 
      }
      
    }

    const std::string& name() const { return m_name; }
    const std::string& seed() const { return m_l1_seed; }
   
    ToolHandle<IHltTauSelectionTool> getHltTauSelectionTool() const {
      return m_HltTauSelectionTool;
    }
    
    ToolHandle<ILevel1SelectionTool> getLevel1SelectionTool() const {
      return m_Level1SelectionTool;
    }

    bool isTau() const { 
      return (m_name.find("tau") != std::string::npos ? true : false);
    }
};

#endif

