
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

#define WINDOW_SIZE 0.25

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
	// GET THE MIDI FILE NAME 	
	char smidiname[50], *sscorename;
    printf("Name of the midi file: ");
	scanf("%s", smidiname);

	// READ IN THE MIDI FILE
    Alg_seq_ptr seq = read_file(smidiname);

	// MERGE TRACKES
	seq->merge_tracks();

	// CHANGE INTERNAL REPRESENTATION OF TIME AND DURATION TO SECONDS
	seq->convert_to_seconds();
	// INITIALIZATION
	double nowtime = 0.5 * WINDOW_SIZE;
	int framevalue = 0;
	double onset[12];
	int i, k, noteoctave;
	double notetime;
	long frames = 0;
	for (k = 0; k < 12; k++) {
		onset[k] = -1.0;
	}
	
	sscorename = strcat(smidiname, ".score");
	FILE *outf = fopen(sscorename, "w");

	// GO THROUGH EACH NOTE EVENT IN THE SEQUENCE AND OUTPUT FRAME VALUES STEP BY STEP
	Alg_events &notes = *seq->tracks[0]; 
	for (i = 0; i < notes.len; i++) { 
		Alg_event_ptr e = notes[i]; 
		if (e->type == 'n') {
			Alg_note_ptr note = (Alg_note_ptr)e;
			while (note->time >= nowtime) {
				for (k = 0; k < 12; k++) {
					if (onset[k] >= nowtime) {
						framevalue += 1 << k;
					}
				}
				fprintf(outf, "%d, ", framevalue);
				nowtime += WINDOW_SIZE;
				framevalue = 0;
				frames++;
			}
			notetime = note->dur + note->time;
			noteoctave = note->key % 12;
			//noteoctave = (int)(fmod(note->pitch, 12.0) + 0.5);
			if (onset[noteoctave] < notetime) onset[noteoctave] = notetime;			
		}
	} 

	// OUTPUT THE VALUES OF THE REMAIN FRAMES
	double totaltime = nowtime;
	for (k = 0; k < 12; k++) {
		if (onset[k] > totaltime) totaltime = onset[k];
	}
	while (totaltime >= nowtime) {
			for (k = 0; k < 12; k++) {
				if (onset[k] >= nowtime) {
					framevalue += 1 << k;
				}
			}
			fprintf(outf, "%d, ", framevalue);
			nowtime += WINDOW_SIZE;
			framevalue = 0;
			frames++;
	}

	fclose(outf);
	printf("Total frames: %d (WINDOW_SIZE = %f)\n", frames, WINDOW_SIZE);
	printf("Score written to %s\n", sscorename);

    /* DELETE THE DATA */
    delete seq;
    /**/

    return 0;
}
