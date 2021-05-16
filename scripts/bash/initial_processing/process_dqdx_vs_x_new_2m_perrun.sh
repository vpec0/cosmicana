#!/bin/bash

###
# Takes optional arguments
# 1. data version
# 2... range of batches
###


data_version=v08_50_00
[[ $# > 0 ]] && data_version=$1

range=$(echo 2000{31..40}00)
[[ $# > 1 ]] && range=${@:2}

#echo $data_version
#echo $range

data_type=""
noRuns=1

CUT=2m_track_cut

SCRIPT_NAME=scripts/stdAna/process_dqdx_vs_x_new.C
OUTPUTDIR=plots/$data_version/elifetime/$CUT/byrun
LOGDIR=logs/$data_version/elifetime/$CUT/byrun

MAX_JOBS=10


for batch in $range ; do
    newdir=$OUTPUTDIR/batch_$batch
    newlogdir=$LOGDIR/batch_$batch
    install -d $newdir
    install -d $newlogdir
    declare -A cur_jobs=()
    for part in {0..99} ; do
	# make sure only MAX_JOBS are running
	while (( ${#cur_jobs[@]} >= MAX_JOBS )) ; do
	    sleep 2
	    for pid in "${!cur_jobs[@]}" ; do
		kill -0  "$pid" 2>/dev/null || unset cur_jobs[$pid]
	    done
	done

	OUTPREFIX=$newdir/new_batch_${batch}_part${part}_
	LOGFILE=$newlogdir/process_dqdx_vs_x_new_batch_${batch}_part${part}.log
	START_RUN=$[ part * noRuns ]
	nice /usr/bin/time root -l -b -q "${SCRIPT_NAME}(\"\",\"${OUTPREFIX}\", ${batch}, $noRuns, $START_RUN, \"$data_version/$data_type\", \"\")" &> $LOGFILE &
	cur_jobs[$!]=1
    done
done
