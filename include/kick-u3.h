#include <stddef.h>
#include <unistd.h>      /* UNIX standard function definitions */
#include <time.h>        /* Time definitions */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>      /* String function definitions */
#include <ctype.h>
#include <float.h>
#include <errno.h>       /* Error number definitions   */
#include <stdbool.h>     /* Boolean definitions */
#include <termios.h>     /* POSIX terminal control definitions */
#include <math.h>        /* Math definitions */
#include <signal.h>      /* Signal interrupt definitions */
#include <fcntl.h>       /* File control definitions */
#include <sys/types.h>   /* For shared and mapped memory definitions */
#include <sys/mman.h>    /* Mapped memory definitions */

struct sec_us {
  int sec;
  int ms;  
  int us;
} ;

struct mtc_par {
  int com0;                        // commands for programs to respond - main conduit between kick-u3 and kick-read
  int com1;                        // commands for programs to respond - update kick-mon time labels
  int com2;                        // commands for programs to respond - tapefaults
  int gtkstat;                     // code for status for gtk monitor program
  int onoff;                       // cycle running 
  int numMove;                     // number of moves the tape should do to remove sample
  int bkgRatio;                    // number of data cycles to 1 bkg cycle ( <0 = all data)
  time_t time0;                       // time since start of program
  time_t time1;                       // current time
  int cyclesData;                  // number of data cycles
  int cyclesBkg;                   // number of background cycles
  int cycleCurrent;                // cycle currently running
  int trigDT;                      // width of trigger signal
  uint8 meas[4];
  uint8 beam[4];
  uint8 mtc[4];
  uint8 lite[4];
  uint8 trig[4];
  uint8 kck[4];
  uint8 move[4];
  uint8 bkg[4];
  uint8 pulse_e[9];                  // vector storing output code word channels if eio
  uint8 pulse_c[9];                  // vector storing output code word channels if cio
  pid_t pid;                       // process ID
  HANDLE ljh;
  long int lj;
  int ljnum;
  bool trigbeam;            // trigger with beam on or not
  bool beammeas;            // beam on and measure (always true in takeaway mode)
  bool measbeam;            // measure with beam on or not (option for normal mode to make next sample)
  bool laserbeam;           // do the laser with beam or without
  bool normal;              // normal mode
  bool pause;               // pause mode
  bool takeaway;            // takeaway mode
  bool background;          // a background cycle
  bool tapeFault;           // tape fault 
  bool tapeBreak;            // read tape fault 
  struct sec_us tmove;             // time for tape to move in ms
  struct sec_us bon;               // time for beam on in ms
  struct sec_us boff;              // time for beam off in ms
  struct sec_us lon;               // time for lite on in ms
  struct sec_us pon;               // time for pause on in ms
  struct sec_us tdt;              // time of trigger width in ms
} ;

struct mtc_par *mtcptr;
#define FILESIZE sizeof(struct mtc_par)
#define FILEPATH "../data/mtc.bin"
#define kick_conf "../include/kick-u3.conf"
#define pulse_conf "../include/kick-u3-pulse.conf"
