// vim: ts=2 sw=2

#include "TrigTauEmulation/HltChain.h"
#include "TrigTauEmulation/ChainRegistry.h"

ChainRegistry::ChainRegistry(const std::string & name) 
  : asg::AsgTool(name)
{}

ChainRegistry::~ChainRegistry() {}

ChainRegistry::ChainRegistry(const ChainRegistry& other): asg::AsgTool(other.name() + "_copy") {
}

HltChain& ChainRegistry::getChain(const std::string &name) {
  if(m_chains.count(name) == 0) {
    std::stringstream e;
    e << "Chain " << name << " does not exist";
    throw std::invalid_argument(e.str());
  }

  return m_chains[name];
}

StatusCode ChainRegistry::addChain(const std::string &name, const HltChain &chain) {
  // we presume that this chain relies on items that are already defined at this stage

  if(m_chains.count(name) != 0) {
    std::stringstream e;
    e << "Chain " << name << " already exists";
    throw std::invalid_argument(e.str());
  }

  m_chains[name] = chain;
  return StatusCode::SUCCESS;
}
  
StatusCode ChainRegistry::initialize() {

  // pick up the default chains
  m_chains = HltParsing::chains();

  return StatusCode::SUCCESS;
}
