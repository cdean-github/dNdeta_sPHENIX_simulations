import sys, os
from os import environ
import argparse
import math

parser = argparse.ArgumentParser(description='dN/deta Simulation Job Creator')
parser.add_argument('-i', '--generator', default="HIJING", help='Generator: PYTHIA8, HIJING, SIMPLE')
parser.add_argument('-e', '--nEventsPerJob', default=200, type=int, help='Number of events to generate each job')
parser.add_argument('-n', '--nTotEvents', default=-1, type=int, help='Total number of events to generate')
parser.add_argument('--generatorOnly', help='Disable the detector simulation', action="store_true")
parser.add_argument('--magOn', help='Enable the magnetic field', action="store_true")
parser.add_argument('--misalign', help='Misalign the detectors', action="store_true")
parser.add_argument('--startNumber', default=0, type=int, help='Set first file number')
parser.add_argument('--revisionNumber', default=0, type=int, help='Set file revision number')

args = parser.parse_args()

inputType = args.generator.upper()

types = {'PYTHIA8', 'HIJING', 'SIMPLE'}
if inputType not in types:
  print("The generator, {}, was not known. Use --help to see available generators".format(args.inputType))
  sys.exit()

myShell = str(environ['SHELL'])
goodShells = ['/bin/bash']
if myShell not in goodShells:
    print("Your shell {} was not recognised".format(myShell))
    sys.exit()

softwareVersion = 'ana.376'

simType = 'generatorOnly' if args.generatorOnly else 'fullSim'
simTypeBool = 'false'  if args.generatorOnly else 'true'

magnet = 'magOn' if args.magOn else 'magOff'
magnetBool = 'true'  if args.magOn else 'false'

alignment = 'detectorMisaligned' if args.misalign else 'detectorAligned'
alignmentBool = 'false'  if args.misalign else 'true'

outputDir = '/sphenix/tg/tg01/bulk/dNdeta_INTT_run2023/data/simulation/{0}/{1}/{2}/{3}/{4}'.format(softwareVersion, inputType, simType, magnet, alignment)
outputFile = 'dNdeta-sim-{0}-{1:03d}'.format(inputType, args.revisionNumber, args.startNumber)

nJob = math.ceil(args.nTotEvents/args.nEventsPerJob)


memory = 6 if inputType == "HIJING" else 4

def makeCondorJob():
    print("Creating condor submission script for {} simulations".format(inputType))
    os.makedirs("{}".format(outputDir), exist_ok=True)
    myOutputPath = os.getcwd()
    condorDir = "{}/condorJob".format(myOutputPath)
    os.makedirs("{}/log".format(condorDir), exist_ok=True)
    condorFileName = "{0}/my{1}.job".format(condorDir, inputType)
    condorFile = open("{}".format(condorFileName), "w")
    condorFile.write("Universe           = vanilla\n")
    condorFile.write("initialDir         = {}\n".format(myOutputPath))
    condorFile.write("Executable         = $(initialDir)/runSimulation.sh\n")
    condorFile.write("PeriodicHold       = (NumJobStarts>=1 && JobStatus == 1)\n")
    condorFile.write("request_memory     = {}GB\n".format(memory))
    condorFile.write("Priority           = 20\n")
    condorFile.write("job_lease_duration = 3600\n")
    condorFile.write("condorDir          = $(initialDir)/condorJob\n")
    condorFile.write("fileNumber          = $INT(Process) + {} \n".format(args.startNumber))
    condorOutputInfo = "$(condorDir)/log/condor-{0}-{1:03d}-$INT(fileNumber,%05d)".format(inputType, args.revisionNumber)
    condorFile.write("Output             = {0}.out\n".format(condorOutputInfo))
    condorFile.write("Error              = {0}.err\n".format(condorOutputInfo))
    condorFile.write("Log                = {0}.log\n".format(condorOutputInfo))
    condorFile.write("Arguments          = \"{0} {1} {2}-$INT(fileNumber,%05d).root {3} {4} {5} {6} {7}\"\n".format(args.nEventsPerJob, outputDir, outputFile, inputType, simTypeBool, magnetBool, alignmentBool, softwareVersion))
    condorFile.write("Queue {}\n".format(nJob))
    print("Submission setup complete!")
    print("This setup will submit {} subjobs".format(nJob))
    print("You can submit your job with the script:\n{}".format(condorFileName))

makeCondorJob()
