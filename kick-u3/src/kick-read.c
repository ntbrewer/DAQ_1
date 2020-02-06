
#include "../../include/u3.h"
#include "../../include/kick-u3.h"

//#define FILEPATH "data/mtc.bin"
//#define kick_conf "include/kick-u3.conf"

struct sec_us time_In_ms (struct sec_us xx);   // my timer routines and structure for times

int mmapSetup();          // setup memory map files
void menu();              // list of control options
void clearParameters();   // clear parameters
int toggle(int jj);       // toggle a bool or int
int scan2int ();          // read an int as string and make sure its an int : -1 = error
int parameters();         // options to use
void clearStats();        // clear cycle stats
void help();              // help
void WaveformDiagram();   // visualization of cycle
char ESC=27;

/***********************************************************/
int main(int argc, char **argv){
  //  int fd=0;
  int ans=0, mapKick=0;
  int kk=0;
/*
  Shared memory creation and attachment
  the segment number is stored in lnptr->pid
*/
  mapKick = mmapSetup();
  if (mapKick == -1) {
    printf(" Error on setting up memory map ... exiting \n");
    return 0;
  }
/*
  Scroll through options
*/
  while (ans != 100){
    menu();
    ans = scan2int();
    //    scanf ("%i", &ans);          // read in ans (pointer is indicated)     
    if (ans < 0 || ans > 100) ans = 1;   // give status in response
    if (mtcptr->com0 == 100){
      printf("kick-u3 must have received an exit command from another kick-read program so this program is ending too!\n");
      ans=100;
    }
    switch (ans){
    case 0:                    // end program but not deg-u3
      ans=100;
      break;
    case 1:                   // status
      mtcptr->com0 = 0;
      break;
    case 2:                   // run pulsing
      mtcptr->onoff = 1;
      mtcptr->com0 = 0;
      break;
    case 3:                   // stop pulsing
      mtcptr->onoff = 0;
      mtcptr->com0 = 0;
      break;
    case 4:                   // continuous beam
      mtcptr->onoff = 0;
      mtcptr->com0 = 3;
      break;
    case 5:                   // stop beam   
      mtcptr->onoff = 0;
      mtcptr->com0 = 2;
      break;
    case 6:                   // move MTC
      mtcptr->onoff = 0;
      mtcptr->com0 = 6;
      break;
    case 7:                   // set parameters
      kk = parameters();
      if (kk == 0) {
	mtcptr->com0 = 7;     // mode and parameters changed so load Arrays must be called
	mtcptr->com1 = 1;     // mode and parameters changed so kick-mon must be told
      }
	break;
    case 8:                   // clear stats
      clearStats();
      break;
    case 9:                    // reset mtc faults
      mtcptr->tapeBreak = 0;
      mtcptr->tapeFault = 0;
      mtcptr->com2 = 0;
      break;
    case 10:                  // laser calibrator on
      mtcptr->onoff = 0;
      mtcptr->com0 = 4;
      break;

    case 11:                   // all LJ off
      mtcptr->onoff = 0;
      mtcptr->com0 = 5;
      break;
          
    case 12:                   // all LJ off
      mtcptr->onoff = 0;
      mtcptr->com0 = 8;
      break;

    case 99:                   // help
      help();
      break;
    case 100:                 // End ALL deg-u3 programs by breaking out of while statement
      ans = 100;       //send end command to main program
      mtcptr->onoff=0;
      mtcptr->com0 = 100;
      break;

    default:                  // Do nothing and go back to the list of options
      ans = 0;
      break;      
    }  
  }

/*
  Ending the program so release the memory map and close file
*/

  if (munmap(mtcptr, sizeof (struct mtc*)) == -1) {
    perror("Error un-mapping the file");
  }
  close(mapKick);

  printf(" File closed and file unmapped \n");
  
  return (0);
}

/**************************************************************/
void clearParameters(){
  
  mtcptr->onoff = 0;     // set all modes off

  mtcptr->normal = 0; 
  mtcptr->takeaway = 0;
  mtcptr->pause = 0;

  mtcptr->lon.ms=0;                         // clear lite calibrator time
  mtcptr->lon = time_In_ms(mtcptr->lon);
  mtcptr->pon.ms=0;                         // clear pause time
  mtcptr->pon = time_In_ms(mtcptr->pon);
  
  mtcptr->beammeas = 0;    // always with takeaway
  mtcptr->measbeam = 0;    // option for normal and pause

  mtcptr->trigbeam = 1;    // option: default is true
  mtcptr->laserbeam = 0;   // option for MTAS
  mtcptr->background = 0;  // option for MTAS
  mtcptr->bkgRatio = -1;    // option for MTAS
  mtcptr->numMove = 1;     // option for normal/pause mode
 
  mtcptr->trigDT=32;
  return;
}

