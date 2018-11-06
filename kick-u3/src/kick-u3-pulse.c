
#include "../../include/u3-err-msg.h"
#include "../../include/u3.h"
#include "../../include/labjackusb.h"
#include "../../include/kick-u3.h"

#define FILEPATH "../data/mtc.bin"
//#define kick_conf "include/kick-u3.conf"

int mmapSetup();          // setup memory map files

void cmdLJ(uint8 *c, uint8 jj, uint8 kk);                    // send the bit pattern for all 20 Digital IO lines
void cmdpauseLJ(uint8 *c, uint8 jj, uint8 kk, uint8 mm);     // wait for mm *16 us before setting the bit pattern for all 20 Digital IO lines
void cmdwaitLJ(uint8 *c, uint8 jj, uint8 kk, uint8 mm);      // wait for mm*16 ms before setting the bit pattern for all 20 Digital IO lines

int readConf();
uint8 findLJchan0(char *aaa);     // used by readConf
uint8 findLJchan1(char *aaa);     // used by readConf
void loadArrays();                // creates the entries in the uint8 arrays below
int modeData();                   // data cycle (all mode options included)
int modeBackground();             // background cycle (all mode options included)
void clearParameters();           // clear parameters
void clearStats();                // clear satistics
//bool readMTC();                 // read the dgital IO to see if tape is OK
bool readMTC(long int kk);        // read the dgital IO to see if tape is OK
long int findLJchan2(char *aaa);  // used by readConf to identify digital input from the MTC controller tape faults
void pulseLaser(HANDLE hu3, long int tcStart, double numPulses);  // used to issue trigger pulses to the laser - fixed at 1 pulse per ms  

/* Arrays that control the cycle   */
uint8 arrMTC[14];         // array of commands sent to labjack for MTC movement (mtc + kick)
uint8 arrBeamOn[14];      // array of commands sent to labjack for beam ON (beam)
uint8 arrBeamOnMeas[14];  // array of commands sent to labjack for beam ON and measuring signal (beam + measure)
uint8 arrBeamOff[14];     // array of commands sent to labjack for beam OFF (kick)
uint8 arrBeamOffMeas[14]; // array of commands sent to labjack for beam OFF and measuring signal (kick + measure)
uint8 arrLite[14];        // array of commands sent to labjack for laser lite (laser + kick)
uint8 arrLiteBeam[14];    // array of commands sent to labjack for laser lite (laser + beam)
uint8 arrTrig[16];        // array of commands sent to labjack for trgger signal (trigger + kick)
uint8 arrTrigBeam[16];    // array of commands sent to labjack for trgger signal (trigger + beam)
uint8 arrAllOff[14];      // array of commands sent to labjack for trgger signal (0)

//??  uint8 arrLiteTrigOff[14];
//??  uint8 arrMTCTrigOff[14];

uint8 arrMTC_BKG[14];                  // array of commands sent to labjack for MTC movement background (mtc + kick + bkg)
uint8 arrBeamOn_BKG[14];               // array of commands sent to labjack for beam ON background (beam + bkg)
uint8 arrBeamOnMeas_BKG[14];           // array of commands sent to labjack for beam on and measuring signal (beam + measure + bkg)
uint8 arrBeamOff_BKG[14];              // array of commands sent to labjack for beam OFF background (kick + bkg)
uint8 arrBeamOffMeas_BKG[14];          // array of commands sent to labjack for beam OFF background (kick + measure + bkg)
uint8 arrLite_BKG[14];                 // array of commands sent to labjack for laser lite background (laser + kick + bkg)
uint8 arrLiteBeam_BKG[14];             // array of commands sent to labjack for laser lite background (laser + beam + bkg)
uint8 arrTrig_BKG[16];                 // array of commands sent to labjack for trgger signal background (trigger + kick + bkg)
uint8 arrTrigBeam_BKG[16];             // array of commands sent to labjack for trgger signal background (trigger + beam + bkg)
uint8 arrAllOff_BKG[14];               // array of commands sent to labjack for trgger signal (bkg)

long int mtcBreak=0, mtcFault=0;       // digital I/O channels for reading MTC faults
long int tcStart=0;                    // digital timer (start) channels for sending fixed number of triggers to laser

/* Gather all signal handler and timer routines */
void handlerCommand(int sig, siginfo_t *siginfo, void *context);  // main handler for signal notification and decisions
void signalBlock(int pp);
sigset_t mask;
sigset_t orig_mask;
struct sigaction act;
struct itimerval tt;

struct sec_us time_In_ms (struct sec_us xx);   // my timer routines and structure for times
void beginTimer(struct sec_us x);
void stopTimer();

/* Gather labjack routines */
struct labjack {
  HANDLE hand;
  long int lj;
  int ljnum;                    // number of labjack to relate this specific to a particular labjack such as calibrations
  u3CalibrationInfo caliInfo;
} labj[5];
int labjackSetup(long int lj, int num, int ljnum);           // sets up the LabJack U3
void resetU3(uint8 res, HANDLE ljh);                         // reset labjack U3
void writeLJ(HANDLE hu3, uint8 *arr, long int count);

/* Other global variables   */
int endwait=1;          // used in timer loop (begin and end that's why its global) so we can do nanosleep

