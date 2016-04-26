#!/bin/bash


for file; do
    ./midi2gro "$file" gro/
    echo
done