/**************************************************************/
void clearStats(){
  mtcptr->cyclesData = 0;
  mtcptr->cyclesBkg = 0;
  
  return;
}
/**************************************************************/
int parameters(){
  short unsigned int ans = 0x00ff;
  char yn[10]="\0", yes[10]="y";
  int ii=0, jj=0;
  
  if (mtcptr->onoff) {
    printf(" Any changes will take effect immediately unless we stop the current run - stop? y/n \n");
    scanf("%s",yn);
    if ( !strcmp(yn,yes) ) mtcptr->onoff = 0;
    //    if (strncmp(yn,"y",1) || strncmp(yn,"Y",1)) mtcptr->onoff = 0;
    else return (1);
  }

// once things are set-up initially one usually only needs to change the grow-in, decay-out, and pause times
// so offer the user a chance to leave this routine
  
  printf (" Beam on (ms), current setting = %i \n", mtcptr->bon.ms);
  //  scanf ("%i",&ii);
  mtcptr->bon.ms = scan2int ();
  //  mtcptr->bon.ms = ii;
  mtcptr->bon = time_In_ms(mtcptr->bon);     // set sec and us values in structure
  
  printf (" Beam off (ms), current setting = %i \n", mtcptr->boff.ms);
  //  scanf ("%i",&ii);
  mtcptr->boff.ms = scan2int ();//ii;
  mtcptr->boff = time_In_ms(mtcptr->boff);     // set sec and us values in structure

  if (mtcptr->pause) {
    printf (" Pause on - For how long (ms) ?\n");
    //    scanf ("%i",&ii);
    mtcptr->pon.ms = scan2int ();//ii;
    mtcptr->pon = time_In_ms(mtcptr->pon);     // set sec and us values in structure
  }

  strcpy(yn,"\0");
  printf(" Are you finished with your changes ? y/n \n");
  scanf("%s",yn);
  if ( !strcmp(yn,yes) ) return (0);

  // reset the other parameters and go to the defaults

  //  clearParameters();   // clear all changeable parameters
  
    printf (" Which mode ? \n    1 - normal \n    2 - takeaway \n    3 - pause \n");
    ii = scan2int ();   //scanf ("%i",&ii);
    if (ii == 3) {
      mtcptr->pause = 1;  // pause is an option on normal mode so set both
      mtcptr->normal = 1;
      mtcptr->takeaway = 0;
      printf (" Pause on - For how long (ms) ?\n");
      //      scanf ("%i",&ii);
      mtcptr->pon.ms = scan2int ();//ii;
      mtcptr->pon = time_In_ms(mtcptr->pon);     // set sec and us values in structure
    }
    else if (ii == 2) {
      mtcptr->takeaway = 1;
      mtcptr->pause = 0;
      mtcptr->normal = 0;
      mtcptr->beammeas = 1;
      mtcptr->measbeam = 0;
      mtcptr->numMove = 1;
      mtcptr->trigbeam = 1; // deliver beam with the trigger
      ans -= 0x0038;       // takeaway always measures grow-in, has 1 mtc move, and can't have beam on during decay-out 
    }
    else {
      mtcptr->normal = 1;
      mtcptr->takeaway = 0;
      mtcptr->pause = 0;
    }
    printf (" Current values: normal: %i, takeaway: %i, pause: %i\n",mtcptr->normal,mtcptr->takeaway,mtcptr->pause);

  while (ans != 0) {
    printf (" Type number of parameter to change; current value in parantheses)\n");

    if (ans & 0x0001) printf ("   1 - set the duration of the tape movement (%i ms)\n",mtcptr->tmove.ms);
    if (ans & 0x0002) printf ("   2 - set laser calibration parameters (%i ms)\n",mtcptr->lon.ms);
    if (ans & 0x0004) printf ("   3 - set the background data ratio (%i)\n",mtcptr->bkgRatio);
    printf ("\n");
    if (ans & 0x0008) printf ("   4 - number of tape moves at end of normal/pause cycles (%i)\n",mtcptr->numMove);
    if (ans & 0x0010) printf ("   5 - measure while the beam is on   (%i)\n",mtcptr->beammeas);
    if (ans & 0x0020) printf ("   6 - put the beam on when measuring decay (%i)\n",mtcptr->measbeam);
    printf ("\n");
    if (ans & 0x0040) printf ("   7 - trigger signal with beam (%i)\n",mtcptr->trigbeam);
    if (ans & 0x0080) printf ("   8 - trigger signal width (%i us)\n",mtcptr->trigDT);
    printf ("\n");
    printf ("   9 - accept the rest of the default parameters \n");
    printf ("   0 - end this section ans = %x\n",ans);
    
    jj = scan2int ();//scanf("%i",&jj);

    switch (jj){
    case 1:
      printf (" Tape move (ms), current setting = %i \n", mtcptr->tmove.ms);
      //      scanf ("%i",&ii);
      mtcptr->tmove.ms = scan2int ();//ii;
      mtcptr->tmove = time_In_ms(mtcptr->tmove);     // set sec and us values in structure
      printf (" Current value: %i ms\n",mtcptr->tmove.ms);
      ans -= 0x0001;
      break;
    case 2:
      printf (" Laser on (ms), current setting = %i \n", mtcptr->lon.ms);
      //      scanf ("%i",&ii);
      mtcptr->lon.ms = scan2int ();//ii;
      mtcptr->lon = time_In_ms(mtcptr->lon);     // set sec and us values in structure

      if (mtcptr->laserbeam) printf("Toggle beam off with laser? y/n \n");
      else printf("Toggle beam on with laser? y/n \n");
      scanf ("%s",yn);
      if (!strcmp(yn,yes)) mtcptr->laserbeam = toggle(mtcptr->laserbeam) ;
      if (mtcptr->laserbeam) printf (" Laser w/ beam for  %i ms\n",mtcptr->lon.ms);
      else  printf (" Laser w/out beam for  %i ms\n",mtcptr->lon.ms);
      ans -= 0x0002;
      break;
    case 3:
      printf (" Number of data cycles for 1 background cycle, current setting = %i \n",mtcptr->bkgRatio);
      printf (" -1 for all data: \n");
      printf ("  0 for all background \n");
      //      scanf ("%i",&ii);
      mtcptr->bkgRatio = scan2int ();//ii;
      printf (" Current value: %i \n",mtcptr->bkgRatio);
      ans -= 0x0004;
      break;
    case 4:
      printf (" Tape moves 1 or 2 to end the cycle?, current setting = %i \n", mtcptr->numMove);
      ii = scan2int ();//scanf ("%i",&ii);
      if (ii == 1 || ii == 2) mtcptr->numMove = ii;
      else  mtcptr->numMove = 1;
      printf (" Current value: %i \n",mtcptr->numMove);
      ans -= 0x0008;
      break;
    case 5:
      if (mtcptr->beammeas) printf (" Measure with beam off (grow-in cycle)?  y/n  \n");
      else printf (" Measure with beam on (grow-in cycle)? y/n  \n");
      scanf("%s",yn);
      if (!strcmp(yn,yes)) mtcptr->beammeas = toggle(mtcptr->beammeas) ;
      printf (" Current value: %i \n",mtcptr->beammeas);
      ans -= 0x0010;
      break;
    case 6:
      if (mtcptr->measbeam) printf (" Measure without beam making the next sample ? y/n \n");
      else printf (" Measure with beam making the next sample ? y/n  \n");
      scanf("%s",yn);
      if (!strcmp(yn,yes)) mtcptr->measbeam = toggle(mtcptr->measbeam) ;
      printf (" Current value: %i \n",mtcptr->measbeam);
      ans -= 0x0020;
      break;
    case 7:
      if (mtcptr->trigbeam) printf (" Deliver beam without the trigger signal ? y/n \n");
      else printf (" Deliver beam with the trigger signal? y/n  \n");
      scanf("%s",yn);
      if (!strcmp(yn,yes)) mtcptr->trigbeam = toggle(mtcptr->trigbeam) ;
      printf (" Current value: %i \n",mtcptr->trigbeam);
      ans -= 0x0040;
      break;
    case 8:
      printf("Width of trigger signal (< 4000 us), current setting = %i us\n",mtcptr->trigDT);
      ii = scan2int ();//scanf ("%i",&ii);
      if (ii >= 16) mtcptr->trigDT = ii;
      else mtcptr->trigDT = 16;
    printf("before decrement ans = %x\n",ans);
      ans -= 0x0080;
    printf("after decrement ans = %x\n",ans);
      printf (" Current value: %i us\n",mtcptr->trigDT);
      break;
    case 9:
      ans = 0;
      break;
    case 0:
      break;
    default:
      break;
    }
  }
  
  return (0);
}
/**************************************************************/
int mmapSetup() {
  int fd=0;     // mapped file descriptor

  /* Open a file for writing.
   *  - Creating the file if it doesn't exist.
   *  - Truncating it to 0 size if it already exists. (not really needed)
   *
   * Note: "O_WRONLY" mode is not sufficient when mmaping.
   */
  fd = open(FILEPATH, O_RDWR, (mode_t)0600);
  if (fd == -1) {
    perror("Error opening file path for writing");
    exit(EXIT_FAILURE);
  }
        
  /* Now the file is ready to be mmapped.
   */
  mtcptr = (struct mtc_par*) mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (mtcptr == MAP_FAILED) {
    close(fd);
    perror("Error mmapping the file");
    exit(EXIT_FAILURE);
  }
  /* Don't forget to free the mmapped memory ... usually at end of main
   */
   
  return (fd);
}