/*********************************************************************************/
/*********************************************************************************/
int main(int argc, char **argv){
  int mapKick=0;
  pid_t pid=0;
  time_t curtime;
  int kk=0;
  int ljmax=5;
  int cycD=-1;
  struct sec_us sec1;   // one second timer that sleeps every cycle when tape cycling not on
  
/*
  Set up LabJack U3  ***PUT NOTHING BEFORE THIS AND SHARED MEMORY

  Shared memory creation and attachment
  the segment number is stored in lnptr->pid
*/
  mapKick = mmapSetup();
  if (mapKick == -1) {
    printf(" Error on setting up memory map ... exiting \n");
    return 0;
  }
  //  shmSetup();
/*  
   Read setup file on disk and load into shared memory
*/
  clearParameters(); //This sets default width used by readConf();
  clearStats();
  printf("Read conf file...\n");
  ljmax = readConf();                          // this routine calls labjackSetup since we allow more than 1 labjack

  pid = getpid();         // this gets process number of this program
  mtcptr->pid= pid;       // this is the pid number for the SIGALRM signals
/*  
   Set up the signal capture routine for when the reading
   program wants something changed
*/
  memset (&act,'\0', sizeof(act));
  act.sa_sigaction = &handlerCommand;
  act.sa_flags = SA_SIGINFO;
  if (sigaction(SIGALRM,&act,NULL) < 0){
    perror("sigaction");
    return 1;
  }

/*  
  Setup times based on starting of the program
*/
  curtime = time(NULL);
  mtcptr->time0 =  curtime;     // program start time ...store in memory
  mtcptr->time1 =  curtime;     // program start time ...store in memory

  /*  
  Setup arrays and parameters based on starting of the program
*/

  mtcptr->com0=0;
  mtcptr->com1=0;
  mtcptr->com2=0;
  
  mtcptr->normal = 1;      // one of these always has to be selected
  mtcptr->pause = 0;
  mtcptr->takeaway = 0;

  mtcptr->background = 0;  // background mode

  mtcptr->beammeas = 0;    // other options: first is the action, 2nd is the option
  mtcptr->measbeam = 0;    // so the beam action has measurement as an option and measurement 
  mtcptr->laserbeam = 0;   // has a beam option as does the laser and the trigger.
  mtcptr->trigbeam = 1;    // tape movements have options based on modes above

  
  loadArrays();
/*  
  Put control options and switches here
*/
  arrMTC[11]=0xf0;   // test to view changes
  sec1.ms=1000;
  sec1 = time_In_ms(sec1);
  printf ("entering loops...\n");
  while (mtcptr->com0 != 100) {

    switch (mtcptr->com0){
    case 0:
      cycD = mtcptr->bkgRatio;                            // set ratio after a stop of cycles
      while (mtcptr->onoff){
	mtcptr->time1 = time(NULL);
	if (mtcptr->bkgRatio < 0) {                                          // all data
	  if (mtcptr->normal || mtcptr->takeaway || mtcptr->pause) kk = modeData();
	  if (kk == 1) {
	    mtcptr->cyclesData++;
	    printf(" completed %i data cycle\n", mtcptr->cyclesData);
	  }
	  else 	  printf(" interupted cycle! (all data option)\n");
	}
	else if (mtcptr->bkgRatio > 0 && cycD > 0) {                          // data + bkg
	  if (mtcptr->normal || mtcptr->takeaway || mtcptr->pause) kk = modeData();
	  if (kk == 1) {
	    mtcptr->cyclesData++;
	    cycD--;                                    // decrement a data cycle, do bkg when cycD=0
	    printf(" completed %i data cycle - ratio countdown = %i\n", mtcptr->cyclesData,cycD);
	  }
	  else 	  printf(" interupted cycle! (data+bkg option)\n");
	}
	else {                                                                 // bkg cycle
	  mtcptr->background = true;
	  if (mtcptr->normal || mtcptr->takeaway || mtcptr->pause) kk = modeBackground();
	  if (kk == 1) {
	    mtcptr->cyclesBkg++;
	    cycD = mtcptr->bkgRatio;                   // reset cycD to count the data cycles
	    printf(" completed %i background cycle\n",mtcptr->cyclesBkg);
	    mtcptr->background = false;
	  }
	  else 	  printf(" interupted cycle! (bkg option) \n");
	}
      }
      writeLJ(mtcptr->ljh, arrBeamOff, 14);     // cycle is stopped or interrupted: stop beam
      mtcptr->com0 = -1;                        // reset the switch variable to default
      break;
    case 2:
      mtcptr->onoff = 0;
      mtcptr->gtkstat = 10;                     // report status for gtk monitor program
      writeLJ(mtcptr->ljh, arrBeamOff, 14);     // kick beam
      mtcptr->com0 = -1;                        // reset the switch variable to default
      break;
    case 3:
      mtcptr->onoff = 0;                        // beam on
      mtcptr->gtkstat = 11;                     // report status for gtk monitor program
      writeLJ(mtcptr->ljh, arrAllOff, 14);      // all LJ channels off
      mtcptr->com0 = -1;                        // reset the switch variable to default
      break;
    case 4:
      mtcptr->onoff = 0;
      mtcptr->gtkstat = 13;                     // report status for gtk monitor program
      writeLJ(mtcptr->ljh, arrLite, 14);        // laser with beam off
      mtcptr->com0 = -1;                        // reset the switch variable to default
      break;
    case 5:
      mtcptr->onoff = 0;                        // beam on
      mtcptr->gtkstat = 11;                     // report status for gtk monitor program
      writeLJ(mtcptr->ljh, arrAllOff, 14);      // all LJ channels off
      mtcptr->com0 = -1;                        // reset the switch variable to default
      break;
    case 6:
      mtcptr->onoff = 0;
      mtcptr->gtkstat = 12;                     // report status for gtk monitor program
      writeLJ(mtcptr->ljh, arrMTC, 14);         // move tape
      mtcptr->com0 = -1;                        // reset the switch variable to default
      break;
    case 7:
      printf ("Changing arrays for labjack control \n");
      loadArrays();
      //      nanosleep (100000000);     // 100 ms sleep to allow monitor to update values
      mtcptr->com0 = -1;                        // reset the switch variable to default
      break;
    case 100:
      mtcptr->onoff = 0;
      mtcptr->com0 = 100;
    default:                                    // do nothing
      break;
    }

    //  beginTimer(mtcptr->tmove);
    //      
    //       beginTimer(mtcptr->lon);
    
    //  

    if (mtcptr->onoff==0) beginTimer(sec1);   // start the timer(nanosleep) if you aren't cycling so you don't burn cycles
  }
  
/*
  Ending the program so release the memory map, close file, and close the labjack
*/

  if (munmap(mtcptr, sizeof (struct mtc*)) == -1) {
    perror("Error un-mapping the kick-u3 file");
  }
  close(mapKick);
  
  printf(" File closed and file unmapped \n");

/*
 Close USB connection to all labjacks and end program
*/
//  fclose(fileTherm);
  printf ("Total labjack number %i to close\n",ljmax);

  for (kk=0; kk<ljmax; kk++){
    closeUSBConnection(labj[kk].hand);
    printf("USB to LabJack %li closed\n",labj[kk].lj);
  }

  return 0;
}
/*********************************************************************************/
/*********************************************************************************/
int modeData(){
  
  //printf("Trigger \n");                           // trigger on
  if (mtcptr->trigbeam){
    writeLJ(mtcptr->ljh, arrTrigBeam, 16);
  }
  else {
    writeLJ(mtcptr->ljh, arrTrig, 16);
  }

  if (mtcptr->onoff == 0) return (0);         // check if new commands have come in
  mtcptr->gtkstat = 1;                        // report status for gtk monitor program
  //  printf("Beam on \n"); 

  if (mtcptr->beammeas){                          // beam on
    writeLJ(mtcptr->ljh, arrBeamOnMeas, 14);   //beam on pulse
    printf ("beam on no measure %i, %i \n",arrBeamOnMeas[12], arrBeamOnMeas[13]); 
    beginTimer(mtcptr->tdt);                //wait for width
    writeLJ(mtcptr->ljh, arrBeamOn, 14);  //check to see beam trigger signal persists (needs to)
    beginTimer(mtcptr->bon);                   //wait for corr. time
    writeLJ(mtcptr->ljh, arrBeamOnMeas, 14);
    beginTimer(mtcptr->tdt);                //wait for width
    writeLJ(mtcptr->ljh, arrBeamOff, 14); //end it
  }
  else {
    writeLJ(mtcptr->ljh, arrBeamOn, 14);
    printf ("beam on no measure %i, %i \n",arrBeamOn[12], arrBeamOn[13]); 
    beginTimer(mtcptr->tdt);               //wait for width
    //writeLJ(mtcptr->ljh, arrBeamOff, 14); don't turn off beam...
    beginTimer(mtcptr->bon);                  //wait for corr. time
    //writeLJ(mtcptr->ljh, arrBeamOn, 14);
    beginTimer(mtcptr->tdt);               //wait for width
    writeLJ(mtcptr->ljh, arrBeamOff, 14); //end it
  }

  if (mtcptr->onoff == 0) return (0);         // check if new commands have come in

/* --------------- Mode option begin ------------------------------ */
  if(mtcptr->pause){                        // Pause option: stop beam, wait w/out meas, mtc on
    //printf("Pause on\n");     
    // not necessary?? writeLJ(mtcptr->ljh, arrBeamOff, 14);
    mtcptr->gtkstat = 2;                        // report status for gtk monitor program
    beginTimer(mtcptr->pon);
    writeLJ(mtcptr->ljh, arrMTC, 14);
    beginTimer(mtcptr->tmove);
    mtcptr->tapeFault = readMTC(mtcBreak);
    mtcptr->tapeFault = readMTC(mtcFault);
  }

  if (mtcptr->onoff == 0) return (0);         // check if new commands have come in

/* --------------- Mode option end ------------------------------ */
/*  printf("Laser\n");
  if (mtcptr->lon.ms > 0) {
    if (mtcptr->laserbeam) {                        // laser with beam on or off BEFORE MEASURING
      writeLJ(mtcptr->ljh, arrLiteBeam, 14);
  mtcptr->gtkstat = 3;                        // report status for gtk monitor program
      beginTimer(mtcptr->lon);
    }
    else {
      writeLJ(mtcptr->ljh, arrLite, 14);
      beginTimer(mtcptr->lon);
    }
  }
  if (mtcptr->onoff == 0) return (0);         // check if new commands have come in
*/

/* --------------- Mode option begin ------------------------------ */
  if (mtcptr->normal){                     // mtc on for normal mode; skip it for takeaway
    //    printf("MTC on for normal \n");
    writeLJ(mtcptr->ljh, arrMTC, 14);      //begin move
    mtcptr->gtkstat = 4;                        // report status for gtk monitor program
    beginTimer(mtcptr->tdt);
    //writeLJ(mtcptr->ljh, arrBeamOff, 14);   //needs separate signal for acq MTC on/off
    beginTimer(mtcptr->tmove);
    //writeLJ(mtcptr->ljh, arrMTC, 14);
    beginTimer(mtcptr->tdt);            //end move
    mtcptr->tapeFault = readMTC(mtcBreak);
    mtcptr->tapeFault = readMTC(mtcFault);
    printf ("normal tape move %i, %i \n",arrMTC[12], arrMTC[13]);
    //writeLJ(mtcptr->ljh, arrBeamOn, 14);   //beam on for width? no.
    //printf ("beam on no measure %i, %i \n",arrBeamOn[12], arrBeamOn[13]); 
    //beginTimer(mtcptr->tdt);
    writeLJ(mtcptr->ljh, arrBeamOff, 14); //end it 
  }
/* --------------- Mode option end ------------------------------ */
  
  if (mtcptr->onoff == 0) return (0);         // check if new commands have come in
//  printf("Measure  \n"); 
  mtcptr->gtkstat = 5;                        // report status for gtk monitor program

  if (mtcptr->measbeam){                         // measure with beam on or off
    /*writeLJ(mtcptr->ljh, arrBeamOnMeas, 14);
    beginTimer(mtcptr->boff);*/
    printf ("beam on while measuring %i, %i \n",arrBeamOffMeas[12], arrBeamOffMeas[13]); 
    writeLJ(mtcptr->ljh, arrBeamOnMeas, 14);   //beam on pulse
    beginTimer(mtcptr->tdt);                //wait for width
    writeLJ(mtcptr->ljh, arrBeamOn, 14);  //check to see beam trigger signal persists (needs to)
    beginTimer(mtcptr->boff);                   //wait for corr. time
    writeLJ(mtcptr->ljh, arrBeamOnMeas, 14);
    beginTimer(mtcptr->tdt);                //wait for width
    writeLJ(mtcptr->ljh, arrBeamOff, 14); //end it
  }
  else {
    printf ("beam off while measuring %i, %i \n",arrBeamOffMeas[12], arrBeamOffMeas[13]);
    writeLJ(mtcptr->ljh, arrBeamOffMeas, 14);
    beginTimer(mtcptr->tdt);
    writeLJ(mtcptr->ljh, arrBeamOff, 14);
    beginTimer(mtcptr->boff);
    //printf ("wait %i, %i \n",arrBeamOff[12], arrBeamOff[13]);
    writeLJ(mtcptr->ljh, arrBeamOffMeas, 14);
    beginTimer(mtcptr->tdt);
    //printf ("beam off while measuring %i, %i \n",arrBeamOffMeas[12], arrBeamOffMeas[13]); 
    writeLJ(mtcptr->ljh, arrBeamOff, 14);
  }
  if (mtcptr->onoff == 0) return (0);         // check if new commands have come in

/* --------------- Mode option end ------------------------------ */
//  printf("Laser\n");
  if (mtcptr->lon.ms > 0) {
    mtcptr->gtkstat = 6;                        // report status for gtk monitor program

    if (mtcptr->laserbeam) {                        // laser with beam on or off AFTER MEASURING
      writeLJ(mtcptr->ljh, arrLiteBeam, 14);
      usleep(1000);                               // add slight delay between labjack commands maybe for others
      pulseLaser(mtcptr->ljh, tcStart, (double) mtcptr->lon.ms);        // issue a pulse at set 1 kHz frequency to trigger the laser
      beginTimer(mtcptr->lon);
    }
    else {
      writeLJ(mtcptr->ljh, arrLite, 14);
      usleep(1000);                               // add slight delay between labjack commands
      pulseLaser(mtcptr->ljh, tcStart, (double) mtcptr->lon.ms);        // issue a pulse at set 1 kHz frequency to trigger the laser
      beginTimer(mtcptr->lon);
    } //lon adjusted for 'slight delay?'

   printf ("lasering %i, %i \n",arrLite[12], arrLite[13]); 
  }
  if (mtcptr->onoff == 0) return (0);         // check if new commands have come in

/* --------------- Mode option begin ------------------------------ */
//  printf("MTC on\n");           
  mtcptr->gtkstat = 7;                        // report status for gtk monitor program
  writeLJ(mtcptr->ljh, arrMTC, 14);                 // mtc on = only tape movement in takeaway, common in all modes;
  printf ("first  tape move %i, %i \n",arrMTC[12], arrMTC[13]); 
  //beginTimer(mtcptr->tdt);
  //writeLJ(mtcptr->ljh, arrBeamOff, 14); //end it 
  //beginTimer(mtcptr->tmove);
  //writeLJ(mtcptr->ljh, arrMTC, 14);
  //beginTimer(mtcptr->tdt);
  mtcptr->tapeFault = readMTC(mtcBreak);
  mtcptr->tapeFault = readMTC(mtcFault);
  writeLJ(mtcptr->ljh, arrBeamOff, 14);
  
  /*beginTimer(mtcptr->tmove);
  mtcptr->tapeFault = readMTC(mtcBreak);
  mtcptr->tapeFault = readMTC(mtcFault);*/
  if (mtcptr->numMove == 2){
    mtcptr->gtkstat = 8;                        // report status for gtk monitor program
    //    writeLJ(mtcptr->ljh, arrAllOff, 14);
    writeLJ(mtcptr->ljh, arrBeamOff, 14);
    usleep(10000);
    writeLJ(mtcptr->ljh, arrMTC, 14);
    beginTimer(mtcptr->tdt);
    //writeLJ(mtcptr->ljh, arrBeamOff, 14); 
    beginTimer(mtcptr->tmove);
    //writeLJ(mtcptr->ljh, arrMTC, 14);
    beginTimer(mtcptr->tdt);
    mtcptr->tapeFault = readMTC(mtcBreak);
    mtcptr->tapeFault = readMTC(mtcFault);
    //writeLJ(mtcptr->ljh, arrBeamOff, 14);
  
    /*beginTimer(mtcptr->tmove);
    mtcptr->tapeFault = readMTC(mtcBreak);
    mtcptr->tapeFault = readMTC(mtcFault);*/
   printf ("second tape move %i, %i \n",arrMTC[12], arrMTC[13]); 
  }
  
/* --------------- Mode option end ------------------------------ */  
  return (1);
}

