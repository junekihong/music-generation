// Allegro: music representation system, with
//      extensible in-memory sequence structure
//      upward compatible with MIDI
//      implementations in C++ and Serpent
//      external, text-based representation
//      compatible with Aura
/* CHANGE LOG:
04 apr 03 -- fixed bug in add_track that caused infinite loop
*/

#include "assert.h"
#include "stdlib.h"
#include "allegro.h"
#include "string.h"
//#include "memory.h"
#include "trace.h"


Alg_atoms symbol_table;

bool within(double d1, double d2, double epsilon)
{
    d1 -= d2;
    return d1 < epsilon && d1 > -epsilon;
}


void Alg_events::expand()
{
    max = (max + 5); // extra growth for small sizes
    max += (max >> 2); // add 25%
    Alg_event_ptr *new_events = new Alg_event_ptr[max];
    // now do copy
    memcpy(new_events, events, len * sizeof(Alg_event_ptr));
    if (events) delete[] events;
    events = new_events;
}


Alg_events::~Alg_events()
{
    // individual events are not deleted, only the array
    if (events) {
        delete[] events;
    }
}


void Alg_events::insert(Alg_event_ptr event)
{
    if (max <= len) {
        expand();
    }
    events[len] = event;
    len++;
    // find insertion point:
    for (int i = 0; i < len; i++) {
        if (events[i]->time > event->time) {
            // insert event at i
            memmove(&events[i + 1], &events[i], 
                    sizeof(Alg_event_ptr) * (len - i - 1));
            events[i] = event;
            return;
        }
    }
}


void Alg_events::append(Alg_event_ptr event)
{
    if (max <= len) {
        expand();
    }
    events[len++] = event;
}


void Alg_atoms::expand()
{
    max = (max + 5); // extra growth for small sizes
    max += (max >> 2); // add 25%
    char **new_atoms = new Alg_attribute[max];
    // now do copy
    memcpy(new_atoms, atoms, len * sizeof(Alg_attribute));
    if (atoms) delete[] atoms;
    atoms = new_atoms;
}


char *heapify(char *s)
{
    char *h = new char[strlen(s) + 1];
    strcpy(h, s);
    return h;
}


Alg_attribute Alg_atoms::insert_new(char *name, char attr_type)
{
    if (len == max) expand();
    char *h = new char[strlen(name) + 2];
    strcpy(h + 1, name);
    *h = attr_type;
    atoms[len++] = h;
    return h;
}


Alg_attribute Alg_atoms::insert_attribute(Alg_attribute attr)
{
    for (int i = 0; i < len; i++) {
        if (strcmp(attr, atoms[i]) == 0) {
            return atoms[i];
        }
    }
    return insert_new(attr + 1, attr[0]);
}


Alg_attribute Alg_atoms::insert_string(char *name)
{
    char attr_type = name[strlen(name) - 1];
    for (int i = 0; i < len; i++) {
        if (attr_type == atoms[i][0] &&
            strcmp(name, atoms[i] + 1) == 0) {
            return atoms[i];
        }
    }
    return insert_new(name, attr_type);
}


void Alg_parameters::insert_real(Alg_parameters **list, char *name, double r)
{
    Alg_parameters_ptr a = new Alg_parameters(*list);
    *list = a;
    a->parm.set_attr(symbol_table.insert_string(name));
    a->parm.r = r;
    assert(a->parm.attr_type() == 'r');
}


void Alg_parameters::insert_string(Alg_parameters **list, char *name, char *s)
{
    Alg_parameters_ptr a = new Alg_parameters(*list);
    *list = a;
    a->parm.set_attr(symbol_table.insert_string(name));
    // string is deleted when parameter is deleted
    a->parm.s = heapify(s);
    assert(a->parm.attr_type() == 's');
}

void Alg_parameters::insert_integer(Alg_parameters **list, char *name, long i)
{
    Alg_parameters_ptr a = new Alg_parameters(*list);
    *list = a;
    a->parm.set_attr(symbol_table.insert_string(name));
    a->parm.i = i;
    assert(a->parm.attr_type() == 'i');
}

