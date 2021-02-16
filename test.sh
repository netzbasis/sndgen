#!/bin/ksh -e

CMD=obj/sndgen

function sndtest {
	[ "x$2" == "x" ] && TIMO=2 || TIMO=$2
	$CMD $1 &
	PID=$!
	sleep $TIMO && kill $PID
}

echo "Starting tests"
echo "White noise -> left -> right"

sndtest ""
sndtest "-rl" #ignore r
sndtest "-lr" #ignore l

echo "Sine -> left -> right"

sndtest "-c -s 150" #ignore c
sndtest "-s 250 -l"
sndtest "-s 350 -r"

echo "Chirp -> left -> right"

sndtest "-c"
sndtest "-s 1000 -c" #ignore s
sndtest "-s 1000 -c -rl" #ignore sr
sndtest "-s 1000 -c -lr" #ignore sl

echo "White noise delay -> left -> right"

sndtest "-d 2" 6
sndtest "-d 1 -l" 6
sndtest "-d 2 -r" 6

echo "Buffer dump to ./obj/"

$CMD -s500 -r -o > obj/s16nat-44.1k-2ch.pcm
$CMD -c -o > obj/chirp.pcm