/*********************************************************************************/
bool readMTC(long int kk){
  long int ii=0, jj=0;

  if (mtcptr->tapeFault) return (1);          // do not overwrite a previous error on mtcptr->tapeRead
  if (mtcptr->tapeBreak) return (1);          // do not overwrite a previous error on mtcptr->tapeRead
  jj = eDI(mtcptr->ljh,1,kk,&ii);             // read labjack channel kk
  if (jj != 0) mtcptr->com2 = 1;              // on read error return 1 = TRUE (a tape read fault)
  return(ii);                                 // good read of no signal so return 0 = FALSE (no tape fault)
}
/*********************************************************************************
bool readMTC(){
  long int ii=0, jj=0, kk=0;
  //  printf (" reading dio \n");
  if ( (jj = eDI(mtcptr->ljh,1,5,&ii)) != 0) mtcptr->tapeRead = 1;  // on error return 1 = TRUE (a tape fault)
  //  printf(" ii = %li kk = %li jj=%li \n",ii,kk,jj);
  if ( (jj = eDI(mtcptr->ljh,1,6,&kk)) != 0) mtcptr->tapeRead = 1;  // on error return 1 = TRUE (a tape fault)
  printf(" ii = %li kk = %li jj=%li \n",ii,kk,jj);
  if (ii != 0 || kk != 0) return(1);                     // on read of any TRUE channels return 1 = TRUE (a tape fault)
  return(0);                                             // good reads of no signal so return 0 = FALSE (no tape fault)
}
*/
/*********************************************************************************/
/*
uint8 arrMTC[14];         // array of commands sent to labjack for MTC movement
uint8 arrBeamOn[14];      // array of commands sent to labjack for beam ON
uint8 arrBeamOnMeas[14];  // array of commands sent to labjack for beam on and measuring signal
uint8 arrBeamOff[14];     // array of commands sent to labjack for beam OFF
uint8 arrBeamOffMeas[14]; // array of commands sent to labjack for beam OFF
uint8 arrLite[14];        // array of commands sent to labjack for laser lite
uint8 arrLiteBeam[14];    // array of commands sent to labjack for laser lite
uint8 arrTrig[16];        // array of commands sent to labjack for trgger signal
uint8 arrTrigBeam[16];    // array of commands sent to labjack for trgger signal
*/
/*********************************************************************************/
int modeBackground(){

/* --------------- Mode option begin ------------------------------ */
  //  printf("Trigger \n"); 
  if (mtcptr->trigbeam){                          // trigger on
    writeLJ(mtcptr->ljh, arrTrig_BKG, 16);
  }
  else {
    writeLJ(mtcptr->ljh, arrTrigBeam_BKG, 16);
  }
  if (mtcptr->onoff == 0) return (0);         // check if new commands have come in
  mtcptr->gtkstat = 1;                        // report status for gtk monitor program

/* --------------- Mode option begin ------------------------------ */
  if (mtcptr->beammeas){                           // beam on ... measure or not
    writeLJ(mtcptr->ljh, arrBeamOnMeas_BKG, 14);
    beginTimer(mtcptr->bon);
  }
  else {
    writeLJ(mtcptr->ljh, arrBeamOn_BKG, 14);
    beginTimer(mtcptr->bon);
  }
  if (mtcptr->onoff == 0) return (0);         // check if new commands have come in

/* --------------- Mode option begin ------------------------------ */
  if(mtcptr->pause){                      // Pause option: stop beam, wait w/out meas, mtc on
    //    printf("Pause on\n");       
    mtcptr->gtkstat = 2;                        // report status for gtk monitor program
    writeLJ(mtcptr->ljh, arrBeamOff_BKG, 14);   
    beginTimer(mtcptr->pon);
    writeLJ(mtcptr->ljh, arrMTC_BKG, 14);
    beginTimer(mtcptr->tmove);
    mtcptr->tapeFault = readMTC(mtcBreak);
    mtcptr->tapeFault = readMTC(mtcFault);
  }
  if (mtcptr->onoff == 0) return (0);         // check if new commands have come in

/* --------------- Mode option begin ------------------------------ */
/* Option to uncomment and fire laser after pause...
//  printf("Laser\n");
  if (mtcptr->lon.ms > 0) {
    mtcptr->gtkstat = 3;                        // report status for gtk monitor program
    if (mtcptr->laserbeam) {                        // laser with beam on or off
      writeLJ(mtcptr->ljh, arrLiteBeam_BKG, 14);
      beginTimer(mtcptr->lon);
    }
    else {
      writeLJ(mtcptr->ljh, arrLite_BKG, 14);
      beginTimer(mtcptr->lon);
    }
  }
  if (mtcptr->onoff == 0) return (0);         // check if new commands have come in
*/

/* --------------- Mode option begin ------------------------------ */
  if (mtcptr->normal){                           // mtc on for normal mode; skip it for takeaway
    //    printf("MTC on for normal \n");  
    mtcptr->gtkstat = 4;                        // report status for gtk monitor program
    writeLJ(mtcptr->ljh, arrMTC_BKG, 14);
    beginTimer(mtcptr->tmove);
    mtcptr->tapeFault = readMTC(mtcBreak);
    mtcptr->tapeFault = readMTC(mtcFault);
  }
  if (mtcptr->onoff == 0) return (0);         // check if new commands have come in

/* --------------- Mode option begin ------------------------------ */
//  printf("Measure  \n");  
    mtcptr->gtkstat = 5;                        // report status for gtk monitor program
  if (mtcptr->measbeam){                        // measure with beam on or off
    writeLJ(mtcptr->ljh, arrBeamOnMeas_BKG, 14);
    beginTimer(mtcptr->boff);
  }
  else {
    writeLJ(mtcptr->ljh, arrBeamOffMeas_BKG, 14);
    beginTimer(mtcptr->boff);
  }
  if (mtcptr->onoff == 0) return (0);         // check if new commands have come in

/* --------------- Mode option begin ------------------------------ */
//  printf("Laser\n");
    mtcptr->gtkstat = 6;                        // report status for gtk monitor program
  if (mtcptr->lon.ms > 0) {
    if (mtcptr->laserbeam) {                        // laser with beam on or off
      writeLJ(mtcptr->ljh, arrLiteBeam_BKG, 14);
      usleep(1000);                               // add slight delay between labjack commands
      pulseLaser(mtcptr->ljh, tcStart, (double) mtcptr->lon.ms);        // issue a pulse at set 1 kHz frequency to trigger the laser
      beginTimer(mtcptr->lon);
    }
    else {
      writeLJ(mtcptr->ljh, arrLite_BKG, 14);
      usleep(1000);                               // add slight delay between labjack commands
      pulseLaser(mtcptr->ljh, tcStart, (double) mtcptr->lon.ms);        // issue a pulse at set 1 kHz frequency to trigger the laser
      beginTimer(mtcptr->lon);
    }
  }
  if (mtcptr->onoff == 0) return (0);         // check if new commands have come in//  printf("MTC on\n");

/* --------------- Mode option begin ------------------------------ */
//   printf("MTC\n");   
  mtcptr->gtkstat = 7;                        // report status for gtk monitor program
  writeLJ(mtcptr->ljh, arrMTC_BKG, 14);         // mtc on = only tape movement in takeaway, common in all modes
  beginTimer(mtcptr->tmove);
  mtcptr->tapeFault = readMTC(mtcBreak);
  mtcptr->tapeFault = readMTC(mtcFault);

  if (mtcptr->numMove == 2){
    mtcptr->gtkstat = 8;                        // report status for gtk monitor program
    //    writeLJ(mtcptr->ljh, arrAllOff_BKG, 14);
    writeLJ(mtcptr->ljh, arrBeamOff_BKG, 14);
    usleep(10000);
    writeLJ(mtcptr->ljh, arrMTC_BKG, 14);
    beginTimer(mtcptr->tmove);
    mtcptr->tapeFault = readMTC(mtcBreak);
    mtcptr->tapeFault = readMTC(mtcFault);
  }
/* --------------- Mode option end ------------------------------ */  
  return (1);
}

