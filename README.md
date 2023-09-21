# dNdeta sPHENIX simulations

## Overview

This is a simulation framework for the run 2023 dN/deta analysis using INTT data. This package is designed to run standalone or in batch mode on condor which is its main use. It's capable of generating events with a particle gun, pythia8 or HiJing and the user can turn the detector on or off and enable or disable the magnetic field. Work will be done later to misalign the detector as well.

Note that before you can use this package, there is a set of classes for writing metadata to the DSTs. These are custom and need to be compiled before you can use the Fun4All macro (or you can comment out the package's lines in the macro but you won't have metadata fixed to the DST which is risky).

All data should be written to 
```
/sphenix/tg/tg01/bulk/dNdeta_INTT_run2023/data/simulation
```
for analysis preservation but you can change the output directory in `macro/makeCondorJobs.py` to point somewhere else for local testing.

The Fun4All simulation can be launched by itself. It will create a subdiretory to store the output DSTs and there is an `inPreparation` folder that the files are sotred in while they are being generated. The macro will automatically figure out which folder revision to produce, based on the folder and files previous existence, and will move DSTs out of the `inPreparation` folder automatically when they've been writted. Basically, any DST that is not in the `inPreparation` folder is ready for analysis.

There is a shell script for running the macro, `runSimulation.sh`, as well and I would recommend using this for local testing as it will set the correct environment and write some metadata files and do and md5sum check. Note that a user will likely need to update this script so that the environment can find their copy of the metadata classes. This should be a one line change of
```bash
export MYINSTALL=/sphenix/u/cdean/sPHENIX/install
```
to wherever your install area is.

## Building the metadata classes

Two modules should be built before running the scripts, `metadata` and `metadatacontainer`. I have some handy bash scripts for building these
```bash
alias buildThisProject="mkdir build && cd build && ../autogen.sh --prefix=$MYINSTALL && make && make install && cd ../"
```
Navigate first to `metadata` then `metadatacontainer` and execute the above command. Remember to have your work environment set up to allow local software builds with something like what I keep in my `.bash_profile`
```bash
source /opt/sphenix/core/bin/sphenix_setup.sh -n new
export SPHENIX=/sphenix/u/cdean/sPHENIX
export MYINSTALL=$SPHENIX/install
export LD_LIBRARY_PATH=$MYINSTALL/lib:$LD_LIBRARY_PATH
export ROOT_INCLUDE_PATH=$MYINSTALL/include:$ROOT_INCLUDE_PATH
source /opt/sphenix/core/bin/setup_local.sh $MYINSTALL
```

## Running the simulation

You can run the simulation from the `macro` folder with the Fun4All script as is. This is useful to check that it actually runs but is not recommended for real testing or large scale production. There's a python script to set large scale production and I will detail how to use this as well. 

The best way to locally test if the code will work is with the given shell script, make sure the permissions are correct first. If not, set them with
```bash
chmod u+x runSimulation.sh
```
You can generate events using this shell script with, for example, 
```bash
./runSimulation.sh 1 ./ G4sPHENIX-000-00000.root PYTHIA8 true false true new
```
which will generate one event (the 1), in the same folder as wher the Fun4All macro is located (the ./) with an output DST name of G4sPHENIX-000-00000.root. Pythia8 will be used as the generator and the detector simulation (trackers, beampipe and MBD) will be turned on (this is the first true). The magnet will be disabled (the false argument) and the alignment will be ideal (the second true) but there is not coded option yet to misalign the detector. The software stack used will be `new` as stated in the final argument.

To batch produce events, the python script `makeCondorJobs.py` should be used. To see how it works, run it with the `--help` option
```bash
python makeCondorJobs.py --help
usage: makeCondorJobs.py [-h] [-i GENERATOR] [-e NEVENTSPERJOB] [-n NTOTEVENTS] [--generatorOnly] [--magOn] [--misalign] [--startNumber STARTNUMBER] [--revisionNumber REVISIONNUMBER]

dN/deta Simulation Job Creator

options:
  -h, --help            show this help message and exit
  -i GENERATOR, --generator GENERATOR
                        Generator: PYTHIA8, HIJING, SIMPLE
  -e NEVENTSPERJOB, --nEventsPerJob NEVENTSPERJOB
                        Number of events to generate each job
  -n NTOTEVENTS, --nTotEvents NTOTEVENTS
                        Total number of events to generate
  --generatorOnly       Disable the detector simulation
  --magOn               Enable the magnetic field
  --misalign            Misalign the detectors
  --startNumber STARTNUMBER
                        Set first file number
  --revisionNumber REVISIONNUMBER
                        Set file revision number
```

You must specify the number of events to generate with `-n` but other than that it will default to generate HIJING events with the magnet off with full simulation and perfect alignment. After you execute the script, you will have a new folder in that directory called `condorJob` with a submission script called myGENERATOR.job where GENERATOR is replaced by the one you chose. You can submit the jobs with
```bash
condor_submit myGENERATOR.job
```
and the output files will be located where the path you specified said to go, with subfolders detailing the simulation steps. An associated metadata file will also be written. Do not worry about the temporary log file in the macro directory. This will be deleted when the job has finished processing and is used to read the metadata values that go into the associated text file.

NOTE: You can dry run the condor submission script to make sure (almost) everything is good. You can't check that the Fun4All macro executes properly but this is why you should run the shell script first. Dry run condor with
```bash
condor_submit myGENERATOR.job -dry-run log.txt
```
You can alter `log.txt` to be whatever you like.
