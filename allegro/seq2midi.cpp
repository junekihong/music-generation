// seq2midi.cpp -- simple sequence player, intended to help test/demo
// the allegro code

#include "stdlib.h"
#include "string.h"
#include "assert.h"
#include "stdio.h" // for debugging
#include "porttime.h"
#include "portmidi.h"
#include "midicode.h"
#include "allegro.h"

#define ROUND(x) (int) ((x)+0.5)


double time_elapsed()
{
    return Pt_Time() * 0.001;
}


#include "windows.h"

void wait_until(double time)
{
	// print "." to stdout while waiting
	static double last_time = 0.0;
	double now = time_elapsed();
	if (now < last_time) last_time = now;
    while (now < time) {
		Sleep(1);
		now = time_elapsed();
        long now_sec = (long) now;
		long last_sec = (long) last_time;
		if (now_sec > last_sec) {
			fprintf(stdout, ".");
			fflush(stdout);
			last_time = now;
		}
    }
}


#define never 1000000 // represents infinite time

void midi_note_on(PortMidiStream *midi, double when, int chan, int key, int loud)
{
    unsigned long timestamp = (unsigned long) (when * 1000);
    chan = chan & 15;
	if (key > 127) key = 127;
	if (key < 0) key = 0;
	if (loud > 127) loud = 127;
	if (loud < 0) loud = 0;
    unsigned long data = Pm_Message(0x90 + chan, key, loud);
    Pm_WriteShort(midi, timestamp, data);
}


static void midi_channel_message_2(PortMidiStream *midi, double when, 
								   int status, int chan, int data)
{
	unsigned long timestamp = (unsigned long) (when * 1000);
	chan = chan & 15;
	if (data > 127) data = 127;
	if (data < 0) data = 0;
	unsigned long msg = Pm_Message(status + chan, data, 0);
	Pm_WriteShort(midi, timestamp, msg);
}


static void midi_channel_message(PortMidiStream *midi, double when, 
								 int status, int chan, int data, int data2)
{
	unsigned long timestamp = (unsigned long) (when * 1000);
	chan = chan & 15;
	if (data > 127) data = 127;
	if (data < 0) data = 0;
	if (data2 > 127) data2 = 127;
	if (data2 < 0) data2 = 0;
	unsigned long msg = Pm_Message(status + chan, data, data2);
	Pm_WriteShort(midi, timestamp, msg);
}


typedef class Work {
public:
    char type; // 'n' for note, 'o' for off
    double time; // when to do it
	Alg_event_ptr event;
	// long track_num; // which track
    // long index; // of event
    Work *next;
    Work(char t, double when, Alg_event_ptr e, Work *n) {
        type = t; time = when; event = e; next = n; }
} *Work_ptr;


// walk down the list and insert work in time order, return 
// the resulting list (which may have work at the front now)
//
static Work_ptr insert(Work_ptr pending, Work_ptr work)
{
    if (pending == NULL) {
        return work;
    } else if (pending->time < work->time) {
        pending->next = insert(pending->next, work);
        return pending;
    } else {
        work->next = pending;
        return work;
    }
}

static Alg_attribute pressure_attr;
static Alg_attribute bend_attr;
static Alg_attribute program_attr;


void send_midi_update(Alg_update_ptr u, PortMidiStream *midi)
{
	u->parameter.attr;
    if (u->parameter.attr == pressure_attr) {
		if (u->key < 0) {
			midi_channel_message_2(midi, u->time, MIDI_TOUCH, u->chan,
				                   (int) (u->parameter.r * 127));
		} else {
			midi_channel_message(midi, u->time, MIDI_POLY_TOUCH, u->chan, 
								 u->key, (int) (u->parameter.r * 127));
		}
	} else if (u->parameter.attr == bend_attr) {
		int bend = ROUND((u->parameter.r + 1) * 8192);
		if (bend > 8191) bend = 8191;
		if (bend < 0) bend = 0;
		midi_channel_message(midi, u->time, MIDI_BEND, u->chan, 
							 bend >> 7, bend & 0x7F);
	} else if (u->parameter.attr == program_attr) {
		midi_channel_message_2(midi, u->time, MIDI_CH_PROGRAM, 
							   u->chan, u->parameter.i);
	} else if (strncmp("control", u->parameter.attr_name(), 7) == 0 &&
		       u->parameter.attr_type() == 'r') {
		int control = atoi(u->parameter.attr_name() + 7);
		int val = ROUND(u->parameter.r * 127);
		midi_channel_message(midi, u->time, MIDI_CTRL, u->chan, control, val);
	}
}


void seq2midi(Alg_seq_ptr seq, PortMidiStream *midi)
{
	// prepare by doing lookup of important symbols
    pressure_attr = symbol_table.insert_string("pressurer");
	bend_attr = symbol_table.insert_string("bendr");
	program_attr = symbol_table.insert_string("programi");

    bool done = false;
	seq->iteration_begin();
    Work_ptr pending = NULL;
	Alg_event_ptr e = seq->iteration_next();
	if (e) {
		pending = new Work('n', e->time, e, NULL);
	}
    Pt_Start(1, NULL, NULL); // initialize time
    while (pending) {
        double next_time = pending->time;
        wait_until(next_time);
        Work_ptr w = pending;
        pending = pending->next;
        Alg_note_ptr n = (Alg_note_ptr) (w->event);
        if (w->type == 'n') { // turn it on
            if (n->type == 'n') { // process notes here
                // printf("Note at %g: chan %d key %d loud %d\n",
                //        next_time, n->chan, n->key, (int) n->loud);
                midi_note_on(midi, next_time, n->chan, n->key, (int) n->loud);
                // add pending note-off
                pending = insert(pending, 
                                 new Work('o', n->time + n->dur, n, NULL));
            } else if (n->type == 'u') { // process updates here
                Alg_update_ptr u = (Alg_update_ptr) n; // coerce to proper type
                send_midi_update(u, midi);
            } 
            // add next note
			e = seq->iteration_next();
            if (e) {
                pending = insert(pending,
                                 new Work('n', e->time, e, NULL));
            }
        } else { // a note-off
            midi_note_on(midi, next_time, n->chan, n->key, 0);
        }
        delete w;
    }
	seq->iteration_end();
}


void seq_play(Alg_seq_ptr seq)
{
    PortMidiStream *mo;
    PmDeviceID dev = Pm_GetDefaultOutputDeviceID();
	// note that the Pt_Time type cast is required because Pt_Time does 
	// not take an input parameter, whereas for generality, PortMidi
	// passes in a void * so the time function can get some context.
	// It is safe to call Pt_Time with a parameter -- it will just be ignored.
    if (Pm_OpenOutput(&mo, dev, NULL, 256, 
		              (long (*)(void *))&Pt_Time, NULL, 100) == pmNoError) {
		seq2midi(seq, mo);
		wait_until(time_elapsed() + 1);
		Pm_Close(mo);
	}
    return;
}