/*********************************************************************************/
void loadArrays(){
  uint8 ii=0,jj=0,kk=0;

  //  printf("mtc %x   %x \n",mtcptr->mtc[0],mtcptr->mtc[1]);
  ii = mtcptr->mtc[0] + mtcptr->kck[0];     // move mtc with no beam  (beamOFF)
  jj = mtcptr->mtc[1] + mtcptr->kck[1];
  printf("MTC %x  %x \n",ii,jj);
  cmdLJ(arrMTC,ii,jj);
  ii += mtcptr->bkg[0];
  jj += mtcptr->bkg[1];
  printf("MTC_BKG %x  %x \n",ii,jj);  
  cmdLJ(arrMTC_BKG,ii,jj);
  
  ii = mtcptr->beam[0];                     // beam ON
  jj = mtcptr->beam[1];
  printf("beamOn %x  %x \n",ii,jj);
  cmdLJ(arrBeamOn,ii,jj);
  ii += mtcptr->bkg[0] + mtcptr->kck[0];
  jj += mtcptr->bkg[1] + mtcptr->kck[1];
  printf("beamOn_BKG %x  %x \n",ii,jj);
  cmdLJ(arrBeamOn_BKG,ii,jj);

  ii = mtcptr->beam[0] + mtcptr->meas[0];   // measuring with beam   (beam ON)
  jj = mtcptr->beam[1] + mtcptr->meas[1];
  printf("beamOnMeas %x  %x \n",ii,jj);
  cmdLJ(arrBeamOnMeas,ii,jj);
  ii += mtcptr->bkg[0] + mtcptr->kck[0];
  jj += mtcptr->bkg[1] + mtcptr->kck[1];
  printf("beamOnMeas_BKG %x  %x \n",ii,jj);
  cmdLJ(arrBeamOnMeas_BKG,ii,jj);

  ii = mtcptr->meas[0] + mtcptr->kck[0];    // measuring with no beam (beamOFF)
  jj = mtcptr->meas[1] + mtcptr->kck[1];
  printf("beamOffmeas %x  %x \n",ii,jj);
  cmdLJ(arrBeamOffMeas,ii,jj);
  ii += mtcptr->bkg[0];
  jj += mtcptr->bkg[1];
  printf("beamOffmeas_BKG %x  %x \n",ii,jj);
  cmdLJ(arrBeamOffMeas_BKG,ii,jj);
  
  ii = mtcptr->lite[0] + mtcptr->kck[0];    // laser lite with measuring but no beam  (beam OFF)
  jj = mtcptr->lite[1] + mtcptr->kck[1];
  //  ii = mtcptr->lite[0] + mtcptr->kck[0] + mtcptr->meas[0];    // laser lite with measuring but no beam  (beam OFF)
  //  jj = mtcptr->lite[1] + mtcptr->kck[1] + mtcptr->meas[1];
  printf("Lite %x  %x \n",ii,jj);
  cmdLJ(arrLite,ii,jj);
  ii += mtcptr->bkg[0];
  jj += mtcptr->bkg[1];
  printf("Lite_BKG %x  %x \n",ii,jj);  
  cmdLJ(arrLite_BKG,ii,jj);
  
  ii = mtcptr->lite[0] + mtcptr->beam[0];    // laser lite with measuring and with beam  (beam ON)
  jj = mtcptr->lite[1] + mtcptr->beam[1];
  //  ii = mtcptr->lite[0] + mtcptr->beam[0] + mtcptr->meas[0];    // laser lite with measuring and with beam  (beam ON)
  //  jj = mtcptr->lite[1] + mtcptr->beam[1] + mtcptr->meas[1];
  printf("LiteBeam %x  %x \n",ii,jj);
  cmdLJ(arrLiteBeam,ii,jj);
  ii += mtcptr->bkg[0] + mtcptr->kck[0];
  jj += mtcptr->bkg[1] + mtcptr->kck[1];
  printf("LiteBeam_BKG %x  %x \n",ii,jj);  
  cmdLJ(arrLiteBeam_BKG,ii,jj);
  
  ii = mtcptr->kck[0];                      // just beam OFF no measuring (beam OFF)
  jj = mtcptr->kck[1];
  printf("BeamOff %x  %x \n",ii,jj);
  cmdLJ(arrBeamOff,   ii,jj);
  ii += mtcptr->bkg[0];
  jj += mtcptr->bkg[1];
  printf("BeamOff_BKG %x  %x \n",ii,jj);  
  cmdLJ(arrBeamOff_BKG,   ii,jj);

  ii = mtcptr->trig[0] + mtcptr->beam[0];   // trigger cycle (beam ON)
  jj = mtcptr->trig[1] + mtcptr->beam[1];
  kk = (uint8) (mtcptr->trigDT/16); //readConf sets trigDt at least 16 us and less than 4080 ie kk=<255
  printf("TrigBeam %x %x %x\n",ii,jj,kk);
  cmdpauseLJ(arrTrigBeam, ii,jj,kk);
  ii += mtcptr->bkg[0] + mtcptr->kck[0];
  jj += mtcptr->bkg[1] + mtcptr->kck[1];
  printf("TrigBeam_BKG %x %x %x\n",ii,jj,kk);  
  cmdpauseLJ(arrTrigBeam_BKG, ii,jj,kk);

  ii = mtcptr->trig[0] + mtcptr->kck[0];   // trigger cycle (beam OFF)
  jj = mtcptr->trig[1] + mtcptr->kck[1];
  printf("Trig %x %x %x\n",ii,jj,kk);
  cmdpauseLJ(arrTrig, ii,jj,kk);
  ii += mtcptr->bkg[0];
  jj += mtcptr->bkg[1];
  printf("Trig_BKG %x %x %x\n",ii,jj,kk);
  cmdpauseLJ(arrTrig_BKG, ii,jj,kk);

  ii=0;
  jj=0;
  printf("AllOff %x  %x\n",ii,jj);  
  cmdLJ(arrAllOff, ii,jj);                // turn all labjack channels off
  ii += mtcptr->bkg[0];
  jj += mtcptr->bkg[1];
  printf("AllOff_BKG %x  %x\n",ii,jj);    
  cmdLJ(arrAllOff_BKG, ii,jj);            // turn all labjack channels off except background
    
  return;
}

