#ifndef _bb_h_
#define _bb_h_

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef byte
typedef unsigned char byte;
#endif
#ifndef uint
typedef unsigned int uint;
#endif

#ifndef BBWRAP
typedef enum { false = 0, true = 1 } bool;
#endif

#define memSize 65536

#endif // _bb_h_
