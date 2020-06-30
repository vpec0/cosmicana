#!/bin/bash

data_version=new_production_diffusion_2
noRuns=10

CUT=2m_track_cut

SCRIPT_NAME=scripts/stdAna/process_dqdx_vs_x_new.C
OUTPUTDIR=plots/${data_version}/elifetime/$CUT
LOGDIR=logs/$data_version/elifetime/$CUT

for batch in 20005100 ; do
    newdir=$OUTPUTDIR/batch_$batch
    newlogdir=$LOGDIR/batch_$batch
    mkdir -p $newdir
    mkdir -p $newlogdir
    pids=()
    for part in {0..9} ; do
	OUTPREFIX=$newdir/new_batch_${batch}_part${part}_
	LOGFILE=$newlogdir/process_dqdx_vs_x_new_batch_${batch}_part${part}.log
	START_RUN=$[ part * noRuns ]
	root -l -b -q "${SCRIPT_NAME}(\"\",\"${OUTPREFIX}\", ${batch}, $noRuns, $START_RUN, \"$data_version\", \"kumar\")" &> $LOGFILE &
	pids[$part]=$!
    done

    # wait for all parts to finish before moving on to next batch
    # for pid in ${pids[*]} ; do
    # 	wait $pid
    # done
done
