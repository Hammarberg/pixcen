#include "cruncher.h"
#include <stdio.h>
#include <stdlib.h>

#define log(format, ...)
//#define log(format, ...) fprintf (stderr, format, ## __VA_ARGS__)

#define NUM_BITS_SHORT_0 3
#define NUM_BITS_SHORT_1 6
#define NUM_BITS_SHORT_2 8
#define NUM_BITS_SHORT_3 10
#define NUM_BITS_LONG_0 4
#define NUM_BITS_LONG_1 7
#define NUM_BITS_LONG_2 10
#define NUM_BITS_LONG_3 13

#define LEN_SHORT_0 (1 << NUM_BITS_SHORT_0)
#define LEN_SHORT_1 (1 << NUM_BITS_SHORT_1)
#define LEN_SHORT_2 (1 << NUM_BITS_SHORT_2)
#define LEN_SHORT_3 (1 << NUM_BITS_SHORT_3)
#define LEN_LONG_0 (1 << NUM_BITS_LONG_0)
#define LEN_LONG_1 (1 << NUM_BITS_LONG_1)
#define LEN_LONG_2 (1 << NUM_BITS_LONG_2)
#define LEN_LONG_3 (1 << NUM_BITS_LONG_3)

#define COND_SHORT_0(o) ((o) >= 0 && (o) < LEN_SHORT_0)
#define COND_SHORT_1(o) ((o) >= LEN_SHORT_0 && (o) < LEN_SHORT_1)
#define COND_SHORT_2(o) ((o) >= LEN_SHORT_1 && (o) < LEN_SHORT_2)
#define COND_SHORT_3(o) ((o) >= LEN_SHORT_2 && (o) < LEN_SHORT_3)
#define COND_LONG_0(o) ((o) >= 0 && (o) < LEN_LONG_0)
#define COND_LONG_1(o) ((o) >= LEN_LONG_0 && (o) < LEN_LONG_1)
#define COND_LONG_2(o) ((o) >= LEN_LONG_1 && (o) < LEN_LONG_2)
#define COND_LONG_3(o) ((o) >= LEN_LONG_2 && (o) < LEN_LONG_3)

#define MAX_OFFSET LEN_LONG_3
#define MAX_OFFSET_SHORT LEN_SHORT_3

#define DECRUNCHER_LENGTH 0xd5
byte decrCode[DECRUNCHER_LENGTH] = {
  0x0b, 0x08, 0x00, 0x00, 0x9e, 0x32, 0x30, 0x36, 0x31, 0x00, 0x00, 0x00, 0x78, 0xa9, 0x34, 0x85,
  0x01, 0xa2, 0xb7, 0xbd, 0x1e, 0x08, 0x95, 0x0f, 0xca, 0xd0, 0xf8, 0x4c, 0x10, 0x00, 0xbd, 0xd6,
  0x07, 0x9d, 0x00, 0xff, 0xe8, 0xd0, 0xf7, 0xc6, 0x12, 0xc6, 0x15, 0xa5, 0x12, 0xc9, 0x07, 0xb0,
  0xed, 0x20, 0xa0, 0x00, 0xb0, 0x17, 0x20, 0x8e, 0x00, 0x85, 0x36, 0xa0, 0x00, 0x20, 0xad, 0x00,
  0x91, 0x77, 0xc8, 0xc0, 0x00, 0xd0, 0xf6, 0x20, 0x83, 0x00, 0xc8, 0xf0, 0xe4, 0x20, 0x8e, 0x00,
  0xaa, 0xe8, 0xf0, 0x71, 0x86, 0x7b, 0xa9, 0x00, 0xe0, 0x03, 0x2a, 0x20, 0x9b, 0x00, 0x20, 0x9b,
  0x00, 0xaa, 0xb5, 0xbf, 0xf0, 0x07, 0x20, 0x9b, 0x00, 0xb0, 0xfb, 0x30, 0x07, 0x49, 0xff, 0xa8,
  0x20, 0xad, 0x00, 0xae, 0xa0, 0xff, 0x65, 0x77, 0x85, 0x74, 0x98, 0x65, 0x78, 0x85, 0x75, 0xa0,
  0x00, 0xb9, 0xad, 0xde, 0x99, 0x00, 0x00, 0xc8, 0xc0, 0x00, 0xd0, 0xf5, 0x20, 0x83, 0x00, 0xd0,
  0xa0, 0x18, 0x98, 0x65, 0x77, 0x85, 0x77, 0x90, 0x02, 0xe6, 0x78, 0x60, 0xa9, 0x01, 0x20, 0xa0,
  0x00, 0x90, 0x05, 0x20, 0x9b, 0x00, 0x10, 0xf6, 0x60, 0x20, 0xa0, 0x00, 0x2a, 0x60, 0x06, 0xbe,
  0xd0, 0x08, 0x48, 0x20, 0xad, 0x00, 0x2a, 0x85, 0xbe, 0x68, 0x60, 0xad, 0xed, 0xfe, 0xe6, 0xae,
  0xd0, 0x02, 0xe6, 0xaf, 0x60, 0xa9, 0x37, 0x85, 0x01, 0x4c, 0x00, 0x00, 0x80, 0xdf, 0xfb, 0x00,
  0x80, 0xef, 0xfd, 0x80, 0xf0
};


