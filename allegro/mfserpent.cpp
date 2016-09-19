// midifile reader for serpent

#include "flincl.h"
#include "mfmidi.h"
#include "mfserpent.h"

static Symbol_ptr s_starttrack;
static Symbol_ptr s_endtrack;
static Symbol_ptr s_eot;
static Symbol_ptr s_header;
static Symbol_ptr s_note_on;
static Symbol_ptr s_note_off;
static Symbol_ptr s_pressure;
static Symbol_ptr s_controller;
static Symbol_ptr s_pitchbend;
static Symbol_ptr s_program;
static Symbol_ptr s_channel_pressure;
static Symbol_ptr s_sysex;
static Symbol_ptr s_arbitrary;
static Symbol_ptr s_metamisc;
static Symbol_ptr s_seqnum;
static Symbol_ptr s_smpte;
static Symbol_ptr s_time_signature;
static Symbol_ptr s_tempo;
static Symbol_ptr s_key_signature;
static Symbol_ptr s_sq_specific;
static Symbol_ptr s_text;


void Serp_midifile_reader::call_none(Symbol_ptr meth)
{
    mach->send_method(object, meth);
    mach->call();
    mach->pop();
}


void Serp_midifile_reader::call_i(Symbol_ptr meth, int i1)
{
    mach->send_method(object, meth);
    mach->param_long(i1);
    mach->call();
    mach->pop();
}


void Serp_midifile_reader::call_ii(Symbol_ptr meth, int i1, int i2)
{
    mach->send_method(object, meth);
    mach->param_long(i1);
    mach->param_long(i2);
    mach->call();
    mach->pop();
}


void Serp_midifile_reader::call_iii(Symbol_ptr meth, int i1, int i2, int i3)
{
    mach->send_method(object, meth);
    mach->param_long(i1);
    mach->param_long(i2);
    mach->param_long(i3);
    mach->call();
    mach->pop();
}


void Serp_midifile_reader::call_ls(Symbol_ptr meth, int len, char *msg)
{
    mach->send_method(object, meth);
    FString_ptr result = FString::create_len(len, mach);
    if (!result) return;
    memcpy(result->get_string(), msg, len);
    mach->param_fval(result);
    mach->call();
    mach->pop();
}


void Serp_midifile_reader::call_ils(Symbol_ptr meth, int type, 
                                   int len, char *msg)
{
    mach->send_method(object, meth);
    mach->param_long(type);
    FString_ptr result = FString::create_len(len, mach);
    if (!result) return;
    memcpy(result->get_string(), msg, len);
    mach->param_fval(result);
    mach->call();
    mach->pop();
}


void Serp_midifile_reader::initialize(Object_ptr obj, FILE *f)
{
    file = f;
    object = obj;

    // it is redundant to look up all symbols every time
    // you read a file, but this should be fast
    // compared to parsing the midi file:
    s_starttrack = Symbol::create("starttrack", mach);
    s_endtrack = Symbol::create("endtrack", mach);
    s_eot = Symbol::create("eot", mach);
    s_header = Symbol::create("header", mach);
    s_note_on = Symbol::create("note_on", mach);
    s_note_off = Symbol::create("note_off", mach);
    s_pressure = Symbol::create("pressure", mach);
    s_controller = Symbol::create("controller", mach);
    s_pitchbend = Symbol::create("pitchbend", mach);
    s_program = Symbol::create("program", mach);
    s_channel_pressure = Symbol::create("channel_pressure", mach);
    s_sysex = Symbol::create("sysex", mach);
    s_arbitrary = Symbol::create("arbitrary", mach);
    s_metamisc = Symbol::create("metamisc", mach);
    s_seqnum = Symbol::create("seqnum", mach);
    s_smpte = Symbol::create("smpte", mach);
    s_time_signature = Symbol::create("time_signature", mach);
    s_tempo = Symbol::create("tempo", mach);
    s_key_signature = Symbol::create("key_signature", mach);
    s_sq_specific = Symbol::create("sq_specific", mach);
    s_text = Symbol::create("text", mach);

    // go ahead and parse the file
    midifile();
}


