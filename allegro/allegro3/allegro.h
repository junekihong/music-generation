ALL MY CORRECTIONS/COMMENTS ARE IN CAPS LIKE THIS. WHERE I ADD COMMENTS
TO BE INCLUDED IN THE CODE, I'LL MARK THE SPOT WITH "#RBD#" YOU SHOULD
TAKE THESE MARKERS OUT WHEN YOU MERGE WITH YOUR VERSION. -RBD

A GENERAL QUESTION: EVENTS CAN END UP WITH NEGATIVE TIMES, E.G. IF YOU
CUT WITH all=true, THEN AN EVENT MIGHT START BEFORE THE BEGINNING OF THE
CUT REGION. IS THIS OK? PROBABLY YES, BUT THAT MEANS THE SEQUENCER MAY
WANT TO CHECK AFTER A PASTE TO SEE IF NOTES ENDED UP WITH NEGATVIE TIMES,
AND IF SO PERHAPS THEY SHOULD BE ALTERED TO START AT ZERO.

// Allegro: music representation system, with
//      extensible in-memory sequence structure
//      upward compatible with MIDI
//      implementations in C++ and Serpent
//      external, text-based representation
//      compatible with Aura

#ifndef __ALLEGRO__
#define __ALLEGRO__
#include <assert.h>

#ifdef NEED_BOOL
#define bool unsigned char
#define true 1
#define false 0
#endif


// are d1 and d2 within epsilon of each other?
bool within(double d1, double d2, double epsilon);