void Alg_parameters::insert_logical(Alg_parameters **list, char *name, bool l)
{
    Alg_parameters_ptr a = new Alg_parameters(*list);
    *list = a;
    a->parm.set_attr(symbol_table.insert_string(name));
    a->parm.l = l;
    assert(a->parm.attr_type() == 'l');
}

void Alg_parameters::insert_atom(Alg_parameters **list, char *name, char *s)
{
    Alg_parameters_ptr a = new Alg_parameters(*list);
    *list = a;
    a->parm.set_attr(symbol_table.insert_string(name));
    a->parm.a = symbol_table.insert_string(s);
    assert(a->parm.attr_type() == 'a');
}


Alg_parameters *Alg_parameters::remove_key(Alg_parameters **list, char *name)
{
    while (*list) {
        if (strcmp((*list)->parm.attr_name(), name) == 0) {
            Alg_parameters_ptr p = *list;
            *list = p->next;
            p->next = NULL;
            return p; // caller should free this pointer
        }
        list = &((*list)->next);
    }
    return NULL;
}


Alg_parameter::~Alg_parameter()
{
    if (attr && attr_type() == 's' && s) {
        delete[] s;
    }
}


Alg_note::~Alg_note()
{
    while (parameters) {
        Alg_parameters_ptr to_delete = parameters;
        parameters = parameters->next;
        delete to_delete;
    }
}


void Alg_beats::expand()
{
    max = (max + 5); // extra growth for small sizes
    max += (max >> 2); // add 25%
    Alg_beat_ptr new_beats = new Alg_beat[max];
    // now do copy
    memcpy(new_beats, beats, len * sizeof(Alg_beat));
    if (beats) delete[] beats;
    beats = new_beats;
}


void Alg_beats::insert(long i, Alg_beat_ptr beat)
{
    assert(i >= 0 && i <= len);
    if (max <= len) {
        expand();
    }
    memmove(&beats[i], &beats[i + 1], sizeof(Alg_beat) * (len - i));
    memcpy(&beats[i], beat, sizeof(Alg_beat));
    len++;
}




long Alg_time_map::locate_time(double time)
{
    int i = 0;
    while ((i < beats.len) && (time > beats[i].time)) {
        i++;
    }
    return i;
}


long Alg_time_map::locate_beat(double beat)
{
    int i = 0;
    while (( i < beats.len) && (beat > beats[i].beat)) {
        i++;
    }
    return i;
}


double Alg_time_map::beat_to_time(double beat)
{
    Alg_beat_ptr mbi;
    Alg_beat_ptr mbi1;
    if (beat <= 0) {
        return beat;
    }
    int i = locate_beat(beat);
    if (i == beats.len) {
        if (last_tempo_flag) {
            return beats[i - 1].time + 
                   (beat - beats[i - 1].beat) / last_tempo;
        } else if (i == 1) {
            return beat * 0.5; // default midi tempo is 120
				// so we use that as default allegro tempo too
        } else {
            mbi = &beats[i - 2];
            mbi1 = &beats[i - 1];
        }
    } else {
        mbi = &beats[i - 1];
        mbi1 = &beats[i];
    }
    // whether w extrapolate or interpolate, the math is the same
    double time_dif = mbi1->time - mbi->time;
    double beat_dif = mbi1->beat - mbi->beat;
    return mbi->time + (beat - mbi->beat) * time_dif / beat_dif;
}


double Alg_time_map::time_to_beat(double time)
{
    Alg_beat_ptr mbi;
    Alg_beat_ptr mbi1;
    if (time <= 0.0) return time;
    int i = locate_time(time);
    if (i == beats.len) {
        if (last_tempo_flag) {
            return beats[i - 1].beat + 
                   (time - beats[i - 1].time) * last_tempo;
        } else if (i == 1) {
            return time * 2.0; // default tempo is 120
        } else {
            mbi = &beats[i - 2];
            mbi1 = &beats[i - 1];
        }
    } else {
        mbi = &beats[i - 1];
        mbi1 = & beats[i];
    }
    double time_dif = mbi1->time - mbi->time;
    double beat_dif = mbi1->beat - mbi->beat;
    return mbi->beat + (time - mbi->time) * beat_dif / time_dif;
}
         

