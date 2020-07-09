#!/bin/bash

data_version=new_production
data_type=no_lt_diffusion_inf_e_lifetime_100_jobs
noRuns=10

SCRIPT_NAME=scripts/stdAna/process_dqdx_vs_x_new.C
OUTPUTDIR=plots/v08_50_00/$data_type/lifetime
LOGDIR=logs/v08_50_00/$data_type

for batch in 90001000 ; do
    newdir=$OUTPUTDIR/batch_$batch
    newlogdir=$LOGDIR/batch_$batch
    install -d $newdir
    install -d $newlogdir
    pids=()
    for part in {0..9} ; do
	LOGFILE=$newlogdir/process_dqdx_vs_x_new_batch_${batch}_part${part}.log
	START_RUN=$[ part * noRuns ]
	nice /usr/bin/time root -l -b -q "${SCRIPT_NAME}(\"\",\"$newdir/new_batch_${batch}_part${part}_\", ${batch}, $noRuns, $START_RUN, \"$data_version/$data_type\", \"kumar\")" &> $LOGFILE &
	pids[$part]=$!
    done

    # wait for all parts to finish before moving on to next batch
    # for pid in ${pids[*]} ; do
    # 	wait $pid
    # done
done