void *Serp_midifile_reader::Mf_malloc(size_t size)
{
    return mach->memget(size);
}


void Serp_midifile_reader::Mf_free(void *obj, size_t size)
{
    mach->memfree(obj, size);
}


void Serp_midifile_reader::Mf_starttrack()
{
    call_none(s_starttrack);
}


void Serp_midifile_reader::Mf_endtrack()
{
    call_none(s_endtrack);
}


int Serp_midifile_reader::Mf_getc()
{
    return getc(file);
}


void Serp_midifile_reader::Mf_eot()
{
    call_none(s_eot);
}


void Serp_midifile_reader::Mf_error(char *msg)
{
    mach->report_error(msg);
}


void Serp_midifile_reader::Mf_header(int format, int ntrks, int division)
{
    call_iii(s_header, format, ntrks, division);
}


void Serp_midifile_reader::Mf_on(int chan, int key, int vel)
{
    call_iii(s_note_on, chan, key, vel);
}


void Serp_midifile_reader::Mf_off(int chan, int key, int vel)
{
    call_iii(s_note_off, chan, key, vel);
}


void Serp_midifile_reader::Mf_pressure(int chan, int key, int val)
{
    call_iii(s_pressure, chan, key, val);
}


void Serp_midifile_reader::Mf_controller(int chan, int control, int val)
{
    call_iii(s_controller, chan, control, val);
}


void Serp_midifile_reader::Mf_pitchbend(int chan, int c1, int c2)
{
    call_ii(s_pitchbend, chan, (c1 << 7) + c2);
}


void Serp_midifile_reader::Mf_program(int chan, int program)
{
    call_ii(s_program, chan, program);
}


void Serp_midifile_reader::Mf_chanpressure(int chan, int val)
{
    call_ii(s_channel_pressure, chan, val);
}


void Serp_midifile_reader::Mf_sysex(int len, char *msg)
{
    call_ls(s_sysex, len, msg);
}


void Serp_midifile_reader::Mf_arbitrary(int len, char *msg)
{
    call_ls(s_arbitrary, len, msg);
}


void Serp_midifile_reader::Mf_metamisc(int type, int len, char *msg)
{
    call_ils(s_metamisc, type, len, msg);
}


void Serp_midifile_reader::Mf_seqnum(int n)
{
    call_i(s_seqnum, n);
}


void Serp_midifile_reader::Mf_smpte(int i1, int i2, int i3, int i4, int i5)
{
    mach->send_method(object, s_smpte);
    mach->param_long(i1);
    mach->param_long(i2);
    mach->param_long(i3);
    mach->param_long(i4);
    mach->param_long(i5);
    mach->call();
    mach->pop();
}


void Serp_midifile_reader::Mf_timesig(int i1, int i2, int i3, int i4)
{
    mach->send_method(object, s_time_signature);
    mach->param_long(i1);
    mach->param_long(i2);
    mach->param_long(i3);
    mach->param_long(i4);
    mach->call();
    mach->pop();
}


void Serp_midifile_reader::Mf_tempo(int tempo)
{
    call_i(s_tempo, tempo);
}


void Serp_midifile_reader::Mf_keysig(int i1, int i2)
{
    call_ii(s_key_signature, i1, i2);
}


void Serp_midifile_reader::Mf_sqspecific(int len, char *msg)
{
    call_ls(s_sq_specific, len, msg);
}


void Serp_midifile_reader::Mf_text(int type, int len, char *msg)
{
    call_ils(s_text, type, len, msg);
}


long Serp_midifile_reader_descriptor::mark(FExtern_ptr obj, Machine_ptr m)
{
    if_node_make_gray(((Serp_midifile_reader *) obj)->object, m);
    return 2;
}
