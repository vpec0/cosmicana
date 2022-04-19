#!/bin/bash

[[ $# < 5 ]] && echo "Give at least 5 arguments" && exit -1

source /cvmfs/dune.opensciencegrid.org/products/dune/setup_dune.sh
setup root v6_22_08d -q "e20:p392:prof"

DEVDIR=/home/pec/dune/cosmics/cosmicana

DATADIR=$1
BATCH=$2
INFILE_PREFIX=$3
OUTDIR=$4
NPARTS=$5
OUTFILE_SUFFIX=""
[ -n "$6" ] && OUTFILE_SUFFIX=$6
STARTPART=0
[ -n "$7" ] && STARTPART=$7

SCRIPT_NAME=$DEVDIR/scripts/stdAna/fitLifeTime_langau_new_batch.C

LOGDIR=$OUTDIR/logs

mkdir -p $OUTDIR
mkdir -p $LOGDIR

TMPINFNAME=$(ls $DATADIR/${INFILE_PREFIX}*part${STARTPART}_anahists.root)
OUTPREF=$OUTDIR/$(basename $TMPINFNAME)
OUTPREF=${OUTPREF%_part*_anahists.root}_$OUTFILE_SUFFIX
LOGFNAME=${LOGDIR}/$(basename $OUTPREF)MPV_fits.log


for part in $(seq $STARTPART $[NPARTS-1]) ; do
    INFILE=$(ls $DATADIR/${INFILE_PREFIX}*part${part}_anahists.root)

    nice /usr/bin/time root -l -b -q "${SCRIPT_NAME}($BATCH, $part, \"$OUTPREF\", \"\", \"MPV\", \"$INFILE\")"
done |& tee $LOGFNAME

# # Merge resulting trees
# TOMERGE=${OUTPREF_TEMPLATE/BATCHNO/$batchno}MPV_fits.root
# TOMERGE=${TOMERGE/GROUPNO/%d}
# MERGEOUT=${TOMERGE/group%d/tree_merged}
# LOGFILE=${MERGEOUT/plots/logs}
# LOGFILE=${LOGFILE/.root/.log}
# echo $TOMERGE $MERGEOUT $LOGFILE
# echo "
# auto tree = new TChain(\"elifetime\");
# for (int i=0;i<$NGROUPS;++i) {
#     tree->Add(Form(\"$TOMERGE\", i));
# }
# auto outf = TFile::Open(\"$MERGEOUT\",\"recreate\");
# tree->CloneTree(-1,\"fast\"); outf->Write(); outf->Close();" |root -l -b &> $LOGFILE