/*********************************************************************************/
void cmdLJ(uint8 *c, uint8 jj, uint8 kk){
  int ii=0;

  c[ii++]=(uint8)(0x00);   // 0 - checksum will populate
  c[ii++]=(uint8)(0xf8);   // 1 - fixed command for feedback
  c[ii++]=(uint8)(0x04);   // 2 - number of data words (number of i*2 words in the array - 3)=> 24/2-3=9
  c[ii++]=(uint8)(0x00);   // 3 - extended command number ... for feedback =0
  c[ii++]=(uint8)(0x00);   // 4 - checksum will populate
  c[ii++]=(uint8)(0x00);   // 5 - checksum will populate
  c[ii++]=(uint8)(0x00);   // 6 - echo 0

  c[ii++]=(uint8)(0x1B);   // 7 - IOType #1 => 27 PortWriteState
  c[ii++]=(uint8)(0xf0);   // 8 - 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  c[ii++]=(uint8)(0xff);   // 9 - start of "high current" outs EIO/CIO 
  c[ii++]=(uint8)(0x0f);   // 10 - upper 4 bits undefined..only 20 channels
  c[ii++]=(uint8)(0x00);   // 11 - 3 bytes of state, first 4 bits are analog
  c[ii++]=jj;              // 12 - set the bit pattern in jj
  c[ii++]=kk;              // 13 - set the bit pattern in kk ; upper 4 bits undefined
  
  extendedChecksum(c,14);  // checksum load array elements 0, 4, and 5

  return;
}