void Alg_time_sigs::expand()
{
    max = (max + 5); // extra growth for small sizes
    max += (max >> 2); // add 25%
    Alg_time_sig_ptr new_time_sigs = new Alg_time_sig[max];
    // now do copy
    memcpy(new_time_sigs, time_sigs, len * sizeof(Alg_time_sig));
    if (time_sigs) delete[] time_sigs;
    time_sigs = new_time_sigs;
}


void Alg_time_sigs::insert(double beat, double num, double den)
{
    // find insertion point:
    for (int i = 0; i < len; i++) {
		if (within(time_sigs[i].beat, beat, 0.000001)) {
			// overwrite location i with new info
            time_sigs[i].beat = beat;
            time_sigs[i].num = num;
            time_sigs[i].den = den;
			return;
		} else if (time_sigs[i].beat > beat) {
			// make room for new event
			if (max <= len) expand();
			len++;
            // insert new event at i
            memmove(&time_sigs[i], &time_sigs[i + 1], 
                    sizeof(Alg_time_sig) * (len - i));
            time_sigs[i].beat = beat;
            time_sigs[i].num = num;
            time_sigs[i].den = den;
			return;
		}
    }
	// if we fall out of loop, then this goes at end
    if (max <= len) expand();
    time_sigs[len].beat = beat;
    time_sigs[len].num = num;
    time_sigs[len].den = den;
    len++;
}


Alg_tracks::~Alg_tracks()
{
    // Alg_events objects (track data) are not deleted, only the array
    if (tracks) {
        delete[] tracks;
    }
}


void Alg_tracks::expand_to(int new_max)
{
	max = new_max;
    Alg_events_ptr *new_tracks = new Alg_events_ptr[max];
    // now do copy
    memcpy(new_tracks, tracks, len * sizeof(Alg_events_ptr));
    if (tracks) delete[] tracks;
    tracks = new_tracks;
}	


void Alg_tracks::expand()
{
    max = (max + 5); // extra growth for small sizes
    max += (max >> 2); // add 25%
	expand_to(max);
}


void Alg_tracks::append(Alg_events_ptr track)
{
    if (max <= len) {
        expand();
    }
    tracks[len] = track;
    len++;
}


void Alg_tracks::add_track(int track_num)
    // Create a new track at index track_num.
	// If track already exists, this call does nothing.
	// If highest previous track is not at track_num-1, then
	// create tracks at len, len+1, ..., track_num.
{
	assert(track_num >= 0);
	if (track_num == max) {
		// use eponential growth to insert tracks sequentially
		expand();
	} else if (track_num > max) {
		// grow to exact size for random inserts
		expand_to(track_num + 1);
	}
	if (track_num < len) return; // don't add if already there
	while (len <= track_num) {
		tracks[len] = new Alg_events;
		len++;
	}
}


void Alg_tracks::reset()
{
    // all track events are incorporated into the seq,
    // so all we need to delete are the arrays of pointers
    for (int i = 0; i < len; i++) {
        delete tracks[i];
    }
    if (tracks) delete [] tracks;
    tracks = NULL;
    len = 0;
	max = 0;								// Modified by Ning Hu	Nov.19 2002
}


Alg_seq::~Alg_seq()
{
	int i, j;
	// Tracks does not delete Alg_events elements
	for (j = 0; j < tracks.len; j++) {
		Alg_events_ptr notes = tracks[j];
        // Alg_events does not delete notes 
        for (i = 0; i < notes->len; i++)
            delete notes->events[i];
		delete notes;
	}
}


long Alg_seq::seek_time(double time, int track_num)
// find index of first score event after time
{
    long i;
	Alg_events &notes = *tracks[track_num];
	for (i = 0; i < notes.len; i++) {
		if (notes[i]->time > time) {
			break;
		}
    }
    return i;
}


void Alg_seq::convert_to_beats()
// modify all times and durations in notes to beats
{
    if (units_are_seconds) {
        units_are_seconds = false;
		long i, j;
		for (j = 0; j < tracks.len; j++) {
			Alg_events &notes = *tracks[j];
			for (i = 0; i < notes.len; i++) {
				Alg_event_ptr e = notes[i];
				double beat = map.time_to_beat(e->time);
				if (e->type == 'n') {
					Alg_note_ptr n = (Alg_note_ptr) e;
					n->dur = map.time_to_beat(n->time + n->dur) - beat;
				}
				e->time = beat;
			}
		}
	}
}