byte *ibuf;
byte *obuf;
uint ibufSize;
int get; //points to ibuf[]
uint put; //points to obuf[]

typedef struct {
  uint cost;
  uint next;
  uint litLen;
  uint offset;
} node;

typedef struct {
  byte value;
  byte valueAfter;
  uint length;
} RLEInfo;

node *context;
uint *link;
RLEInfo *rleInfo;
uint first[65536];
uint last[65536];

byte curByte;
byte curCnt;
uint curIndex;

void wBit(uint bit) {
  if(curCnt == 0) {
    obuf[curIndex] = curByte;
    curIndex = put;
    curCnt = 8;
    curByte = 0;
    put++;
  }

  curByte <<= 1;
  curByte |= (bit & 1);
  curCnt--;
}

void wFlush() {
  while(curCnt != 0) {
    curByte <<= 1;
    curCnt--;
  }
  obuf[curIndex] = curByte;
}

void wByte(uint b) {
  obuf[put] = b;
  put++;
}

void wBytes(uint get, uint len) {
  uint i;
  for(i = 0; i < len; i++) {
    wByte(ibuf[get]);
    get++;
  }
}

void wLength(uint len) {
//  if(len == 0) return; // Should never happen

  byte bit = 0x80;
  while((len & bit) == 0) {
    bit >>= 1;
  }

  while(bit > 1) {
    wBit(1);
    bit >>= 1;
    wBit(((len & bit) == 0) ? 0 : 1);
  }

  if(len < 0x80) {
    wBit(0);
  }
}

void wOffset(uint offset, uint len) {
  uint i = 0;
  uint n = 0;
  uint b;

  if(len == 1) {
    if(COND_SHORT_0(offset)) {
      i = 0;
      n = NUM_BITS_SHORT_0;
    }
    if(COND_SHORT_1(offset)) {
      i = 1;
      n = NUM_BITS_SHORT_1;
    }
    if(COND_SHORT_2(offset)) {
      i = 2;
      n = NUM_BITS_SHORT_2;
    }
    if(COND_SHORT_3(offset)) {
      i = 3;
      n = NUM_BITS_SHORT_3;
    }
  } else {
    if(COND_LONG_0(offset)) {
      i = 0;
      n = NUM_BITS_LONG_0;
    }
    if(COND_LONG_1(offset)) {
      i = 1;
      n = NUM_BITS_LONG_1;
    }
    if(COND_LONG_2(offset)) {
      i = 2;
      n = NUM_BITS_LONG_2;
    }
    if(COND_LONG_3(offset)) {
      i = 3;
      n = NUM_BITS_LONG_3;
    }
  }

  // First write number of bits
  wBit(((i & 2) == 0) ? 0 : 1);
  wBit(((i & 1) == 0) ? 0 : 1);

  if(n >= 8) { // Offset is 2 bytes

    // Then write the bits less than 8
    b = 1 << n;
    while(b > 0x100) {
      b >>= 1;
      wBit(((b & offset) == 0) ? 0 : 1);
    };

    // Finally write a whole byte, if necessary
    wByte(offset & 255 ^ 255); // Inverted(!)
    offset >>= 8;

  } else { // Offset is 1 byte

    // Then write the bits less than 8
    b = 1 << n;
    while(b > 1) {
      b >>= 1;
      wBit(((b & offset) == 0) ? 1 : 0); // Inverted(!)
    };

  }
}


/*
 * Cost functions
 */
