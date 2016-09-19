// midifile reader

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"
#include "allegro.h"
#include "mfmidi.h"
#include "allegrosmfrd.h"
#include "trace.h"
#include "memory.h" // for memcpy()


typedef class Alg_pending {
public:
    Alg_note_ptr note;
    class Alg_pending *next;
    Alg_pending(Alg_note_ptr n, class Alg_pending *list) { 
		note = n; next = list; }
} *Alg_pending_ptr;


class Alg_midifile_reader: public Midifile_reader {
public:
    FILE *file;
    Alg_seq_ptr seq;
    int divisions;
    Alg_pending_ptr pending;
    Alg_events_ptr track;
	long channel_offset_per_track; // used to encode track number into channel
		// chan is actual_channel + channel_offset_per_track * track_num
	    // default is 100, set this to 0 to merge all tracks to 16 channels
    int channel_offset;

    Alg_midifile_reader() {
		file = NULL;
		pending = NULL;
		seq = NULL;
	}
    // delete destroys the seq member as well, so set it to NULL if you
    // copied the pointer elsewhere
    ~Alg_midifile_reader();
    // the following is used to load the Alg_seq from the file:
    void initialize(FILE *file);

    void set_nomerge(bool flag) { Mf_nomerge = flag; }
    void set_skipinit(bool flag) { Mf_skipinit = flag; }
    long get_currtime() { return Mf_currtime; }

protected:
    double get_time();
    void update(int chan, int key, Alg_parameter_ptr param);
    void *Mf_malloc(size_t size) { return malloc(size); }
    void Mf_free(void *obj, size_t size) { free(obj); }
    /* Methods to be called while processing the MIDI file. */
    void Mf_starttrack();
    void Mf_endtrack();
    int Mf_getc();
    void Mf_eot();
    void Mf_error(char *);
    void Mf_header(int,int,int);
    void Mf_on(int,int,int);
    void Mf_off(int,int,int);
    void Mf_pressure(int,int,int);
    void Mf_controller(int,int,int);
    void Mf_pitchbend(int,int,int);
    void Mf_program(int,int);
    void Mf_chanpressure(int,int);
    void Mf_sysex(int,char*);
    void Mf_arbitrary(int,char*);
    void Mf_metamisc(int,int,char*);
    void Mf_seqnum(int);
    void Mf_smpte(int,int,int,int,int);
    void Mf_timesig(int,int,int,int);
    void Mf_tempo(int);
    void Mf_keysig(int,int);
    void Mf_sqspecific(int,char*);
    void Mf_text(int,int,char*);
};


Alg_midifile_reader::~Alg_midifile_reader()
{
    while (pending) {
        Alg_pending_ptr to_be_freed = pending;
        pending = pending->next;
        delete to_be_freed;
    }
	finalize(); // free Mf reader memory
}


/* 
void Alg_midifile_reader::merge_tracks()
{
    double next;
    int track = -1;
    // keep an array of indexes into tracks
    long *current = new long[tracks.len];
    long sum = 0;
    long i;
    for (i = 0; i < tracks.len; i++) {
        current[i] = 0;
        sum = sum + tracks[i]->len;
    }
    // preallocate array for efficiency:
    Alg_event_ptr *notes = new Alg_event_ptr[sum];
    long notes_index = 0;

    bool done = false;
    while (!done) {
        Alg_events_ptr tr;  // a track
        long cur;       // a track index
        // find lowest next time of any track:
        next = 1000000.0;
        for (i = 0; i < tracks.len; i++) {
            tr = tracks[i];
            cur = current[i];
            if (cur < tr->len && (*tr)[cur]->time < next) {
                next = (*tr)[cur]->time;
                track = i;
            }
        }
        // insert a track event
        if (next < 1000000.0) {
            notes[notes_index++] = (*tracks[track])[current[track]++];
        } else {
            done = true;
        }
    }
    tracks.reset(); // don't need them any more
    seq->set_events(notes, sum, sum);
    delete[] current;
}
*/

void Alg_midifile_reader::initialize(FILE *f)
{
    file = f;
    channel_offset = 0;
    if (seq) delete seq;
    seq = new Alg_seq();
    midifile();
}


void Alg_midifile_reader::Mf_starttrack()
{
    // printf("starting new track\n");
    track = new Alg_events;
}


void Alg_midifile_reader::Mf_endtrack()
{
	seq->convert_to_seconds(); // track event times are in seconds
    seq->tracks.append(track);
    // printf("finished track, length %d number %d\n", track->len, track_num / 100);
    channel_offset += seq->channel_offset_per_track;
    track = NULL;
}


int Alg_midifile_reader::Mf_getc()
{
    return getc(file);
}


void Alg_midifile_reader::Mf_eot()
{
}


void Alg_midifile_reader::Mf_error(char *msg)
{
    fprintf(stdout, "Midifile reader error: %s\n", msg);
}


void Alg_midifile_reader::Mf_header(int format, int ntrks, int division)
{
    if (format > 1) {
        char msg[80];
        sprintf(msg, "file format %d not implemented", format);
        Mf_error(msg);
    }
    divisions = division;
}


double Alg_midifile_reader::get_time()
{
    double beat = ((double) get_currtime()) / divisions;
    return seq->map.beat_to_time(beat);
}


void Alg_midifile_reader::Mf_on(int chan, int key, int vel)
{
    if (vel == 0) {
        Mf_off(chan, key, vel);
        return;
    }
    Alg_note_ptr note = new Alg_note();
    pending = new Alg_pending(note, pending);
    /*    trace("on: %d at %g\n", key, get_time()); */
    note->time = get_time();
    note->chan = chan + channel_offset;
    note->dur = 0;
    note->key = key;
    note->pitch = key;
    note->loud = vel;
    track->append(note);
}


