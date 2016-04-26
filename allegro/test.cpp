
#include "stdlib.h"
#include "stdio.h"
#include "assert.h"
#include "allegro.h"
#include "mfmidi.h"
#include "allegrosmfrd.h"
#ifndef LINUX
#include "portmidi.h"
// because portmidi for linux isn't ready yet...
#include "seq2midi.h"
#endif
#include "allegrowr.h"
#include "string.h"
#include "strparse.h"
#include "allegrord.h"
#include "allegrosmfwr.h"

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


Alg_seq_ptr read_file(char *name)
{
    FILE *inf = fopen(name, "rb");
    if (!inf) {
	    printf("could not open midi file\n");
	    exit(1);
    }
    Alg_seq_ptr seq = alg_smf_read(inf);
    fclose(inf);
    return seq;
}


int main()
{
    // Set the debug heap to report memory leaks when the process terminates,
    // and to keep freed blocks in the linked list.
#ifndef LINUX
    //_CrtSetDbgFlag( _CRTDBG_LEAK_CHECK_DF | _CRTDBG_DELAY_FREE_MEM_DF );
    /* TEST FOR MEMORY LEAKS:*/
    _CrtMemState memstate;
    _CrtMemState memstate2;
    _CrtMemState memstate3;
    _CrtMemCheckpoint(&memstate);
#endif

    // this is a little test to create an Alg_seq and add a note
    Alg_seq_ptr seq = new Alg_seq;
	seq->tracks.add_track(0); // create a track
    int i;
    for (i = 0; i < 1; i++) {
        Alg_note_ptr event = new Alg_note;
        event->time = i;
        event->chan = 0;
        event->key = 60;
        event->pitch = 60;
        event->dur = 1;
        event->loud = 100;
        Alg_parameters::insert_string(&event->parameters, "ps", 
									  "this is a string");
        seq->add_event(event, 0);
    }
    seq->insert_tempo(100, 4);
    seq->set_time_sig(12, 4, 4);
    delete seq;

#ifndef LINUX
    _CrtMemCheckpoint(&memstate2);
    _CrtMemDifference(&memstate3, &memstate, &memstate2);
    _RPT0(_CRT_WARN, "Here's what we allocated some notes...\n");
    _CrtMemDumpStatistics(&memstate3);
    _CrtMemDumpAllObjectsSince( &memstate );
#endif

    printf("reading the file scale.mid...\n");
    Alg_seq_ptr seq2 = read_file("scale.mid");

    /* SAVE THE FILE AS TEXT: */
    printf("save as text in scale.gro...\n");
    FILE *outf = fopen("scale.gro", "w");
    alg_write(seq2, outf); 
    fclose(outf);
    /**/

    /* PLAY THE FILE VIA MIDI:
    printf("type return to play midi file: ");
    char input[80];
    gets(input);
    seq_play(seq2);
    /**/

    /* REREAD THE TEXT VERSION: */
    printf("reread the text version scale.gro...\n");
    FILE *inf2 = fopen("scale.gro", "r");
    assert(inf2);
    Alg_seq_ptr seq3 = alg_read(inf2);
    fclose(inf2);

    /* REWRITE THE TEXT VERSION: */
    printf("rewrite a text version as scale2.gro...\n");
    FILE *outf2 = fopen("scale2.gro", "w");
    alg_write(seq3, outf2);
    fclose(outf2);

    /* PLAY THE FILE REREAD FROM TEXT VERSION:
    printf("type return to play midi file: ");
    seq_play(seq3);
    /**/

	/* SAVE THE FIRST FILE AS A SMF */
	printf("save scale.mid as scale2.mid...\n");
	FILE *outf3 = fopen("scale2.mid", "wb");
    alg_smf_write(seq2, outf3);
	fclose(outf3);
	/**/

	/* READ THE SMF JUST WRITTEN */
	printf("read scale2.mid...\n");
	Alg_seq_ptr seq4 = read_file("scale2.mid");
    /**/

	/* SAVE THE FILE AS TEXT */
	printf("save as text in scale3.gro...\n");
	FILE *outf4 = fopen("scale3.gro", "w");
	alg_write(seq4, outf4);
	fclose(outf4);

    /* DELETE THE DATA */
    delete seq2;
	delete seq3;
	delete seq4;

#ifndef LINUX
    _RPT0(_CRT_WARN, "Dumping memory stats using &memstate\n");
    _CrtMemCheckpoint(&memstate2);
    _CrtMemDifference(&memstate3, &memstate, &memstate2);
    _RPT0(_CRT_WARN, "Here's what we allocated to convert scale.mid to text...\n");
    _CrtMemDumpStatistics(&memstate3);
#endif

    for (int reps = 0; reps < 0; reps++) {

#ifndef LINUX
        _RPT0(_CRT_WARN, "STARTING TO READ AR2\n");
        Alg_seq_ptr seq5 = read_file("4BROSRVG.MID");
        // cause big leak: delete ar2;

        _CrtMemCheckpoint(&memstate2);
        _CrtMemDumpStatistics(&memstate2);
#endif
    }

#ifndef LINUX
    _CrtDumpMemoryLeaks( );
#endif
    /**/

    return 0;
}