/**************************************************************/
int toggle(int jj) {
  if (jj == 0) jj = 1;
  else jj = 0;
  return (jj);
}

/*********************************************************************************/
struct sec_us time_In_ms (struct sec_us xx) {
  //  struct sec_us  xx;                // structure to hold ms times in sec, ms, us
  
  xx.sec = xx.ms / 1000;                      // integer division to convert ms to sec
  xx.us  = (xx.ms - (xx.sec * 1000))*1000;    // integer arithmatic to get fraction of second in ms
  
  return (xx);
}

/******************************************************************/
void menu(){

  printf ("----------------------------------------------------------------------------\n");
  printf ("      Controls        |         Options         |    Current settings   \n");
  printf ("----------------------------------------------------------------------------\n");
  if (mtcptr->normal)
    printf ("1 - status            |  TRUE - Normal mode     | Grow-in     = %i ms\n",mtcptr->bon.ms);
  else
    printf ("1 - status            | FALSE - Normal mode     | Grow-in     = %i ms\n",mtcptr->bon.ms);
  if (mtcptr->takeaway)
    printf ("2 - run cycling       |  TRUE - Takeaway mode   | Decay-out   = %i ms\n",mtcptr->boff.ms);
  else
    printf ("2 - run cycling       | FALSE - Takeaway mode   | Decay-out   = %i ms\n",mtcptr->boff.ms);
  if (mtcptr->pause)
    printf ("3 - stop cycling      |  TRUE - Pause mode      | MTC move    = %i ms\n",mtcptr->tmove.ms);
  else
    printf ("3 - stop cycling      | FALSE - Pause mode      | MTC move    = %i ms\n",mtcptr->tmove.ms);
  if (mtcptr->beammeas)
    printf ("4 - beam ON           |  TRUE - Beam w/ measure | Lite time   = %i ms\n",mtcptr->lon.ms);
  else
    printf ("4 - beam ON           | FALSE - Beam w/ measure | Lite time   = %i ms\n",mtcptr->lon.ms);
  if (mtcptr->measbeam)
    printf ("5 - beam OFF          |  TRUE - Measure w/ beam | Pause time  = %i ms\n",mtcptr->pon.ms);
  else
    printf ("5 - beam OFF          | FALSE - Measure w/ beam | Pause time  = %i ms\n",mtcptr->pon.ms);
  if (mtcptr->trigbeam)
    printf ("6 - mtc move          |  TRUE - Trigger w/ beam | Trig width  = %i us\n",mtcptr->trigDT);
  else
    printf ("6 - mtc move          | FALSE - Trigger w/ beam | Trig width  = %i us\n",mtcptr->trigDT);
  if (mtcptr->laserbeam)
    printf ("7 - change parameters |  TRUE - Laser w/ beam   | Num moves   = %i \n",mtcptr->numMove);
  else
    printf ("7 - change parameters | FALSE - Laser w/ beam   | Num moves   = %i \n",mtcptr->numMove);
    printf ("8 - reset cycle stats |-------------------------|");
  if (mtcptr->onoff) printf ("%c[1m Cycling ON %c[0m    \n",ESC,ESC);
  else printf ("%c[1m Cycling OFF %c[0m   \n",ESC,ESC);

  printf ("9 - reset mtc faults  |");

  if (mtcptr->tapeFault) printf ("%c[1m  TRUE - TapeFault error%c[0m |",ESC,ESC);
  else printf (" FALSE - TapeFault error |");
  if (!mtcptr->tapeFault && !mtcptr->tapeBreak) printf ("%c[1m MTC OK %c[0m    \n",ESC,ESC);
  else printf ("%c[1m MTC ERROR %c[0m  \n",ESC,ESC);

  printf ("10- laser calib on    |");
  if (mtcptr->tapeBreak) printf ("%c[1m  TRUE - TapeBreak error%c[0m | \n",ESC,ESC);
  else printf (" FALSE - TapeBreak error |   \n");

  printf ("11- all LJ chan OFF   |                         |                  \n");
  printf ("12- Send trig / start |                         |                  \n");
  printf ("99- help              | LabJack SN: %8li   | pid: %6i            \n", mtcptr->lj, mtcptr->pid);
  printf ("----------------------------------------------------------------------------\n");
  printf ("---- Data:Back Ratio -----  Data  ---- Background --------  Total  ---------\n");
  if (mtcptr->bkgRatio == -1)
    printf ("        All data           %6i        %6i             %6i               \n",
	  mtcptr->cyclesData, mtcptr->cyclesBkg, (mtcptr->cyclesData + mtcptr->cyclesBkg));
  else if (mtcptr->bkgRatio == 0)
    printf ("        All Bkg            %6i        %6i             %6i               \n",
	  mtcptr->cyclesData, mtcptr->cyclesBkg, (mtcptr->cyclesData + mtcptr->cyclesBkg));
  else
    printf ("        %2i:1              %6i        %6i             %6i               \n",
	  mtcptr->bkgRatio, mtcptr->cyclesData, mtcptr->cyclesBkg, (mtcptr->cyclesData + mtcptr->cyclesBkg));
  //  printf ("----------------------------------------------------------------------------\n");
  //  printf ("------ Labjack SN --------  pid  ------------------------------------------------\n");
  //  printf ("       %8li           %i                                     \n",mtcptr->lj, mtcptr->pid);
  printf ("----------------------------------------------------------------------------\n");
  printf ("0 - end this program | 100 - End all programs  | Time runing: %li s  \n", mtcptr->time1 - mtcptr->time0);
  printf ("----------------------------------------------------------------------------\n");
  if (mtcptr->com2 != 0)printf ("****** TAPE CONTROLLER SIGNALS NOT BEING READ PROPERLY! ******\n");

  return;
}
 
