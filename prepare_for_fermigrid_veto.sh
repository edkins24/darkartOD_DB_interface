#!/bin/bash

runnum=$1
phase=$2
fhiclFile=$3
subrun=""
if [ $# -eq 4 ]; then
    subrun=$4
fi

fullRunNum=`printf %06d ${runnum}`
ds50app="/ds50/app/user/${USER}"
ds50data="/ds50/data/user/${USER}"
subdir="DarkArtTests"

# Make the condor-tmp directory
tmpDir=${ds50data}/condor-tmp/ODRun${fullRunNum}
mkdir -p $tmpDir
chmod g+w $tmpDir
templateScript=${ds50data}/run_od_TEMPLATE_dcache.sh

# Generate the run script
runScript=${ds50app}/condor-exec/run_od_${runnum}
#Make sure the script has a unique number at the end to avoid conflicting with other scripts
scriptIdx=0
while [ -e ${runScript}.sh ]; do
    scriptIdx=$[$scriptIdx+1]
    if [ $scriptIdx -eq 1 ]; then
        runScript="${runScript}_1"
    else
        runScript="${runScript%_*}_${scriptIdx}"
    fi
done
runScript="${runScript}.sh"

sed -e s/TEMPLATE/${runnum}/g $templateScript > $runScript
sed -i s/PHASE/${phase}/g $runScript
chmod g+x $runScript

# Make the output directory
#outdir="${ds50data}/${subdir}/VetoOutput_Run${fullRunNum}"
outdir="/ds50/data/test_processing/lsv/darkartod/v1_02/VetoOutput_Run${fullRunNum}"
mkdir -p $outdir
chmod g+w $outdir

# Figure out which SPE file to use
speFile=""
if [ $runnum -lt 34 ]; then
    speFile="spe45-37-22Jul2013.txt"
elif [ $runnum -lt 141 ]; then
    speFile="spe45-37-1Aug2013.txt"
elif [ $runnum -lt 327 ]; then
    speFile="spe_WTandLSV-29Oct2013.txt"
elif [ $runnum -lt 420 ]; then
    speFile="spe_LSV327_WT83-Nov2013.txt"
else
    speFile="spe420-Jan2014.txt"
fi
speFile="spe_OD/${speFile}"

# Figure out which prompt offset to use
promptOffset=0
if [ $runnum -lt 611 ]; then
    promptOffset=-726;
elif [ $runnum -lt 943 ]; then
    promptOffset=0 # these runs have no prompt signal
elif [ $runnum -lt 12348 ]; then
    promptOffset=-6564
else
    promptOffset=-6580
#    promptOffset=3930
fi

# Figure out which channels to disable
disabledChans=""
if [ $runnum -lt 9496 ]; then
    disabledChans="'10,33,27,198,201,207,213,215,21,224,225,231'"
else
    disabledChans="'10,27,198,201,224,225,231'"
fi

# Generate the jobsub command for the user to execute
command="jobsub_submit.py -G darkside --resource-provides=usage_model=DEDICATED,OPPORTUNISTIC --role=Analysis --OS=SL6 -Q -N 1 -dMYOUTPUT \"$outdir\" file://$runScript $fhiclFile $speFile $promptOffset $disabledChans $subrun"

echo $command

