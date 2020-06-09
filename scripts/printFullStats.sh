#!/bin/bash

DATAVERSION=v08_34_00
[ -n "$1" ] && DATAVERSION=$1

LOGDIR=logs/$DATAVERSION

total=0
events=0
tracks=0

echo -e "Batch/Part\t""Total\t""Events\t""Tracks\t"
for batch in 20002100 ; do
    for part in {0..9} ; do
	LOGFILE=$LOGDIR/process_dqdx_vs_x_batch_${batch}_part${part}.log
	totalentries=$(tail $LOGFILE | grep "Processed" | cut -d" " -f2)
	eventspassing=$(tail $LOGFILE | grep "Events passing" | cut -d" " -f4)
	trackspassing=$(tail $LOGFILE | grep "Tracks passing" | cut -d" " -f4)
	echo -e $batch/$part"\t"$totalentries"\t"$eventspassing"\t"$trackspassing
	total=$[total + totalentries]
	events=$[events + eventspassing]
	tracks=$[tracks + trackspassing]
    done
done


echo -e "Batch\t\t""Total\t""Events\t""Tracks\t"
echo -e "20002100\t"$total"\t"$events"\t"$tracks
for batch in 2000{22..30}00 ; do
    LOGFILE=$LOGDIR/process_dqdx_vs_x_batch_${batch}_part0.log
    totalentries=$(tail $LOGFILE | grep "Processed" | cut -d" " -f2)
    eventspassing=$(tail $LOGFILE | grep "Events passing" | cut -d" " -f4)
    trackspassing=$(tail $LOGFILE | grep "Tracks passing" | cut -d" " -f4)
    echo -e $batch"\t"$totalentries"\t"$eventspassing"\t"$trackspassing

    total=$[total + totalentries]
    events=$[events + eventspassing]
    tracks=$[tracks + trackspassing]
done
echo -e "Total\t\t"$total"\t"$events"\t"$tracks