void Alg_midifile_reader::Mf_off(int chan, int key, int vel)
{
    double time = get_time();
    Alg_pending_ptr *p = &pending;
    while (*p) {
        if ((*p)->note->key == key && (*p)->note->chan == chan + channel_offset) {
            (*p)->note->dur = time - (*p)->note->time;
            // trace("updated %d dur %g\n", (*p)->note->key, (*p)->note->dur);
            Alg_pending_ptr to_be_freed = *p;
            *p = to_be_freed->next;
            delete to_be_freed;
        } else {
            p = &((*p)->next);
        }
    }
}


void Alg_midifile_reader::update(int chan, int key, Alg_parameter_ptr param)
{
    Alg_update_ptr update = new Alg_update;
    update->time = get_time();
	update->chan = chan;
	if (chan != -1) {
		update->chan = chan + channel_offset;
	}
    update->key = key;
    update->parameter = *param;
    // prevent the destructor from destroying the string twice!
    // the new Update takes the string from param
    if (param->attr_type() == 's') param->s = NULL;
    track->append(update);
}


void Alg_midifile_reader::Mf_pressure(int chan, int key, int val)
{
    Alg_parameter parameter;
    parameter.set_attr(symbol_table.insert_string("pressurer"));
    parameter.r = val / 127.0;
    update(chan, key, &parameter);
}


void Alg_midifile_reader::Mf_controller(int chan, int control, int val)
{
    Alg_parameter parameter;
    char name[32];
    sprintf(name, "control%dr", control);
    parameter.set_attr(symbol_table.insert_string(name));
    parameter.r = val / 127.0;
    update(chan, -1, &parameter);
}


void Alg_midifile_reader::Mf_pitchbend(int chan, int c1, int c2)
{
    Alg_parameter parameter;
    parameter.set_attr(symbol_table.insert_string("bendr"));
    parameter.r = ((c1 << 7) + c2) / 8192.0 - 1.0;
    update(chan, -1, &parameter);
}


void Alg_midifile_reader::Mf_program(int chan, int program)
{
    Alg_parameter parameter;
    parameter.set_attr(symbol_table.insert_string("programi"));
    parameter.i = program;
    update(chan, -1, &parameter);
}


void Alg_midifile_reader::Mf_chanpressure(int chan, int val)
{
    Alg_parameter parameter;
    parameter.set_attr(symbol_table.insert_string("pressurer"));
    parameter.r = val / 127.0;
    update(chan, -1, &parameter);
}


void Alg_midifile_reader::Mf_sysex(int len, char *msg)
{
    Mf_error("sysex message ignored - not implemented");
}


void Alg_midifile_reader::Mf_arbitrary(int len, char *msg)
{
    Mf_error("arbitrary data ignored");
}


void Alg_midifile_reader::Mf_metamisc(int type, int len, char *msg)
{
    Mf_error("metamisc data ignored");
}


void Alg_midifile_reader::Mf_seqnum(int n)
{
    Mf_error("seqnum data ignored");
}


void Alg_midifile_reader::Mf_smpte(int i1, int i2, int i3, int i4, int i5)
{
    Mf_error("SMPTE data ignored");
}


void Alg_midifile_reader::Mf_timesig(int i1, int i2, int i3, int i4)
{
	seq->set_time_sig(get_currtime() / divisions, i1, 1 << i2);
}


void Alg_midifile_reader::Mf_tempo(int tempo)
{
    double beat = get_currtime();
    beat = beat / divisions; // convert to quarters
    // 6000000 us/min / n us/beat => beat / min
    double bps = 60000000.0 / tempo;
    seq->insert_tempo(bps, beat);
}


void Alg_midifile_reader::Mf_keysig(int key, int mode)
{
    Alg_parameter key_parm;
    key_parm.set_attr(symbol_table.insert_string("keysigi"));
    // use 0 for C major, 1 for G, -1 for F, etc., that is,
    // the number of sharps, where flats are negative sharps
    key_parm.i = key; //<<<---- fix this
    // use -1 to mean "all channels"
    update(-1, -1, &key_parm);
    Alg_parameter mode_parm;
    mode_parm.set_attr(symbol_table.insert_string("modea"));
    mode_parm.a = (mode == 0 ? symbol_table.insert_string("majora") :
                               symbol_table.insert_string("minora"));
    update(-1, -1, &mode_parm);
}


void Alg_midifile_reader::Mf_sqspecific(int len, char *msg)
{
    Mf_error("sq specific data ignored");
}


char *heapify2(int len, char *s)
{
    char *h = new char[len + 1];
    memcpy(h, s, len);
    h[len] = 0;
    return h;
}


void Alg_midifile_reader::Mf_text(int type, int len, char *msg)
{
    Alg_parameter text;
    text.s = heapify2(len, msg);
    char *attr = "miscs";
    if (type == 1) attr = "texts";
    else if (type == 2) attr = "copyrights";
    else if (type == 3) attr = "names";
    else if (type == 4) attr = "instruments";
    else if (type == 5) attr = "lyrics";
    else if (type == 6) attr = "markers";
    else if (type == 7) attr = "cues";
    text.set_attr(symbol_table.insert_string(attr));
    update(-1, -1, &text);
}


Alg_seq_ptr alg_smf_read(FILE *file)
{
    Alg_midifile_reader ar;
    ar.initialize(file);
    return ar.seq;
}


