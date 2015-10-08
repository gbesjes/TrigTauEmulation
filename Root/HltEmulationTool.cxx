// vim: ts=2 sw=2
//Local Includes

#include "TrigTauEmulation/HltEmulationTool.h"
#include "TrigDecisionTool/Conditions.h"

namespace TrigTauEmul {

  // Default constructor
  HltEmulationTool::HltEmulationTool(const std::string& name) 
    : asg::AsgTool(name),
    m_l1_emulation_tool(),
    m_hlt_tau_tools()

  {

    declareProperty("hlt_chains", m_hlt_chains_vec, "Vector of the HLT chains to be evaluated");
    declareProperty("PerformL1Emulation", m_perform_l1_emulation=true);
    declareProperty("Level1EmulationTool", m_l1_emulation_tool);
    declareProperty("HltTauTools", m_hlt_tau_tools);

    declareProperty("HLTTriggerCondition", m_HLTTriggerCondition=TrigDefs::Physics);
    declareProperty("L1TriggerCondition", m_L1TriggerCondition=TrigDefs::Physics);

#ifdef ASGTOOL_STANDALONE
    std::string tdt_name;
    declareProperty("TrigDecTool", tdt_name="TrigDecTool");
    m_trigdec_tool = new ToolHandle<Trig::TrigDecisionTool> (tdt_name);
#endif

#ifdef ASGTOOL_ATHENA
    m_trigdec_tool = new ToolHandle<Trig::TrigDecisionTool> ();
    declareProperty("TrigDecTool", *m_trigdec_tool);
#endif

    m_matching_tool = new HltL1MatchingTool(name + "_matching");

  }

  HltEmulationTool::~HltEmulationTool() {
    delete m_trigdec_tool;
    delete m_matching_tool;
  }

  // Copy constructor
  HltEmulationTool::HltEmulationTool(const HltEmulationTool& other): asg::AsgTool(other.name() + "_copy")
  {}

  // Initialize
  StatusCode HltEmulationTool::initialize() {
    m_chains.clear();
    if (m_hlt_chains_vec.size() !=0) {
      std::map<std::string, HltChain> all_chains = HltParsing::chains();
      for (auto ch: m_hlt_chains_vec) { 
        if(all_chains.find(ch) == all_chains.end()) {
          ATH_MSG_INFO("Skipping unknown HLT chain " << ch);
          continue;
        }
        ATH_MSG_INFO("Grabbing chain " << ch);
        m_chains[ch] = all_chains[ch];
      }
    } else {
      ATH_MSG_WARNING("No HLT Chains are passed to the tool");
    }
    // m_chains = HltParsing::chains();

    if(m_chains.size() == 0){
      ATH_MSG_FATAL("No initialised HLT tools found!");
      return StatusCode::FAILURE;
    }

    // Build the list of l1 tools 
    std::set<std::string> l1_items;
    std::set<std::string> l1_seeds;
    for (auto &chain : m_chains)  {
      ATH_MSG_INFO("Consider " << chain.first << " with " << chain.second.n_tau_items() << " taus.");

      // 1) consider the l1 accept
      l1_items.insert("L1_" + chain.second.l1_accept());
      m_L1_tau_decision[chain.second.l1_accept()] = false;
      m_L1_tau_decision_calculated[chain.second.l1_accept()] = false;

      // 2) loop over the items and consider the seed
      for (auto &item : chain.second.items()) {
        ATH_MSG_INFO("Item " << item.name());
        l1_items.insert("L1_" + item.seed());
        l1_seeds.insert(item.seed());
        m_L1_tau_decision[item.seed()] = false;
        m_L1_tau_decision_calculated[item.seed()] = false;
      }
    }

    // ATH_MSG_INFO("Initializing " << m_l1_emulation_tool->name());
    // ATH_CHECK(m_l1_emulation_tool->initialize());  
    ATH_CHECK(m_l1_emulation_tool.retrieve());  

    auto l1tool_chains = m_l1_emulation_tool->GetL1Chains();
    for (auto &ch: l1_items) {
      bool found = std::find(l1tool_chains.begin(), l1tool_chains.end(), ch) != l1tool_chains.end();
      if (not found) {
        ATH_MSG_ERROR("The chain "<< ch 
            <<" has not been passed to the Level1EmulationTool "<< m_l1_emulation_tool->name());
        return StatusCode::FAILURE;
      }
    }

    ATH_MSG_DEBUG("Available L1 seeds:");
    for (auto s: l1_seeds) {
      ATH_MSG_DEBUG("\t" << s);
    }

    // pass these on to the matching tool
    ATH_CHECK(m_matching_tool->setProperty("L1seeds", l1_seeds));
    ATH_CHECK(m_matching_tool->initialize());
    m_matching_tool->msg().setLevel(this->msg().level());
    
    //clean up obsolete L1 tools
    ATH_MSG_INFO("removing unused L1 tools");
    ATH_CHECK(m_l1_emulation_tool->removeUnusedTools(l1_seeds));

    ATH_CHECK(m_trigdec_tool->retrieve());

    // erasing from a ToolHandleArray creates an odd segfault, so write a new one
    ToolHandleArray<IHltTauSelectionTool> used_hlt_tau_tools;
    for(auto it: m_hlt_tau_tools) {
      if(m_chains.find("HLT_"+it->name()) == m_chains.end()){
        ATH_MSG_INFO("not using tool " << it->name());
        //++it;
      } else {
        ATH_MSG_INFO("will use tool " << it->name());
        used_hlt_tau_tools.push_back(it);
        //++it;
      }
    }

    m_hlt_tau_tools = used_hlt_tau_tools;

    for (auto it: m_hlt_tau_tools) {
      ATH_MSG_INFO("Initializing " << it->name());
      ATH_CHECK(it->initialize());
      // it->msg().setLevel(this->msg().level());
    }
    return StatusCode::SUCCESS;
  }