uint costOfLength(uint len) {
  if(len == 1) return 1;
  if(len >= 2 && len <= 3) return 3;
  if(len >= 4 && len <= 7) return 5;
  if(len >= 8 && len <= 15) return 7;
  if(len >= 16 && len <= 31) return 9;
  if(len >= 32 && len <= 63) return 11;
  if(len >= 64 && len <= 127) return 13;
  if(len >= 128 && len <= 255) return 14;

  printf("costOfLength got wrong value: %i\n", len);
  return 10000;
}

uint costOfOffset(uint offset, uint len) {
  if(len == 1) {
    if(COND_SHORT_0(offset)) return NUM_BITS_SHORT_0;
    if(COND_SHORT_1(offset)) return NUM_BITS_SHORT_1;
    if(COND_SHORT_2(offset)) return NUM_BITS_SHORT_2;
    if(COND_SHORT_3(offset)) return NUM_BITS_SHORT_3;
  } else {
    if(COND_LONG_0(offset)) return NUM_BITS_LONG_0;
    if(COND_LONG_1(offset)) return NUM_BITS_LONG_1;
    if(COND_LONG_2(offset)) return NUM_BITS_LONG_2;
    if(COND_LONG_3(offset)) return NUM_BITS_LONG_3;
  }

  printf("costOfOffset got wrong offset: %i\n", offset);
  return 10000;
}

uint calculateCostOfMatch(uint len, uint offset) {
  uint cost = 1; // Copy-bit
  cost += costOfLength(len - 1);
  cost += 2; // NumOffsetBits
  cost += costOfOffset(offset - 1, len - 1);
  return cost;
}

uint calculateCostOfLiteral(uint oldCost, uint litLen) {
  uint newCost = oldCost + 8;

  // FIXME, what if litLen > 255?
  //
  // FIXME, cost model for literals does not work.
  // Quick wins on short matches are prioritized before
  // a longer literal run, which in the end results in a
  // worse result.
  // Most obvious on files hard to crunch.
  switch(litLen) {
  case 1:
  case 128:
    newCost++;
    break;
  case 2:
  case 4:
  case 8:
  case 16:
  case 32:
  case 64:
    newCost += 2;
    break;
  default:
    break;
  }

  return newCost;
}


void setupHelpStructures() {
  uint i;

  // Setup RLE-info
  get = ibufSize - 1;
  while (get > 0) {

    byte cur = ibuf[get];
    if (cur == ibuf[get-1]) {

      uint len = 2;
      while ((get >= len) && 
	     (cur == ibuf[get-len])) {
	len++;
      }

      rleInfo[get].length = len;
      if (get >= len) {
	rleInfo[get].valueAfter = ibuf[get-len];
      } else {
	rleInfo[get].valueAfter = cur; // Avoid accessing ibuf[-1]
      }

      get -= len;
    } else {
      get--;
    }
  }


  // Setup Linked list
  for (i = 0; i < 65536; i++) {
    first[i] = 0;
    last[i] = 0;
  }

  get = ibufSize - 1;
  uint cur = ibuf[get];

  while (get > 0) {

    cur = ((cur << 8) | ibuf[get-1]) & 65535;

    if (first[cur] == 0) {
      first[cur] = last[cur] = get;
    } else {
      link[last[cur]] = get;
      last[cur] = get;
    }

    if (rleInfo[get].length == 0) { // No RLE-match here..
      get--;
    } else { // if RLE-match..
      get -= (rleInfo[get].length - 1);
    }

  }
}


