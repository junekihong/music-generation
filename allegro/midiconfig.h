/* midiconfig.h -- system-specific definitions */
/* 
  NOTES: each implementation must have its own .h and .c files,
  e.g. midiwin32.h and midiwin32.c. These files implement the following:

1) Either LINUX, WIN32, or MACINTOSH should be defined.

2) The following function declaration:
	void midi_fail(char *msg);
or
	#define midi_fail some_function
(note that midi_fail must be a function pointer)

5) min() must be defined (either a macro or a function)

6) max() must be defined (either a macro or a function)

8) The following function to report failure and exit:
    void midi_fail(char *msg);

9) The following memory allocation routines:
    void *midi_alloc(size_t s);
    void midi_free(void *a);

10) midi_string_max -- string length for filenames, etc.

*/

#define midi_string_max 258
#define MIDI_MAX_BUF_LEN 512

#ifdef __cplusplus
extern "C" {
#endif

void midi_fail(char *msg);
void *midi_alloc(size_t s);
void midi_free(void *a);

#ifdef __cplusplus
}
#endif

#if defined(__linux__)
  #include "midilinux.h"

#elif defined(IRIX)
  #include "midiirix.h"

#elif defined(_WIN32)
  #ifndef WIN32
    #define WIN32
  #endif
  #include "midiwin32.h"

#endif