// abstract superclass of Alg_note and Alg_update:
typedef class Alg_event {
protected:
   	bool selected;
    char type; // 'e' event, 'n' note, 'u' update
	long key; // note identifier
	static const char* description; // static buffer for debugging (in Alg_event)
public:
    double time;
    long chan;

    // Note: there is no Alg_event() because Alg_event is an abstract class.
    bool is_note();    // tell whether an Alg_event is a note
    bool is_update();  // tell whether an Alg_event is a parameter update
    char get_type() { return type; }   // return 'n' for note, 'u' for update
    bool get_selected() { return selected; }         
    void set_selected(bool b) { selected = b; }     
    // Note: notes are identified by a (channel, identifier) pair. 
    // For midi, the identifier is the key number (pitch). The identifier
    // does not have to represent pitch; it's main purpose is to identify
    // notes so that they can be named by subsequent update events.
    long get_identifier() { return key; } // get MIDI key or note identifier of note or update
    void set_identifier(long i) { key = i; } // set the identifier
    // In all of these set_ methods, strings are owned by the caller and
    // copied as necessary by the callee. For notes, an attribute/value
    // pair is added to the parameters list. For updates, the single
    // attribute/value parameter pair is overwritten. In all cases, the
    // attribute (first argument) must agree in type with the second arg.
    // The last letter of the attribute implies the type (see below).
    virtual void set_string_value(char *attr, char *value);
    virtual void set_real_value(char *attr, double value);
    virtual void set_logical_value(char *attr, bool value);
    virtual void set_integer_value(char *attr, long value);
    virtual void set_atom_value(char *attr, char *atom);

    // Some note methods. These fail (via assert()) if this is not a note:
    //
    double get_pitch();// get pitch in steps -- use this even for MIDI
    // times are in seconds or beats, depending upon the units_are_seconds 
    // flag in the containing sequence
    double get_start_time(); // get start time in seconds or beats
    double get_end_time();   // get end time in seconds or beats
    double get_duration();   // get duration in seconds or beats
    void set_pitch(double);
    void set_duration(double);

    // Notes have lists of attribute values. Attributes are converted
    // to/from strings in this API to avoid explicit use of Alg_attribute
    // types. Attribute names end with a type designation: 's', 'r', 'l',
    // 'i', or 'a'.
    //
    bool has_attribute(char *attr);      // test if note has attribute/value pair
    char get_attribute_type(char *attr); // get the associated type: 
        // 's' = string, 
        // 'r' = real (double), 'l' = logical (bool), 'i' = integer (long),
        // 'a' = atom (char *), a unique string stored in Alg_seq
    #RBD# I ADDED DEFAULT VALUES TO ALL THESE FUNCTIONS
    const char *get_string_value(char *attr, char *default = NULL); // get the string value
    double get_real_value(char *attr, double default = 0.0 // get the real value
    bool get_logical_value(char *attr, bool default = false);  // get the logical value
    long get_integer_value(char *attr, long default = 0);  // get the integer value
    char *get_atom_value(char *attr, char *default = NULL); // get the atom value
    void delete_attribute(char *attr);   // delete an attribute/value pair
        // (ignore if no matching attribute/value pair exists)

    // Some attribute/value methods. These fail if this is not an update.
    // Attributes are converted to/from strings to avoid explicit use
    // of Alg_attribute types.
    //
    #RBD# NOTE THAT THIS IS STATIC: (YOU CAN CALL IT AS A PURE FUNCTION)
    static char *get_attribute();    // get the update's attribute
    char get_update_type();   // get the update's type: 's' = string, 
        // 'r' = real (double), 'l' = logical (bool), 'i' = integer (long),
        // 'a' = atom (char *), a unique string stored in Alg_seq
    char *get_string_value(); // get the update's string value
        // Notes: Caller does not own the return value. Do not modify.
        // Do not use after underlying Alg_seq is modified.
    double get_real_value();  // get the update's real value
    bool get_logical_value(); // get the update's logical value
    long get_integer_value(); // get the update's integer value
    char *get_atom_value();   // get the update's atom value
        // Notes: Caller does not own the return value. Do not modify.
        // The return value's lifetime is forever.
    const char *GetDescription(); // computes a text description of this event
	// the result is in a static buffer, not thread-safe, just for debugging.
    Alg_event() { selected = false; }
    virtual ~Alg_event() {}
} *Alg_event_ptr;



[YOU SHOULD MERGE THIS WITH ALG_EVENT_LIST, IE INSTEAD OF HAVING EVERY 
ALG_EVENT_LIST POINT TO AN INSTANCE OF ALG_EVENTS, JUST PUT THE 
MEMBER VARIABLES MAX, LEN, AND EVENTS IN ALG_EVENT_LIST] NO, ON SECOND
THOUGHT, YOU ARE RIGHT -- Alg_events is a dynamic array. Alg_seq 
does not have one of these -- instead it has a dynamic array of tracks.
So it's best I think to have this as a separate object pointed to and
"owned" by Alg_event_list and Alg_track, but the pointer will be NULL
for an Alg_seq.

// a sequence of Alg_event objects
typedef class Alg_events {
private:
    long max;
    void expand();
    // #RBD# insert and append are for use by Alg_track and Alg_seq
    // Allegro users should consider Alg_events_list to be read-only.
    void insert(Alg_event_ptr event);
    void append(Alg_event_ptr event);
public:
    long len;
    Alg_event_ptr *events; // events is array of pointers
    Alg_event_ptr &operator[](int i) {
        assert(i >= 0 && i < len);
        return events[i];
    }
    Alg_events() {
        max = len = 0;
        events = NULL;
    }
    ~Alg_events();
    void set_events(Alg_event_ptr *e, long l, long m) {
        if (events) delete [] events;
        events = e; len = l; max = m; }
} *Alg_events_ptr;


// An Alg_event_list is essentially a set of events. The events are
// unordered (in time), and only references (pointers) are in the set.
// The actual event objects are "owned" by an Alg_track or Alg_seq.
// In addition to a set of pointers, an Alg_event_list has a duration
// measured in beats (beat_dur) and in seconds (real_dur), and an
// events_owner, the owner of the event data. The events_owner may
// have a tempo map and other relevant information not carried by the
// Alg_event_list.
//
class Alg_event_list {
protected:
    #RBD#: // The type tells us the actual type of this object.
    // Note that we could avoid this member variable by having
    // a virtual function, get_type(), that subclasses override,
    // but by putting this member variable here, the debugger
    // will display the type character, making debugging a bit easier.
    char type; // 'e' Alg_event_list, 't' Alg_track, 's' Alg_seq
	static const char *last_error_message;
	Alg_track *events_owner; // if this is an Alg_event_list,
	    // the events are owned by an Alg_track or an Alg_seq
	int sequence_number;   // this sequence number is incremented
	    // whenever an edit is performed on an Alg_track or Alg_seq.
	    // When an Alg_event_list is created to contain pointers to
	    // a subset of an Alg_track or Alg_seq (the events_owner), 
	    // the Alg_event_list gets a copy of the events_owner's 
	    // sequence_number. If the events_owner is edited, the pointers
	    // in this Alg_event_list will become invalid. This is detected
	    // (for debugging) as differing sequence_numbers.

    // every event list, track, and seq has a duration.
    // Usually the duration is set when the list is constructed, e.g.
    // when you extract from 10 to 15 seconds, the duration is 5 secs.
    // The duration does not tell you when is the last note-off.
    // duration is recorded in both beats and seconds:
    double beat_dur; 
    double real_dur;
public:
	Alg_events_ptr events;
    // the client should not create one of these, but these are
    // returned from various track and seq operations. An
    // Alg_event_list "knows" the Alg_track or Alg_seq that "owns"
    // the events. All events in an Alg_event_list must belong
    // to the same Alg_track or Alg_seq structure.
    // When applied to an Alg_seq, events are enumerated track
    // by track with increasing indices. This operation is not
    // particularly fast on an Alg_seq.
    THIS MUST BE VIRTUAL SO WE CAN OVERRIDE IN ALG_SEQ
    virtual Alg_event *operator[](int i) { return events->events[i]; }
    ALG_EVENT_LIST WILL INCLUDE A LIST OF EVENTS, SO THIS INITIALIZER
    WILL CHANGE
    Alg_event_list() { events = new Alg_events(); sequence_number = 0; 
        beat_dur = 0.0; real_dur = 0.0; events_owner = NULL; }
    Alg_event_list(Alg_track *owner) {
        events = new Alg_events(); events_owner = owner;
        sequence_number = owner->sequence_number; 
        beat_dur = 0.0; real_dur = 0.0; }

    // #RBD# The destructor does not free events because they are owned
    // by a track or seq structure.
    virtual ~Alg_event_list();
    virtual int length() { return events->len; }

    // Returns the duration of the sequence in beats or seconds
    double get_beat_dur() { return beat_dur; }
    void set_beat_dur(double d) { beat_dur = d; }
    double get_real_dur() { return real_dur; }
    void set_real_dur(double d) { real_dur = d; }

    // Events are stored in time order, so when you change the time of
    // an event, you must adjust the position. When you call set_start_time
    // on an Alg_event_list, the Alg_event_list is not modified, but the
    // Alg_track that "owns" the event is modified. If the owner is an 
    // Alg_seq, this may require searching the seq for the track containing
    // the event. This will mean a logN search of every track in the seq
    // (but if this turns out to be a problem, we can store each event's
    // track owner in the Alg_event_list.)
    virtual void set_start_time(Alg_event *event, double);
    // get text description of run-time errors detected, clear error
    const char *get_last_error_message() { return last_error_message; }
    // Implementation hint: keep a sequence number on each Alg_track that is 
    // incremented anytime there is a structural change. (This behavior is
    // inherited by Alg_seq as well.) Copy the sequence number to any
    // Alg_event_list object when it is created. Whenever you access an 
    // Alg_event_list, using operator[], assert that the Alg_event_list sequence
    // number matches the Alg_seq sequence number. This will guarantee that you
    // do not try to retain pointers to events beyond the point where the events
    // may no longer exist.
};


// Alg_attribute is an atom in the symbol table
// with the special addition that the last
// character is prefixed to the string; thus,
// the attribute 'tempor' (a real) is stored
// as 'rtempor'. To get the string name, just
// use attribute+1.
typedef char *Alg_attribute;


// Alg_atoms is a symbol table of Alg_attributes
class Alg_atoms {
public:
    Alg_atoms() {
        max = len = 0;
        atoms = NULL;
    }
    // insert/lookup an atttribute
    Alg_attribute insert_attribute(Alg_attribute attr);
    // insert/lookup attribute by name (without prefixed type)
    Alg_attribute insert_string(char *name);
private:
    long max;
    long len;
    char **atoms;

    // insert an Attriubute not in table after moving attr to heap
    Alg_attribute insert_new(char *name, char attr_type);
    void expand(); // make more space
};

extern Alg_atoms symbol_table;


// an attribute/value pair. Since Alg_attribute names imply type,
// we try to keep attributes and values packaged together as
// Alg_parameter class
typedef class Alg_parameter {
public:
    // Note: There is no constructor, but you MUST set the attribute
    // before the parameter is deleted or before a local parameter
    // variable leaves its scope.

    // Note: a parameter owns its string value, so if this is a
    // string, the string will be freed. If you do not want to 
    // free the string, set s to NULL before the parameter is
    // deleted (or before a local parameter variable leaves its scope).
    ~Alg_parameter();
    Alg_attribute attr;
    union {
        double r;// real
        char *s; // string
        long i;  // integer
        bool l;  // logical
        char *a; // symbol (atom)
    }; // anonymous union
    void copy(Alg_parameter *); // copy from another parameter
    char attr_type() { return attr[0]; }
    char *attr_name() { return attr + 1; }
    void set_attr(Alg_attribute a) { attr = a; }
} *Alg_parameter_ptr;


// a list of attribute/value pairs
typedef class Alg_parameters {
public:
    class Alg_parameters *next;
    Alg_parameter parm;

    Alg_parameters(Alg_parameters *list) {
        next = list;
    }

    //~Alg_parameters() { }

    // each of these routines takes address of pointer to the list
    // insertion is performed without checking whether or not a
    // parameter already exists with this attribute. See find() and
    // remove_key() to assist in checking for and removing existing 
    // parameters.
    // Note also that these insert_* methods convert name to an
    // attribute. If you have already done the symbol table lookup/insert
    // you can do these operations faster (in which case we should add
    // another set of functions that take attributes as arguments.)
    static void insert_real(Alg_parameters **list, char *name, double r);
    // insert string will copy string to heap
    static void insert_string(Alg_parameters **list, char *name, char *s);
    static void insert_integer(Alg_parameters **list, char *name, long i);
    static void insert_logical(Alg_parameters **list, char *name, bool l);
    static void insert_atom(Alg_parameters **list, char *name, char *s);
    static Alg_parameters *remove_key(Alg_parameters **list, char *name);
    // find an attribute/value pair
    Alg_parameter_ptr find(char *name);
} *Alg_parameters_ptr;


typedef class Alg_note : public Alg_event {
public:
    virtual ~Alg_note();
    Alg_note(Alg_note *); // copy constructor
    double pitch; // pitch in semitones (69 = A440)
    // Note: The member variables start (inherited) and dur give the
    // time interval of the note. There is no start or end_time
    // member variable.
    double dur;   // duration in seconds (normally to release point)
    double loud;  // dynamic corresponding to MIDI velocity
    Alg_parameters_ptr parameters; // attribute/value pair list

    void set_string_value(char *attr, char *value);
    void set_real_value(char *, double);
    void set_logical_value(char *, bool);
    void set_integer_value(char *, long);
    void set_atom_value(char *attr, char *atom);
    // Note: selected is set to false in Alg_event()
    Alg_note() { type = 'n'; parameters = NULL; }
} *Alg_note_ptr;


typedef class Alg_update : public Alg_event {
public:
    virtual ~Alg_update() {};
    Alg_update(Alg_update *); // copy constructor
    Alg_parameter parameter; // an update contains one attr/value pair

    void set_string_value(char *attr, char *value);
    void set_real_value(char *, double);
    void set_logical_value(char *, bool);
    void set_integer_value(char *, long);
    void set_atom_value(char *attr, char *atom);
    // Note: selected is set to false in Alg_event()
    Alg_update() { type = 'u'; }
} *Alg_update_ptr;

// Alg_beat is used to contruct a tempo map
typedef class Alg_beat {
public:
    double time;
    double beat;
} *Alg_beat_ptr;


// Alg_beats is a list of Alg_beat objects used in Alg_seq
typedef class Alg_beats {
private:
    long max;
    void expand();
public:
    long len;
    Alg_beat_ptr beats;
    Alg_beat &operator[](int i) {
        assert(i >= 0 && i < len);
        return beats[i];
    }
    Alg_beats() {
        max = len = 0;
        beats = NULL;
        expand();
        beats[0].time = 0;
        beats[0].beat = 0;
        len = 1;
    }
    ~Alg_beats() {
        if (beats) delete[] beats;
    }
    void insert(long i, Alg_beat_ptr beat);
} *Alg_beats_ptr;


class Alg_time_map {
public:
    Alg_beats beats; // array of Alg_beat
    double last_tempo;
    bool last_tempo_flag;
    Alg_time_map() {
        last_tempo = 2.0; // note: this value ignored until
			              // last_tempo_flag is set; nevertheless, the default
			              // tempo is 120.
        last_tempo_flag = false;
    }
    long locate_time(double time);
    long locate_beat(double beat);
    double beat_to_time(double beat);
    double time_to_beat(double time);
    // Time map manipulations: it is prefered to call the corresponding
    // methods in Alg_seq. If you manipulate an Alg_time_map directly,
    // you should take care to convert all tracks that use the time map
    // to beats or seconds as appropriate: Normally if you insert a beat
    // you want tracks to be in time units and if you insert a tempo change
    // you want tracks to be in beat units.
    bool insert_beat(double time, double beat);   // add a point to the map
    bool insert_tempo(double tempo, double beat); // insert a tempo change
    // set the tempo over a region
    bool set_tempo(double tempo, double start_beat, double end_beat);
};


// Global time_map, tempo 120
static Alg_time_map const_map; #RBD# I THINK YOU DON'T NEED/WANT THIS: = Alg_time_map();

// WARNING: Tracks have a pointer to a time_map, but tracks do not "own"
// the time map. When a track is deleted, the corresponding time_map may
// need to be deleted.
//
class Alg_track : public Alg_event_list {
protected:
	Alg_time_map *time_map;
    bool units_are_seconds;
public:
    void set_start_time(Alg_event *event, double);

    Alg_track() { units_are_seconds = false; time_map = &const_map; }
    Alg_track(Alg_time_map *map); // initialize empty track with a time map
    Alg_track(Alg_track &track);  // copy constructor
    virtual ~Alg_track() { delete time_map; }

    Alg_event_ptr copy_event(Alg_event_ptr event); // make a complete copy

    // Returns a buffer containing a serialization of the
    // file.  It will be an ASCII representation unless midi is true.
    virtual void serialize(char **buffer, int *len, bool midi);

    // Try to read from a memory buffer.  Automatically guess
    // whether it's MIDI or text.
    virtual bool unserialize(char *buffer, int len);

    // Are we using beats or seconds?
    bool get_units_are_seconds() { return units_are_seconds; }
    // Change units 
    virtual void convert_to_beats();
    virtual void convert_to_seconds();

    // Every Alg_track may have an associated time_map. If no map is
    // specified, or if you set_time_map(NULL), then the behavior 
    // should be as if there is a constant tempo of 120 beats/minute.
    // Recommendation: create a static global tempo map object. When
    // any operation that needs a tempo map gets NULL, use the global
    // tempo map. (Exception: any operation that would modify the
    // tempo map should raise an error -- you don't want to change the
    // default tempo map.)
    void set_time_map(Alg_time_map *map);
    Alg_time_map_ptr get_time_map() { return time_map; }
    

    // Methods to create events. The returned event is owned by the caller.
    // Use delete to get rid of it unless you call add() -- see below.
    //
    Alg_event *create_note(double time, int channel, int identifier, 
                           double pitch, double loudness, double duration);
    // Note: after create_update(), caller should use set_*_value() to
    // initialize the attribute/value pair:
    Alg_event *create_update(double time, int channel, int identifier);
    // Adds a new event - it is automatically inserted into the
    // correct order in the sequence based on its timestamp.
    // The ownership passes from the caller to this Alg_seq. The
    // event is not copied.
    void add(Alg_event *event); #RBD#: EDITED THIS
    
	//
    // Editing regions
    //

    // Deletes the notes that start within the given region
    // and returns them in a new sequence.  The start times
    // of the notes in the returned sequence should be shifted
    // over by t.  The notes after the region get shifted over
    // to fill the gap. In an Alg_seq, the tempo track is edited
    // in a similar way
    // and the cut tempo information is retained in the new seq.
    // ONLY NOTES THAT START WITHIN THE REGION ARE CUT unless
    // "all" is true in which case all notes that intersect
    // the region are copied. CUT NOTES
    // MAY EXTEND BEYOND THE DURATION OF THE RESULTING SEQ.
    // The return type is the same as this (may be Alg_seq).
    virtual Alg_track *cut(double t, double len, bool all);

    // Like cut() but doesn't remove the notes from the original
    // sequence. The Alg_events are copied, not shared. ONLY EVENTS
    // THAT START WITHIN THE REGION ARE COPIED unless "all" is true
    // in which case all notes that intersect the region are
    // copied. COPIED NOTES MAY
    // EXTEND BEYOND THE DURATION OF THE RESULTING SEQ.
    // The return type is the same as this (may be Alg_seq).
    virtual Alg_track *copy(double t, double len, bool all);

    // Inserts a sequence in the middle, shifting some notes
    // over by the duration of the seq, which is first converted
    // to the same units (seconds or beats) as this. (This makes
    // a differece because the pasted data may change the tempo,
    // and notes that overlap the borders will then experience
    // a tempo change.)
    // THE SEQ PARAMETER IS NOT MODIFIED, AND Alg_event's ARE
    // COPIED, NOT SHARED.
    // The type of this must be Alg_seq if seq is an Alg_seq, or
    // Alg_track if seq is an Alg_track or an Alg_event_list.
    virtual void paste(double t, Alg_event_list *seq); // Shifts notes

    // Merges two sequences with a certain offset. The offset is
    // interpreted as either beats or seconds according to the 
    // current units of this, and seq is converted to the same
    // units as this. Except for a possible conversion to beats
    // or seconds, the tempo track of seq (if any) is ignored. 
    // (There is no way to merge tempo tracks.)
    // THE SEQ PARAMETER IS NOT MODIFIED, AND Alg_event's ARE
    // COPIED, NOT SHARED.
    // The type of seq must be Alg_seq if seq is an Alg_seq, or
    // Alg_track if seq is an Alg_track or an Alg_event_list.
    virtual void merge(double t, Alg_event_list *seq);

    // Deletes and shifts notes to fill the gap. The tempo track
    // is also modified accordingly. ONLY EVENTS THAT START WITHIN
    // THE REGION ARE DELETED unless "all" is true, in which case
    // all notes that intersect the region are cleared.
    // NOTES THAT EXTEND FROM BEFORE THE
    // REGION INTO THE REGION RETAIN THEIR DURATION IN EITHER
    // BEATS OR SECONDS ACCORDING TO THE CURRENT UNITS OF this.
    virtual void clear(double t, double len, bool all);

    // Deletes notes but doesn't shift.  If the "all" argument
    // is true, deletes all notes that intersect the range at all,
    // not just those that start within it. The tempo track is 
    // not affected.
    virtual void silence(double t, double len, bool all);

    // Simply shifts notes past time t over by len, which is given
    // in either beats or seconds according to the units of this.
    // The resulting interveal (t, t+len) may in fact contain notes
    // that begin before t. The durations of notes are not changed.
    // If this is an Alg_seq, the tempo track is expanded at t also.
    void insert_silence(double t, double len);

    //
    // Accessing for screen display
    //

    // Find all notes and updates that intersect the
    // given range (all == true) so they can be displayed on-screen.
    // If all == false, only notes completely within range are returned.
    // If this is an Alg_seq, events will come from multiple tracks
    // (or call track[i].find_events_in_range()).
    // 
    virtual Alg_event_list *find_events_in_range(
                              double t, double len, bool all);

    // A useful generic function to retrieve only certain
    // types of events.  The masks should be bit-masks defined
    // somewhere else. Part of the mask allows us to search for 
    // selected events. If this is an Alg_seq, search all tracks
    // (otherwise, call track[i].find())
    // If channel_mask == 0, accept ALL channels
    virtual Alg_event_list *find(double t, double len, bool all,
                                 long channel_mask, int event_type_mask);

    //
    // MIDI playback
    //
    // See Alg_iterator
};


// Alg_time_sig represents a single time signature;
// although not recommended, time_signatures may have arbitrary
// floating point values, e.g. 4.5 beats per measure
typedef class Alg_time_sig {
public:
    double beat; // when does this take effect?
    double num;  // what is the "numerator" (top number?)
    double den;  // what is the "denominator" (bottom number?)
    Alg_time_sig(double b, double n, double d) {
        beat = b; num = n; den = d;
    }
    Alg_time_sig() {
        beat = 0; num = 0; den = 0;
    }
    void beat_to_measure(double beat, double *measure, double *m_beat,
                         double *num, double *den);

} *Alg_time_sig_ptr;


// Alg_time_sigs is a dynamic array of time signatures
class Alg_time_sigs {
private:
    long max;
    void expand(); // make more space
public:
    long len;
    Alg_time_sig_ptr time_sigs;
    Alg_time_sigs() {
        max = len = 0;
        time_sigs = NULL;
    }
    Alg_time_sig &operator[](int i) { // fetch a time signature
        assert(i >= 0 && i < len);
        return time_sigs[i];
    }
    ~Alg_time_sigs() {
        if (time_sigs) delete[] time_sigs;
    }
    void insert(double beat, double num, double den);
};


// a sequence of Alg_events objects
class Alg_tracks {
private:
    long max;
    void expand();
	void expand_to(int new_max);
public:
    long len;
    Alg_events_ptr *tracks; // tracks is array of pointers
    Alg_events_ptr &operator[](int i) {
        assert(i >= 0 && i < len);
        return tracks[i];
    }
    Alg_tracks() {
        max = len = 0;
        tracks = NULL;
    }
	~Alg_tracks();
    void append(Alg_events_ptr track);
	void add_track(int track_num);
    void reset();
};


// An Alg_seq is an array of Alg_events, each a sequence of Alg_event, 
// with a tempo map and a sequence of time signatures
//
typedef class Alg_seq : public Alg_track {
protected:
	long *current; // array of indexes used by iteration methods
public:
	int channel_offset_per_track; // used to encode track_num into channel
	Alg_tracks tracks_list;       // array of Alg_events
    Alg_time_sigs time_sig;
    int beat_x;
    Alg_seq() {
        units_are_seconds = true;
		channel_offset_per_track = 0;
    }
    Alg_seq(Alg_seq &s);           // copy constructor
    Alg_seq(const char *filename); // create from text (Allegro) file
    ~Alg_seq();

    // Returns the number of tracks
    int tracks();

    // Return a particular track. This Alg_seq owns the track, so the
    // caller must not delete the result.
    Alg_track *track(int);

    void insert_silence(double t, double len, bool all);

    long seek_time(double time, int track_num);
    bool insert_beat(double time, double beat);
    bool insert_tempo(double tempo, double beat);
    void add_event(Alg_event_ptr event, int track_num);
    bool set_tempo(double tempo, double start_beat, double end_beat);
    void set_time_sig(double beat, double num, double den);
    void beat_to_measure(double beat, long *measure, double *m_beat,
                         double *num, double *den);
    // void set_events(Alg_event_ptr *events, long len, long max);
	void merge_tracks();    // move all track data into one track
	void iteration_begin(); // prepare to enumerate events in order
	Alg_event_ptr iteration_next(); // return next event (or NULL)
	void iteration_end();   // clean up after enumerating events
} *Alg_seq_ptr;


char *heapify(char *s); // put a string on the heap


#endif
