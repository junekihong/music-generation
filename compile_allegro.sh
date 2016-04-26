#!/bin/sh

cd allegro/
make

g++ -g -O5 -DLINUX midi2gro.cpp -o midi2gro allegro.o allegrosmfwr.o allegrord.o allegrowr.o allegrosmfrd.o mfmidi.o strparse.o && \
    g++ -g -O5 -DLINUX gro2midi.cpp -o gro2midi allegro.o allegrosmfwr.o allegrord.o allegrowr.o allegrosmfrd.o mfmidi.o strparse.o

cp midi2gro gro2midi ..
