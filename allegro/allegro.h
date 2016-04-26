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
public:
    char type; // 'e' event, 'n' note, 'u' update
    double time;
    long chan;
    virtual ~Alg_event() {}
} *Alg_event_ptr;


// a sequence of Alg_event objects
typedef class Alg_events {
private:
    long max;
    void expand();
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
    void insert(Alg_event_ptr event);
    void append(Alg_event_ptr event);
    void set_events(Alg_event_ptr *e, long l, long m) {
        if (events) delete [] events;
        events = e; len = l; max = m; }
} *Alg_events_ptr;


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

#define Alg_atom_type(a) ((a)[0])
#define Alg_atom_name(a) ((a) + 1)


// an attribute/value pair. Since Alg_attribute names imply type,
// we try to keep attributes and values packaged together as
// Alg_parameter class
typedef class Alg_parameter {
public:
	// initialize attribute to NULL so that destructor will not try
	// to check the attribute type and raise a memory exception
	Alg_parameter() { attr = NULL; }
    ~Alg_parameter();
    Alg_attribute attr;
    union {
        double r;// real
        char *s; // string
        long i;  // integer
        bool l;  // logical
        char *a; // symbol (atom)
    }; // anonymous union
    char attr_type() { return Alg_atom_type(attr); }
    char *attr_name() { return Alg_atom_name(attr); }
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
    static void insert_real(Alg_parameters **list, char *name, double r);
    // insert string will copy string to heap
    static void insert_string(Alg_parameters **list, char *name, char *s);
    static void insert_integer(Alg_parameters **list, char *name, long i);
    static void insert_logical(Alg_parameters **list, char *name, bool l);
    static void insert_atom(Alg_parameters **list, char *name, char *s);
    static Alg_parameters *remove_key(Alg_parameters **list, char *name);
} *Alg_parameters_ptr;



typedef class Alg_note: public Alg_event {
public:
    virtual ~Alg_note();
    long key;     // note identifier
    double pitch; // pitch in semitones (69 = A440)
    double dur;   // duration in seconds (normally to release point)
    double loud;  // dynamic corresponding to MIDI velocity
    Alg_parameters_ptr parameters; // attribute/value pair list

    Alg_note() { type = 'n'; parameters = NULL; }
} *Alg_note_ptr;


typedef class Alg_update: public Alg_event {
public:
    virtual ~Alg_update() {};
    long key;     // note identifier (what sound is to be updated?)
    Alg_parameter parameter; // an update contains one attr/value pair
   
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
    };
    long locate_time(double time);
    long locate_beat(double beat);
    double beat_to_time(double beat);
    double time_to_beat(double time);
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
    Alg_time_sig &operator[](int i) {
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
typedef class Alg_seq {
protected:
	long *current; // array of indexes used by iteration methods
public:
	int channel_offset_per_track; // used to encode track_num into channel
	Alg_tracks tracks; // array of Alg_events
    Alg_time_map map;
    Alg_time_sigs time_sig;
    int beat_x;
    bool units_are_seconds;
    Alg_seq() {
        units_are_seconds = true;
		channel_offset_per_track = 0;
    }
    ~Alg_seq();

    long seek_time(double time, int track_num);
    void convert_to_beats();
    void convert_to_seconds();
    bool insert_beat(double time, double beat);
    bool insert_tempo(double tempo, double beat);
    void add_event(Alg_event_ptr event, int track_num);
    bool set_tempo(double tempo, double start_beat, double end_beat);
    void set_time_sig(double beat, double num, double den);
    void beat_to_measure(double beat, long *measure, double *m_beat,
                         double *num, double *den);
    // void set_events(Alg_event_ptr *events, long len, long max);
	void merge_tracks(); // move all track data into one track
	void iteration_begin(); // prepare to enumerate events in order
	Alg_event_ptr iteration_next(); // return next event (or NULL)
	void iteration_end(); // clean up after enumerating events
} *Alg_seq_ptr;

char *heapify(char *s);  // put a string on the heap

#endif
