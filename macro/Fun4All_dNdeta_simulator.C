#ifndef MACRO_FUN4ALLG4SPHENIX_C
#define MACRO_FUN4ALLG4SPHENIX_C

#include <ctime>
#include <dirent.h>
#include <gsl/gsl_rng.h>
#include <unistd.h>

#include <GlobalVariables.C>

#include "G4_Input.C"
#include "G4_TrkrSimulation.C"
#include <DisplayOn.C>
#include <G4Setup_sPHENIX.C>
#include <G4_ActsGeom.C>
#include <G4_Centrality.C>
#include <G4_Global.C>
#include <G4_Magnet.C>
#include <G4_ZDC.C>

#include <centrality/CentralityReco.h>
#include <calotrigger/MinimumBiasClassifier.h>
#include <globalvertex/GlobalVertexReco.h>

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

#include <metadata/MetadataContainer.h>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libffamodules.so)
R__LOAD_LIBRARY(libmetadatacontainer.so)
R__LOAD_LIBRARY(libcentrality_io.so)
R__LOAD_LIBRARY(libcentrality.so)
R__LOAD_LIBRARY(libg4centrality.so)
R__LOAD_LIBRARY(libcentrality.so)
R__LOAD_LIBRARY(libcalotrigger.so)

bool checkForDir(string name)
{
    DIR *dir = opendir(name.c_str());

    return dir == NULL ? 0 : 1;
}

