#!/bin/bash

##
# run from the log directory containing batch_ subdirectories

total_tracks=0
for batch in 2000{21..30}00 ; do
    for part in {0..9} ; do
	tracks=$(tail batch_$batch/process_dqdx_vs_x_new_batch_*_part$part.log | grep "Tracks passing"|cut -d" " -f4) ; total_tracks=$[total_tracks + tracks]
    done
done
echo $total_tracks

total_events=0
for batch in 2000{21..30}00 ; do
    for part in {0..9} ; do
	events=$(tail batch_$batch/process_dqdx_vs_x_new_batch_*_part$part.log | grep "Events passing"|cut -d" " -f4) ; total_events=$[total_events + events]
    done
done ;
echo $total_events