  StatusCode HltEmulationTool::execute(const xAOD::EmTauRoIContainer* l1taus, 
      const xAOD::JetRoIContainer* l1jets,
      const xAOD::MuonRoIContainer* l1muons,
      const xAOD::EnergySumRoI* l1xe,
      const xAOD::TauJetContainer* hlt_taus,
      const DataVector<xAOD::TrackParticle>* fast_tracks)
  {
    ATH_CHECK(m_l1_emulation_tool->calculate(l1taus, l1jets, l1muons, l1xe));
    ATH_CHECK(m_matching_tool->execute(hlt_taus, l1taus));
    ATH_CHECK(calculate(hlt_taus, l1taus, fast_tracks));

    return StatusCode::SUCCESS;

  }
  
  StatusCode HltEmulationTool::execute(const xAOD::EmTauRoIContainer* l1taus, 
      const xAOD::JetRoIContainer* l1jets,
      const xAOD::MuonRoIContainer* l1muons,
      const xAOD::EnergySumRoI* l1xe,
      const xAOD::TauJetContainer* hlt_taus,
      const DataVector<xAOD::TrackParticle>* preselTracksIso,
      const DataVector<xAOD::TrackParticle>* preselTracksCore)
  {
    ATH_CHECK(m_l1_emulation_tool->calculate(l1taus, l1jets, l1muons, l1xe));
    ATH_CHECK(m_matching_tool->execute(hlt_taus, l1taus));
    ATH_CHECK(calculate(hlt_taus, l1taus, preselTracksIso, preselTracksCore));

    return StatusCode::SUCCESS;

  }
  
  StatusCode HltEmulationTool::execute(const xAOD::EmTauRoIContainer* l1taus, 
      const xAOD::JetRoIContainer* l1jets,
      const xAOD::MuonRoIContainer* l1muons,
      const xAOD::EnergySumRoI* l1xe,
      const std::vector<DecoratedHltTau>& hlt_taus)
  {
    ATH_CHECK(m_l1_emulation_tool->calculate(l1taus, l1jets, l1muons, l1xe));
    ATH_CHECK(m_matching_tool->execute(hlt_taus, l1taus));
    ATH_CHECK(calculate(hlt_taus, l1taus));

    return StatusCode::SUCCESS;

  }
  