bool checkForFile(string dir, string base)
{
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

// https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
std::string exec(const char *cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}

void Intt_ZClustering()
{
    int verbosity = std::max(Enable::VERBOSITY, Enable::INTT_VERBOSITY);
    Fun4AllServer *se = Fun4AllServer::instance();

    InttClusterizer *inttclusterizer = new InttClusterizer("InttClusterizer", G4MVTX::n_maps_layer, G4MVTX::n_maps_layer + G4INTT::n_intt_layer - 1);
    inttclusterizer->Verbosity(verbosity);
    // no Z clustering for Intt type 1 layers (we DO want Z clustering for type 0 layers)
    // turning off phi clustering for type 0 layers is not necessary, there is only one strip
    // per sensor in phi
    for (int i = G4MVTX::n_maps_layer; i < G4MVTX::n_maps_layer + G4INTT::n_intt_layer; i++)
    {
        if (G4INTT::laddertype[i - G4MVTX::n_maps_layer] == PHG4InttDefs::SEGMENTATION_PHI)
        {
            inttclusterizer->set_z_clustering(i, true);
        }
    }
    se->registerSubsystem(inttclusterizer);
}

int Fun4All_dNdeta_simulator(const int nEvents = 1, const string &outputDir = "./", const string &outputFile = "G4sPHENIX-000-0000.root", const string generator = "PYTHIA8", const bool fullSim = true, const bool turnOnMagnet = true, const bool idealAlignment = true, const string logFile = "logFile.txt", const string particleType = "pi-")
{
    // gSystem->Load("libcentrality");
    // gSystem->Load("libcalotrigger");

    bool enable_inttzclustering = true;

    // Get base file name
    DstOut::OutputDir = outputDir;
    string outputDirLastChar = DstOut::OutputDir.substr(DstOut::OutputDir.size() - 1, 1);
    if (outputDirLastChar != "/")
        DstOut::OutputDir += "/";

    unsigned int revisionWidth = 5;
    unsigned int revisionNumber = 0;
    ostringstream dstRevision;
    dstRevision << setfill('0') << setw(revisionWidth) << to_string(revisionNumber);
    DstOut::OutputDir += "dstSet_" + dstRevision.str();
    while (checkForDir(DstOut::OutputDir))
    {
        bool dstFileStatus = checkForFile(DstOut::OutputDir, outputFile);
        if (!dstFileStatus)
            break;
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

    // General F4A setup
    Enable::VERBOSITY = 0;
    Input::VERBOSITY = 0;
    Fun4AllServer *se = Fun4AllServer::instance();
    se->Verbosity(Input::VERBOSITY);

    // Make a reproducible set of random seeds
    PHRandomSeed::Verbosity(1);
    recoConsts *rc = recoConsts::instance();
    bool fix_seed = false;

    if (fix_seed)
    {
        std::unique_ptr<gsl_rng, Deleter> m_rng;
        const uint seed = PHRandomSeed();
        m_rng.reset(gsl_rng_alloc(gsl_rng_mt19937));
        gsl_rng_set(m_rng.get(), seed);
        int myRandomSeed = abs(ceil(gsl_rng_uniform_pos(m_rng.get()) * 10e8));
        rc->set_IntFlag("RANDOMSEED", myRandomSeed);
    }

    //===============
    // conditions DB flags
    //===============
    Enable::CDB = true;
    // CDB::global_tag = "ProdA_2023";
    CDB::timestamp = 20869;

    // global tag
    rc->set_StringFlag("CDB_GLOBALTAG", CDB::global_tag);
    // 64 bit timestamp
    rc->set_uint64Flag("TIMESTAMP", CDB::timestamp);

    pair<int, int> runseg = Fun4AllUtils::GetRunSegment(outputFile);
    int runnumber = runseg.first;
    int segment = runseg.second;
    if (runnumber != 0)
    {
        // rc->set_IntFlag("RUNNUMBER", 20869);
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
    else if (generator == "HIJING" || generator == "EPOS" || generator == "AMPT")
    {
        Input::HEPMC = true;

        string fileNumber = outputFile;
        size_t findLastDash = fileNumber.find_last_of("-");
        if (findLastDash != string::npos)
            fileNumber.erase(0, findLastDash + 1);
        string remove_this = ".root";
        size_t pos = fileNumber.find(remove_this);
        if (pos != string::npos)
            fileNumber.erase(pos, remove_this.length());

        unsigned int width = 5;
        ostringstream correctFileNumber;
        if (generator == "HIJING")
            correctFileNumber << setfill('0') << setw(width) << fileNumber;
        else
            correctFileNumber << to_string(stoi(fileNumber));

        if (generator == "HIJING")
            INPUTHEPMC::filename = "/sphenix/sim/sim01/sphnxpro/MDC1/sHijing_HepMC/data/sHijing_0_20fm-0000000001-" + correctFileNumber.str() + ".dat";
        else if (generator == "EPOS")
            INPUTHEPMC::filename = "/sphenix/tg/tg01/commissioning/CaloCalibWG/sli/EPOS/condor1M/OutDir" + correctFileNumber.str() + "/z-expl6.hepmc";
        else
            INPUTHEPMC::filename = "/sphenix/tg/tg01/commissioning/CaloCalibWG/sli/AMPT/condor1M/OutDir" + correctFileNumber.str() + "/ana/ampt.hepmc";

        INPUTHEPMC::FLOW = true;
        INPUTHEPMC::FERMIMOTION = true;

        if (generator == "EPOS" || generator == "AMPT")
            INPUTHEPMC::REACTIONPLANERAND = true;

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
        INPUTGENERATOR::SimpleEventGenerator[0]->add_particles(particleType, 1);
        INPUTGENERATOR::SimpleEventGenerator[0]->set_vertex_distribution_function(PHG4SimpleEventGenerator::Gaus, PHG4SimpleEventGenerator::Gaus, PHG4SimpleEventGenerator::Gaus);
        INPUTGENERATOR::SimpleEventGenerator[0]->set_vertex_distribution_mean(0., 0., -19.8);
        INPUTGENERATOR::SimpleEventGenerator[0]->set_vertex_distribution_width(120e-4, 120e-4, 5.2);
        INPUTGENERATOR::SimpleEventGenerator[0]->set_eta_range(-0.5, 2.5);
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

    MagnetInit();
    MagnetFieldInit();
    G4MAGNET::magfield_rescale = (turnOnMagnet == true) ? 1. : 0.;

    /*
     * Run detector simulation
     */
    if (fullSim)
    {
        Enable::MBD = true;
        Enable::MBD_VERBOSITY = 0;
        Enable::MBDRECO = true;
        Enable::PIPE = true;
        Enable::MVTX = true;
        Enable::INTT = true;
        Enable::INTT_VERBOSITY = 0;
        Enable::TPC = true;
        Enable::MICROMEGAS = true;
        Enable::ZDC = false;
        Enable::ZDC_TOWER = false;
        Enable::BEAMLINE = false;
    }

    G4Init();
    G4Setup();

    if (fullSim)
    {
        // Centrality detectors
        Mbd_Reco();
        // ZDCInit();

        Mvtx_Cells();
        Intt_Cells();
        TrackingInit();
        Mvtx_Clustering();

        if (enable_inttzclustering)
            Intt_ZClustering();
        else
            Intt_Clustering();
    }

    // if (generator == "HIJING" || generator == "EPOS" || generator == "AMPT")
    // {
    //     CentralityReco *cr = new CentralityReco();
    //     cr->Verbosity(1);
    //     se->registerSubsystem(cr);
    //     //
    //     // MinimumBiasClassifier *mb = new MinimumBiasClassifier();
    //     // mb->Verbosity(INT_MAX);
    //     // se->registerSubsystem(mb);

    //     // Centrality();
    // }

    InputManagers();

    cout << "Your final output path is " << DstOut::OutputDir << endl;
    Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", outputRecoFile);
    se->registerOutputManager(out);

    //-----------------
    // Event processing
    //-----------------
    // Put together the metadata
    time_t now = time(0);
    char hostname[HOST_NAME_MAX];
    char username[LOGIN_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);
    getlogin_r(username, LOGIN_NAME_MAX);
    string getSeeds = "grep 'seed:' " + logFile;

    std::vector<std::pair<std::string, std::string>> metadataInfo;
    std::pair<std::string, std::string> theDate("DATE/TIME", asctime(localtime(&now)));
    std::pair<std::string, std::string> theHostname("HOSTNAME", hostname);
    std::pair<std::string, std::string> theUsername("USERNAME", username);
    std::pair<std::string, std::string> theVersion("SOFTWARE VERSION", exec("echo $OFFLINE_MAIN | awk -F \"/\" '{print $NF}' | tr -d '\n'"));
    std::pair<std::string, std::string> theHash("GIT HASH", exec("git rev-parse --short HEAD | tr -d '\n'"));
    std::pair<std::string, std::string> theGenerator("GENERATOR", generator);
    std::pair<std::string, std::string> theMagnet("MAGNET", turnOnMagnet ? "ON" : "OFF");
    std::pair<std::string, std::string> theSimulation("SIMULATION", fullSim ? "WITH DETECTORS" : "GENERATOR ONLY");
    std::pair<std::string, std::string> theAlignment("ALIGNMENT", idealAlignment ? "IDEAL" : "MISALIGNED");
    std::pair<std::string, std::string> theSeeds("SEEDS", exec(getSeeds.c_str()));
    metadataInfo.push_back(theDate);
    metadataInfo.push_back(theHostname);
    metadataInfo.push_back(theUsername);
    metadataInfo.push_back(theVersion);
    metadataInfo.push_back(theHash);
    metadataInfo.push_back(theGenerator);
    metadataInfo.push_back(theMagnet);
    metadataInfo.push_back(theSimulation);
    metadataInfo.push_back(theAlignment);
    metadataInfo.push_back(theSeeds);
    cout << "Metadata information" << endl;
    for (auto &info : metadataInfo)
        cout << info.first << ": " << info.second << endl;

    MetadataContainer *myMetadata = new MetadataContainer("METADATA");
    myMetadata->Verbosity(1);
    myMetadata->addMetadataStrings(metadataInfo);
    se->registerSubsystem(myMetadata);

    // if we use a negative number of events we go back to the command line here
    if (nEvents < 0)
    {
        return 0;
    }
    // if we run the particle generator and use 0 it'll run forever
    if (nEvents == 0 && !Input::HEPMC && !Input::READHITS)
    {
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