/*************************************************************************************************/
void help(){
  printf("*************************************************************************************\n");
  printf("********************************* Kicker Help ***************************************\n");
  printf("*************************************************************************************\n");
  printf("Commands: \n");
  printf(" 1 - status - display the menu and hence current status \n");
  printf(" 2 - run cycling - take data according to the selected cycle\n");
  printf("   - possible cycles are: \n");
  printf("      - takeaway: sample is made in front of the detectors\n");
  printf("      - normal: sample is moved to the detectors\n");
  printf("      - pause: sample delivery in normal mode is delayed to allow shorter half-lves to decay\n");
  printf(" 3 - stop cycling - end the run but can be restarted \n");
  printf(" 4 - beam ON - turn off all signals on LabJack so that beam is delivered to tape \n");
  printf(" 5 - beam OFF - turn on the kicker so that beam is deflected from the tape \n");
  printf(" 6 - mtc move - issue a move command to the MTC controller \n");
  printf(" 7 - change parameters - alter the cycle configuration \n");
  printf("      - initially set beam on (grow-in) and beam off (decay-out) cycles \n");
  printf("      - can return to cycling at this point sinc ethis is most common change\n");
  printf("      - other parameters can be changed; note that if the settings are OK and you skip to the end\n");
  printf("      - options are not shown once they have been modified\n");
  printf(" 8 - reset cycle stats - reset the cycle statistics for data, background, and total\n");
  printf(" 9 - reset mtc faults - reset the values that are read after each mtc move \n");
  printf("10 - laser calib on - controls the laser gain correction part of the cycle (untested)  \n");
  printf("11 - all LJ chan OFF - essentially ensures all digital output signals set to 0 \n");
  printf("99 - help - print this menu  \n");
  printf(" 0 - end this program - can be done at any time and restarted to control cycles.  Can have multiple copies running\n");
  printf("100- End all programs - end this and all kicker-related programs \n");
  printf(" \n");
  printf("Options: \n");
  printf("  Boolean value have 0=FALSE, 1=TRUE\n");
  printf("  Modes = TRUE ; Normal, Takeaway, or Pause (in pause mode normal is also TRUE)\n");
  printf("  Beam w/ measure = TRUE; Take data while making the sample (always TRUE in takeaway mode)\n");
  printf("  Measure w/ beam = TRUE: Option in normal and Pause modes to make the next sample while measuring previous sample\n");
  printf("  Trigger w/ beam = TRUE: Have the beam on when trigger signal indicates start of the cycle\n");
  printf("  Laser w/ beam = TRUE: Have the beam on when taking laser calibration data\n");
  printf("  Data/Back Ratio: ratio of data cycles to background cycle (beam is kicked away for entire cycle)\n");
  printf("     -1 = all data\n");
  printf("      0 = all background\n");
  printf("  Num = number of data cycle for each background cycle \n");
  printf(" \n");
  printf("Settings/Statistics: \n");
  printf("  Tape fault error from status of MTC controller\n");
  printf("  Tape break error from status of MTC controller\n");
  printf("  Grow-in time in milliseconds: beam ON time\n");
  printf("  Decay-out time in milliseconds: beam OFF time\n");
  printf("  MTC move  time in milliseconds: time for tape to complete its movement\n");
  printf("  Lite time: time in milliseconds: time for laser to be on (untested) \n");
  printf("  Pause time: time in milliseconds: time for waiting for contaminents to decay\n");
  printf("  Trig width: width of trigger signal that indicates start of a cycle - steps of 16 us will be issued \n");
  printf("  Num moves: the number of moves after normal and pause modes - this allows an old sample to be moved further away\n");
  printf("  Cycling ON/OFF - indicates the status of the cycling\n");
  printf("  MTC OK/ERROR - indicates the status of the MTC controller\n");
  printf("  LabJack SN: serial number of the Labjack - nust be given in configuration file in sub-directory include\n");
  printf("  pid: process ID of the memory map so that you can be sure you got the right one\n");
  printf("  Time running: the time in seconds since the start of the rpogram: If it changes the main program is updating the memory mapped file \n");
  printf("\n");
  WaveformDiagram();
  printf("\n");
  printf("*************************************************************************************\n");
  return;
}

