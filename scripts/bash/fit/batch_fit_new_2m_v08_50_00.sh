#!/bin/bash

CUT=2m_track_cut

data_version=v08_50_00

base_dir=plots/$data_version/no_lt_diffusion_100_jobs/elifetime/$CUT
outdir=$base_dir
logdir=logs/$data_version/no_lt_diffusion_100_jobs/elifetime/$CUT

mkdir -p $outdir
mkdir -p $logdir

for batchno in 60001000 ; do #2000{31..40}00 ; do
    {
	for part in {0..9} ; do
	    echo "Part $part:"
	    nice /usr/bin/time root -l -b -q "scripts/stdAna/fitLifeTime_langau_new_batch.C($batchno, $part, \"$outdir/new_batch_${batchno}_\", \"$base_dir\")"
	done
    } &> $logdir/fitLifeTime_langau_new_batch_${batchno}.log &
done
