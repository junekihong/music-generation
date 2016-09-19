// Allegro: music representation system, with
//      extensible in-memory sequence structure
//      upward compatible with MIDI
//      implementations in C++ and Serpent
//      external, text-based representation
//      compatible with Aura
//
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


char *heapify(char *s)
{
    char *h = new char[strlen(s) + 1];
    strcpy(h, s);
    return h;
}


void Alg_atoms::expand()
{
    max = (max + 5);   // extra growth for small sizes
    max += (max >> 2); // add 25%
    char **new_atoms = new Alg_attribute[max];
    // now do copy
    memcpy(new_atoms, atoms, len * sizeof(Alg_attribute));
    if (atoms) delete[] atoms;
    atoms = new_atoms;
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


void Alg_parameter::copy(Alg_parameter_ptr parm)
{
    *this = *parm; // copy all fields
    // if the value is a string, copy the string
    if (attr_type() == 's') {
        s = heapify(s);
    }
}


Alg_parameter::~Alg_parameter()
{
    if (attr_type() == 's' && s) {
        delete[] s;
    }
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


Alg_parameter_ptr Alg_parameters::find(Alg_attribute *attr)
{
    assert(attr);
    Alg_parameters_ptr temp = this;
    while (temp) {
        if (temp->parm.attr == *attr) {
            return &(temp->parm);
        }
    }
    return NULL;
}


int Alg_event::get_type_code()
{
	const char* attr = get_attribute();
    if (!is_note())
	{
        if (attr == "gate")         // volume change
	        return ALG_GATE;
        if (attr == "bend")         // pitch bend     
	        return ALG_BEND;
        if (attr == "control")      // control change
    	    return ALG_CONTROL;
        if (attr == "program")      // program change
	        return ALG_PROGRAM;
        if (attr == "pressure")     // pressure change
	        return ALG_PRESSURE;
        if (attr == "keysig")       // key signature  
	        return ALG_KEYSIG;
        if (attr == "timesig_num")  // time signature numerator
    	    return ALG_TIMESIG_NUM;
        if (attr == "timesig_den")  // time signature denominator
            return ALG_TIMESIG_DEN;
		return ALG_OTHER;
	}
	return 0; // it is a note
}


void Alg_event::set_parameter(Alg_parameter_ptr new_parameter)
{
    Alg_parameter_ptr parm;
    if (is_note()) {
        Alg_note_ptr note = (Alg_note_ptr) this;
        parm = note->parameters->find(&(new_parameter->attr));
        if (!parm) {
            note->parameters = new Alg_parameters(note->parameters);
            parm = &(note->parameters->parm);
        }
    } else { // update
        Alg_update_ptr update = (Alg_update_ptr) this;
        parm = &(update->parameter);
    }
    parm = new_parameter; // copy entire parameter
}


void Alg_event::set_string_value(char *a, char *value)
{
    assert(a); // must be non-null
    Alg_attribute attr = symbol_table.insert_string(a);
    assert(a[0] == 's');
    Alg_parameter parm;
    parm.set_attr(a);
    parm.s = value;
    set_parameter(&parm);
    parm.s = NULL; // do this to prevent string from being freed
}


void Alg_event::set_real_value(char *a, double value)
{
    assert(a); // must be non-null
    Alg_attribute attr = symbol_table.insert_string(a);
    assert(a[0] == 'r');
    Alg_parameter parm;
    parm.set_attr(a);
    parm.r = value;
    set_parameter(&parm);
	// since tpye is 'r' we don't have to NULL the string
}


void Alg_event::set_logical_value(char *a, bool value)
{
    assert(a); // must be non-null
    Alg_attribute attr = symbol_table.insert_string(a);
    assert(a[0] == 'l');
    Alg_parameter parm;
    parm.set_attr(a);
    parm.l = value;
    set_parameter(&parm);
	// since tpye is 'r' we don't have to NULL the string
}


void Alg_event::set_integer_value(char *a, long value)
{
    assert(a); // must be non-null
    Alg_attribute attr = symbol_table.insert_string(a);
    assert(a[0] == 'i');
    Alg_parameter parm;
    parm.set_attr(a);
    parm.i = value;
    set_parameter(&parm);
	// since tpye is 'r' we don't have to NULL the string
}


void Alg_event::set_atom_value(char *a, char *value)
{
    assert(a); // must be non-null
    Alg_attribute attr = symbol_table.insert_string(a);
    assert(a[0] == 'a');
    Alg_parameter parm;
    parm.set_attr(a);
    parm.a = value;
    set_parameter(&parm);
	// since tpye is 'r' we don't have to NULL the string
}


double Alg_event::get_pitch()
{
    assert(is_note());
    Alg_note* note = (Alg_note *) this;
    return note->pitch;
}


double Alg_event::get_start_time()
{
    assert(is_note());
	Alg_note* note = (Alg_note *) this;
	return note->time;
}


double Alg_event::get_end_time()
{
    assert(is_note());
	Alg_note* note = (Alg_note *) this;
	return note->time + note->dur;
}


double Alg_event::get_duration()
{
    assert(is_note());
	Alg_note* note = (Alg_note *) this;
	return note->dur;
}


void Alg_event::set_pitch(double p)
{
    assert(is_note());
	Alg_note* note = (Alg_note *) this;
	note->pitch = p;
}


void Alg_event::set_duration(double d)
{
    assert(is_note());
	Alg_note* note = (Alg_note *) this;
    note->dur = d;
}


bool Alg_event::has_attribute(char *a)
{
    assert(is_note());
    assert(a); // must be non-null
    Alg_note* note = (Alg_note *) this;
    Alg_attribute attr = symbol_table.insert_string(a);
    Alg_parameter_ptr parm = note->parameters->find(&attr);
    return parm != NULL;
}


char Alg_event::get_attribute_type(char *a)
{
	assert(is_note());
    assert(a);
    return a[strlen(a) - 1];
}


char *Alg_event::get_string_value(char *a, char *value)
{
	assert(is_note());
	assert(a); // must be non-null
	Alg_note* note = (Alg_note *) this;
	Alg_attribute attr = symbol_table.insert_string(a);
	assert(a[0] == 's'); // must be of type string
    Alg_parameter_ptr parm = note->parameters->find(&attr);
    if (parm) return parm->s;
    return value;
}


double Alg_event::get_real_value(char *a, double value)
{	
	assert(is_note());
	assert(a);
	Alg_note* note = (Alg_note *) this;
	Alg_parameters_ptr temp = note->parameters;
    Alg_attribute attr = symbol_table.insert_string(a);
    assert(a[0] == 'r'); // must be of type real
    Alg_parameter_ptr parm = note->parameters->find(&attr);
    if (parm) return parm->r;
    return value;
}


bool Alg_event::get_logical_value(char *a, bool value)
{	
	assert(is_note());
	assert(a);
	Alg_note* note = (Alg_note *) this;
	Alg_parameters_ptr temp = note->parameters;
    Alg_attribute attr = symbol_table.insert_string(a);
    assert(a[0] == 'l'); // must be of type logical
    Alg_parameter_ptr parm = note->parameters->find(&attr);
    if (parm) return parm->l;
    return value;
}


long Alg_event::get_integer_value(char *a, long value)
{	
	assert(is_note());
	assert(a);
	Alg_note* note = (Alg_note *) this;
	Alg_parameters_ptr temp = note->parameters;
    Alg_attribute attr = symbol_table.insert_string(a);
    assert(a[0] == 'i'); // must be of type integer
    Alg_parameter_ptr parm = note->parameters->find(&attr);
    if (parm) return parm->i;
    return value;
}


char *Alg_event::get_atom_value(char *a, char *value)
{	
	assert(is_note());
	assert(a);
	Alg_note* note = (Alg_note *) this;
	Alg_parameters_ptr temp = note->parameters;
    Alg_attribute attr = symbol_table.insert_string(a);
    assert(a[0] == 'a'); // must be of type atom
    Alg_parameter_ptr parm = note->parameters->find(&attr);
    if (parm) return parm->a;
	// if default is a string, convert to an atom (unique
    // string in symbol table) and return it
	return (value == NULL ? NULL :
              symbol_table.insert_string(value));
}


void Alg_event::delete_attribute(char *a)
{
	assert(is_note());
	Alg_note* note = (Alg_note *) this;
    Alg_parameters::remove_key(&(note->parameters), a);
}


const char *Alg_event::get_attribute()
// Note: this returns a string, not an Alg_attribute
{
	assert(is_update());
	Alg_update* update = (Alg_update *) this;
	return update->parameter.attr_name();
}


char Alg_event::get_update_type()
{
	assert(is_update());
	Alg_update* update = (Alg_update *) this;
	return update->parameter.attr_type();
}


char *Alg_event::get_string_value()
{
	assert(is_update());
	Alg_update* update = (Alg_update *) this;
    assert(get_update_type() == 's');
	return update->parameter.attr_name();
}


double Alg_event::get_real_value()
{
	assert(is_update());
	Alg_update* update = (Alg_update *) this;
    assert(get_update_type() == 'r');
	return update->parameter.r;
}


bool Alg_event::get_logical_value()
{
	assert(is_update());
	Alg_update* update = (Alg_update *) this;
    assert(get_update_type() == 'l');
	return update->parameter.l;
}


long Alg_event::get_integer_value()
{
	assert(is_update());
	Alg_update* update = (Alg_update *) this;
    assert(get_update_type() == 'i');
	return update->parameter.i;
}


char *Alg_event::get_atom_value()
{
	assert(is_update());
	Alg_update* update = (Alg_update *) this;
    assert(get_update_type() == 'a');
	return update->parameter.a;
}


bool Alg_event::overlap(double t, double len, bool all)
{
	// event starts within region
    if (time >= t && time <= t + len)
		return true;
    if (all && is_note()) {
		// note ends within region
        if (time + ((Alg_note_ptr) this)->dur >= t &&
			time + ((Alg_note_ptr) this)->dur <= t + len)
			return true;
		// note starts before and ends after region
		if (time <= t && time + ((Alg_note_ptr) this)->dur >= t + len)
			return true;
	}
	// does not overlap
	return false;
}


Alg_note::Alg_note(Alg_note_ptr note)
{
    *this = *note; // copy all fields
    // parameters is now a shared pointer. We need to copy the 
    // parameters
    Alg_parameters_ptr next_param_ptr = parameters;
    while (next_param_ptr) {
        Alg_parameters_ptr new_params = new Alg_parameters(next_param_ptr->next);
        new_params->parm.copy(&(next_param_ptr->parm)); // copy the attribute and value
        next_param_ptr = new_params->next;
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


Alg_update::Alg_update(Alg_update_ptr update)
{
    *this = *update; // copy all fields
    // parameter requires careful copy to possibly duplicate string value:
    this->parameter.copy(&(update->parameter));
}


void Alg_events::expand()
{
    max = (max + 5);   // extra growth for small sizes
    max += (max >> 2); // add 25%
    Alg_event_ptr *new_events = new Alg_event_ptr[max];
    // now do copy
    memcpy(new_events, events, len * sizeof(Alg_event_ptr));
    if (events) delete[] events;
    events = new_events;
}


void Alg_events::insert(Alg_event_ptr event)
{
    if (max <= len) {
        expand();
    }
    // Note: if the new event is the last one, the assignment
    // events[i] = event; (below) will never execute, so just
    // in case, we do the assignment here. events[len] will
    // be replaced during the memmove() operation below if
    // this is not the last event.
    events[len] = event;
    len++;
    // find insertion point: (this could be a binary search)
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


Alg_events::~Alg_events()
{
    // individual events are not deleted, only the array
    if (events) {
        delete[] events;
    }
}


Alg_event_list::~Alg_event_list()
{
	// note that the events contained in the list are not destroyed
    if (events)
	    delete[] events;
}


void Alg_event_list::set_start_time(Alg_event *event, double t)
{
    // For Alg_event_list, find the owner and do the update there
    // For Alg_track, change the time and move the event to the right place
    // For Alg_seq, find the track and do the update there
    
    Alg_event_list *track;
    long index, i;
    if (type == 'e') { // this is an Alg_event_list
        // make sure the owner has not changed its event set
        assert(events_owner && 
               sequence_number == events_owner->sequence_number);
        // do the update on the owner
        events_owner->set_start_time(event, t);
        return;
    } else if (type == 't') { // this is an Alg_track
        // find the event in the track
        track = (Alg_track *) this;
        // this should be a binary search since events are in time order
        // probably there should be member function to do the search
        for (index = 0; index < length(); index++) {
            if ((*track)[index] == event) goto found_event;
        }
    } else { // type == 's', an Alg_seq
        Alg_seq_ptr seq = (Alg_seq_ptr) this;
        for (i = 0; i < seq->tracks(); i++) {
            track = (Alg_track * ) seq->tracks_list[i];
            // if you implemented binary search, you could call it
            // instead of this loop too.
            for (index = 0; index < length(); index++) {
                if ((*track)[index] == event) goto found_event;
            }
        }
    }
    assert(false); // event not found seq or track!
  found_event:
    // at this point, track[index] == event
    // we could be clever and figure out exactly what notes to move
    // but it is simpler to just remove the event and reinsert it:
    memmove(&track[index + 1], &track[index],
            sizeof(Alg_event_ptr) * (length() - i - 1));
    event->time = t;
    track->insert(event);
}


void Alg_beats::expand()
{
    max = (max + 5);   // extra growth for small sizes
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


bool Alg_time_map::insert_beat(double time, double beat)
{
	if (this == &const_map)
	    return false;
	Alg_beat_ptr newbeat = new Alg_beat();
	newbeat->time = time;
	newbeat->beat = beat;
	int i = 0;
	while (i < beats.len)
	{
		if (beat > beats[i].beat)
	      i++;
		else
		{
		  beats.insert(i, newbeat);
		  return true;
		}
	}
	return false;
}


bool Alg_time_map::insert_tempo(double tempo, double beat)
{
    tempo = tempo / 60.0; // convert to beats per second
    // change the tempo at the given beat until the next beat event
    if (beat < 0) return false;
    double time = beat_to_time(beat);
    long i = locate_time(time);
    if (i >= beats.len || !within(beats[i].time, time, 0.000001)) {
        insert_beat(time, beat);
    }
    // now i is index of beat where tempo will change
    if (i == beats.len - 1) {
        last_tempo = tempo;
        last_tempo_flag = true;
    } else { // adjust all future beats
        // compute the difference in beats
        double diff = beats[i + 1].beat - beats[i].beat;
        // convert beat difference to seconds at new tempo
        diff = diff / tempo;
        // figure out old time difference:
        double old_diff = beats[i + 1].time - time;
        // compute difference too
        diff = diff - old_diff;
        // apply new_diff to score and beats
        while (i < beats.len) {
            beats[i].time = beats[i].time + diff;
            i++;
        }
    }
    return true;
}


bool Alg_time_map::set_tempo(double tempo, double start_beat, double end_beat)
{
    if (start_beat >= end_beat) return false;
    // algorithm: insert a beat event if necessary at start_beat
    // and at end_beat
    // delete intervening map elements
    // change the tempo
    insert_beat(beat_to_time(start_beat), start_beat);
    insert_beat(beat_to_time(end_beat), end_beat);
    long start_x = locate_beat(start_beat) + 1;
    long stop_x = locate_beat(end_beat) + 1;
    while (stop_x < beats.len) {
        beats[start_x] = beats[stop_x];
        start_x++;
        stop_x++;
    }
    beats.len = start_x; // truncate the map to new length
    return insert_tempo(tempo, start_beat);
}


Alg_track::Alg_track(Alg_time_map *map)
{
    type = 't';
    if (map == NULL)
	{
	    time_map = &const_map;
	    return;
    }
    time_map = map;
}


Alg_event_ptr Alg_track::copy_event(Alg_event_ptr event)
{
    Alg_event *new_event;
    if (event->is_note()) {
        new_event = new Alg_note((Alg_note_ptr) event);
    } else { // update
        new_event = new Alg_update((Alg_update_ptr) event);
    }
    return new_event;
}


Alg_track::Alg_track(Alg_track &track)
{
    type = 't';
	for (int i = 0; i < length(); i++) {
      append(copy_event(track.events[i]));
	}
    time_map = track.time_map;
    units_are_seconds = track.units_are_seconds;
}


void Alg_track::serialize(char **buffer, int *len, bool midi)
{
	// first determine whether this is a seq or a track.
	// if it is a seq, then we will write the time map and a set of tracks
	// if it is a track, we just write the track data and not the time map
	//
	// The code will align doubles on ALIGN boundaries, and longs and
	// floats are aligned to multiples of 4 bytes.
	//
	// The format for a seq is:
	//   'ALGS' -- indicates that this is a sequence
	//   long length of all seq data in bytes starting with 'ALGS'
	//   long channel_offset_per_track
	//   long units_are_seconds
	//   time_map:
	//      double last_tempo
	//      long last_tempo_flag
	//      long len -- number of tempo changes
	//      for each tempo change (Alg_beat):
	//         double time
	//         double beat
	//   time_sigs:
	//      long len -- number of time_sigs
	//      long pad
	//      for each time signature:
	//         double beat
	//         double num
	//         double den
	//   tracks:
	//      long len -- number of tracks
	//      long pad
	//      for each track:
	//         'ALGT' -- indicates this is a track
	//         long length of all track data in bytes starting with 'ALGT'
	//         long units_are_seconds
	//         long len -- number of events
	//         double beat_dur
	//         double real_dur
	//         for each event:
	//            long selected
	//            long type
	//            long key
	//            long channel
	//            double time
	//            if this is a note:
	//               double pitch
	//               double dur
	//               double loud
	//               long len -- number of parameters
	//               for each parameter:
	//                  char attribute[] with zero pad to ALIGN
	//                  one of the following, depending on type:
	//                     double r
	//                     char s[] terminated by zero
	//                     long i
	//                     long l
	//                     char a[] terminated by zero
	//               zero pad to ALIGN
	//            else if this is an update
	//               (same representation as parameter above)
	//               zero pad to ALIGN
	//
	// The format for a track is given within the Seq format above
	assert(get_type() == 't');
	char *ptr = ser_buf;
	*len = 0;
	serialize_track(&ptr, len, midi);
	// now, bytes tells us how much memory
	*buffer = new char[*len];
	memcpy(*buffer, seq_buf, *len); 
}


void Alg_seq::serialize(char **buffer, int *len, bool midi)
{	
	assert(get_type() == 's');
	serialize_seq(len, midi);
	*buffer = new char[*len];
	memcpy(*buffer, seq_buff, *len);
}


// to keep things simple and portable, ALIGN will always be 8 (align doubles
// to 64-bit boundaries)
#define ALIGN 8
#define SET_STRING(p, s, l, b) { strcpy((p), s); (p) += (l); (b) += (l); }
#define SET_INT32(p, v, b) {*((long *) (p)) = (v); (p) += 4; (b) += 4; }
#define SET_DOUBLE(p, v, b) {*(double *) (p)) = (v); (p) += 8; (b) += 8; }
#define SET_CHAR(p, v, b) {*(p) = (v); (p)++; (b)++; }
// filling one byte at a time is general but not the most efficient
#define PAD(p, b) while ((p) & 7) SET_CHAR((p), 0, (b))


void Alg_track::check_buffer(long needed, char **ptr)
{
	if (ser_buf_len < needed) { // do we need more space?
		long new_len = ser_buf_len + (ser_buf_len >> 2); // add 25%
		 // make sure new_len is as big as needed
		if (needed > new_len) new_len = needed;
		char *new_buf = new char[new_len]; // allocate space
		memcpy(new_buf, ser_buf, ser_buf_len); // copy from old buffer
		*ptr = new_buf + (*ptr - ser_buf); // relocate ptr to new buffer
		delete ser_buf; // free old buffer
		ser_buf = new_buf; // update buffer information
		ser_buf_len = new_len;
	}
}


Alg_seq::serialize_seq(int *len; bool midi)
{
	int i, j; // loop counters
	char *ptr = ser_buf;
	long bytes = 8;
	// we can easily compute how much buffer space we need until we
	// get to tracks, so expand at least that much
	long needed = 48 + 16 * time_map->beats.len + 24 * time_sig.len;
	check_buffer(needed);
	SET_STRING(ptr, "ALGS", 4, bytes);
	SET_INT32(ptr, 0, bytes); // leave room to come back and write length
	bug -- write code to set this pointer
    SET_INT32(ptr, channel_offset_per_track, bytes);
	SET_INT32(ptr, units_are_seconds, bytes);
	SET_DOUBLE(ptr, time_map->last_tempo, bytes);
	SET_LONG(ptr, time_map->last_tempo_flag, bytes);
	SET_LONG(ptr, time_map->beats.len, bytes);
	for (i = 0; i < time_map->beats.len; i++) {
		SET_DOUBLE(ptr, time_map.beats[i].time, bytes);
		SET_DOUBLE(ptr, time_map.beats[i].beat, bytes);
	}
    SET_LONG(ptr, time_sig.len, bytes);
	PAD(ptr, bytes);
	for (i = 0; i < time_sig.len; i++) {
		SET_DOUBLE(ptr, time_sig[i].beat, bytes);
		SET_DOUBLE(ptr, time_sig[i].num, bytes);
		SET_DOUBLE(ptr, time_sig[i].den, bytes);
	}
	SET_LONG(ptr, tracks(), bytes);
	PAD(ptr, bytes); 
	for (i = 0; i < tracks(); i++) {
		track(i)->serialize_track(&ptr, &bytes, midi);
	}
}


void Alg_track::serialize_track(char **ptr_ptr, char *bytes_ptr, bool midi)
{
	// to simplify the code, copy from parameter addresses to locals
	char *ptr = *ptr_ptr;
	long bytes = *bytes_ptr;
	int i, j; // loop counters

	check_buffer(bytes + 32);
	SET_STRING(ptr, "ALGT", 4, bytes);
	SET_INT32(ptr, 0, bytes); // place to write track length
	bug -- write code to set this length
	SET_INT32(ptr, units_are_seconds, bytes);
	SET_INT32(ptr, len, bytes);
	SET_DOUBLE(ptr, beat_dur, bytes);
	SET_DOUBLE(ptr, real_dur, bytes);
	for (j = 0; j < len; j++) {
		check_buffer(24);
		Alg_event *event = (*this)[i];
		SET_INT32(ptr, event->selected, bytes);
		SET_INT32(ptr, event->type, bytes);
		SET_INT32(ptr, event->key, bytes);
		SET_INT32(ptr, event->channel, bytes);
		SET_DOUBLE(ptr, event->time, bytes);
		if (event->is_note()) {
			check_buffer(28);
			Alg_note *note = (Alg_note *) event;
			SET_DOUBLE(ptr, note->pitch, bytes);
			SET_DOUBLE(ptr, note->dur, bytes);
			SET_DOUBLE(ptr, note->loud, bytes);
			SET_INT32(ptr, 0, bytes); // placeholder for no. parameters
			bug -- write code to set this value
			Alg_parameters_ptr parms = note->parameters;
			while (parms) {
				serialize_parameter(parms->parm, &ptr, &bytes);
				parms = parms->next;
			}
		} else {
			assert(event->is_update());
			Alg_update *update = (Alg_update *) event;
			serialize_parameter(update->parameter, &ptr, &bytes);
		}
		check_buffer(7); // maximum padding possible
		PAD(ptr, bytes);
	}
	*ptr_ptr = ptr; // update formal parameter
	*bytes_ptr = bytes;
}


void Alg_track::serialize_parameter(Alg_parameter *parm, char **ptr_ptr, 
									long *bytes_ptr)
{
	char *ptr = *ptr_ptr;
	long bytes = *bytes_ptr;
	// add eight to account for name + zero end-of-string and the
	// possibility of adding 7 padding bytes
	int len = strlen(parm->attr_name()) + 8;
	check_buffer(len);
	SET_STRING(ptr, parm->attr_name(), len, bytes);
	PAD(ptr, bytes);
	switch (parm->attr_type()) {
	case 'r':
		check_buffer(8);
		SET_DOUBLE(ptr, parm->r, bytes);
		break;
	case 's':
		len = strlen(parm->s) + 1;
		check_buffer(len);
		SET_STRING(ptr, parm->s, len, bytes);
		break;
	case 'i':
		check_buffer(4);
		SET_INT32(ptr, parm->i, bytes);
		break;
	case 'l':
		check_buffer(4);
		SET_INT32(ptr, parm->l, bytes);
		break;
	case 'a':
		len = strlen(parm->a) + 1;
		check_buffer(len);
		SET_STRING(ptr, parm->a, len, bytes);
		break;
	}
	*ptr_ptr = ptr;
	*bytes_ptr = bytes;
}


char *Alg_track::get_string(char **p, char *b)
{
	char *s = *p;
	long len = strlen(*p);
	(*p) += len;
    (*b) += len;
	return s;
}


long Alg_track::get_int32(char **p, char *b)
{
	long i = *((long *) (p));
	(*p) += 4;
	(*b) += 4;
	return i;
}


double Alg_track::get_double(char **p, char *b)
{
	double d = *((long *) (p));
	(*p) += 8;
	(*b) += 8;
	return d;
}


#define GETPAD(p, b) { \
    while ((p) & 7) { (p)++; (b)++; } assert(bytes <= len); }

#define CHECK_BUFFER(needed) assert(bytes + (needed) <= len)

Alg_track *Alg_track::unserialize(char *buffer, int len)
{
	assert(len > 8);
	assert(*buffer == 'A'); buffer++;
	assert(*buffer == 'L'); buffer++;
	assert(*buffer == 'G'); buffer++;
	long bytes = 4;
    if (*buffer == 'S') {
		Alg_seq *seq = new Alg_seq;
		seq->unserialize_seq(buffer, len);
		return seq;
	} else {
		assert(*buffer == 'T');
		Alg_track *track = new Alg_track;
		track->unserialize_track(buffer, len);
		return track;
	}
}


void Alg_seq::unserialize_seq(char *buffer, int len)
{
	buffer += 4; // we've already seen 'ALGS' header
	long bytes = 4; // count how many bytes we've read
	CHECK_BUFFER(28);
	long i = get_int32(&buffer, &bytes);
	assert(len >= i);
	len = i; // len is now just the length of the seq data
	channel_offset_per_track = get_int32(&buffer, &bytes);
	units_are_seconds = get_int32(&buffer, &bytes);
	time_map = Alg_time_map();
	time_map->last_tempo = get_double(&buffer, &bytes);
	time_map->last_tempo_flag = (bool) get_int32(&buffer, &bytes);
	long beats = get_int32(&buffer, &bytes);
	CHECK_BUFFER(beats * 16 + 4);
	for (i = 0; i < beats; i++) {
		double time = get_double(&buffer, &bytes);
		double beat = get_double(&buffer, &bytes);
		time_map->insert_beat(time, beat);
	}
	long time_sig_len = get_int32(&buffer, &bytes);
	CHECK_BUFFER(time_sig_len * 24 + 8);
	for (i = 0; i < time_sig_len; i++) {
		double beat = get_double(&buffer, &bytes);
		double num = get_double(&buffer, &bytes);
		double den = get_double(&buffer, &bytes);
		time_sig.insert(beat, num, den);
	}
	long tracks_num = get_int32(&buffer, &bytes);
	GETPAD(buffer, bytes);
	track_list.add_track(tracks_num - 1); // create tracks_num tracks
	for (i = 0; i < tracks_num; i++) {
		track(i)->unserialize_track(buffer, len - bytes);
	}
	assert(bytes == len);
}


void Alg_track::unserialize_track(char *buffer, int len)
{
	int bytes = 0;
	CHECK_BUFFER(32);
	assert(*buffer == 'A'); buffer++;
	assert(*buffer == 'L'); buffer++;
	assert(*buffer == 'G'); buffer++;
	assert(*buffer == 'T'); buffer++;
	bytes = 4;
	int i = get_int32(&buffer, &bytes);
	assert(i <= len);
	len = i; // len is now length of this particular track
	units_are_seconds = (bool) get_int32(&buffer, &bytes);
	int event_count = get_int32(&buffer, &bytes);
	beat_dur = get_double(&buffer, &bytes);
	real_dur = get_double(&buffer, &bytes);
	for (i = 0; i < event_count; i++) {
		CHECK_BUFFER(24);
		long selected = get_int32(&buffer, &bytes);
		char type = (char) get_int32(&buffer, &bytes);
		long key = get_int32(&buffer, &bytes);
		long channel = get_int32(&buffer, &bytes);
		double time = get_double(&buffer, &bytes);
		if (type == 'n') {
			CHECK_BUFFER(28);
			Alg_note *note = Alg_note();
			note->selected = selected;
			note->key = key;
			note->channel = channel;
			note->time = time;
			note->pitch = get_double(&buffer, &bytes);
			note->dur = get_double(&buffer, &bytes);
			note->loud = get_double(&buffer, &bytes);
			long param_num = get_int32(&buffer, &bytes);
			int j;
			// this builds a list of parameters in the correct order
			Alg_parameters_ptr *list = &parameters;
			for (j = 0; j < param_num; j++) {
				unserialize_parameter(&(note->parm), &buffer, &bytes, len);
				delete p;
				list = &((*list)->next);
			}
			*list = NULL;
			append(note);
		} else {
			assert(type == 'u');
			Alg_update *update = Alg_update();
			unserialize_parameter(&(update->parameter), &buffer, &bytes, len);
			append(update);
		}
		GET_PAD();
	}
	assert(bytes == len);
}


void Alg_track::unserialize_parameter(Alg_parameter_ptr parm_ptr, 
					char **buffer_ptr, long *bytes_ptr, long len)
{
	// copy from references to locals, do not change this; CHECK_BUFFER
	// relies on buffer, bytes, and len
	char *buffer = *buffer_ptr;
	long bytes = *bytes_ptr;
	// Note: this routine may read beyond len if strings are not terminated
	int len = strlen(*buffer); // get length of attribute
	CHECK_BUFFER(len + 1);
	char *attr = get_string(&buffer, &bytes);
	GET_PAD();
	parm_ptr->attr = symbol_table.insert_string(attr);
	switch (parm_ptr->attr_type()) {
	case 'r':
		CHECK_BUFFER(8);
		parm_ptr->r = get_double(&buffer, &byte);
		break;
	case 's':
		len = strlen(buffer) + 1;
		CHECK_BUFFER(len);
		parm_ptr->s = heapify(get_string(&buffer, &byte);
		break;
	case 'i':
		CHECK_BUFFER(4);
		parm_ptr->i = get_int32(&buffer, &byte);
		break;
	case 'l':
		CHECK_BUFFER(4);
		parm_ptr->l = get_int32(&buffer, &byte);
		break;
	case 'a':
		len = strlen(buffer) + 1;
		CHECK_BUFFER(len);
		parm_ptr->a = symbol_table.insert_string(get_string(&buffer, &byte);
		break;
	}
	*buffer_ptr = buffer;
	*bytes_ptr = bytes;
}


void Alg_track::set_time_map(Alg_time_map *map)
{
    if (map == NULL)
	{
	    time_map = &const_map;
	    return;
    }
    time_map = map;
}


void Alg_track::convert_to_beats()
// modify all times and durations in notes to beats
{
    if (units_are_seconds) {
        units_are_seconds = false;
		long i;

		for (i = 0; i < length(); i++) {
	    	Alg_event_ptr e = events[i];
			double beat = time_map->time_to_beat(e->time);
			if (e->is_note()) {
				Alg_note_ptr n = (Alg_note_ptr) e;
				n->dur = time_map->time_to_beat(n->time + n->dur) - beat;
			}
			e->time = beat;
		}
	}
}


void Alg_track::convert_to_seconds()
// modify all times and durations in notes to seconds
{
    if (!units_are_seconds) {
        units_are_seconds = true;
		long i;
		for (i = 0; i < length(); i++) {
			Alg_event_ptr e = events[i];
			double time = time_map->beat_to_time(e->time);
			if (e->is_note()) {
				Alg_note_ptr n = (Alg_note_ptr) e;
				n->dur = time_map->beat_to_time(n->time + n->dur) - time;
				n->time = time;
			}
		}
	}
}


Alg_event *Alg_track::create_note(double time, int channel, int identifier, 
                           double pitch, double loudness, double duration)
{
    Alg_note *note = new Alg_note();
	note->time = time;
    note->chan = channel;
	note->set_identifier(identifier);
	note->pitch = pitch;
	note->loud = loudness;
	note->dur = duration;
    return note;
}


Alg_event *Alg_track::create_update(double time, int channel, int identifier)
{
    Alg_update *update = new Alg_update();
    update->time = time;
	update->chan = channel;
	update->set_identifier(identifier);
    return update;
}


Alg_track *Alg_track::cut(double t, double len, bool all)
{
    Alg_track *track = new Alg_track(time_map);
    track->units_are_seconds = units_are_seconds;
    int i;
    int move_to = 0;
    for (i = 0; i < length(); i++) {
        Alg_event_ptr event = events[i];
        Alg_note_ptr note = (Alg_note_ptr) event;
        if (event->overlap(t, len, all)) {
            event->time -= t;
            track->append(event);
        } else { // if we're not cutting this event, move it to 
                 // eliminate the gaps in events left by cut events
		events[move_to] = event;
            // adjust times of events after t + len
            if (event->time > t + len) event->time -= len;
            move_to++;
        }
    }
    if (move_to != this->len) { // we cut at least one note
        sequence_number++; // Alg_event_lists based on this track become invalid
    }
    this->len = move_to; // adjust length since we removed events
    return track;
}


Alg_track *Alg_track::copy(double t, double len, bool all)
{
    Alg_track *track = new Alg_track(time_map);
    track->units_are_seconds = units_are_seconds;
    int i;
    int move_to = 0;
    for (i = 0; i < length(); i++) {
        Alg_event_ptr event = events[i];
        if (event->overlap(t, len, all)) {
            Alg_event_ptr event = copy_event(event);
            event->time -= t;
            track->append(event);
        }
    }
    return track;
}


void Alg_track::paste(double t, Alg_event_list *seq)
{
    // type check seq:
    assert(seq->get_type() != 's');
    // Note: in the worst case, seq may contain notes
    // that start almost anytime up to it's duration,
    // so the simplest algorithm is simply a sequence
    // of inserts. If this turns out to be too slow,
    // we can do a merge sort in the case that seq
    // is an Alg_track (if it's an Alg_event_list, we
    // are not guaranteed that the events are in time order
    
    int i;
    for (i = 0; i < length(); i++) {
        if (events[i]->time > t) {
            events[i]->time += t;
        }
    }
    for (i = 0; i < seq->length(); i++) {
        Alg_event *new_event = copy_event(events[i]);
        new_event->time += t;
        insert(new_event);
    }
}


void Alg_track::merge(double t, Alg_event_list *seq)
{
	for (int i = 0; i < seq->length(); i++) {
		Alg_event *new_event;
		if (events[i]->is_note())
		{
            new_event = new Alg_note((Alg_note &)seq->events[i]);
		    ((Alg_note *)new_event)->time = ((Alg_note *)seq->events[i])->time + t;
            ((Alg_note *)new_event)->dur = ((Alg_note *)seq->events[i])->dur + t;
		}
		else
		    new_event = new Alg_update((Alg_update &)events[i]);
		new_event->time += t;
        insert(new_event);
	}
}


void Alg_track::clear(double t, double len, bool all)
{
    int i;
    int move_to = 0;
    for (i = 0; i < length(); i++) {
        Alg_event_ptr event = events[i];
        if (event->overlap(t, len, all)) {
            delete events[i];
        } else { // if we're not clearing this event, move it to 
                 // eliminate the gaps in events left by cleared events
		events[move_to] = event;
            // adjust times of events after t + len
            if (event->time > t + len) event->time -= len;
            move_to++;
        }
    }
    if (move_to != this->len) { // we cleared at least one note
        sequence_number++; // Alg_event_lists based on this track become invalid
    }
    this->len = move_to; // adjust length since we removed events
}


void Alg_track::silence(double t, double len, bool all)
{
    int i;
    int move_to = 0;
    for (i = 0; i < length(); i++) {
        Alg_event_ptr event = events[i];
        if (event->overlap(t, len, all)) {
            delete events[i];
        } else { // if we're not clearing this event, move it to 
                 // eliminate the gaps in events left by cleared events
		events[move_to] = event;
            move_to++;
        }
    }
    if (move_to != this->len) { // we cleared at least one note
        sequence_number++; // Alg_event_lists based on this track become invalid
    }
    this->len = move_to; // adjust length since we removed events
}


void Alg_track::insert_silence(double t, double len)
{
    int i;
    for (i = 0; i < length(); i++) {
        Alg_event_ptr event = events[i];
        if (event->time >= t) event->time += len;
    }
}


Alg_event_list *Alg_track::find_events_in_range(double t, double len, bool all)
{
    int i;
    Alg_event_list *list = new Alg_event_list(this);
    if (units_are_seconds) { // t and len are seconds
        list->set_real_dur(len);
        list->set_beat_dur(get_time_map()->time_to_beat(t + len) - 
                           get_time_map()->time_to_beat(t));
    } else { // t and len are beats
        list->set_real_dur(get_time_map()->beat_to_time(t + len) -
                           get_time_map()->beat_to_time(t));
        list->set_beat_dur(len);
    }
    for (i = 0; i < length(); i++) {
        Alg_event_ptr event = events[i];
        if (event->overlap(t, len, all)) {
            list->append(event);
        }
    }
    return list;
}


Alg_event_list *Alg_track::find(double t, double len, bool all,
                         long channel_mask, int event_type_mask)
{
    int i;
    Alg_event_list *list = new Alg_event_list(this);
    if (units_are_seconds) { // t and len are seconds
        list->set_real_dur(len);
        list->set_beat_dur(get_time_map()->time_to_beat(t + len) - 
                           get_time_map()->time_to_beat(t));
    } else { // t and len are beats
        list->set_real_dur(get_time_map()->beat_to_time(t + len) -
                           get_time_map()->beat_to_time(t));
        list->set_beat_dur(len);
    }
    for (i = 0; i < length(); i++) {
        Alg_event_ptr event = events[i];
        if (event->overlap(t, len, all)) {
            if ((channel_mask == 0 || 
                 (event->chan < 32 && 
                  (channel_mask & (1 << event->chan)))) &&
                ((event_type_mask == 0 ||
                  (event_type_mask & (1 << event->get_type_code()))))) {
                list->append(event);
            }
        }
    }
    return list;
}


void Alg_time_sigs::expand()
{
    max = (max + 5);   // extra growth for small sizes
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
    max = (max + 5);   // extra growth for small sizes
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
		tracks[len] = new Alg_track;
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
	max = 0;               // Modified by Ning Hu Nov.19 2002
}


void Alg_seq::insert_silence(double t, double len, bool all)
{
    int i;
    for (i = 0; i < length(); i++) {
        Alg_event_ptr event = events[i];
        if (all && (event->time >= t))
			event->time += len;
        if (!all && (event->time > t))
			event->time += len;
    }
}


Alg_seq::~Alg_seq()
{
	int i, j;
	// Tracks does not delete Alg_events elements
	for (j = 0; j < tracks_list.length(); j++) {
		Alg_events_ptr notes = tracks_list[j];
        // Alg_events does not delete notes 
        for (i = 0; i < notes->length(); i++)
            delete notes->events[i];
		delete notes;
	}
}


long Alg_seq::seek_time(double time, int track_num)
// find index of first score event after time
{
    long i;
	Alg_events &notes = *tracks_list[track_num];
	for (i = 0; i < notes.length(); i++) {
		if (notes[i]->time > time) {
			break;
		}
    }
    return i;
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
    int i = time_map->locate_time(time); // i is insertion point
    if (i < time_map->beats.len && within(time_map->beats[i].time, time, 0.000001)) {
        // replace beat if time is already in the map
        time_map->beats[i].beat = beat;
    } else {
        Alg_beat point;
        point.beat = beat;
        point.time = time;
        time_map->beats.insert(i, &point);
    }
    // beats[i] contains new beat
    // make sure we didn't generate a zero tempo.
    // if so, space beats by one microbeat as necessary
    long j = i;
    while (j < time_map->beats.len &&
        time_map->beats[j - 1].beat + 0.000001 >= time_map->beats[j].beat) {
        time_map->beats[j].beat = time_map->beats[j - 1].beat + 0.000001;
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
    double time = time_map->beat_to_time(beat);
    long i = time_map->locate_time(time);
    if (i >= time_map->beats.len || !within(time_map->beats[i].time, time, 0.000001)) {
        insert_beat(time, beat);
    }
    // now i is index of beat where tempo will change
    if (i == time_map->beats.len - 1) {
        time_map->last_tempo = tempo;
        time_map->last_tempo_flag = true;
    } else { // adjust all future beats
        // compute the difference in beats
        double diff = time_map->beats[i + 1].beat - time_map->beats[i].beat;
        // convert beat difference to seconds at new tempo
        diff = diff / tempo;
        // figure out old time difference:
        double old_diff = time_map->beats[i + 1].time - time;
        // compute difference too
        diff = diff - old_diff;
        // apply new_diff to score and beats
        while (i < time_map->beats.len) {
            time_map->beats[i].time = time_map->beats[i].time + diff;
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
    tracks_list[track_num]->insert(event);
/*
    if (event->is_note()) {
        Alg_note_ptr n = (Alg_note_ptr) event;
        trace("note %d at %g for %g\n", n->get_identifier(), n->time, n->dur);
    }
 */
}


bool Alg_seq::set_tempo(double tempo, double start_beat, double end_beat)
// set tempo from start_beat to end_beat
{
    if (start_beat >= end_beat) return false;
    convert_to_beats();
    // algorithm: insert a beat event if necessary at start_beat
    // and at end_beat
    // delete intervening map elements
    // change the tempo
    insert_beat(time_map->beat_to_time(start_beat), start_beat);
    insert_beat(time_map->beat_to_time(end_beat), end_beat);
    long start_x = time_map->locate_beat(start_beat) + 1;
    long stop_x = time_map->locate_beat(end_beat) + 1;
    while (stop_x < time_map->beats.len) {
        time_map->beats[start_x] = time_map->beats[stop_x];
        start_x++;
        stop_x++;
    }
    time_map->beats.len = start_x; // truncate the map to new length
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
    current = new long[tracks_list.length()];
	int i;
    for (i = 0; i < tracks_list.length(); i++) {
        current[i] = 0;
	}    
}


Alg_event_ptr Alg_seq::iteration_next()
    // return the next event in time from any track
{
    Alg_events_ptr tr;  // a track
    long cur;           // a track index
    // find lowest next time of any track:
    double next = 1000000.0;
	int i, track;
    for (i = 0; i < tracks_list.length(); i++) {
        tr = tracks_list[i];
        cur = current[i];
        if (cur < tr->length() && (*tr)[cur]->time < next) {
            next = (*tr)[cur]->time;
            track = i;
        }
    }
	if (next < 1000000.0) {
		return (*tracks_list[track])[current[track]++];
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
    for (i = 0; i < tracks_list.length(); i++) {
        sum = sum + tracks_list[i]->length();
    }
    // preallocate array for efficiency:
    Alg_event_ptr *notes = new Alg_event_ptr[sum];
	iteration_begin();
    long notes_index = 0;

	Alg_event_ptr event;
    while (event = iteration_next()) {
        notes[notes_index++] = event;
    }
    tracks_list.reset(); // don't need them any more
	tracks_list.add_track(0);
    tracks_list[0]->set_events(notes, sum, sum);
	iteration_end();
}


// sr_letter_to_type = {"i": 'Integer', "r": 'Real', "s": 'String',
//                     "l": 'Logical', "a": 'Symbol'}


