
#include "stdlib.h"
#include "stdio.h"
#include "assert.h"
#include "allegro.h"
#include "mfmidi.h"
#include "mfallegro.h"
#ifndef LINUX
#include "portmidi.h"
// because portmidi for linux isn't ready yet...
#include "seq2midi.h"
#endif
#include "allegrowr.h"
#include "string.h"
#include "strparse.h"
#include "allegrord.h"
#include "writemidi3.h"

#ifndef LINUX
#include "crtdbg.h" // for memory allocation debugging
#endif

void midi_fail(char *msg)
{
    printf("Failure: %s\n", msg);
    exit(1);
}


void *midi_alloc(size_t s) { return malloc(s); }
void midi_free(void *a) { free(a); }


Alg_midifile_reader *read_file(char *name)
{
    FILE *inf = fopen(name, "rb");
    if (!inf) {
	    printf("could not open midi file\n");
	    exit(1);
    }
    Alg_midifile_reader *ar = new Alg_midifile_reader;
    ar->initialize(inf);
    fclose(inf);
    return ar;
}


int main()
{

    // Set the debug heap to report memory leaks when the process terminates,
    // and to keep freed blocks in the linked list.
#ifndef LINUX
    _CrtSetDbgFlag( _CRTDBG_LEAK_CHECK_DF | _CRTDBG_DELAY_FREE_MEM_DF );
#endif

    // this is a little test to create an Alg_seq and add a note
    Alg_seq_ptr seq = new Alg_seq;
    int i;
    for (i = 0; i < 20; i++) {
        Alg_note_ptr event = new Alg_note;
        event->time = i;
        event->chan = 0;
        event->key = 60;
        event->pitch = 60;
        event->dur = 1;
        event->loud = 100;
        Parameters::insert_string(&event->parameters, "ps", "this is a string");
        seq->add_event(event);
    }
    seq->insert_tempo(100, 4);
    seq->set_time_sig(12, 4, 4);
    delete seq;

    printf("reading the file sample.mid...\n");
    Alg_midifile_reader *ar = read_file("sample.mid");

	Alg_beats &blah=ar->seq->map.beats;
	Alg_beat_ptr amers= &(blah[0]);

    /* SAVE THE FILE AS TEXT: */
    printf("save as text in sample.gro...\n");
    FILE *outf = fopen("sample.gro", "w");
    allegro_write(ar->seq, outf); 
    fclose(outf);
    /**/

    //Write to MIDI file
    WriteMIDI *MFile = new WriteMIDI(ar->seq);
	FILE *outf1 = fopen("sample2.mid", "wb");
	MFile->Write(outf1);

    printf("reading the file sample2.mid...\n");
    Alg_midifile_reader *ar2 = read_file("sample2.mid");

    /* SAVE THE FILE AS TEXT: */
    printf("save as text in sample2.gro...\n");
    FILE *outf2 = fopen("sample2.gro", "w");
    allegro_write(ar2->seq, outf2); 
    fclose(outf2);

    /* DELETE THE DATA */
    delete ar;

    /* TEST FOR MEMORY LEAKS:
    _CrtMemState memstate;
    _CrtMemState memstate2;
    _CrtMemCheckpoint(&memstate);
    _CrtMemDumpStatistics(&memstate);

    for (int reps = 0; reps < 10; reps++) {

        _RPT0(_CRT_WARN, "STARTING TO READ AR2\n");
        Alg_midifile_reader *ar2 = read_file("4BROSRVG.MID");
        delete ar2;

        _CrtMemCheckpoint(&memstate2);
        _CrtMemDumpStatistics(&memstate2);

    }


    _CrtDumpMemoryLeaks( );
    */

    return 0;
}