  StatusCode HltEmulationTool::execute(const xAOD::EmTauRoIContainer* l1taus, 
      const xAOD::JetRoIContainer* l1jets,
      const xAOD::MuonRoIContainer* l1muons,
      const xAOD::EnergySumRoI* l1xe,
      const xAOD::TauJetContainer* hlt_taus,
      const xAOD::TauJetContainer* presel_taus)
  {
    ATH_CHECK(m_l1_emulation_tool->calculate(l1taus, l1jets, l1muons, l1xe));
    ATH_CHECK(m_matching_tool->execute(hlt_taus, l1taus));
    ATH_CHECK(calculate(hlt_taus, presel_taus, l1taus));

    return StatusCode::SUCCESS;

  }
  
  // Event calculate -- The meaty part of this algorithm
  // Method uses EDM tau candidate plus EDM track collection
  StatusCode HltEmulationTool::calculate(const std::vector<DecoratedHltTau>& hlt_taus,
      const xAOD::EmTauRoIContainer* l1taus) {
    
    // Reset the counters to 0;
    reset_tau_decision();
    clearL1Decision();
    
    for (const auto hlt_tau : hlt_taus) {
      for (auto it: m_hlt_tau_tools) {
        hlt_tau.getHltTau()->auxdecor<bool>(it.name()) = false;

        if (it->accept(hlt_tau)) {
          ATH_MSG_DEBUG("ACCEPT FOR " << it.name());
          hlt_tau.getHltTau()->auxdecor<bool>(it.name()) = true;
        } else {
          ATH_MSG_DEBUG("REJECT FOR " << it.name());
        }
      }
    }

    ATH_CHECK(calculate_tau_decision(hlt_taus, l1taus));

    return StatusCode::SUCCESS;
  }

  // Event calculate -- The meaty part of this algorithm
  // Method uses EDM tau candidate plus EDM track collection
  StatusCode HltEmulationTool::calculate(const xAOD::TauJetContainer* hlt_taus,
      const xAOD::EmTauRoIContainer* l1taus,
      const DataVector<xAOD::TrackParticle>* preselTracksIso,
      const DataVector<xAOD::TrackParticle>* preselTracksCore) {
    
    // Reset the counters to 0;
    reset_tau_decision();
    clearL1Decision();
    
    for (const auto hlt_tau : *hlt_taus) {
      for (auto it: m_hlt_tau_tools) {
        hlt_tau->auxdecor<bool>(it.name()) = false;

        if (it->accept(hlt_tau, preselTracksIso, preselTracksCore)) {
          ATH_MSG_DEBUG("ACCEPT FOR " << it.name());
          hlt_tau->auxdecor<bool>(it.name()) = true;
        } else {
          ATH_MSG_DEBUG("REJECT FOR " << it.name());
        }
      }
    }

    ATH_CHECK(calculate_tau_decision(hlt_taus, l1taus));

    return StatusCode::SUCCESS;
  }
  
  // Event calculate -- The meaty part of this algorithm
  // Method uses EDM tau candidate plus EDM track collection
  StatusCode HltEmulationTool::calculate(const xAOD::TauJetContainer* hlt_taus,
      const xAOD::EmTauRoIContainer* l1taus,
      const DataVector<xAOD::TrackParticle>* fast_tracks) {
    
    // Reset the counters to 0;
    reset_tau_decision();
    clearL1Decision();
    
    for (const auto hlt_tau : *hlt_taus) {
      for (auto it: m_hlt_tau_tools) {
        hlt_tau->auxdecor<bool>(it.name()) = false;
        if (it->accept(hlt_tau, fast_tracks)) {
          ATH_MSG_DEBUG("ACCEPT FOR " << it.name());
          hlt_tau->auxdecor<bool>(it.name()) = true;
        } else {
          ATH_MSG_DEBUG("REJECT FOR " << it.name());
        }
      }
    }

    ATH_CHECK(calculate_tau_decision(hlt_taus, l1taus));

    return StatusCode::SUCCESS;
  }

