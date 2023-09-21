#!/bin/bash

source /opt/sphenix/core/bin/sphenix_setup.sh -n $8

export MYINSTALL=/sphenix/u/cdean/sPHENIX/install
export LD_LIBRARY_PATH=$MYINSTALL/lib:$LD_LIBRARY_PATH
export ROOT_INCLUDE_PATH=$MYINSTALL/include:$ROOT_INCLUDE_PATH

source /opt/sphenix/core/bin/setup_local.sh $MYINSTALL

baseFileName=`echo ${3} | cut -d "." -f 1`
metaDataFile=${2}/${baseFileName}.txt
tmpLogFile=${baseFileName}.out

d=`date +%Y/%m/%d`
t=`date +%H:%M`

gitHash=`git rev-parse --short HEAD`

# Print production details to screen and to metadata file simultaneously
cat << EOF | tee ${metaDataFile}
====== Your production details ======
Production started: ${d} ${t}
Production Host: ${HOSTNAME}
Folder hash: ${gitHash}
Software version: $9
Output file: $3
Output dir: $2
Number of events: $1
Generator: $4
fullSim: $5
turnOnMagnet: $6
idealAlignment: $7
=====================================
EOF

# Run Fun4all. Send output to stdout but also capture to temporary local file
echo running root.exe -q -b Fun4All_dNdeta_simulator.C\($1,\"$2\",\"$3\",\"$4\",$5,$6,$7,\"${tmpLogFile}\"\)
root.exe -q -b Fun4All_dNdeta_simulator.C\($1,\"$2\",\"$3\",\"$4\",$5,$6,$7,\"${tmpLogFile}\"\) | tee ${tmpLogFile}

finalPath=`grep 'Your final output path is' ${tmpLogFile} | awk '{print $NF}'`

rc_dst=$?
echo " rc for dst: $rc_dst"

# Do some basic error handling here: is this failed we need to abort! Continuing might cause broken files on all levels.
if [ ".$rc_dst" != ".0" ]
then
  echo " DST production failed. EXIT here, no file copy will be initiated!"
  ls -lhrt $finalPath
  exit $rc_dst
fi

# Scan stdout of Fun4all for random number seeds and add to metadata file
echo production script finished, writing metadata
echo "" >> ${metaDataFile}
echo Seeds: >> ${metaDataFile}
grep 'seed:' ${tmpLogFile} >> ${metaDataFile}
rm ${tmpLogFile}

echo "" >> ${metaDataFile}
echo md5sum: >> ${metaDataFile}
md5sum ${finalPath}/${3} | awk '{print $1}' >> ${metaDataFile}

echo "DST has been created"
mv ${metaDataFile} ${finalPath}
echo "script done"