void findMatches() {

  typedef struct match {
    uint length;
    uint offset;
  } match;

  match matches[256];

  node lastNode;
  uint i;

  get = ibufSize - 1;
  uint cur = ibuf[get];

  lastNode.cost = 0;
  lastNode.next = 0;
  lastNode.litLen = 0;

  while (get >= 0) {

    // Clear matches for current position
    for (i = 0; i < 256; i++) {
      matches[i].length = 0;
      matches[i].offset = 0;
    }

    cur = (cur << 8) & 65535; // Table65536 lookup
    if (get > 0) cur |= ibuf[get-1];
    int scn = first[cur];
    scn = link[scn];

    uint longestMatch = 0;

    if (rleInfo[get].length == 0) { // No RLE-match here..

      // Scan until start of file, or max offset
      while (((get - scn) <= MAX_OFFSET) &&
	     (scn > 0) &&
	     (longestMatch < 255)) {

	// Ok, we have a match of length 2
	// ..or longer, but max 255 or file start
	uint len = 2;
	while ((len < 255) &&
	       (scn >= len) &&
	       (ibuf[scn - len] == ibuf[get - len])) {
	  ++len;
	}

	// Calc offset
	uint offset = get - scn;

	// Store match only if it's the longest so far
	if(len > longestMatch) {
	  longestMatch = len;

	  // Store the match only if first (= best) of this length
	  while(len >= 2 && matches[len].length == 0) {

	    // If len == 2, check against short offset!!
	    if ((len > 2) ||
		((len == 2) && (offset <= MAX_OFFSET_SHORT))) {
	      matches[len].length = len;
	      matches[len].offset = get - scn;
	    }

	    len--;
	  };
	}

	scn = link[scn]; // Table65535 lookup
      };

      first[cur] = link[first[cur]]; // Waste first entry

    } else { // if RLE-match..

      uint rleLen = rleInfo[get].length;
      byte rleValAfter = rleInfo[get].valueAfter;


      // First match with self-RLE, which is always
      // one byte shorter than the RLE itself.
      uint len = rleLen - 1;
      if (len > 1) {
	if (len > 255) len = 255;
	longestMatch = len;

	// Store the match
	while(len >= 2) {
	  matches[len].length = len;
	  matches[len].offset = 1;

	  len--;
	};
      }


      // Search for more RLE-matches..
      // Scan until start of file, or max offset
      while (((get - scn) <= MAX_OFFSET) &&
	     (scn > 0) &&
	     (longestMatch < 255)) {

	// Check for longer matches with same value and after..
	// FIXME, that is not what it does, is it?!
	if ((rleInfo[scn].length > longestMatch) &&
	    (rleLen > longestMatch)) {

	  uint offset = get - scn;
	  len = rleInfo[scn].length;

	  if (len > rleLen)
	    len = rleLen;

	  if ((len > 2) ||
	      ((len == 2) && (offset <= MAX_OFFSET_SHORT))) {
	    matches[len].length = len;
	    matches[len].offset = offset;

	    longestMatch = len;
	  }
	}


	// Check for matches beyond the RLE..
	if ((rleInfo[scn].length >= rleLen) &&
	    (rleInfo[scn].valueAfter == rleValAfter)) {
	 
	  // Here is a match that goes beyond the RLE..
	  // Find out correct offset to use valueAfter..
	  // Then search further to see if more bytes equal.

	  len = rleLen;
	  uint offset = get - scn + (rleInfo[scn].length - rleLen);

	  if (offset <= MAX_OFFSET) {
	    while ((len < 255) &&
		   (get >= (offset + len)) &&
		   (ibuf[get - (offset + len)] == ibuf[get - len])) {
	      ++len;
	    }
	    if (len > longestMatch){
	      longestMatch = len;

	      // Store the match only if first (= best) of this length
	      while(len >= 2 && matches[len].length == 0) {

		// If len == 2, check against short offset!!
		if ((len > 2) ||
		    ((len == 2) && (offset <= MAX_OFFSET_SHORT))) {
		  matches[len].length = len;
		  matches[len].offset = offset;
		}

		len--;
	      }; //while
	    }
	  }
	}

	scn = link[scn]; // Table65535 lookup
      }

      
      if (rleInfo[get].length > 2) {
	// Expand RLE to next position
	rleInfo[get-1].length = rleInfo[get].length - 1;
	rleInfo[get-1].value = rleInfo[get].value;
	rleInfo[get-1].valueAfter = rleInfo[get].valueAfter;
      } else {
	// End of RLE, advance link.
	first[cur] = link[first[cur]]; // Waste first entry
      }
    }


    // Now we have all matches from this position..
    // ..visit all nodes reached by the matches.

    for (i = 255; i > 0; i--) {

      // Find all matches we stored
      uint len = matches[i].length;
      uint offset = matches[i].offset;

      if (len != 0) {

	uint targetI = get - len + 1;
    	node* target = &context[targetI];

    	// Calculate cost for this jump
    	uint currentCost = lastNode.cost;
	currentCost += calculateCostOfMatch(len, offset);

    	// If this match is first or cheapest way to get here
    	// then update node
    	if (target->cost == 0 ||
    	    target->cost > currentCost) {

	  target->cost = currentCost;
    	  target->next = get + 1;
    	  target->litLen = 0;
	  target->offset = offset;
	}
      }
    }


    // Calc the cost for this node if using one more literal
    uint litLen = lastNode.litLen + 1;
    uint litCost = calculateCostOfLiteral(lastNode.cost, litLen);

    // If literal run is first or cheapest way to get here
    // then update node
    node* this = &context[get];
    if (this->cost == 0 ||
    	this->cost >= litCost) {
      this->cost = litCost;
      this->next = get + 1;
      this->litLen = litLen;
    }

    lastNode.cost = this->cost;
    lastNode.next = this->next;
    lastNode.litLen = this->litLen;

    // Loop to the next position
    get--;
  };

}