/*********************************************************************************/
void cmdpauseLJ(uint8 *c, uint8 jj, uint8 kk, uint8 mm){
  int ii=0;

  c[ii++]=(uint8)(0x00);   // 0 - checksum will populate
  c[ii++]=(uint8)(0xf8);   // 1 - fixed command for feedback
  c[ii++]=(uint8)(0x05);   // 2 - number of data words (number of i*2 words in the array - 3)=> 24/2-3=9
  c[ii++]=(uint8)(0x00);   // 3 - extended command number ... for feedback =0
  c[ii++]=(uint8)(0x00);   // 4 - checksum will populate
  c[ii++]=(uint8)(0x00);   // 5 - checksum will populate
  c[ii++]=(uint8)(0x00);   // 6 - echo 0

  c[ii++]=(uint8)(0x1B);   // 7 - IOType #1 => 27 PortWriteState
  c[ii++]=(uint8)(0xf0);   // 8 - 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  c[ii++]=(uint8)(0xff);   // 9 - start of "high current" outs EIO/CIO 
  c[ii++]=(uint8)(0x0f);   // 10 - upper 4 bits undefined..only 20 channels
  c[ii++]=(uint8)(0x00);   // 11 - 3 bytes of state, first 4 bits are analog
  c[ii++]=jj;              // 12 - set the bit pattern in jj
  c[ii++]=kk;              // 13 - set the bit pattern in kk ; upper 4 bits undefined
  
  c[ii++]=(uint8)(0x05);   // 14 - IOType #4 => 5 WaitShort ... time for beam to go before tape move
  c[ii++]=mm;              // 15 - number * 16 us

  extendedChecksum(c,16);  // checksum load array elements 0, 4, and 5

  return;
}
/*********************************************************************************/
void cmdwaitLJ(uint8 *c, uint8 jj, uint8 kk, uint8 mm){
  int ii=0;

  c[ii++]=(uint8)(0x00);   // 0 - checksum will populate
  c[ii++]=(uint8)(0xf8);   // 1 - fixed command for feedback
  c[ii++]=(uint8)(0x05);   // 2 - number of data words (number of i*2 words in the array - 3)=> 24/2-3=9
  c[ii++]=(uint8)(0x00);   // 3 - extended command number ... for feedback =0
  c[ii++]=(uint8)(0x00);   // 4 - checksum will populate
  c[ii++]=(uint8)(0x00);   // 5 - checksum will populate
  c[ii++]=(uint8)(0x00);   // 6 - echo 0

  c[ii++]=(uint8)(0x1B);   // 7 - IOType #1 => 27 PortWriteState
  c[ii++]=(uint8)(0xf0);   // 8 - 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  c[ii++]=(uint8)(0xff);   // 9 - start of "high current" outs EIO/CIO 
  c[ii++]=(uint8)(0x0f);   // 10 - upper 4 bits undefined..only 20 channels
  c[ii++]=(uint8)(0x00);   // 11 - 3 bytes of state, first 4 bits are analog
  c[ii++]=jj;              // 12 - set the bit pattern in jj
  c[ii++]=kk;              // 13 - set the bit pattern in kk ; upper 4 bits undefined
  
  c[ii++]=(uint8)(0x06);   // 14 - IOType #4 => 6 WaitLong ... time for beam to go before tape move
  c[ii++]=mm;              // 15 - number * 16 ms

  extendedChecksum(c,16);  // checksum load array elements 0, 4, and 5

  return;
}

/*********************************************************************************/
void pulseLaser(HANDLE hU3, long int tcStart, double numPulses) {
/*
     used to issue trigger pulses to the laser - fixed at 1 pulse per ms
*/
  long int error = 0;
  long int DIV = 96;
  double time0Value = 250;
  long int alngEnableTimers[2]   = {1, 1};  //Enable Timer0-Timer1
  long int alngEnableCounters[2] = {0, 0};  //Enable Counter0-Counter1
  long int alngTimerModes[2]     = {LJ_tmFREQOUT, LJ_tmTIMERSTOP};  //Set timer modes    
  double adblTimerValues[2]      = {time0Value, numPulses};  //Set frequency to 48MHz/96/(250*2) = 1 kHz, count number of pulses

  
  if((error = eTCConfig(hU3, alngEnableTimers, alngEnableCounters, tcStart, LJ_tc48MHZ_DIV, DIV, alngTimerModes, adblTimerValues, 0, 0)) != 0){
    printf("%s\n", errormsg[error]);         // 48 MHz / 96 * value (250) = 0.5 MHz * 250 = 
    closeUSBConnection(hU3);
    return;
  }


  return;
}

/*********************************************************************************/
struct sec_us time_In_ms (struct sec_us xx) {
  //  struct sec_us  xx;                // structure to hold ms times in sec, ms, us
  
  xx.sec = xx.ms / 1000;                      // integer division to convert ms to sec
  xx.us  = (xx.ms - (xx.sec * 1000))*1000;    // integer arithmatic to get fraction of second in ms
  
  return (xx);
}
		   
/*********************************************************************************/
void beginTimer(struct sec_us xx){
  struct timespec nt;
  
  tt.it_interval.tv_sec = 0;
  tt.it_interval.tv_usec = 0;
  tt.it_value.tv_sec = xx.sec;          // set timer for ii seconds and jj microseconds 
  tt.it_value.tv_usec = xx.us;

  setitimer(ITIMER_REAL, &tt, 0);       // start timer
  signal (SIGALRM, stopTimer);          // function that handles the alarm
/* 
   Now wait for it to time out or get another signal
   Set a never ending loop that contains nanosleep to sleep in 1 ms intervals
   When the timer ends, stopTimer changes endwait to 0 which ends the loop
*/
  endwait=1;
  nt.tv_sec=0;
  nt.tv_nsec=1000000;    // sleep and Kwake up and check if any thing has changed every 1 ms
  while(endwait){
    nanosleep(&nt,0);
    if (mtcptr->onoff==0) {     // stop timer if set to zero
      tt.it_value.tv_sec = 0;
      tt.it_value.tv_usec = 0;
      endwait = 0;
    }
  }

  return;
}

/*********************************************************************************/
void stopTimer(){

  
  getitimer(ITIMER_REAL, &tt);   // start timer

  if (tt.it_value.tv_sec != 0 || tt.it_value.tv_usec != 0) printf("More time on the clock....\n");

  endwait=0;
  signal (SIGALRM, stopTimer);      // function that handles the alarm
  return;
}

/*********************************************************************************/
int labjackSetup(long int lj, int num, int ljnum){
  //  long int localID=-1;
  long int count=0;
  long int error=0;
  HANDLE ljh;
  u3CalibrationInfo caliInfo;
  int kk=0, ljmax = 0;
/*
  Open first found U6 over USB
*/
  printf("opening usb .... ");

  while (count < 5) {
    //  if( (degptr->temps[num].ljh = openUSBConnection(localID)) == NULL){
    if( (ljh = openUSBConnection(lj)) == NULL){
      count++;
      printf("Opening failed; reseting and attempting %li of 5 more times \n",count);
      printf ("....U3 device reset twice \n");
      LJUSB_ResetConnection(ljh);
      sleep(2);
      LJUSB_ResetConnection(ljh);
      sleep(2);
    } 
    else {
      count = 5;
    }
  }

  printf("opened usb\n");

  kk = ljnum;

  labj[kk].hand = ljh;            // use later in labjack structure
  labj[kk].lj = lj;
  mtcptr->ljh = ljh;                      // file opened...load pointer and use later in channel structure
  mtcptr->lj = lj;                       // file opened...load pointer and use later in channel structure
/*
  Get calibration information from U6
*/
  printf("getting calib .... ");
  error = getCalibrationInfo(mtcptr->ljh, &caliInfo);

  if(error != 0){
    printf("\n%li - %s\n",error, errormsg[error]);
    closeUSBConnection(mtcptr->ljh);
    return (0);
  } 
  else {
    labj[kk].caliInfo = caliInfo;                // load calibration info in struct model
    printf("got calib \n");
    ljmax++;                          // number of labjacks successfully set up
  }

  printf("Completed setup of LabJack SN %li \n",mtcptr->lj);

  return (ljmax);
}