/******************************************************************/
void WaveformDiagram(){

  printf("                    Signal waveform                           \n");
  printf("  |<-------- 1 complete cycle or waveform ------------------>|\n");
  printf("   ______                           __________________________________   \n");
  printf("__|      |_________________________|                       |   |      |_____\\ \n");
  printf("   MTC     Grow-in (beam on)      /\\   Decay-out (beam off) /\\   MTC      \n");
  printf("   move                           ||                        ||   move     \n");
  printf("                                Pause                    Laser gain       \n");
  printf("                                Option                   watch option     \n");
  
  return;
}

/******************************************************************/
/* ********************************************************************* *
 * testtime.c
 *
 * Written by:        M.J. Brinkman
 * At the request of: C.J. Gross
 *
 *     (c) 1997  All rights reserved.
 *
 *  A simple test to check to see if we get a nicely formatted time
 *  and date string.
 * ********************************************************************* *
 *  Global includes that need to be there.
 * ********************************************************************* */

/* ********************************************************************* *
 * char *GetDate()
 *
 *  A function that returns the pretty-formatted date/time string.
 * ********************************************************************* */

char *GetDate() {
	/*
	 *  Variable type definitions.  Both of these types found in
	 *  <time.h>.
	 */
    struct tm *timer;
    time_t    myClock;
	
	/*
	 *  Get the current clock time.
	 *  Form:  time(time_t *)
	 *  Stores the current clock time in the time_t pointer.
	 */
    time((time_t *) &myClock);
	
	/*
	 *  Set us in the current time zone,
	 *  Form:  tzset()
	 */
    tzset();
	
	/*
	 *  Convert the current clock to the current time in our
	 *  local time zone.
	 *  Form:  struct tm *localtime(time_t *)
	 */
    timer = (struct tm *) localtime((time_t *) &myClock);
	
	/*
	 *  Now do all the nice formatting.
	 *  Form: char *asctime((struct tm *))
	 */
    return (char *) asctime(timer);
}
/******************************************************************/
int scan2int () {
  char sss[200];
  long int ii=0;
  
  scanf("%s",sss);
  ii=strtol(sss,NULL,10);                            // convert to base 10
  if ((ii == 0) && (strcmp(sss,"0") != 0)) ii = -1;  // check if 0 result is 0 or because input is not number
  return ((int) ii);
}

/******************************************************************/