  // Event calculate -- The meaty part of this algorithm
  // Method uses EDM tau candidate plus associated nCore / nWide
  StatusCode HltEmulationTool::calculate(const xAOD::TauJetContainer* hlt_taus,
      const xAOD::TauJetContainer* presel_taus,
      const xAOD::EmTauRoIContainer* l1taus) {

    // Reset the counters to 0;
    reset_tau_decision();
    clearL1Decision();

    for (const auto hlt_tau : *hlt_taus) {
      for (auto it: m_hlt_tau_tools) {
        hlt_tau->auxdecor<bool>(it.name()) = false;
        if (it->accept(hlt_tau, presel_taus)) {
          hlt_tau->auxdecor<bool>(it.name()) = true;
        }
        ATH_MSG_DEBUG("\t\t HLT TAU "<< hlt_tau->index() 
            << " with (Pt/Eta/Phi/Discri): "<< hlt_tau->pt() 
            << ", " << hlt_tau->eta() << ", " << hlt_tau->phi()
            << ", " << hlt_tau->discriminant(xAOD::TauJetParameters::BDTJetScore)
            <<" has pass = "<< hlt_tau->auxdataConst<bool>(it.name())
            <<" for tool " << it.name());
      }
    }

    ATH_CHECK(calculate_tau_decision(hlt_taus, l1taus));

    return StatusCode::SUCCESS;
  }
  
