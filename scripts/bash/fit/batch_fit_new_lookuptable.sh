#!/bin/bash

data_version=v09_24_01
#range=$(echo 200{1..3}{0..4}000)
range=$(echo 200{1..3}3100)
#range=$(echo 20013000)


base_dir=plots/$data_version/lookuptable
outdir=$base_dir
logdir=logs/$data_version/lookuptable

mkdir -p $outdir
mkdir -p $logdir


# COMMAND="nice /usr/bin/time root -l -b -q scripts/stdAna/fitLifeTime_langau_new_batch.C(20003100, 0, \"test_\", \"\", \"MPV\", \"test_anahists.root\")"


for batchno in $range ; do #2000{31..40}00 ; do
    # clear up if output files exist
    INFILE=$(ls $base_dir/batch_$batchno/*${batchno}_anahists.root)
    rm -f $outdir/lfit_new_batch_${batchno}_MPV_fits.root
    nice /usr/bin/time root -l -b -q "scripts/stdAna/fitLifeTime_langau_new_batch.C($batchno, 0, \"$outdir/lfit_new_batch_${batchno}_\", \"$base_dir\", \"MPV\", \"$INFILE\")" &> $logdir/fitLifeTime_langau_new_lfit_batch_${batchno}_MPV.log &
done
