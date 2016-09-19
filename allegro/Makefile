# allegro Makefile
# 


LIBFLAGS = 
CFLAGS = -g -O5 -DLINUX
CCC = g++ 
CC  = gcc 


ALLEGROOBJS = 	allegro.o  \
		allegrosmfwr.o  \
		allegrord.o \
		allegrowr.o \
		allegrosmfrd.o  \
		mfmidi.o \
	  	strparse.o 

OBJS = $(ALLEGROOBJS) test.o 

CONVERTOBJS = $(ALLEGROOBJS) allegroconvert.o

#	trace.o
#	mfserpent.o  
#	test5.o 
#	seq2midi.o  


all: allegroTest allegroconvert

allegroTest: $(OBJS) Makefile 
	$(CCC) $(CFLAGS) -o allegroTest $(OBJS) $(LIBFLAGS)

allegroconvert: $(CONVERTOBJS) Makefile 
	$(CCC) $(CFLAGS) -o allegroconvert $(CONVERTOBJS) $(LIBFLAGS)

%.o:%.cpp
	$(CCC) -c $(CFLAGS) $< -o $@

clean: 
	rm -f allegroTest
	rm -f *.o 
