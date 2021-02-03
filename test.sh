#!/bin/ksh -e

CMD=obj/sndgen

function sndtest {
	$CMD $1 &
	PID=$!
	sleep 2 && kill $PID
}

echo "Starting tests"
sndtest ""
sndtest "-rl" #ignore r
sndtest "-lr" #ignore l
sndtest "-c -s 150" #ignore c
sndtest "-s 250 -l"
sndtest "-s 350 -r"
sndtest "-c"
sndtest "-s 1000 -c" #ignore s
sndtest "-s 1000 -c -rl" #ignore sr
sndtest "-s 1000 -c -lr" #ignore sl
$CMD -s500 -r -o > obj/s16nat-44.1k-2ch.pcm
$CMD -c -o > obj/chirp.pcm
