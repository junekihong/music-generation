Allegro is an in-memory data structure and a text-based language
for simple score representation.

Allegro is similar to MIDI, but extends it with, among other things,
arbitrary attribute/value pairs, per-note parameter updates, and
no limit on channels.

Allegro is also designed for use with the high-level language Serpent
and the real-time system Aura. See allegro.htm for full details.

This code provides:
    classes for data structure creation and manipulation
    parser for Allegro text files into data structure
    writer for data structure to Allegro text files
    parser for Standard MIDI files into data structure
    (simple) player for data structure to PortMidi API
    writer for Standard MIDI files
Not provided (yet):
    PortMidi implementation for Linux

Current status:

The allegro.* files define the data structure.
 
The code is not optimized.

Also, this code could probably be improved by using templates for 
dynamic arrays (as much as I don't like templates).

The Makefile builds a little test program (see test.c)


    -Roger Dannenberg, 13 Nov 02