  StatusCode HltEmulationTool::calculate_tau_decision(const std::vector<DecoratedHltTau>& hlt_taus, 
      const xAOD::EmTauRoIContainer* l1taus) {
      //TODO: this function should be merged with the next one properly
    
    // 1 -- Loop over all the chains
    for (auto &ch: m_chains) {
      m_HLT_tau_decision[ch.first] = false;
      ATH_MSG_DEBUG(ch.first);

      // Case 1: only 1 HLT tau in the chain
      if (ch.second.n_tau_items() == 1) {
        // 1 -- Iterate over the items
        for (auto &item: ch.second.items()) { 
          ATH_MSG_DEBUG("\t" << item.name() << " / " << item.seed() << "/ isTau = "<< item.isTau());
          // 2 -- check if this is a tau item
          if (not item.isTau()) {
            ATH_MSG_DEBUG("\t\t not a tau -> ignoring");
            continue;
          }

          // 3 -- loop over the hlt taus
          for (const auto decoratedTau : hlt_taus) {
            const auto hlt_tau = decoratedTau.getHltTau();

            ATH_MSG_DEBUG("\t\t HLT TAU "<< hlt_tau->index() 
                << " (Pt/Eta/Phi): "<< hlt_tau->pt() 
                << ", " << hlt_tau->eta() << ", " << hlt_tau->phi());

            if(m_perform_l1_emulation){
              // 3.1 -- only consider hlt taus with a L1 match
              ATH_MSG_DEBUG("Testing l1 match as we'll perform L1 emulation later");
              if (hlt_tau->auxdataConst<int>("l1_index_" + item.seed()) == -1 and hlt_tau->auxdataConst<int>("l1_index") == -1) {
                ATH_MSG_DEBUG("\t\t\t -> not matching L1 item -> ignoring");
                continue;
              }

              // use our seed; if this is -1, the generic has to not be -1
              auto seed_str = "l1_index_" + item.seed();
              ATH_MSG_DEBUG("\t using L1 seed str " << seed_str);
              auto l1_index = hlt_tau->auxdataConst<int>(seed_str);
              if (l1_index == -1) { 
                seed_str = "l1_index";
                l1_index = hlt_tau->auxdataConst<int>("l1_index");
                ATH_MSG_DEBUG("\t seed str not found; using L1 seed str " << seed_str);
              }
              auto l1_matched_tau = l1taus->at(l1_index);

              ATH_MSG_DEBUG("\t\t index: " << l1_index << " => l1 pt/eta/phi = " 
                  << l1_matched_tau->tauClus() << ", " 
                  << l1_matched_tau->eta() << ", " 
                  << l1_matched_tau->phi());
              ATH_MSG_DEBUG("\t\t Is seed " << item.seed() << " passed ? :" 
                  << l1_matched_tau->auxdataConst<bool>(item.seed()));

              // 3.2 -- Check if L1 matched item passes the seed
              if (not l1_matched_tau->auxdataConst<bool>(item.seed())) {
                ATH_MSG_DEBUG("\t\t matched L1 item did not pass trigger -> ignoring");
                continue;
              }
              ATH_MSG_DEBUG("\t\t Is item passed ? :"
                  << hlt_tau->auxdataConst<bool>(item.name()));
            } else {
              ATH_MSG_DEBUG("not looking at L1 decision");
            }

            // 3.3 -- Check if the HLT item passes
            if (not hlt_tau->auxdataConst<bool>(item.name())) {
              ATH_MSG_DEBUG("\t\t HLT item did not pass -> ignoring");
              continue;
            }

            // 3.4 -- declare the decision true and break the loop
            ATH_MSG_DEBUG("\t\t => PASSED <=");
            m_HLT_tau_decision[ch.first] = true;
            break;
          } // loop over hlt_taus
        } // loop over the items (ch.second.items())
      } else if (ch.second.n_tau_items() == 2) {
        ATH_MSG_DEBUG(ch.first << " has 2 hlt taus");
        std::vector<HltItem> tau_items;
        for (auto &item: ch.second.items()) { 
          if (item.isTau()) {
            tau_items.push_back(item);
          }
        }

        // protection
        if (tau_items.size() != 2) {
          ATH_MSG_ERROR("Wrong number of tau items (not taken into account)");
          return StatusCode::FAILURE;
        }

        HltItem item1 = tau_items[0];
        HltItem item2 = tau_items[1];
        int n_item1_alone = 0;
        int n_item2_alone = 0;
        int n_doublets = 0;

        for (const auto decoratedTau : hlt_taus) {
          const auto hlt_tau = decoratedTau.getHltTau();
          bool pass_item1 = false;
          bool pass_item2 = false;

          // skip unmatched taus to L1
          if (hlt_tau->auxdataConst<int>("l1_index") == -1) {
            continue;
          }

          auto l1_matched_tau = l1taus->at(hlt_tau->auxdataConst<int>("l1_index"));

          if (l1_matched_tau->auxdataConst<bool>(item1.seed()) and hlt_tau->auxdataConst<bool>(item1.name())) {
            pass_item1 = true;
          }

          if (l1_matched_tau->auxdataConst<bool>(item2.seed()) and hlt_tau->auxdataConst<bool>(item2.name())) {
            pass_item2 = true;
          }

          if (pass_item1 and not pass_item2) {
            n_item1_alone += 1;
          } else if (not pass_item1 and pass_item2) {
            n_item2_alone += 1;
          } else if (pass_item1 and pass_item2) {
            n_doublets += 1;
          }
        } // loop over hlt_taus

        if ((n_item1_alone > 0 and n_item2_alone > 0) or 
            (n_item1_alone > 0 and n_doublets > 0) or
            (n_item2_alone > 0 and n_doublets > 0) or
            (n_doublets > 1)) {
          m_HLT_tau_decision[ch.first] = true;
        }

      } else {
        ATH_MSG_ERROR("Wrong number of taus (not taken into account");
        return StatusCode::FAILURE;
      }

    }

    return StatusCode::SUCCESS;

  }

