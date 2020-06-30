#!/bin/bash

data_version=v08_34_00
noRuns=10

CUT=thetayz_track_cut

SCRIPT_NAME=scripts/stdAna/process_dqdx_vs_x_new_thetayz.C
OUTPUTDIR=plots/${data_version}/elifetime/$CUT
LOGDIR=logs/$data_version/elifetime/$CUT

for batch in 2000{21..30}00 ; do
    newdir=$OUTPUTDIR/batch_$batch
    newlogdir=$LOGDIR/batch_$batch
    mkdir -p $newdir
    mkdir -p $newlogdir
    pids=()
    for part in {0..9} ; do
	OUTPREFIX=$newdir/new_batch_${batch}_part${part}_
	LOGFILE=$newlogdir/process_dqdx_vs_x_new_batch_${batch}_part${part}.log
	START_RUN=$[ part * noRuns ]
	nice /usr/bin/time root -l -b -q "${SCRIPT_NAME}(\"\",\"${OUTPREFIX}\", ${batch}, $noRuns, $START_RUN, \"$data_version\")" &> $LOGFILE &
	pids[$part]=$!
    done

    # wait for all parts to finish before moving on to next batch
    for pid in ${pids[*]} ; do
	wait $pid
    done
done
