#ifndef MACRO_FUN4ALLG4SPHENIX_C
#define MACRO_FUN4ALLG4SPHENIX_C

#include <ctime>
#include <dirent.h>
#include <gsl/gsl_rng.h>
#include <unistd.h>

#include <GlobalVariables.C>

#include <DisplayOn.C>
#include <G4Setup_sPHENIX.C>
#include <G4_ActsGeom.C>
#include <G4_Bbc.C>
#include <G4_Centrality.C>
#include <G4_Global.C>
#include <G4_Input.C>
#include <G4_Magnet.C>

#include <Trkr_Clustering.C>
#include <Trkr_Reco.C>
#include <Trkr_RecoInit.C>

#include <ffamodules/CDBInterface.h>
#include <ffamodules/FlagHandler.h>
#include <ffamodules/HeadReco.h>
#include <ffamodules/SyncReco.h>

#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllSyncManager.h>
#include <fun4all/Fun4AllUtils.h>

#include <phool/PHRandomSeed.h>
#include <phool/recoConsts.h>

//#include <RooUnblindPrecision.h>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libffamodules.so)

bool checkForDir(string name) {
  DIR *dir = opendir(name.c_str());

  return dir == NULL ? 0 : 1;
}

bool checkForFile(string dir, string base) {
  bool fileExists = false;
  string fileName;

  fileName = dir + "/" + base;
  ifstream file(fileName.c_str());
  if (file.good())
    fileExists = true;

  return fileExists;
}

class Deleter
{
 public:
  void operator()(gsl_rng *rng) const { gsl_rng_free(rng); }
};

