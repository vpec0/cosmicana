#!/bin/bash

#DATAVERSION=v08_34_00
DATAVERSION=v08_50_00

# BATCHNO=20002100
# i=0
# for PART in {0..9} ; do
#     INDIR="plots/${DATAVERSION}/elifetime"
#     OUTPUTDIR=$INDIR/batch_${BATCHNO}/part$PART
#     LOGDIR=logs/$DATAVERSION/elifetime/batch_${BATCHNO}
#     LOGNAME=$LOGDIR/part$PART.log
#     install -d $OUTPUTDIR && install -d $LOGDIR
#     [ $? == 0 ] && root -l -b -q "scripts/stdAna/fitLifeTime_langau.C( \"${INDIR}/batch_${BATCHNO}_part${PART}_anahists.root\" , \"$OUTPUTDIR/batch_${BATCHNO}_part${PART}_\")" &> $LOGNAME &
#     pids[$PART]=$!
# done

# i=0
# for pid in ${pids[*]} ; do
#     wait $pid
#     status[$i]=$?
#     i=$[i+1]
# done

# sleep 10

# for PART in {0..9} ; do
#     OUTDIR=plots/${DATAVERSION}/elifetime/batch_${BATCHNO}
#     tar -C$OUTDIR -cf $OUTDIR/part$PART.tar --remove-files part$PART
# done

pids=()
i=0
SUFFIX=_part0
LOGDIR=logs/$DATAVERSION/elifetime
install -d $LOGDIR
for BATCHNO in 2000{31..40}00 ; do
    INDIR="plots/${DATAVERSION}/elifetime"
    OUTPUTDIR=$INDIR/batch_${BATCHNO}
    LOGNAME=$LOGDIR/batch_${BATCHNO}$SUFFIX.log
    install -d $OUTPUTDIR
    [ $? == 0 ] && root -l -b -q "scripts/stdAna/fitLifeTime_langau.C( \"${INDIR}/batch_${BATCHNO}${SUFFIX}_anahists.root\" , \"$OUTPUTDIR/batch_${BATCHNO}${SUFFIX}_\")" &> $LOGNAME &
    pids[$i]=$!
    i=$[i+1]
done

i=0
for pid in ${pids[*]} ; do
    wait $pid
    status[$i]=$?
    i=$[i+1]
done

#sleep 10

for BATCHNO in 2000{31..40}00 ; do
    OUTDIR=plots/${DATAVERSION}/elifetime/
    INDIR=$OUTDIR/batch_${BATCHNO}
    tar -C $OUTDIR -cf $INDIR.tar --remove-files batch_${BATCHNO}
done
