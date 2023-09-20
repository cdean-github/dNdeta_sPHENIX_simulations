#include "MetadataContainer.h"

#include "Metadata.h"

#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/SubsysReco.h>  // for SubsysReco

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>    // for PHIODataNode
#include <phool/PHNode.h>          // for PHNode
#include <phool/PHNodeIterator.h>  // for PHNodeIterator
#include <phool/PHObject.h>        // for PHObject
#include <phool/getClass.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <HepMC/GenEvent.h>
#pragma GCC diagnostic pop

#include <HepMC/HeavyIon.h>  // for HeavyIon

#include <iterator>  // for operator!=, reverse_iterator
#include <map>       // for _Rb_tree_iterator
#include <utility>   // for pair

MetadataContainer::MetadataContainer(const std::string &name)
  : SubsysReco(name)
{
}

int MetadataContainer::Init(PHCompositeNode *topNode)
{
  PHNodeIterator iter(topNode);
  PHCompositeNode *runNode = dynamic_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "RUN"));
  Metadata *metadata = new Metadata();
  PHIODataNode<PHObject> *newNode = new PHIODataNode<PHObject>(metadata, "Metadata", "PHObject");
  runNode->addNode(newNode);

  return Fun4AllReturnCodes::EVENT_OK;
}

int MetadataContainer::InitRun(PHCompositeNode *topNode)
{
  Metadata *metadata = findNode::getClass<Metadata>(topNode, "Metadata");
  for (auto &info : m_metadata) metadata->set_stringval(info.first, info.second);
  return Fun4AllReturnCodes::EVENT_OK;
}
