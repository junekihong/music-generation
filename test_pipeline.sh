#!/bin/sh


python encode.py "$1"
python decode.py "encoded/$(basename "$1")".encoded

./gro2midi "decoded/$(basename "$1")".decoded midi/
#timidity "midi/$(basename "$1")".mid




#&& ./gro2midi "encoded/$(basename "$1")".processed midi/ && timidity "midi/$(basename "$1")".mid



#./midi2gro 
#pathname=a/b/c
#echo $(basename $pathname)
#echo $(basename $(dirname $pathname))
#echo $(basename $(dirname $(dirname $pathname)))
