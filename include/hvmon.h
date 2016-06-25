#include <unistd.h>  /* UNIX standard function definitions */
#include <time.h>    /* Time definitions */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <ctype.h>
#include <math.h>    /* Math definitions */
#include <signal.h>  /* Signal interrupt definitions */
//#include <stddef.h>
#include <limits.h>
//#include <float.h>
//#include <termios.h> /* POSIX terminal control definitions */
//#include <errno.h>   /* Error number definitions */
//#include <sys/ipc.h>   /* for shared memory stuff */
//#include <sys/shm.h>   /* for shared memory stuff */
#include <fcntl.h>   /* File control definitions */
#include <sys/types.h> /* for shared memory stuff */
#include <sys/mman.h>   /* for memory mapping stuff */
#include <sys/time.h>

#define HVMONDATAPATH "../data/hvmon.bin"            // user data for input to xia2disk
#define HVMONDATASIZE sizeof(struct hvmon)


struct hvchan {
  int type;
  char ip[30];    // ip address used for MPOD and CAEN
  int caenH;    // caen handle
  short unsigned int chan;
  short unsigned int slot;
  float vSet;
  float vRamp; 
  float vMeas;
  float iSet;
  float iMeas;
};

struct hvmon {
  int pid;
  int com0;
  int com1;
  int com2;
  time_t secRunning;
  struct hvchan xx[1000];
}hv;

struct hvmon *hvptr;