//https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
std::string exec(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int Fun4All_dNdeta_simulator(const int nEvents = 1,
                             const string &outputDir = "./",
                             const string &outputFile = "G4sPHENIX-000-0000.root",
                             const int skip = 0,
                             const string generator = "PYTHIA8",
                             const bool fullSim = true,
                             const bool turnOnMagnet = true)
{
  // Get base file name
  DstOut::OutputDir = outputDir;
  string outputDirLastChar = DstOut::OutputDir.substr(DstOut::OutputDir.size() - 1, 1);
  if (outputDirLastChar != "/") DstOut::OutputDir += "/";

  unsigned int revisionWidth = 5;
  unsigned int revisionNumber = 0;
  ostringstream dstRevision;
  dstRevision << setfill('0') << setw(revisionWidth) << to_string(revisionNumber);
  DstOut::OutputDir += "dstSet_" + dstRevision.str();
  while (checkForDir(DstOut::OutputDir)) 
  {
    bool dstFileStatus = checkForFile(DstOut::OutputDir, outputFile);
    if (!dstFileStatus) break;
    DstOut::OutputDir = DstOut::OutputDir.substr(0, DstOut::OutputDir.size() - revisionWidth);
    revisionNumber++;
    dstRevision.str("");
    dstRevision.clear();
    dstRevision << setfill('0') << setw(revisionWidth) << to_string(revisionNumber);
    DstOut::OutputDir += dstRevision.str();
  }

  string productionDir = DstOut::OutputDir + "/inProduction";
  string outputRecoFile = productionDir + "/" + outputFile;
  string makeDirectory = "mkdir -p " + productionDir;
  system(makeDirectory.c_str());

  //Now set a fixed random seed and get metadata
  time_t now = time(0);
cout << "The time is " << asctime(localtime(&now)) << endl;

char hostname[HOST_NAME_MAX];
char username[LOGIN_NAME_MAX];
gethostname(hostname, HOST_NAME_MAX);
getlogin_r(username, LOGIN_NAME_MAX);
cout << "Hostname: " << hostname << ", username: " << username << endl;
  // General F4A setup
  Input::VERBOSITY = 0;
  Fun4AllServer *se = Fun4AllServer::instance();
  se->Verbosity(Input::VERBOSITY);

  // Make a reproducible set o frandom seeds
  PHRandomSeed::Verbosity(1);
  recoConsts *rc = recoConsts::instance();
  bool fix_seed = true;

  if (fix_seed)
  {
    std::unique_ptr<gsl_rng, Deleter>  m_rng;
    const uint seed = PHRandomSeed();
    m_rng.reset(gsl_rng_alloc(gsl_rng_mt19937));
    gsl_rng_set(m_rng.get(), seed);
    int myRandomSeed = abs(ceil(gsl_rng_uniform_pos(m_rng.get())*10e8));
cout << "gsl seed is " << myRandomSeed << endl;
    rc->set_IntFlag("RANDOMSEED", myRandomSeed);
  }

  cout << "The top level random seed is " << rc->get_IntFlag("RANDOMSEED") << endl;

  string gitHash = exec("git rev-parse --short HEAD");

  cout << "The git hash is " << gitHash << endl;

return 0;

  //===============
  // conditions DB flags
  //===============
  Enable::CDB = true;
  // global tag
  rc->set_StringFlag("CDB_GLOBALTAG", CDB::global_tag);
  // 64 bit timestamp
  rc->set_uint64Flag("TIMESTAMP", CDB::timestamp);

  pair<int, int> runseg = Fun4AllUtils::GetRunSegment(outputFile);
  int runnumber = runseg.first;
  int segment = runseg.second;
  if (runnumber != 0) 
  {
    rc->set_IntFlag("RUNNUMBER", runnumber);
    Fun4AllSyncManager *syncman = se->getSyncManager();
    syncman->SegmentNumber(segment);
  }

  // Setup generators
  Input::SIMPLE = Input::PYTHIA8 = Input::HEPMC = false;

  if (generator == "SIMPLE")
  {
    Input::SIMPLE = true;
  }
  else if (generator == "PYTHIA8")
  {
    Input::PYTHIA8 = true;
    Input::BEAM_CONFIGURATION = Input::pp_COLLISION;
    PYTHIA8::config_file = string(getenv("CALIBRATIONROOT")) + "/Generators/HeavyFlavor_TG/phpythia8_minBias_MDC2.cfg";
  } 
  else if (generator == "HIJING") 
  {
    Input::HEPMC = true;
    INPUTHEPMC::filename = "/sphenix/sim/sim01/sphnxpro/MDC1/sHijing_HepMC/data/sHijing_0_20fm-0000000001-00000.dat";
    INPUTHEPMC::FLOW = true;
    INPUTHEPMC::FERMIMOTION = true;
    Input::BEAM_CONFIGURATION = Input::AA_COLLISION;
  }
  else
  {
    cout << "Generator " << generator << " unknown. Exiting!" << endl;
    exit(1);
  }

  InputInit();

  if (generator == "SIMPLE") 
  {
    INPUTGENERATOR::SimpleEventGenerator[0]->add_particles("pi-", 5);
    INPUTGENERATOR::SimpleEventGenerator[0]->set_vertex_distribution_function(PHG4SimpleEventGenerator::Gaus, 
                                                                              PHG4SimpleEventGenerator::Gaus,
                                                                              PHG4SimpleEventGenerator::Gaus);
    INPUTGENERATOR::SimpleEventGenerator[0]->set_vertex_distribution_mean(0., 0., 0.);
    INPUTGENERATOR::SimpleEventGenerator[0]->set_vertex_distribution_width(0.01, 0.01, 5.);
    INPUTGENERATOR::SimpleEventGenerator[0]->set_eta_range(-1, 1);
    INPUTGENERATOR::SimpleEventGenerator[0]->set_phi_range(-M_PI, M_PI);
    INPUTGENERATOR::SimpleEventGenerator[0]->set_pt_range(0.1, 20.);
  }
  else if (generator == "PYTHIA8")
  {
    Input::ApplysPHENIXBeamParameter(INPUTGENERATOR::Pythia8);
  }
  else
  {
    Input::ApplysPHENIXBeamParameter(INPUTMANAGER::HepMCInputManager);
    if (Input::PILEUPRATE > 0)
    {
      INPUTMANAGER::HepMCPileupInputManager->CopyHelperSettings(INPUTMANAGER::HepMCInputManager);
    }
  }

  /*
   * register all input generators with Fun4All
   */
  InputRegister();

  SyncReco *sync = new SyncReco();
  se->registerSubsystem(sync);

  HeadReco *head = new HeadReco();
  se->registerSubsystem(head);

  FlagHandler *flag = new FlagHandler();
  se->registerSubsystem(flag);

  //======================
  // Write the DST
  //======================

  Enable::DSTOUT = true;

  G4MAGNET::magfield_rescale = (turnOnMagnet == true) ? 1. : 0.;
  MagnetInit();
  MagnetFieldInit();

  /*
   * Run detector simulation
   */
  if (fullSim) 
  {
    Enable::BBC = true;
    Enable::PIPE = true;
    Enable::MVTX = true;
    Enable::INTT = true;
    Enable::TPC = true;
    Enable::MICROMEGAS = true;

    G4Init();
    G4Setup();


    Mvtx_Cells();
    Intt_Cells();
    TrackingInit();
    Mvtx_Clustering();
    Intt_Clustering();
  }

  if (generator == "HIJING")
    Centrality();

  InputManagers();

  Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", outputRecoFile);
  se->registerOutputManager(out);

  //-----------------
  // Event processing
  //-----------------

  // if we use a negative number of events we go back to the command line here
  if (nEvents < 0)
  {
    return 0;
  }
  // if we run the particle generator and use 0 it'll run forever
  if (nEvents == 0 && !Input::HEPMC && !Input::READHITS) {
    cout << "using 0 for number of events is a bad idea when using particle generators" << endl;
    cout << "it will run forever, so I just return without running anything" << endl;
    return 0;
  }

  se->run(nEvents);

  //-----
  // Exit
  //-----
  se->End();

  ifstream file(outputRecoFile.c_str());
  if (file.good())
  {
    string moveOutput = "mv " + outputRecoFile + " " + DstOut::OutputDir;
    system(moveOutput.c_str());
  }

  std::cout << "All done" << std::endl;
  delete se;
  gSystem->Exit(0);
  return 0;
}
#endif