void Alg_seq::convert_to_seconds()
// modify all times and durations in notes to seconds
{
    if (!units_are_seconds) {
        units_are_seconds = true;
		long i, j;
		for (j = 0; j < tracks.len; j++) {
			Alg_events &notes = *tracks[j];
			for (i = 0; i < notes.len; i++) {
				Alg_event_ptr e = notes[i];
				double time = map.beat_to_time(e->time);
				if (e->type == 'n') {
					Alg_note_ptr n = (Alg_note_ptr) e;
					n->dur = map.beat_to_time(n->time + n->dur) - time;
					n->time = time;
				}
			}
		}
	}
}


bool Alg_seq::insert_beat(double time, double beat)
// insert a time,beat pair
// return true or false (false indicates an error, no update)
// it is an error to imply a negative tempo or to insert at
// a negative time
{
    if (time < 0 || beat < 0) return false;
    if (time == 0.0 && beat > 0)
        time = 0.000001; // avoid infinite tempo, offset time by 1us
    if (time == 0.0 && beat == 0.0)
        return true; // (0,0) is already in the map!
    convert_to_beats(); // beats are invariant when changing tempo
    int i = map.locate_time(time); // i is insertion point
    if (i < map.beats.len && within(map.beats[i].time, time, 0.000001)) {
        // replace beat if time is already in the map
        map.beats[i].beat = beat;
    } else {
        Alg_beat point;
        point.beat = beat;
        point.time = time;
        map.beats.insert(i, &point);
    }
    // beats[i] contains new beat
    // make sure we didn't generate a zero tempo.
    // if so, space beats by one microbeat as necessary
    long j = i;
    while (j < map.beats.len &&
        map.beats[j - 1].beat + 0.000001 >= map.beats[j].beat) {
        map.beats[j].beat = map.beats[j - 1].beat + 0.000001;
        j++;
    }
    return true;
}


bool Alg_seq::insert_tempo(double tempo, double beat)
{
    tempo = tempo / 60.0; // convert to beats per second
    // change the tempo at the given beat until the next beat event
    if (beat < 0) return false;
    convert_to_beats(); // beats are invariant when changing tempo
    double time = map.beat_to_time(beat);
    long i = map.locate_time(time);
    if (i >= map.beats.len || !within(map.beats[i].time, time, 0.000001)) {
        insert_beat(time, beat);
    }
    // now i is index of beat where tempo will change
    if (i == map.beats.len - 1) {
        map.last_tempo = tempo;
        map.last_tempo_flag = true;
    } else { // adjust all future beats
        // compute the difference in beats
        double diff = map.beats[i + 1].beat - map.beats[i].beat;
        // convert beat difference to seconds at new tempo
        diff = diff / tempo;
        // figure out old time difference:
        double old_diff = map.beats[i + 1].time - time;
        // compute difference too
        diff = diff - old_diff;
        // apply new_diff to score and beats
        while (i < map.beats.len) {
            map.beats[i].time = map.beats[i].time + diff;
            i++;
        }
    }
    return true;
}


void Alg_seq::add_event(Alg_event_ptr event, int track_num)
    // add_event puts an event in a given track (track_num). 
	// The track must exist. The time and duration of the
	// event must be specified in seconds.
{
	convert_to_seconds();
    tracks[track_num]->insert(event);
/*
    if (event->type == 'n') {
        Alg_note_ptr n = (Alg_note_ptr) event;
        trace("note %d at %g for %g\n", n->key, n->time, n->dur);
    }
 */
}