/*********************************************************************************
void resetU3(uint8 res, HANDLE ljh){

//  Resets U3

  uint8 resetIn[4], resetOut[4];
  long int ii=0, error=0;

  for (ii=0; ii<4; ii++){
    resetIn[ii]=0;
    resetOut[ii]=0;
  }
  resetIn[1] = 0x99;
  resetIn[2] = res;
  resetIn[3] = 0x00;
  resetIn[0] = normalChecksum8(resetIn,4);

  if( (error = LJUSB_BulkWrite(ljh, U3_PIPE_EP1_OUT, resetIn, 4)) < 4){
    LJUSB_BulkRead(ljh, U3_PIPE_EP2_IN, resetOut, 4); 
    printf("U3 Reset error: %s\n", errormsg[(int)resetOut[3]]);
    closeUSBConnection(ljh);
    return;
  }
  printf ("....U3 device reset \n");
  return;
}
*/
/*********************************************************************************/
int  mmapSetup() {
  int fd=0, result=0, ii=0;
  /* Open a file for writing.
    *  - Creating the file if it doesn't exist.
    *  - Truncating it to 0 size if it already exists. (not really needed)
    *
    * Note: "O_WRONLY" mode is not sufficient when mmaping.
    */
   fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
   if (fd == -1) {
     perror("Error opening file for writing");
	exit(EXIT_FAILURE);
   }
 /* Stretch the file size to the size of the (mmapped) array of ints
    */
   for (ii=0; ii<sizeof (struct mtc_par); ii++){
     result = write(fd, "D", 1);
     if (result != 1) {
       close(fd);
       perror("Error writing last byte of the file");
       exit(EXIT_FAILURE);
     }
   };
   /*
   result = lseek(fd, FILESIZE-1, SEEK_SET);
   if (result == -1) {
	close(fd);
	perror("Error calling lseek() to 'stretch' the file");
	exit(EXIT_FAILURE);
   }
   */
   /* Something needs to be written at the end of the file to
    * have the file actually have the new size.
    * Just writing an empty string at the current file position will do.
    *
    * Note:
    *  - The current position in the file is at the end of the stretched 
    *    file due to the call to lseek().
    *  - An empty string is actually a single '\0' character, so a zero-byte
    *    will be written at the last byte of the file.
    */
   /*
   result = write(fd, "", 1);
   if (result != 1) {
	close(fd);
	perror("Error writing last byte of the file");
	exit(EXIT_FAILURE);
   }
   */

   /* Now the file is ready to be mmapped.
    */
   mtcptr = (struct mtc_par*) mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
   if (mtcptr == MAP_FAILED) {
	close(fd);
	perror("Error mmapping the file");
	exit(EXIT_FAILURE);
   }
   /* Don't forget to free the mmapped memory
    */

  //   if (munmap(degptr, FILESIZE) == -1) {
   //  if (munmap(mtcptr, sizeof (struct mtc*)) == -1) {
   //	perror("Error un-mmapping the file");
	/* Decide here whether to close(fd) and exit() or not. Depends... */
   //   }

   /* Un-mmaping doesn't close the file, so we still need to do that.
    */
   //   close(fd);

   return (fd);
}

/*********************************************************************************/

void handlerCommand(int sig, siginfo_t *siginfo, void *context){
/*
  Handlers are delicate and don't have all protections the rest
  of the user code has, so get in and out quickly...RLV recommendation!
*/

//  printf ("I caught signal number %i ! \n",degptr->com1);

  return;
}

/*********************************************************************************/
int readConf() {
  FILE *ifile;
  char line[200]="\0";

// see define statement in lnfill.h for deg_conf file name
  int ljnum=0, ljmax=0;
  int ii = 0, lj = 0, t0 = 0, jj=0, mm=0;
  char name[15]="\0", ch0[5]="\0", ch1[5]="\0";
/*
   Read configuration file
*/  

  printf ("Opening configuration file: %s \n",kick_conf);
  if ( ( ifile = fopen (kick_conf,"r+") ) == NULL)
    {
      printf ("*** File on disk (%s) could not be opened: \n",kick_conf);
      printf ("===> %s \n",kick_conf);
      exit (EXIT_FAILURE);
    }
  printf ("Opened: %s \n",kick_conf);

  fgets(line,150,ifile);    // reads column headers
  //   printf ("%s\n",line);
  fgets(line,150,ifile);    // reads ----
  //  printf ("%s\n",line);
/*
 Should be positioned to read file
*/
  ii=0;
  while (ii!=-1) {                   // 1 = true
    fgets(line,150,ifile);
       printf ("%s\n",line);
    if (feof(ifile)) {
      mm = fclose(ifile);
      break;
    }
/*
   A line from the file is read above and processed below
 Index     Labjack SN    Parameter     Labjack parameter       Time(ms)
--------   ----------    ---------    ------------------     -----------
   1        320059423     beam-ON        eio0   eio1            2400                
   2        320059423     beam-OFF       eio2   eio3            2500                   
*/
    mm = sscanf (line,"%i %i %s %s %s %i", &ii, &lj, name, ch0,  ch1,  &t0);
    if (mm == 0) printf ("in values: %i %i %s %s %s %i \n", ii, lj, name, ch0, ch1, t0 );
/*
   Now load the data into the shared memory structure
*/
    mtcptr->pon.ms = 0;      
    /*  
    if (ii == -1) {
      mtcBreak =  findLJchan2(ch0);
      mtcFault = findLJchan2(ch1);
      break;
    }
*/    
    //      printf ("Detected end of conf file after processing %i labjacks and %i channels \n",ljmax,num); 
    //    mtcptr->xx.ms = t0;
    mtcptr->lj = lj;
    switch (ii) {
          case -1:
      mtcBreak =  findLJchan2(ch0);
      mtcFault = findLJchan2(ch1);
      break;
    case 1:
      mtcptr->beam[0] = findLJchan0(ch0);                                  // beam ON in eio
      mtcptr->beam[1] = findLJchan1(ch0);           // beam ON in cio
      mtcptr->beam[0] += findLJchan0(ch1);                                  // beam ON in eio
      mtcptr->beam[1] += findLJchan1(ch1);           // beam ON in cio
      mtcptr->bon.ms = t0;
      mtcptr->bon = time_In_ms(mtcptr->bon);     // set sec and us values in structure
      break;
     case 2:
      mtcptr->meas[0] = findLJchan0(ch0);           // beam OFF values
      mtcptr->meas[1] = findLJchan1(ch0);           // beam ON in cio
      mtcptr->meas[0] += findLJchan0(ch1);
      mtcptr->meas[1] += findLJchan1(ch1);           // beam ON in cio
      mtcptr->boff.ms = t0;                      // load ms times into structure
      mtcptr->boff = time_In_ms(mtcptr->boff);   // set sec and us values in structure
      break;
    case 3:
      mtcptr->kck[0] = findLJchan0(ch0);            // kicker values
      mtcptr->kck[1] = findLJchan1(ch0);           // beam ON in cio
      mtcptr->kck[0] += findLJchan0(ch1);            // kicker values
      mtcptr->kck[1] += findLJchan1(ch1);           // beam ON in cio
      break;
    case 4:
      mtcptr->mtc[0] = findLJchan0(ch0);            // MTC move values
      mtcptr->mtc[1] = findLJchan1(ch0);           // beam ON in cio
      mtcptr->mtc[0] += findLJchan0(ch1);
      mtcptr->mtc[1] += findLJchan1(ch1);           // beam ON in cio
      mtcptr->tmove.ms = t0;
      mtcptr->tmove= time_In_ms(mtcptr->tmove);  // set sec and us values in structure
      break;
    case 5:
      mtcptr->trig[0] = findLJchan0(ch0);            // trig channel
      mtcptr->trig[1] = findLJchan1(ch0);           // beam ON in cio
      mtcptr->trig[0] += findLJchan0(ch1);            // trig channel
      mtcptr->trig[1] += findLJchan1(ch1);           // beam ON in cio
                          //read from file
      if (t0 < 16) mtcptr->trigDT=16;         //ensures 16 us<trigDt<4080 (ie kk=<255)
      else if (t0 > 4080) mtcptr->trigDT=4080;
      else mtcptr->trigDT=t0;
      mtcptr->tdt.ms = t0;
      mtcptr->tdt = time_In_ms(mtcptr->tdt);  // set sec and us values in structure
      break;
    case 6:
      mtcptr->bkg[0] = findLJchan0(ch0);             // bkg channel
      mtcptr->bkg[1] = findLJchan1(ch0);           // beam ON in cio
      mtcptr->bkg[0] += findLJchan0(ch1);             // bkg channel
      mtcptr->bkg[1] += findLJchan1(ch1);           // beam ON in cio
      break;
     case 7:
      mtcptr->lite[0] = findLJchan0(ch0);           // laser lite values
      mtcptr->lite[1] = findLJchan1(ch0);           // beam ON in cio
      mtcptr->lite[0] += findLJchan0(ch1);
      mtcptr->lite[1] += findLJchan1(ch1);           // beam ON in cio
      mtcptr->lon.ms = t0;
      mtcptr->lon= time_In_ms(mtcptr->lon);      // set sec and us values in structure
      break;
    case 8:                                     // timer-counter start for controlling laser pulses
      tcStart = findLJchan2(ch0);
     default:
      break;
    }
  }

/*
  Shrink times for width at the beginning and end (*2)
  It has to go here or else width isn't set. 
*/

  if (mtcptr->bon.ms > 2* mtcptr->trigDT) 
  {
    mtcptr->bon.ms -= 2*mtcptr->trigDT;
    mtcptr->bon = time_In_ms(mtcptr->bon);
  }
  if (mtcptr->boff.ms > 2* mtcptr->trigDT) 
  {
    mtcptr->boff.ms -= 2*mtcptr->trigDT;
    mtcptr->boff = time_In_ms(mtcptr->boff);
  }
  if (mtcptr->boff.ms > 2* mtcptr->trigDT) 
  {
    mtcptr->tmove.ms -= 2*mtcptr->trigDT;
    mtcptr->tmove = time_In_ms(mtcptr->tmove);
  }
  if (mtcptr->lon.ms > 2* mtcptr->trigDT) 
  {
    mtcptr->lon.ms -= 2*mtcptr->trigDT;
    mtcptr->lon = time_In_ms(mtcptr->lon);
  }


/*  
  Setup pause times and labjack
*/
  mtcptr->pon.ms = 0;                        // set default pause values to 0
  mtcptr->pon= time_In_ms(mtcptr->lon);      // set sec and us values in structure

  jj=0;
  mtcptr->lj = lj;
  mtcptr->ljnum = ljnum;                     // store labjack index no. for relation to specific lj calibration
  ljmax=labjackSetup(lj,jj,ljnum);           // set up the labjack of the first one found

  return(ljmax);
}

