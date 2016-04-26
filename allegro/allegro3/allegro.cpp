I DECIDED TO JUST EDIT THIS CODE RATHER THAN ADD A LOT OF COMMENTS
OR EXPLANATION. (THERE ARE PLENTY OF PLACES I INSERTED CAPS LIKE
THIS TOO.) IF YOU HAVE MADE CHANGES TOO, DO A DIFF TO FIND
MY CHANGES. IF YOU NEED THE ORIGINAL TO FIND MY CHANGES, LET ME
KNOW. -ROGER

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


bool Alg_event::is_note()
{
	return (type == 'n');
}


bool Alg_event::is_update()
{
	return (type == 'u');
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


AFTER LOOKING AT ALL THESE, I THINK WE NEED A PRIVATE FUNCTION
IN Alg_parameters TO REPLACE ALL THESE WHILE LOOPS -- TOO MUCH
CODE DUPLICATION FOR MY TASTE.

(THIS FUNCTION SHOULD BE MOVED TO THE OTHER Alg_parameters
IMPLEMENTATIONS)

Alg_parameter_ptr Alg_parameters::find(Alg_attribute *attr)
{
    assert(attr);
    Alg_parameters_ptr temp = this;
    while (temp) {
        if (temp->parm.attr == attr) {
            return &(temp->parm);
        }
    }
    return NULL;
}


bool Alg_event::has_attribute(char *a)
{
    assert(is_note());
    assert(a); // must be non-null
    Alg_note* note = (Alg_note *) this;
    Alg_attribute attr = symbol_table.insert_string(a);
    Alg_parameter_ptr parm = note->parameters->find(attr);
    return parm != NULL;
}


char Alg_event::get_attribute_type(char *a)
// note that attribute names imply the type
// so get_attribute_type() is a pure function
{
    assert(a);
    return name[strlen(name) - 1];
}


char *Alg_event::get_string_value(char *a, char *default = NULL)
{
	assert(is_note());
    assert(a); // must be non-null
	Alg_note* note = (Alg_note *) this;
    Alg_attribute attr = symbol_table.insert_string(a);
    assert(a[0] == 's'); // must be of type string
    #RBD# TODO: I'VE REPLACED THE LOOP CODE HERE, BUT NOT IN THE
	FOLLOWING SET OF "get_*" METHODS
    Alg_parameter_ptr parm = note->parameters->find(attr);
    if (parm) return parm->parm.s;
    return default;
}


double Alg_event::get_real_value(char *a, double default = 0.0)
{	
	assert(is_note());
    assert(a); // must be non-null
	Alg_note* note = (Alg_note *) this;
	Alg_parameters_ptr temp = note->parameters;
    Alg_attribute attr = symbol_table.insert_string(a);
    assert(a[0] == 'r'); // must be of type real
    while (temp)
	{
        if (temp->parm.attr == attr)
		    return temp->parm.r;
		temp = temp->next;
	}
	return default;
}


bool Alg_event::get_logical_value(char *a, bool default = false)
{	
	assert(is_note());
    assert(a); // must be non-null
	Alg_note* note = (Alg_note *) this;
	Alg_parameters_ptr temp = note->parameters;
    Alg_attribute attr = symbol_table.insert_string(a);
    assert(a[0] == 'l'); // must be of type logical
    while (temp)
	{
        if (temp->parm.attr == a)
		    return temp->parm.l;
		temp = temp->next;
	}
	return default;
}


long Alg_event::get_integer_value(char *a, long default = 0)
{	
	assert(is_note());
	assert(a); // must be non-null
	Alg_note* note = (Alg_note *) this;
	Alg_parameters_ptr temp = note->parameters;
    Alg_attribute attr = symbol_table.insert_string(a);
    assert(a[0] == 'i'); // must be of type integer
    while (temp)
	{
        if (temp->parm.attr == a)
		    return temp->parm.i;
		temp = temp->next;
	}
	return default;
}


char *Alg_event::get_atom_value(char *a, char *default = NULL)
{	
	assert(is_note());
	Alg_note* note = (Alg_note *) this;
	Alg_parameters_ptr temp = note->parameters;
    Alg_attribute attr = symbol_table.insert_string(a);
    assert(a[0] == 'a'); // must be of type atom
    while (temp)
	{
        if (temp->parm.attr == a)
		    return temp->parm.a;
		temp = temp->next;
	}
	// if default is a string, convert to an atom (unique
      // string in symbol table) and return it
	return (default == NULL ? NULL :
              symbol_table.insert_string(default));
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
	Alg_update* update= (Alg_update *) this;
	return update->parameter.attr_name();
}


char Alg_event::get_update_type()
{
	assert(is_update());
	Alg_update* update= (Alg_update *) this;
	return update->parameter.attr_type();
}


char *Alg_event::get_string_value()
{
	assert(is_update());
	Alg_update* update= (Alg_update *) this;
    assert(update.attr_type() == 's');
	return update->parameter.s;
}


double Alg_event::get_real_value()
{
	assert(is_update());
	Alg_update* update= (Alg_update *) this;
    assert(update.attr_type() == 'r');
	return update->parameter.r;
}


bool Alg_event::get_logical_value()
{
	assert(is_update());
	Alg_update* update= (Alg_update *) this;
    assert(update.attr_type() == 'l');
	return update->parameter.l;
}


long Alg_event::get_integer_value()
{
	assert(is_update());
	Alg_update* update= (Alg_update *) this;
    assert(update.attr_type() == 'i');
	return update->parameter.i;
}


char *Alg_event::get_atom_value()
{
	assert(is_update());
	Alg_update* update= (Alg_update *) this;
    assert(update.attr_type() == 'a');
	return update->parameter.a;
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

void Alg_event::set_parameter(Alg_parameter_ptr new_parameter)
{
    Alg_parameter_ptr parm;
    if (is_note()) {
        parm = note->parameters->find(new_parameter->attr);
        if (!parm) {
            Alg_note_ptr note = (Alg_note_ptr) this;
            note->parameters = new Alg_parameters(note->parameters);
            parm = note->parameters;
        }
    } else { // update
        Alg_update_ptr update = (Alg_update_ptr) this;
        parm = &(update->parameter;
    }
    parm = *parm; // copy entire parameter
}


NOTE THAT SET_STRING_VALUE BECOMES A MEMBER FUNCTION OF Alg_event, AND
OPERATES ON EITHER A NOTE OR AN UPDATE. SIMILAR CHANGES
SHOULD BE APPLIED TO ALL set_*_value METHODS

void Alg_event::set_string_value(char *a, char *value)
{
    assert(a); // must be non-null
    Alg_attribute attr = symbol_table.insert_string(a);
    assert(attr[0] == 's');
    Alg_parameter parm;
    parm.set_attr(attr);
    parm.s = value;
    set_parameter(&parm);
    parm->s = NULL; // do this to prevent string from being freed
}

(CHANGE THIS TO LOOK LIKE set_string_value)
void Alg_note::set_real_value(char *attr, double value)
{
    Alg_parameters ps = Alg_parameters(parameters);
    ps.parm.set_attr(attr);
    ps.parm.r = value;
	parameters = &ps;
}

(CHANGE THIS TO LOOK LIKE set_string_value)
void Alg_note::set_logical_value(char *attr, bool value)
{
    Alg_parameters ps = Alg_parameters(parameters);
    ps.parm.set_attr(attr);
    ps.parm.l = value;
	parameters = &ps;
}


(CHANGE THIS TO LOOK LIKE set_string_value)
void Alg_note::set_integer_value(char *attr, long value)
{
    Alg_parameters ps = Alg_parameters(parameters);
    ps.parm.set_attr(attr);
    ps.parm.i = value;
	parameters = &ps;
}


(CHANGE THIS TO LOOK LIKE set_string_value)
void Alg_note::set_atom_value(char *attr, char *atom)
{
    Alg_parameters ps = Alg_parameters(parameters);
    ps.parm.set_attr(attr);
    ps.parm.a = atom;
	parameters = &ps;
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
    // Note: if the new event is the last one, the assignment
    // events[i] = event; (below) will never execute, so just
    // in case, we do the assignment here. events[len] will
    // be replaced during the memmove() operation below if
    // this is not the last event.
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
    max = (max + 5);   // extra growth for small sizes
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
    if (attr_type() == 's' && s) {
        delete[] s;
    }
}


void Alg_parameter::copy(Alg_parameter_ptr parm)
{
    *this = *parm; // copy all fields
    // if the value is a string, copy the string
    if (attr_type() == 's') {
        s = heapify(s);
    }
}


Alg_note::Alg_note(Alg_note_ptr note)
{
    *this = *note; // copy all fields
    // parameters is now a shared pointer. We need to copy the 
    // parameters
    Alg_parameters_ptr *next_param_ptr = parameters;
    while (*next_param_ptr) {
        Alg_parameters_ptr new_params = new Alg_parameters((*next_param_ptr)->next);
        new_params->parm.copy((*next_param_ptr)->parm); // copy the attribute and value
        next_param_ptr = &(new_params->next);
    }
}


Alg_update::Alg_update(Alg_update_ptr update)
{
    *this = *update; // copy all fields
    // parameter requires careful copy to possibly duplicate string value:
    this->parameter.copy(&(note->parameter));
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
	if (this == &const_map)
	    return false;

    //***TODO*** SEE CORRESPONDING FUNCTION IN ALG_SEQ
	return true;
}


bool Alg_time_map::set_tempo(double tempo, double start_beat, double end_beat)
{
    if (this == &const_map)
	    return false;

	//***TODO***  SEE CORRESPONDING FUNCTION IN ALG_SEQ
	return true;
}


Alg_event_list::~Alg_event_list()
{
    // Note that we delete the array, but not the events themselves
    if (events)
	    delete[] events;
}


void Alg_event_list::set_start_time(Alg_event *event, double t)
{
    // Note: there are three cases: Alg_event_list, Alg_track, and Alg_seq
    // For Alg_event_list, find the owner and do the update there
    // For Alg_track, change the time and move the event to the right place
    // For Alg_seq, find the track and do the update there
    
    Alg_track &track;
    long index;
    if (type == 'e') { // this is an Alg_event_list
        // make sure the owner has not changed its event set
        assert(events_owner && 
               sequence_number == events_owner->sequence_number);
        // do the update on the owner
        events_owner->set_start_time(event, t);
        return;
    } else if (type == 't') { // this is an Alg_track
        // find the event in the track
        track = *this;
        // this should be a binary search since events are in time order
        // probably there should be member function to do the search
        for (index = 0; index < len; index++) {
            if (track[index] == event) goto found_event;
        }
    } else { // type == 's', an Alg_seq
        Alg_seq_ptr seq = (Alg_seq_ptr) this;
        for (i = 0; i < seq->tracks(); i++) {
            track = seq->tracks_list[i];
            // if you implemented binary search, you could call it
            // instead of this loop too.
            for (index = 0; index < len; index++) {
                if (track[index] == event) goto found_event;
            }
        }
    }
    assert(false); // event not found seq or track!
  found_event:
    // at this point, track[index] == event
    // we could be clever and figure out exactly what notes to move
    // but it is simpler to just remove the event and reinsert it:
    memmove(&track[index + 1], &track[index],
            sizeof(Alg_event_ptr) * (len - j - 1));
    event->time = t;
    track.insert(event);
}


Alg_track::Alg_track(Alg_time_map *map)
{
    if (map == NULL)
	{
	    time_map = &const_map;
	    return;
    }
    time_map = map;
}


Alg_track::Alg_track(Alg_track &track)
{
    TRACKS OWN THEIR EVENTS, SO THIS COPY CONSTRUCTOR SHOULD
    COPY ALL OF THE EVENTS, NOT POINTERS TO EVENTS
    *events = *(track.events);
    time_map = track.time_map;
    units_are_seconds = track.units_are_seconds;
}


void Alg_track::serialize(char **buffer, int *len, bool midi)
{
    // ***TODO***
}


bool Alg_track::unserialize(char *buffer, int len)
{
    // ***TODO***
	return true;
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
		long i, j;

WHY ARE THERE TWO NESTED LOOPS HERE. A TRACK HAS ONE ARRAY OF
EVENT POINTERS. MAYBE THIS CODE WAS COPIED FROM Alg_seq -- AN
Alg_seq HAS AN ARRAY OF TRACKS, EACH OF WHICH HAS AN ARRAY OF
EVENTS.
		for (j = 0; j < events->len; j++) {
			Alg_events &notes = events[j];
			for (i = 0; i < notes.len; i++) {
				Alg_event_ptr e = notes[i];
				double beat = time_map->time_to_beat(e->time);
				if (e->is_note()) {
					Alg_note_ptr n = (Alg_note_ptr) e;
					n->dur = time_map->time_to_beat(n->time + n->dur) - beat;
				}
				e->time = beat;
			}
		}
	}
}


void Alg_track::convert_to_seconds()
// modify all times and durations in notes to seconds
{
    if (!units_are_seconds) {
        units_are_seconds = true;
		long i, j;

SEE NOTE ABOVE ABOUT NESTED LOOPS.

		for (j = 0; j < events->len; j++) {
			Alg_events &notes = events[j];
			for (i = 0; i < notes.len; i++) {
				Alg_event_ptr e = notes[i];
				double time = time_map->beat_to_time(e->time);
				if (e->is_note()) {
					Alg_note_ptr n = (Alg_note_ptr) e;
					n->dur = time_map->beat_to_time(n->time + n->dur) - time;
					n->time = time;
				}
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


void Alg_track::add(int track, Alg_event *event)
{
    events->insert(event);
	// ***TODO*** take care of "track"
    I THINK AFTER MERGING Alg_events with Alg_event_list, THIS
    METHOD WILL BECOME SIMPLY:
    insert(event);
QUESTION: WHEN DO WE INCREMENT THE SEQUENCE NUMBER? I'M LEANING
TOWARD THE FOLLOWING: the Alg_event_list is just a subset of the
events in a track or seq. The object should be valid as long as
it remains a subset (and therefore there can be no dangling
pointers to freed events). THEREFORE, only increment the 
sequence number when you REMOVE an event from the track or seq.
}


Alg_track *Alg_track::cut(double t, double len, bool all)
{
    Alg_track *track = new Alg_track(time_map);
    track->units_are_seconds = units_are_seconds;
    int i;
    int move_to = 0;
    for (i = 0; i < length(); i++) {
        Alg_event_ptr event = &events[i];
        Alg_note_ptr note = (Alg_note_ptr) event;
        PLEASE CHECK THIS CONDITIONAL -- ANYTHING THIS BIG HAS TO BE
        WRONG, AT LEAST THE FIRST DRAFT!
        SINCE THIS EXPRESSION IS GOING TO GET USED ELSEWHERE, IT
        SHOULD BE A NEW METHOD: overlap(t, len, all)
        if (all && // event overlaps region if event starts in region
                   // or note ends in region OR note starts before and
                   // ends after
            ((event->time >= t && event->time <= t + len) ||
             (event->is_note() && ((event->time + event->dur >= t) &&
                                   (event->time + event->dur <= t + len) ||
                                   (event->time <= t) && 
                                   (event->time + event->dur >= t + len)))) ||
            // if not all, then note must be within region:
            (event->time >= t && 
             event->time <= t + len &&
             (event->is_update() ||
              event->time + event->dur <= t + len))) {
            track->events->append(event);
            event->time -= t;
        } else { // if we're not cutting this event, move it to 
                 // eliminate the gaps in events left by cut events
		events[move_to] = event;
            // adjust times of events after t + len
            if (event->time > t + len) event->time -= len;
            move_to++;
        }
    }
    if (move_to != events->len) { // we cut at least one note
        sequence_number++; // Alg_event_lists based on this track become invalid
    }
    events->len = move_to; // adjust length since we removed events
    return track;
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


Alg_track *Alg_track::copy(double t, double len, bool all)
{
    Alg_track *track = new Alg_track(time_map);
    track->units_are_seconds = units_are_seconds;
    int i;
    int move_to = 0;
    for (i = 0; i < length(); i++) {
        Alg_event_ptr event = &events[i];
        if (event->overlap(t, len, all)) {
            Alg_event_ptr event = copy_event(event);
            event->time -= t;
            track->events->append(event);
        }
    }
    return track;
}


void Alg_track::paste(double t, Alg_event_list *seq)
{
    // type check seq:
    assert(seg->type != 's');
    // Note: in the worst case, seq may contain notes
    // that start almost anytime up to it's duration,
    // so the simplest algorithm is simply a sequence
    // of inserts. If this turns out to be too slow,
    // we can do a merge sort in the case that seq
    // is an Alg_track (if it's an Alg_event_list, we
    // are not guaranteed that the events are in time order
    
    // ***TODO*** Need to shift units (seconds/beats)
    int i;
    for (i = 0; i < length(); i++) {
        CAN'T WE JUST SAY events[i] INSTEAD OF events->events[i]?
        if (events->events[i]->time > t) {
            events->events[i]->time += get_duration();
        }
    }
    for (i = 0; i < seq->length(); i++) {
        Alg_event *new_event = copy_event(events[i]);
        new_event->time += t;
        events->insert(new_event);
    }
}


void Alg_track::merge(double t, Alg_event_list *seq)
{
	// ***TODO*** Need to shift units (seconds/beats)
	for (int i = 0; i < seq->length(); i++) {
		Alg_event *new_event;
		if (events->events[i]->is_note())
		{
            new_event = new Alg_note((Alg_note &)seq->events->events[i]);
		    ((Alg_note *)new_event)->start = ((Alg_note *)seq->events->events[i])->start + t;
            ((Alg_note *)new_event)->end = ((Alg_note *)seq->events->events[i])->end + t;
		}
		else
		    new_event = new Alg_update((Alg_update &)events->events[i]);
		new_event->time += t;
        events->insert(new_event);
	}
}


void Alg_track::clear(double t, double len, bool all)
{
    int i;
    int move_to = 0;
    for (i = 0; i < length(); i++) {
        Alg_event_ptr event = &events[i];
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
    if (move_to != events->len) { // we cleared at least one note
        sequence_number++; // Alg_event_lists based on this track become invalid
    }
    events->len = move_to; // adjust length since we removed events
    return track;
}


void Alg_track::silence(double t, double len, bool all)
{
    int i;
    int move_to = 0;
    for (i = 0; i < length(); i++) {
        Alg_event_ptr event = &events[i];
        if (event->overlap(t, len, all)) {
            delete events[i];
        } else { // if we're not clearing this event, move it to 
                 // eliminate the gaps in events left by cleared events
		events[move_to] = event;
            move_to++;
        }
    }
    if (move_to != events->len) { // we cleared at least one note
        sequence_number++; // Alg_event_lists based on this track become invalid
    }
    events->len = move_to; // adjust length since we removed events
}



void Alg_track::insert_silence(double t, double len)
{
    int i;
    for (i = 0; i < length(); i++) {
        Alg_event_ptr event = &events[i];
        if (event->time >= t) event->time += len;
    }
}


Alg_event_list *Alg_track::find_events_in_range(double t, double len, bool all)
{
    int i;
    Alg_event_list_ptr list = new Alg_event_list(this);
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
        Alg_event_ptr event = &events[i];
        if (event->overlap(t, len, all) {
            list->append(event);
        }
    }
    return list;
}

NEED TO DECLARE AND IMPLEMENT Alg_event::get_type_code() TO RETURN
0 = Alg_event_note
1 = Alg_event_volume_change
2 = Alg_event_pitch_bend
... OTHER USEFUL TYPES GO HERE
N = Alg_event_other_control
THESE ARE ALL DETERMINED BY ATTRIBUTE NAMES (SEE DOCUMENTATION FOR ALLEGRO)

Alg_event_list *Alg_track::find(double t, double len,
                         int channel_mask, int event_type_mask)
{
    int i;
    Alg_event_list_ptr list = new Alg_event_list(this);
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
        Alg_event_ptr event = &events[i];
        if (event->overlap(t, len, all) {
            if ((channel_mask == 0 || 
                 (event->chan < 32 && 
                  (channel_mask & (1 << event->chan)))) &&
                ((event_type_mask == 0 ||
                  (event_type_mask & (1 << event->get_type_code())))) {
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
	max = 0;               // Modified by Ning Hu Nov.19 2002
}


void Alg_seq::insert_silence(double t, double len, bool all)
{
    for (int i = 0; i < tracks(); i++) {
        track(i)->insert_silence(t, len, all);
    }
    // ***TODO*** expand tempo track by t
    I'M SURE THIS CODE EXISTS TO INSERT AND DELETE TIME SEGMENTS FROM 
    A TEMPO TRACK
}


Alg_seq::~Alg_seq()
{
	int i, j;
	// Tracks does not delete Alg_events elements
	for (j = 0; j < tracks_list.len; j++) {
		Alg_events_ptr notes = tracks_list[j];
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
	Alg_events &notes = *tracks_list[track_num];
	for (i = 0; i < notes.len; i++) {
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
    current = new long[tracks_list.len];
	int i;
    for (i = 0; i < tracks_list.len; i++) {
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
    for (i = 0; i < tracks_list.len; i++) {
        tr = tracks_list[i];
        cur = current[i];
        if (cur < tr->len && (*tr)[cur]->time < next) {
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
    for (i = 0; i < tracks_list.len; i++) {
        sum = sum + tracks_list[i]->len;
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