bool Alg_seq::set_tempo(double tempo, double start_beat, double end_beat)
// set tempo from start_beat to end_beat
{
    if (start_beat >= end_beat) return false;
    convert_to_beats();
    // algorithm: insert a beat event if necessary at start_beat
    //    and at end_beat
    // delete intervening map elements
    // change the tempo
    insert_beat(map.beat_to_time(start_beat), start_beat);
    insert_beat(map.beat_to_time(end_beat), end_beat);
    long start_x = map.locate_beat(start_beat) + 1;
    long stop_x = map.locate_beat(end_beat) + 1;
    while (stop_x < map.beats.len) {
        map.beats[start_x] = map.beats[stop_x];
        start_x++;
        stop_x++;
    }
    map.beats.len = start_x; // truncate the map to new length
    return insert_tempo(tempo, start_beat);
}


void Alg_seq::set_time_sig(double beat, double num, double den)
{
    time_sig.insert(beat, num, den);
}


void Alg_seq::beat_to_measure(double beat, long *measure, double *m_beat,
                          double *num, double *den)
{
    // return [measure, beat, num, den]
    double m = 0; // measure number
    double bpm;
    int tsx;

    for (tsx = 0; tsx < time_sig.len; tsx++) {
        bpm = 4;
        // assume 4/4 if no time signature
        double prev_beat = 0;
        double prev_num = 4;
        double prev_den = 4;
        if (tsx > 0) {
            bpm = time_sig[tsx].num * 4 / time_sig[tsx].den;
            prev_beat = time_sig[tsx].beat;
            prev_num = time_sig[tsx].num;
            prev_den = time_sig[tsx].den;
        }
        if (time_sig[tsx].beat > beat) {
            m = m + (beat - prev_beat) / bpm;
            *measure = (long) m;
            *m_beat = (m - *measure) * bpm * prev_den * 0.25;
            *num = prev_num;
            *den = prev_den;
            return;
        }
        // round m up to an integer (but allow for a small
        // numerical inaccuracy)
        m = m + (long) (0.99 + (time_sig[tsx].beat - prev_beat) / bpm);
    }
    // if we didn't return yet, compute after last time signature
    Alg_time_sig initial(0, 4, 4);
    Alg_time_sig_ptr prev = &initial;
    if (tsx > 0) { // use last time signature
        prev = &time_sig[time_sig.len - 1];
    }
    bpm = prev->num * 4 / prev->den;
    m = m + (beat - prev->beat) / bpm;
    *measure = (long) m;
    *m_beat = (m - *measure) * bpm * prev->den * 0.25;
    *num = prev->num;
    *den = prev->den;
}

/*
void Alg_seq::set_events(Alg_event_ptr *events, long len, long max)
{
    convert_to_seconds(); // because notes are in seconds
    notes.set_events(events, len, max);
}
*/


void Alg_seq::iteration_begin()
{
    // keep an array of indexes into tracks
    current = new long[tracks.len];
	int i;
    for (i = 0; i < tracks.len; i++) {
        current[i] = 0;
	}    
}


Alg_event_ptr Alg_seq::iteration_next()
    // return the next event in time from any track
{
    Alg_events_ptr tr;  // a track
    long cur;       // a track index
    // find lowest next time of any track:
    double next = 1000000.0;
	int i, track;
    for (i = 0; i < tracks.len; i++) {
        tr = tracks[i];
        cur = current[i];
        if (cur < tr->len && (*tr)[cur]->time < next) {
            next = (*tr)[cur]->time;
            track = i;
        }
    }
	if (next < 1000000.0) {
		return (*tracks[track])[current[track]++];
	} else {
		return NULL;
	}
}


void Alg_seq::iteration_end()
{
    delete[] current;
}


void Alg_seq::merge_tracks()
{
    int track = -1;
    long sum = 0;
    long i;
    for (i = 0; i < tracks.len; i++) {
        sum = sum + tracks[i]->len;
    }
    // preallocate array for efficiency:
    Alg_event_ptr *notes = new Alg_event_ptr[sum];
	iteration_begin();
    long notes_index = 0;

	Alg_event_ptr event;
    while (event = iteration_next()) {
        notes[notes_index++] = event;
    }
    tracks.reset(); // don't need them any more
	tracks.add_track(0);
    tracks[0]->set_events(notes, sum, sum);
	iteration_end();
}


// sr_letter_to_type = {"i": 'Integer', "r": 'Real', "s": 'String',
//                     "l": 'Logical', "a": 'Symbol'}


