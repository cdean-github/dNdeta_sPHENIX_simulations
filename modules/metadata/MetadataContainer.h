// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef FFAMODULES_HEADRECO_H
#define FFAMODULES_HEADRECO_H

#include <fun4all/SubsysReco.h>

#include <string>  // for string

class PHCompositeNode;

class MetadataContainer : public SubsysReco
{
 public:
  MetadataContainer(const std::string &name = "MetadataContainer");
  ~MetadataContainer() override {}
  int Init(PHCompositeNode *topNode) override;
  int InitRun(PHCompositeNode *topNode) override;
  int process_event(PHCompositeNode *topNode) override;

  void addMetadataStrings(std::vector<std::pair<std::string, std::string>> inputInfo) { m_metadata = inputInfo };

 protected:
   
   std::vector<std::pair<std::string, std::string>> m_metadata;
};

#endif

