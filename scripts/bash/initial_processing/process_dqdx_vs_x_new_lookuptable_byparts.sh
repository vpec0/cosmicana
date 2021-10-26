#!/bin/bash


data_version=v09_24_01
# range=$(echo 200{1..3}{0..4}000)
# noRuns=10
range=$(echo 200{1..3}3100)
noRuns=100

data_type=""

NPARTS=100
RUNS_PER_PART=1


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

    START_RUN=0
    for part in $(seq 0 $[NPARTS-1]) ; do
	# make sure only MAX_JOBS are running
	while (( ${#cur_jobs[@]} >= MAX_JOBS )) ; do
	    sleep 2
	    for pid in "${!cur_jobs[@]}" ; do
		kill -0  "$pid" 2>/dev/null || unset cur_jobs[$pid]
	    done
	done

	OUTPREFIX=$newdir/new_batch_${batch}_singlerun_part${part}_
	LOGFILE=$newlogdir/process_dqdx_vs_x_new_batch_${batch}_singlerun_part${part}.log
	nice /usr/bin/time root -l -b -q "${SCRIPT_NAME}(\"\",\"${OUTPREFIX}\", ${batch}, $RUNS_PER_PART, $START_RUN, \"$data_version\", \"\", \"$TOPDIR\")" &> $LOGFILE &
	cur_jobs[$!]=1
	START_RUN=$[ START_RUN + RUNS_PER_PART ]
    done
done
