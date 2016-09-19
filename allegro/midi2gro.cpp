
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


int main( int argc, char *argv[] )
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
    
#ifndef LINUX
    _CrtMemCheckpoint(&memstate2);
    _CrtMemDifference(&memstate3, &memstate, &memstate2);
    _RPT0(_CRT_WARN, "Here's what we allocated some notes...\n");
    _CrtMemDumpStatistics(&memstate3);
    _CrtMemDumpAllObjectsSince( &memstate );
#endif

    if( argc < 3 ) {
        printf("Two arguments expected.\n");
        printf("Need to run with one MIDI file and a specified directory.\n");
        return 0;
    }
    if( argc == 3 ){
        fprintf(stderr, "MIDI file to read in: %s\n", argv[1]);
        fprintf(stderr, "Directory to store files: %s\n", argv[2]);
    }
    if( argc > 3 ) {
        printf("Too many arguments supplied.\n");
        printf("Need to run with one MIDI file and a specified directory.\n");
        return 0;
    }
    
    char * FILENAME = argv[1];
    Alg_seq_ptr seq = read_file(FILENAME);
    char * slash = strrchr(FILENAME,'/');
    if(slash != 0){
        //printf("%s\n", slash+1);
        FILENAME = slash + 1;
    }
    
    char FILENAME_gro[1000];
    char * DIRECTORY = argv[2];
    int i = 0;
    for(; i < strlen(DIRECTORY); i++){
        FILENAME_gro[i] = DIRECTORY[i];
    }
    int j = 0;
    for(; j < strrchr(FILENAME, '.') - FILENAME && j < 1000-5; j++){
        FILENAME_gro[i+j] = FILENAME[j];
    }
    FILENAME_gro[i+j] = '.';
    FILENAME_gro[i+j+1] = 'g';
    FILENAME_gro[i+j+2] = 'r';
    FILENAME_gro[i+j+3] = 'o';
    FILENAME_gro[i+j+4] = '\0';

    // Saving MIDI file as .gro text
    fprintf(stderr, "save as text in %s...\n", FILENAME_gro);

    FILE *outf = fopen(FILENAME_gro, "w");
    alg_write(seq, outf);
    fclose(outf);
    /**/

    /* DELETE THE DATA */
    delete seq;
    
#ifndef LINUX
    _CrtDumpMemoryLeaks( );
#endif
    /**/

    return 0;
}
