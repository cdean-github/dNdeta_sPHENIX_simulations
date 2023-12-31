#include "MetadataContainer.h"

#include <metadata/Metadata.h>

#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/SubsysReco.h>  // for SubsysReco
#include <fun4all/Fun4AllBase.h> 

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>    // for PHIODataNode
#include <phool/PHNode.h>          // for PHNode
#include <phool/PHNodeIterator.h>  // for PHNodeIterator
#include <phool/PHObject.h>        // for PHObject
#include <phool/getClass.h>

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
  if (Verbosity() >= VERBOSITY_SOME) metadata->identify();

  return Fun4AllReturnCodes::EVENT_OK;
}