/*********************************************************************************/
uint8 findLJchan0(char *aaa){
  uint8 ii;
  ii=0;                           // set = 0 as default and in case its a cio channel
  if (strcmp(aaa,"eio0\0") == 0) ii = 0x01;
  if (strstr(aaa,"eio1\0") != NULL) ii = 0x02;
  if (strstr(aaa,"eio2\0") != NULL) ii = 0x04;
  if (strstr(aaa,"eio3\0") != NULL) ii = 0x08;
  if (strstr(aaa,"eio4\0") != NULL) ii = 0x10;
  if (strstr(aaa,"eio5\0") != NULL) ii = 0x20;
  if (strstr(aaa,"eio6\0") != NULL) ii = 0x40;
  if (strstr(aaa,"eio7\0") != NULL) ii = 0x80;
  //  printf ("lj eio chan = %x\n",ii);
  return (ii);
}
/*********************************************************************************/
uint8 findLJchan1(char *aaa){
  uint8 ii;
  ii=0;                        // set = 0 as default and in case its a eio channel
  if (strstr(aaa,"cio0\0") != NULL) ii = 0x01;
  if (strstr(aaa,"cio1\0") != NULL) ii = 0x02;
  if (strstr(aaa,"cio2\0") != NULL) ii = 0x04;
  if (strstr(aaa,"cio3\0") != NULL) ii = 0x08;

  //  printf ("lj cio chan = %x\n",ii);
  return (ii);
}

/*********************************************************************************/
long int findLJchan2(char *aaa){                   // used to id digital i/o signals
  long int ii;
  ii=0;                        // set = 0 as default and in case its a eio channel
  if (strstr(aaa,"fio0\0") != NULL) ii = 0;
  if (strstr(aaa,"fio1\0") != NULL) ii = 1;
  if (strstr(aaa,"fio2\0") != NULL) ii = 2;
  if (strstr(aaa,"fio3\0") != NULL) ii = 3;
  if (strstr(aaa,"fio4\0") != NULL) ii = 4;
  if (strstr(aaa,"fio5\0") != NULL) ii = 5;
  if (strstr(aaa,"fio6\0") != NULL) ii = 6;
  if (strstr(aaa,"fio7\0") != NULL) ii = 7;

  //  printf ("lj cio chan = %x\n",ii);
  return (ii);
}
/*********************************************************************************/
void writeLJ(HANDLE hU3, uint8 arr[], long int num){
  unsigned long int count = 0, error = 0;
  //  BYTE cycleRead[100];     // used for LJUSB_WriteTO
  //  unsigned int timeout;    // used for LJUSB_WriteTO

  count = num;
  
  printf ("Writing to labjack ... 12 = %x   13 = %x  \n",arr[12],arr[13]);
  usleep (200);                          // build in a little pause to give LabJack time to recover from last command
  error = LJUSB_Write(hU3, arr, count);  // sleep was needed when all bits were not set!!!  not sure why
  if (error < num){                      // found at ANL VANDLE run 2015
    if (error == 0) printf("Feedback setup error : write failed\n");
    else printf("Feedback setup error : did not write all of the buffer\n");
  }
  /*  
  error = LJUSB_Write(hU3, arr, count);
  if (error < num){
    if (error == 0) printf("Feedback setup error : write failed\n");
    else printf("Feedback setup error : did not write all of the buffer\n");
  }
  */    
  /*
  sleep(1);
  timeout = 0;
  while (error < 0){
    error = LJUSB_Read(hU3, cycleRead, count);
    if (timeout++ > 10) break;           // added to allow 10 seconds to finish bulkread
  }
  sleep(1);
  printf ("      %li bytes read end cycle %li\n",error,ddxx++);
  */
  return;
}

/*********************************************************************************/
void clearParameters(){
  
  mtcptr->onoff = 0;     // set all modes off

  mtcptr->normal = 0; 
  mtcptr->takeaway = 0;
  mtcptr->pause = 0;

  mtcptr->beammeas = 0;    // always with takeaway
  mtcptr->measbeam = 0;    // option for normal and pause

  mtcptr->trigbeam = 0;    // option: default is true
  mtcptr->laserbeam = 0;   // option for MTAS
  mtcptr->background = 0;  // option for MTAS
  mtcptr->bkgRatio = -1;    // option for MTAS; -1 = all data
  mtcptr->numMove = 1;     // option for normal/pause mode

  mtcptr->trigDT=32;     // width of trigger signal
  return;
}


/**************************************************************/
void clearStats(){
  mtcptr->cyclesData = 0;
  mtcptr->cyclesBkg = 0;
  
  return;
}
/**************************************************************/