// Returns margin
int writeOutput() {
  uint i;

  put = 0;

  curByte = 0;
  curCnt = 8;
  curIndex = put;
  put++;

  int maxDiff = 0;

  bool needCopyBit = true;

  for (i = 0; i < ibufSize;) {

    uint link = context[i].next;
    uint cost = context[i].cost;
    uint litLen = context[i].litLen;
    uint offset = context[i].offset;

    if (litLen == 0) {
      // Put Match
      uint len = link - i;

      log("$%04x: Mat(%i, %i)\n", i, len, offset);
  
      if(needCopyBit) {
	wBit(1);
      }
      wLength(len - 1);
      wOffset(offset - 1, len - 1);

      i = link;

      needCopyBit = true;
    } else {
      // Put LiteralRun
      needCopyBit = false;

      while(litLen > 0) {
	uint len = litLen < 255 ? litLen : 255;

	log("$%04x: Lit(%i)\n", i, len);

	wBit(0);
	wLength(len);
	wBytes(i, len);

	if (litLen == 255) {
	  needCopyBit = true;
	}

	litLen -= len;
	i += len;
      };
    }

    if ((int)(i - put) > maxDiff) {
      maxDiff = i - put;
    }

  }

  wBit(1);
  wLength(0xff);
  wFlush();

  int margin = (maxDiff - (i - put));

  return margin;
}


bool crunch(File *aSource,
	    File *aTarget,
	    uint address,
	    bool isExecutable,
	    bool isRelocated)
{
  uint i;
  byte *target;

  ibufSize = aSource->size - 2;
  ibuf = (byte*)malloc(ibufSize);
  context = (node*)malloc(sizeof(node) * ibufSize);
  link = (uint*)malloc(sizeof(uint) * ibufSize);
  rleInfo = (RLEInfo*)malloc(sizeof(RLEInfo) * ibufSize);

  // Load ibuf and clear context
  for(i = 0; i < ibufSize; ++i) {
    ibuf[i] = aSource->data[i + 2];
    context[i].cost = 0;
    link[i] = 0;
    rleInfo[i].length = 0;
  }

  setupHelpStructures();
  findMatches();
  obuf = (byte*)malloc(memSize);
  int margin = writeOutput();

  uint packLen = put;
  uint fileLen = put;
  uint decrLen = 0;
  if(isExecutable) {
    decrLen = DECRUNCHER_LENGTH;
    fileLen += decrLen + 2;
  } else {
    fileLen += 4;
  }

  aTarget->size = fileLen;
  aTarget->data = (byte*)malloc(aTarget->size);
  target = aTarget->data;

  if(isExecutable) {
    uint startAddress = 0x10000 - packLen;
    uint transfAddress = fileLen + 0x6ff;

    decrCode[0x1f] = transfAddress & 0xff; // Transfer from..
    decrCode[0x20] = transfAddress >> 8;   //
    decrCode[0xbc] = startAddress & 0xff; // Depack from..
    decrCode[0xbd] = startAddress >> 8;   //
    decrCode[0x85] = aSource->data[0]; // Depack to..
    decrCode[0x86] = aSource->data[1]; //
    decrCode[0xca] = address & 0xff; // Jump to..
    decrCode[0xcb] = address >> 8;   //

    target[0] = 0x01;
    target[1] = 0x08;

    for(i = 0; i < decrLen; ++i) {
      target[i + 2] = decrCode[i];
    }

    for(i = 0; i < put; ++i) {
      target[i + 2 + decrLen] = obuf[i];
    }

  } else { // Not executable..

    // Experimantal decision of start address
//    uint startAddress = 0xfffa - packLen - 2;
    uint startAddress = (aSource->data[1] << 8) | aSource->data[0];
    startAddress += (ibufSize - packLen - 2 + margin);

    if (isRelocated) {
      startAddress = address - packLen - 2;
    }

    target[0] = startAddress & 0xff; // Load address
    target[1] = startAddress >> 8;
    target[2] = aSource->data[0]; // Depack to address
    target[3] = aSource->data[1];

    for(i = 0; i < put; ++i) {
      target[i + 4] = obuf[i];
    }
  }

  free(ibuf);
  free(context);
  free(link);
  free(rleInfo);

  return true;
}
