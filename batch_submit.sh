#!/bin/bash

#runnum=11770
#runnum=8000

RUNLIST=$1
fhiclFile="darkartOD_TEMPLATE.fcl"

# Read runlist

for runnum in `cat $RUNLIST`; do
    echo "Submitting run ${runnum}"
    # Get the number of subruns
    fullRunNum=`printf %06d ${runnum}`
    runstart="${fullRunNum%????}"
    NPARTS=0

    basedir="gsiftp://fndca4a.fnal.gov:2811/raw/"
    phase=none
    rundir=""
    if [ $runnum -lt 1000 ]; then
	rundir="${basedir}commissioning/lsv/ODRun${fullRunNum%??}xx/"
	echo "Looking for run in $rundir"
	phase="commissioning"
	NPARTS=`uberftp -ls ${rundir}/ODRun${fullRunNum}_*.dat | wc -l`
	echo "Found $NPARTS parts"
    else 
	rundir="/lsv/ODRun${runstart}xxxx/ODRun${fullRunNum%?}x/"

	echo "Checking calibration directory"
	NPARTS_calib=`uberftp -ls ${basedir}calibration${rundir}ODRun${fullRunNum}_*.dat | wc -l`
	echo "Checking commissioning directory"
	NPARTS_comm=`uberftp -ls  ${basedir}commissioning${rundir}ODRun${fullRunNum}_*.dat | wc -l`
	
	if [ $NPARTS_calib -gt 0 ]; then
	    NPARTS=$NPARTS_calib
	    phase="calibration"
	elif [ $NPARTS_comm -gt 0 ]; then
	    NPARTS=$NPARTS_comm
	    phase="commissioning"
	else
	    echo "ERROR: COULD NOT FIND RUN NUMBER $runnum"
	fi
    fi
    
    # Submit each subrun
    for i in `seq 1 $NPARTS`; do
	echo "subrun $i out of $NPARTS"
	prepare_cmd="prepare_for_fermigrid_veto.sh $runnum $phase $fhiclFile $i"
	jobsub_cmd=$(eval $prepare_cmd)
	
	echo "submitting run $runnum part $i out of $NPARTS....."
	echo -e "\t$jobsub_cmd"
	jobsub_cmd_output=$(eval $jobsub_cmd)
	jobid=`echo $jobsub_cmd_output | sed 's/.*job: //' | sed 's/ Use.*//'`
	date=$(eval date)
	script=`echo $jobsub_cmd | sed 's@.*exec/@@' | sed 's@dark.*@@'`
	echo "$date :   $jobid   $runnum   $i   $script " >> submitLogs.txt
	echo ".....done"
    done
done