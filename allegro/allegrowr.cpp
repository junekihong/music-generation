// allegrowr.cpp -- write sequence to an Allegro file (text)

#include "stdlib.h"
#include "stdio.h"
#include "assert.h"
#include "allegro.h"
#include "allegrowr.h"
#include "string.h"
#include "strparse.h"


void parameter_print(FILE *file, Alg_parameter_ptr p)
{
    char str[256];
    fprintf(file, " -%s:", p->attr_name());
    switch (p->attr_type()) {
    case 'a':
        fprintf(file, "'%s'", Alg_atom_name(p->a));
        break;
    case 'i':
        fprintf(file, "%d", p->i);
        break;
    case 'l':
        fprintf(file, "%s", p->l ? "true" : "false");
        break;
    case 'r':
        fprintf(file, "%g", p->r);
        break;
    case 's':
        string_escape(str, p->s, "\"");
        fprintf(file, "%s", str);
        break;
    }
}


void alg_write(Alg_seq_ptr seq, FILE *file)
{
    int i, j;
    // first write the tempo map
	fprintf(file, "#track 0\n");
    Alg_beats &beats = seq->map.beats;
    for (i = 0; i < beats.len - 1; i++) {
        Alg_beat_ptr b = &(beats[i]);
        fprintf(file, "TW%g ", seq->map.time_to_beat(b->time) / 4);
        double tempo = (beats[i + 1].beat - beats[i].beat) /
                       (beats[i + 1].time - beats[i].time);
        fprintf(file, "-tempor:%g\n", tempo * 60);
    }
    if (seq->map.last_tempo_flag) { // we have final tempo:
        double time = seq->map.time_to_beat(beats[beats.len - 1].time) / 4;
        fprintf(file, "TW%g ", time);
        fprintf(file, "-tempor:%g\n", seq->map.last_tempo * 60.0);
    }

	// write the time signatures
	Alg_time_sigs &time_sig = seq->time_sig;
	for (i = 0; i < time_sig.len; i++) {
		Alg_time_sig_ptr ts = &(time_sig[i]);
		fprintf(file, "TW%g V- -timesig_numr:%g\n", ts->beat / 4, ts->num);
		fprintf(file, "TW%g V- -timesig_denr:%g\n", ts->beat / 4, ts->den);
	}

	for (j = 0; j < seq->tracks.len; j++) {
		Alg_events &notes = *seq->tracks[j];
		if (j != 0) fprintf(file, "#track %d\n", j);
	    // now write the notes at beat positions
	    for (i = 0; i < notes.len; i++) {
			Alg_event_ptr e = notes[i];
			double start = seq->map.time_to_beat(e->time);
			fprintf(file, "TW%g ", start / 4);
			// write the channel as Vn or V-
			if (e->chan == -1) fprintf(file, "V-");
			else fprintf(file, "V%d", e->chan);
			// write the note or update data
			if (e->type == 'n') {
				Alg_note_ptr n = (Alg_note_ptr) e;
				double dur = seq->map.time_to_beat(n->time + n->dur) - start;
				fprintf(file, " K%d P%g Q%g L%g", n->key, n->pitch, dur, n->loud);
				Alg_parameters_ptr p = n->parameters;
				while (p) {
					parameter_print(file, &(p->parm));
					p = p->next;
				}
			} else { // an update
				assert(e->type == 'u');
				Alg_update_ptr u = (Alg_update_ptr) e;
				if (u->key != -1) {
					fprintf(file, " K%d", u->key);
				}
				parameter_print(file, &(u->parameter));
			}
			fprintf(file, "\n");
		}
	}
}



