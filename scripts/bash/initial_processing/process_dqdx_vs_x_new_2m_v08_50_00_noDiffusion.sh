#!/bin/bash

data_version=v08_50_00
sample_type=(no_l_diffusion_100_jobs no_t_diffusion_100_jobs no_lt_diffusion_100_jobs)
batchno=(40001000 50001000 60001000)
noRuns=10

CUT=2m_track_cut

SCRIPT_NAME=scripts/stdAna/process_dqdx_vs_x_new.C

for i in {0..2} ; do
    OUTPUTDIR=plots/${data_version}/${sample_type[i]}/elifetime/$CUT
    LOGDIR=logs/$data_version/${sample_type[i]}/elifetime/$CUT
    batch=${batchno[i]}
    newdir=$OUTPUTDIR/batch_$batch
    newlogdir=$LOGDIR/batch_$batch
    install -d $newdir
    install -d $newlogdir
    pids=()
    for part in {0..9} ; do
	OUTPREFIX=$newdir/new_batch_${batch}_part${part}_
	LOGFILE=$newlogdir/process_dqdx_vs_x_new_batch_${batch}_part${part}.log
	START_RUN=$[ part * noRuns ]
	nice /usr/bin/time root -l -b -q "${SCRIPT_NAME}(\"\",\"$OUTPREFIX\", ${batch}, $noRuns, $START_RUN, \"new_production/${sample_type[i]}\", \"kumar\")" &> $LOGFILE &
	pids[$part]=$!
    done

    # wait for all parts to finish before moving on to next batch
    for pid in ${pids[*]} ; do
    	wait $pid
    done
done
