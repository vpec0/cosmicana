#!/bin/bash


data_version=v09_24_01
# range=$(echo 200{1..3}{0..4}000)
# noRuns=10
range=$(echo 200{1..3}3100)
noRuns=100

data_type=""


TOPDIR=data2
SCRIPT_NAME=scripts/stdAna/process_dqdx_vs_x_new.C
OUTPUTDIR=plots/$data_version/lookuptable
LOGDIR=logs/$data_version/lookuptable

MAX_JOBS=10

declare -A cur_jobs=()

for batch in $range ; do
    newdir=$OUTPUTDIR/batch_$batch
    newlogdir=$LOGDIR/batch_$batch
    install -d $newdir
    install -d $newlogdir
    # make sure only MAX_JOBS are running
    while (( ${#cur_jobs[@]} >= MAX_JOBS )) ; do
	sleep 2
	for pid in "${!cur_jobs[@]}" ; do
	    kill -0  "$pid" 2>/dev/null || unset cur_jobs[$pid]
	done
    done

    OUTPREFIX=$newdir/new_batch_${batch}_
    LOGFILE=$newlogdir/process_dqdx_vs_x_new_batch_${batch}.log
    START_RUN=0
    nice /usr/bin/time root -l -b -q "${SCRIPT_NAME}(\"\",\"${OUTPREFIX}\", ${batch}, $noRuns, $START_RUN, \"$data_version\", \"\", \"$TOPDIR\")" &> $LOGFILE &
    cur_jobs[$!]=1
done
