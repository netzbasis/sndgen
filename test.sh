#!/bin/ksh -e

CMD=obj/sndgen

function sndtest {
	$CMD $1 &
	PID=$!
	sleep 2 && kill $PID
}

echo "Starting tests"
sndtest ""
sndtest "-l"
sndtest "-lr" #-l should be ignored
sndtest "-s 150"
sndtest "-s 250 -l"
sndtest "-s 350 -r"