  StatusCode HltEmulationTool::calculate_tau_decision(const xAOD::TauJetContainer* hlt_taus, 
      const xAOD::EmTauRoIContainer* l1taus) {
    
    // 1 -- Loop over all the chains
    for (auto &ch: m_chains) {
      m_HLT_tau_decision[ch.first] = false;
      ATH_MSG_DEBUG(ch.first);

      // Case 1: only 1 HLT tau in the chain
      if (ch.second.n_tau_items() == 1) {
        // 1 -- Iterate over the items
        for (auto &item: ch.second.items()) { 
          ATH_MSG_DEBUG("\t" << item.name() << " / " << item.seed() << "/ isTau = "<< item.isTau());
          // 2 -- check if this is a tau item
          if (not item.isTau()) {
            ATH_MSG_DEBUG("\t\t not a tau -> ignoring");
            continue;
          }

          // 3 -- loop over the hlt taus
          for (const auto hlt_tau : *hlt_taus) {
            ATH_MSG_DEBUG("\t\t HLT TAU "<< hlt_tau->index() 
                << " (Pt/Eta/Phi): "<< hlt_tau->pt() 
                << ", " << hlt_tau->eta() << ", " << hlt_tau->phi());

            if(m_perform_l1_emulation){
              // 3.1 -- only consider hlt taus with a L1 match
              ATH_MSG_DEBUG("Testing l1 match as we'll perform L1 emulation later");
              if (hlt_tau->auxdataConst<int>("l1_index_" + item.seed()) == -1 and hlt_tau->auxdataConst<int>("l1_index") == -1) {
                ATH_MSG_DEBUG("\t\t\t -> not matching L1 item -> ignoring");
                continue;
              }

              // use our seed; if this is -1, the generic has to not be -1
              auto seed_str = "l1_index_" + item.seed();
              ATH_MSG_DEBUG("\t using L1 seed str " << seed_str);
              auto l1_index = hlt_tau->auxdataConst<int>(seed_str);
              if (l1_index == -1) { 
                seed_str = "l1_index";
                l1_index = hlt_tau->auxdataConst<int>("l1_index");
                ATH_MSG_DEBUG("\t seed str not found; using L1 seed str " << seed_str);
              }
              auto l1_matched_tau = l1taus->at(l1_index);

              ATH_MSG_DEBUG("\t\t index: " << l1_index << " => l1 pt/eta/phi = " 
                  << l1_matched_tau->tauClus() << ", " 
                  << l1_matched_tau->eta() << ", " 
                  << l1_matched_tau->phi());
              ATH_MSG_DEBUG("\t\t Is seed " << item.seed() << " passed ? :" 
                  << l1_matched_tau->auxdataConst<bool>(item.seed()));

              // 3.2 -- Check if L1 matched item passes the seed
              if (not l1_matched_tau->auxdataConst<bool>(item.seed())) {
                ATH_MSG_DEBUG("\t\t matched L1 item did not pass trigger -> ignoring");
                continue;
              }
              ATH_MSG_DEBUG("\t\t Is item passed ? :"
                  << hlt_tau->auxdataConst<bool>(item.name()));
            } else {
              ATH_MSG_DEBUG("not looking at L1 decision");
            }

            // 3.3 -- Check if the HLT item passes
            if (not hlt_tau->auxdataConst<bool>(item.name())) {
              ATH_MSG_DEBUG("\t\t HLT item did not pass -> ignoring");
              continue;
            }

            // 3.4 -- declare the decision true and break the loop
            ATH_MSG_DEBUG("\t\t => PASSED <=");
            m_HLT_tau_decision[ch.first] = true;
            break;
          } // loop over hlt_taus
        } // loop over the items (ch.second.items())
      } else if (ch.second.n_tau_items() == 2) {
        ATH_MSG_DEBUG(ch.first << " has 2 hlt taus");
        std::vector<HltItem> tau_items;
        for (auto &item: ch.second.items()) { 
          if (item.isTau()) {
            tau_items.push_back(item);
          }
        }

        // protection
        if (tau_items.size() != 2) {
          ATH_MSG_ERROR("Wrong number of tau items (not taken into account)");
          return StatusCode::FAILURE;
        }

        HltItem item1 = tau_items[0];
        HltItem item2 = tau_items[1];
        int n_item1_alone = 0;
        int n_item2_alone = 0;
        int n_doublets = 0;

        for (const auto hlt_tau : *hlt_taus) {
          bool pass_item1 = false;
          bool pass_item2 = false;

          // skip unmatched taus to L1
          if (hlt_tau->auxdataConst<int>("l1_index") == -1) {
            continue;
          }

          auto l1_matched_tau = l1taus->at(hlt_tau->auxdataConst<int>("l1_index"));

          if (l1_matched_tau->auxdataConst<bool>(item1.seed()) and hlt_tau->auxdataConst<bool>(item1.name())) {
            pass_item1 = true;
          }

          if (l1_matched_tau->auxdataConst<bool>(item2.seed()) and hlt_tau->auxdataConst<bool>(item2.name())) {
            pass_item2 = true;
          }

          if (pass_item1 and not pass_item2) {
            n_item1_alone += 1;
          } else if (not pass_item1 and pass_item2) {
            n_item2_alone += 1;
          } else if (pass_item1 and pass_item2) {
            n_doublets += 1;
          }
        } // loop over hlt_taus

        if ((n_item1_alone > 0 and n_item2_alone > 0) or 
            (n_item1_alone > 0 and n_doublets > 0) or
            (n_item2_alone > 0 and n_doublets > 0) or
            (n_doublets > 1)) {
          m_HLT_tau_decision[ch.first] = true;
        }

      } else {
        ATH_MSG_ERROR("Wrong number of taus (not taken into account");
        return StatusCode::FAILURE;
      }

    }

    return StatusCode::SUCCESS;
  }


