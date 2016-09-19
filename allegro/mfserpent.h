// midifile reader interface for serpent

class Serp_midifile_reader: public Midifile_reader {
public:
    Machine *mach;
    FILE *file;
    Object_ptr object;

    void initialize(Object_ptr obj, FILE *f);
    void *operator new(unsigned int size, Machine_ptr m) {
        Serp_midifile_reader *smfr = (Serp_midifile_reader *) m->memget(size);
        smfr->mach = m;
        return smfr; }
    Serp_midifile_reader() { file = NULL; }

    void set_nomerge(bool flag) { Mf_nomerge = flag; }
    void set_skipinit(bool flag) { Mf_skipinit = flag; }
    long get_currtime() { return Mf_currtime; }

    // these are not static because we might have 2 copies of the 
    // machine in one address space. Symbols are per-machine.
    Symbol_ptr s_starttrack;
    Symbol_ptr s_endtrack;
    Symbol_ptr s_eot;
    Symbol_ptr s_header;
    Symbol_ptr s_note_on;
    Symbol_ptr s_note_off;
    Symbol_ptr s_pressure;
    Symbol_ptr s_controller;
    Symbol_ptr s_pitchbend;
    Symbol_ptr s_program;
    Symbol_ptr s_channel_pressure;
    Symbol_ptr s_sysex;
    Symbol_ptr s_arbitrary;
    Symbol_ptr s_metamisc;
    Symbol_ptr s_seqnum;
    Symbol_ptr s_smpte;
    Symbol_ptr s_time_signature;
    Symbol_ptr s_tempo;
    Symbol_ptr s_key_signature;
    Symbol_ptr s_sq_specific;
    Symbol_ptr s_text;

protected:
    void call_none(Symbol_ptr meth);
    void call_i(Symbol_ptr meth, int i1);
    void call_ii(Symbol_ptr meth, int i1, int i2);
    void call_iii(Symbol_ptr meth, int i1, int i2, int i3);
    void call_ls(Symbol_ptr meth, int len, char *msg);
    void call_ils(Symbol_ptr meth, int i, int len, char *msg);

    void *Mf_malloc(size_t size); /* malloc() */
    void Mf_free(void *obj, size_t size);
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

class Serp_midifile_reader_descriptor : public Descriptor {
public:
    long mark(FExtern_ptr obj, Machine_ptr m);
    size_t get_size() { return sizeof(Serp_midifile_reader); }
};

/*SER class Serp_midifile_reader PENT*/

#define smfr_create(m) new(m) Serp_midifile_reader
/*SER extern Serp_midifile_reader smfr_create(Machine) PENT*/
#define smfr_initialize(smfr, obj, file) (smfr)->initialize(obj, file)
/*SER void smfr_initialize(extern Serp_midifile_reader, Object_ptr, FILE *) PENT*/
#define smfr_set_nomerge(smfr, b) (smfr)->set_nomerge(b)
/*SER void smfr_set_nomerge(extern Serp_midifile_reader, bool) PENT*/
#define smfr_set_skipinit(smfr, b) (smfr)->set_skipinit(b)
/*SER void smfr_set_skipinit(extern Serp_midifile_reader, bool) PENT*/
#define smfr_get_currtime(smfr) (smfr)->get_currtime()
/*SER long smfr_get_currtime(extern Serp_midifile_reader) PENT*/
