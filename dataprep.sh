#!/bin/bash


#for file; do
#    ./midi2gro "$file" gro/
#    echo
#done


for file; do
    bash test_pipeline.sh "$file"
done




#for file in gro/*; do
#    echo "$file"
#    ./gro2midi "$file" midi/
#    read key
#done