  bool HltEmulationTool::decision(const std::string& chain_name) {
    ATH_MSG_DEBUG("HltEmulationTool::decision(): consider " << chain_name);
    HltChain chain = m_chains[chain_name];

    if(m_perform_l1_emulation){
      ATH_MSG_DEBUG("Check the L1 accept: "<< chain.l1_accept());
    
      bool pass_L1 = false;
      if(not m_L1_tau_decision_calculated[chain.l1_accept()]){
        ATH_MSG_DEBUG("Calculating and caching L1 decision for L1" << chain.l1_accept());
        m_L1_tau_decision_calculated[chain.l1_accept()] = true;
        m_L1_tau_decision[chain.l1_accept()] = m_l1_emulation_tool->decision("L1_" + chain.l1_accept());
      } else {
        ATH_MSG_DEBUG("Using cached L1 decision for L1" << chain.l1_accept());
      }
      pass_L1 = m_L1_tau_decision[chain.l1_accept()]; 

      //bool pass_L1 = m_l1_emulation_tool->decision("L1_" + chain.l1_accept());

      if (not pass_L1) {
        ATH_MSG_DEBUG(" => Did not pass the emulated L1 accept");
        return false;
      }

      ATH_MSG_DEBUG(" => Passed the emulated L1 accept");
    } else {
      ATH_MSG_DEBUG("Not emulating the L1 accept; consider HLT only");
    }

    // Compute the tau part
    if (chain.n_tau_items() > 0) {
      if (not m_HLT_tau_decision[chain_name]) {
        ATH_MSG_DEBUG("Not passing HLT tau decision for " + chain_name);
        return false;
      }
    }
    ATH_MSG_DEBUG("Pass the HLT tau decision");

    // non-tau part from the TDT
    for (auto &item: chain.items()) {
      if (not item.isTau()) {
        ATH_MSG_DEBUG("Evaluate "<< item.seed());
        bool pass_l1 = (*m_trigdec_tool)->isPassed("L1_" + item.seed(), m_L1TriggerCondition);
        ATH_MSG_DEBUG(item.seed() << " has pass = " << pass_l1);
        if (not pass_l1) {
          return false;
        }

        ATH_MSG_DEBUG("Evaluate "<< item.name());
        bool pass = (*m_trigdec_tool)->isPassed("HLT_" + item.name(), m_HLTTriggerCondition);
        ATH_MSG_DEBUG(item.name() << " has pass = " << pass);
        if (not pass) { 
          return false;
        }
      }

      ATH_MSG_DEBUG("Pass the non-tau decision");


    }

    // If passes all the step, returns true
    return true;

  }

  // Finalize
  StatusCode HltEmulationTool::finalize() {

    return StatusCode::SUCCESS;
  }

  void HltEmulationTool::clearL1Decision() {
    ATH_MSG_DEBUG("Clearing L1 decisions");
    for (auto &it: m_L1_tau_decision) {
      it.second = false;
    }
    for (auto &it: m_L1_tau_decision_calculated) {
      it.second = false;
    }
  }

  void HltEmulationTool::reset_tau_decision() {
    for (auto &it: m_HLT_tau_decision) {
      it.second = false;
    }
  }

} // end of the namespace
