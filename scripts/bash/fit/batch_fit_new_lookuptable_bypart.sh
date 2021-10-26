#!/bin/bash

data_version=v09_24_01
#range=$(echo 200{1..3}{0..4}000)
range=$(echo 200{1..3}3100)
#range=$(echo 20013100)

MAX_JOBS=6

NPARTS=10
NGROUPS=10
NSUBPARTS=$[NPARTS/NGROUPS]


base_dir=plots/$data_version/lookuptable
outdir=$base_dir
logdir=logs/$data_version/lookuptable

mkdir -p $outdir
mkdir -p $logdir


# COMMAND="nice /usr/bin/time root -l -b -q scripts/stdAna/fitLifeTime_langau_new_batch.C(20003100, 0, \"test_\", \"\", \"MPV\", \"test_anahists.root\")"

# OUTPREF_TEMPLATE=$outdir/lfit_new_batch_BATCHNO_singlerun_byparts_groupGROUPNO_
OUTPREF_TEMPLATE=$outdir/lfit_new_batch_BATCHNO_byparts_groupGROUPNO_

for batchno in $range ; do
    # clear up if output files exist
    for group in $(seq 0 $[NGROUPS-1]) ; do
	# make sure only MAX_JOBS are running
    	while (( ${#pids[@]} >= MAX_JOBS )) ; do
    	    sleep 2
    	    for tmpgrp in "${!pids[@]}" ; do
    		kill -0  "${pids[$tmpgrp]}" 2>/dev/null || unset pids[$tmpgrp]
    	    done
    	done

	# start running a single group processing NSUBPARTS files in the beackground
    	OUTPREF=${OUTPREF_TEMPLATE/BATCHNO/$batchno}
    	OUTPREF=${OUTPREF/GROUPNO/$group}
    	rm -f ${OUTPREF}MPV_fits.root
    	{
    	    for subpart in $(seq 0 $[NSUBPARTS-1]) ; do
    		part=$[group*NSUBPARTS + subpart]
    		#INFILE=$(ls $base_dir/batch_$batchno/*00_singlerun_part${part}_anahists.root)
    		INFILE=$(ls $base_dir/batch_$batchno/*00_part${part}_anahists.root)
    		nice /usr/bin/time root -l -b -q "scripts/stdAna/fitLifeTime_langau_new_batch.C($batchno, $part, \"$OUTPREF\", \"$base_dir\", \"MPV\", \"$INFILE\")"
    	    done
    	} &> ${OUTPREF/plots/logs}MPV.log &
	# recorde the process ID for bookkeeping of running jobs
    	pids[$group]=$!
    done

    # wait for all groups within the batch to finish before moving on to next batch
    for pid in ${pids[*]} ; do
    	wait $pid
    done

    # Merge resulting trees
    TOMERGE=${OUTPREF_TEMPLATE/BATCHNO/$batchno}MPV_fits.root
    TOMERGE=${TOMERGE/GROUPNO/%d}
    MERGEOUT=${TOMERGE/group%d/tree_merged}
    LOGFILE=${MERGEOUT/plots/logs}
    LOGFILE=${LOGFILE/.root/.log}
    echo $TOMERGE $MERGEOUT $LOGFILE
    echo "
auto tree = new TChain(\"elifetime\");
for (int i=0;i<$NGROUPS;++i) {
    tree->Add(Form(\"$TOMERGE\", i));
}
auto outf = TFile::Open(\"$MERGEOUT\",\"recreate\");
tree->CloneTree(-1,\"fast\"); outf->Write(); outf->Close();" |root -l -b &> $LOGFILE

done
