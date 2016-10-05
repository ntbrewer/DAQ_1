/******************************************************************/
/* Program kelvin is used to control the LabJack U3-HV or U3-LV   */
/*    to measure the temperature near the experiment              */
/*                                                                */
/* To be compiled with:                                           */
/*    gcc -lm -llabjackusb u3.o -o kelvin-u3 kelvin-u3.c          */
/*                                                                */
/* C. J. Gross, December 2014                                     */
/******************************************************************/

/* Gather all include files in directory include */

#include "../include/u3-err-msg.h"
#include "../include/u3.h"
#include "../include/labjackusb.h"
#include "../include/kelvin.h"

/* Gather all shared memory and signal handler routines */
sigset_t mask;
sigset_t orig_mask;
struct sigaction act;

void setTimer ();                 // sets the timer for signals
void alarm_wakeup ();
struct itimerval tout_val;        // needed the timer for signals
void handlerCommand(int sig, siginfo_t *siginfo, void *context);  // main handler for signal notification and decisions
void signalBlock(int pp);
//void shmSetup();                  // sets up the shared memory

/* Gather all set up routines  here */
void readConf();                  // processes configuration file
void labjackSetup(long int lj, int num, int ljnum);             // sets up the LabJack U6
void resetU3(uint8 res, HANDLE ljh);
int mmapSetup();       

struct labjack {
  HANDLE hand;
  long int lj;
  int ljnum;    // number of labjack to relate this specific to a particular labjack such as calibrations
  u3CalibrationInfo caliInfo;
} labj[5];

time_t curtime = -1;

#define INTERVAL 60    // interval to read thermometers...60 seconds

long int ddxx=0;
/*
Make U3 stuff global
 */

long int celkelfah=0, thermNum=0;   // 0=celcius; 1=kelvin; 2=fahrenheit and number of thermometers
//long int adc_init(int adc, HANDLE ljh, int range, int ljnum);
//double tempCKF(int ii);
double tempCKF(int ii, long int init);

char *GetDate();

long int maxchan = 0;

//long int Menu();
long int flag=0;

/***********************************************************/ 
/***********************************************************/
int main(int argc, char **argv){
  //  long int localID=-1;
  //  long int error=0, init=0;
  //  long int count=0;
  //  long int ii=0,lj=0, kk=0;
  int kk=0;
  pid_t pid;
  int mapKelvin=0, init=0;

/*
  Set up LabJack U3  ***PUT NOTHING BEFORE THIS AND SHARED MEMORY

  Shared memory creation and attachment
  the segment number is stored in lnptr->pid
*/
  mapKelvin = mmapSetup();
  if (mapKelvin == -1) return 0;

  //    shmSetup();
/*  
   Read setup file on disk and load into shared memory
*/
  printf("Read conf file...\n");
  readConf();                          // this routine calls labjackSetup since we allow more than 1 labjack

  pid = getpid();         // this gets process number of this program
  degptr->pid= pid;       // this is the pid number for the SIGALRM signals
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
    Set up thermometers, read start value, and open
    and write first data to output file.  Purchased from LabJack.
    Using Universal Temperature Probe MODEL EI-1034 with silicon type LM34 sensor
    Typical room accuracy +/-0.4 F waterproof stainless steel tube.
    Using Universal Temperature Probe MODEL EI-1022 with silicon type LM335A sensor
    Typical room accuracy +/-0.1 F waterproof plasticl tube.

    Wiring: Red +5V, Black = ground; White = signal (add 10 kOhm resistor in series for +25 foot length)
    Range is 0-300 F, add negative 5-15 V to ground to extend range below 0 F.
    See manual for extending the range to -50F
*/

/*  
  Setup times based on first read of thermometers
*/
  curtime = time(NULL);
  degptr->time0 = curtime;     // program start time ...store in memory

  //  time1 = time0 + INTERVAL;      // set up next read of RTD...usually every minute
/*  
  Setup monitoring loop to look for changes/minute and requests   
*/
  degptr->interval = INTERVAL;    // default time is 60 seconds
  degptr->com1 = 2;               // set up so it does an initial read and sleep cycle
  degptr->com2 = 1;              // second command will tell deg-u3-log what to do -> 1 = log data every minute

  while(degptr->com1 != -1) {

/*  
  Try to match each switch statement to a corresponding one in deg-u3-read
*/

    switch ( degptr->com1 ) { 
    case  1:              // do nothing as calling program will use what is in shared memory
      degptr->com1 = 2;   // set comand back to regular reading
      break;

    case  2: 
      printf("caught sig 2.");                  // do a regular or forced read...curtime is reset
      curtime = time(NULL);
      degptr->tim.time1 = curtime;        // load current times into shared/mapped memory
      init=0;
      for (kk=0; kk<degptr->maxchan; kk++){                            // load temperatures into shared/mapped memory
	degptr->temps[kk].degree = tempCKF(kk,init);
	degptr->temps[kk].data = (int) (10* degptr->temps[kk].degree); 
      }
      break;

    case  3:              // interval was changed for regular read and sleep cycle
      degptr->com1 = 2;   // set comand back to regular reading
      break;

    default:
      degptr->com1 = 2;          // do nothing and set comand back to regular reading
      break;
    }

   sleep (degptr->interval);
  }

/*
   Release the shared memory and close the labjack
*/
  degptr->com3 = 0;                  // send com to daq program to turn off
  //  shmdt(degptr);                     // detach from shared memory segment
  //  printf("detached from SHM\n");

  //  shmctl(shmid, IPC_RMID, NULL);    // remove the shared memory segment hopefully forever
  //  printf("removed from SHM\n");

  if (munmap(degptr, sizeof (struct thermometer*)) == -1) {
    perror("Error un-mmapping the file");
/* Decide here whether to close(fd) and exit() or not. Depends... */
  }
  close(mapKelvin);

/*
 Close USB connection to all labjacks and end program
*/

  printf ("Total labjack number %i to close\n",ljmax);

  for (kk=0; kk<ljmax; kk++){
    closeUSBConnection(labj[kk].hand);
    printf("USB to LabJack %li closed\n",labj[kk].lj);
  }
  return 0;
}

/***********************************************************/
void signalBlock(int pp){
/*
    Block (p=1) and unblock (p=0) signal SIGALRM
*/

  sigemptyset(&mask);
  sigaddset (&mask,SIGALRM);

  if (pp == 1) {                        
       printf("Block signals attempt\n");
    //    sigemptyset(&mask);
    //   sigaddset (&mask,SIGALRM);
    
    if (sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0) {
      perror ("sigprocmask");
      return;
    }  
  }
  else {    
       printf("release block signals ....\n");
    if (sigprocmask(SIG_SETMASK, &orig_mask, NULL) < 0) {
      perror ("sigprocmask");
      return;
    }
  }
  return;

}

/***********************************************************/
void handlerCommand(int sig, siginfo_t *siginfo, void *context){
/*
  Handlers are delicate and don't have all protections the rest
  of the user code has, so get in and out quickly...RLV recommendation!
*/

//  printf ("I caught signal number %i ! \n",degptr->com1);

  return;
}

/***********************************************************/
void setTimer () {
/*
  Set timer to check status every second
*/

  tout_val.it_interval.tv_sec = 0;
  tout_val.it_interval.tv_usec = 0;

  tout_val.it_value.tv_sec = degptr->interval;  //INTERVAL; // set timer for "INTERVAL (1) seconds at top of file
  tout_val.it_value.tv_usec = 0;

  setitimer(ITIMER_REAL, &tout_val,0);   // start the timer

  printf ("In set timer!\n");
  //  signal(SIGALRM,alarm_wakeup);   // set the Alarm signal capture 

  return;
}

/***********************************************************/
void alarm_wakeup () {
  //  time_t curtime;
  //  GtkWidget *widget;
/*
   Routine where timer ends up on every interval
*/

  tout_val.it_value.tv_sec = degptr->interval;  //INTERVAL; // set timer for "INTERVAL (10) seconds 

  setitimer(ITIMER_REAL, &tout_val,0);   // this starts the timer which will issue the alarm signal...needed to restart

    //  signal(SIGALRM,alarm_wakeup);    // set the Alarm signal capture ... may not be needed 

  return;
 }

/**************************************************************/
int mmapSetup() {
  //#define FILEPATH "data/mmapped.bin"
  //#define FILESIZE sizeof(struct thermometer)
  int fd=0, result=0, ii=0;
  /* Open a file for writing.
    *  - Creating the file if it doesn't exist.
    *  - Truncating it to 0 size if it already exists. (not really needed)
    *
    * Note: "O_WRONLY" mode is not sufficient when mmaping.
    */
  //   fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0644);
   fd = open(KELVINDATAPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0644);
   if (fd == -1) {
	perror("Error opening kelvin file for writing");
	exit(EXIT_FAILURE);
   }

  /* Open a file for writing.
    *  - Creating the file if it doesn't exist.
    *  - Truncating it to 0 size if it already exists. (not really needed)
    *
    * Note: "O_WRONLY" mode is not sufficient when mmaping.
    
   fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
   if (fd == -1) {
	perror("Error opening file for writing");
	exit(EXIT_FAILURE);
   }
*/
 /* Stretch the file size to the size of the (mmapped) array of ints
    */
   //   for (ii=0; ii<sizeof (struct thermometer); ii++){
   for (ii=0; ii<KELVINDATASIZE; ii++){
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
   degptr = (struct thermometer*) mmap(0, KELVINDATASIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
   if (degptr == MAP_FAILED) {
	close(fd);
	perror("Error mmapping the kelvin file");
	exit(EXIT_FAILURE);
   }
   /* Don't forget to free the mmapped memory
    */
   /*
  //   if (munmap(degptr, FILESIZE) == -1) {
  if (munmap(degptr, sizeof (struct thermometer*)) == -1) {
    perror("Error un-mmapping the file");
    close(fd);
    fd = -1;
  }
   */
   return (fd);
}


/**************************************************************/
void labjackSetup(long int lj, int num, int ljnum){
  //  long int localID=-1;
  long int count=0;
  long int error=0;
  HANDLE ljh;
  u3CalibrationInfo caliInfo;
  int kk;
/*
  Open first found U6 over USB
*/
  printf("opening usb .... ");

  while (count < 5) {
    //  if( (degptr->temps[num].ljh = openUSBConnection(localID)) == NULL){
    if( (ljh = openUSBConnection(lj)) == NULL){
      count++;
      printf("Opening failed; reseting and attempting %li of 5 more times \n",count);
      printf ("....U3 device reset \n");
      resetU3(0x01,ljh);                       // 0x00 = soft reset; 0x01 = reboot
      sleep (2);
      resetU3(0x01,ljh);                       // 0x00 = soft reset; 0x01 = reboot
      sleep (2);
      if (count > 5) return;
    } 
    else {
      count = 5;
    }
  }

  printf("opened usb\n");

  kk = ljnum;

  labj[kk].hand = ljh;            // use later in labjack structure
  labj[kk].lj = lj;
  degptr->temps[num].ljh = ljh;       // file opened...load pointer and use later in channel structure

/*
  Get calibration information from U6
*/
  printf("getting calib .... ");
  error = getCalibrationInfo(degptr->temps[num].ljh, &caliInfo);

  if(error != 0){
    printf("\n%li - %s\n",error, errormsg[error]);
    closeUSBConnection(degptr->temps[num].ljh);
    return;
  } 
  else {
    labj[kk].caliInfo = caliInfo;                // load calibration info in struct model
    printf("got calib \n");
    ljmax++;                          // number of labjacks successfully set up
  }

  printf("Completed setup of LabJack SN %i \n",degptr->temps[num].lj);

  return;
}

/***********************************************************/
void resetU3(uint8 res, HANDLE ljh){
/*
  Resets U3
*/
  uint8 resetIn[4], resetOut[4];
  int ii=0, error=0;

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

/**************************************************************/

void readConf() {
  FILE *ifile;
  char line[200]="\0";

// see define statement in lnfill.h for deg_conf file name

  int lj = 0;
  int jj=0, mm=0;
  int ii=0, onoff=0, chan=0, unit=0, para=0, model=0, range=0, ljnum=0, init=1; 
  char name[10]="\0";
  int num=0, kk=0, kflag=0;

/*
   Read configuration file
*/  

  printf ("Opening configuration file: %s \n",KELVINCONF);
  if ( ( ifile = fopen (KELVINCONF,"r+") ) == NULL)
    {
      printf ("*** File on disk (%s) could not be opened: \n",KELVINCONF);
      printf ("===> %s \n",KELVINCONF);
      exit (EXIT_FAILURE);
    }
  printf ("Opened: %s \n",KELVINCONF);

  fgets(line,150,ifile);    // reads column headers
  //   printf ("%s\n",line);
  fgets(line,150,ifile);    // reads ----
  //  printf ("%s\n",line);
/*
 Should be positioned to read file
*/
  num = 0;   // number of lines in conf file


  while (1) {                   // 1 = true
    fgets(line,150,ifile);
       printf ("%s\n",line);
    if (feof(ifile)) {
      mm = fclose(ifile);
      break;
    }
/*
   A line from the file is read above and processed below
 Num   Name   Model    OnOff   LabJack   AinX     CKF    Daq Parameter  30 = U3-LV, 31 = U3-HV, 32 = U3 extended range
---   ----   -----    -----   -------   -----   -----   -------------   0 = Celcius, 1 = Kelvin, 2 = Fahrenheit 
 1     A1    1022       1       30        0       0          21
 2     B2    1022       1       30        1       0          22
   
*/
    mm = sscanf (line,"%i %s %i %i %i %i %i %i %i", &ii, name, &model, &onoff, &lj, &range, &chan, &unit, &para);
    printf ("Read in %i values\n",mm);
    printf ("in values: %i %s %i %i %i %i %i %i %i \n", ii, name, model, onoff, lj, range, chan, unit, para);
/*
   Now load the data into the shared memory structure
*/
    if (ii == 999){
      printf ("Detected end of conf file after processing %i labjacks and %i channels \n",ljmax,num); 
      printf ("Now read least and most signficant byte pairs for current time \n"); 
      fgets(line,150,ifile);
      //      printf ("%s \n",line); 
      mm = sscanf (line,"%s %i", name, &para);
      degptr->timeMSB = para;
      //      printf ("%i %i \n",degptr->timeMSB,para); 

      fgets(line,150,ifile);
      //      printf ("%s \n",line); 
      mm = sscanf (line,"%s %i", name, &para);
      degptr->timeLSB = para;
      //      printf ("%i %i \n",degptr->timeLSB,para); 
      break;
    }
    jj = ii-1;
    printf("hi jj=%li \n",degptr->shm);
    strcpy(degptr->temps[jj].name,name);
    printf("hi jj=%i degptr-> %i \n",jj,degptr->temps[0].onoff);
    degptr->temps[jj].onoff = onoff;
    degptr->temps[jj].lj = lj;
    degptr->temps[jj].model = model;
    degptr->temps[jj].range = range;
    degptr->temps[jj].chan = chan;
    degptr->temps[jj].unit = unit;
    degptr->temps[jj].para = para;
    /*
    strcpy(degptr->temps[jj].name,name);
    degptr->temps[jj].onoff = onoff;
    degptr->temps[jj].lj = lj;
    degptr->temps[jj].model = model;
    degptr->temps[jj].range = range;
    degptr->temps[jj].chan = chan;
    degptr->temps[jj].unit = unit;
    degptr->temps[jj].para = para;
    */
    if (ljnum == 0) {                                // step though and initialize labjacks as we get them
      degptr->temps[jj].ljnum = ljnum;               // store labjack index no. for relation to specific lj calibration
      labjackSetup(lj,jj,ljnum);                     // set up the labjack of the first one found
      ljnum++;
    }
    else if (ljnum > 0){                             // look for multiple labjacks
      kflag=0;                                       // set to see if multiple lab jacks are being used
      for (kk=0; kk<num; kk++){                      // scroll thru each parameter to see if lj is already set up
	if (lj == degptr->temps[kk].lj) {      
	  degptr->temps[jj].lj = degptr->temps[kk].lj;         // known labjack, copy the parameters
	  degptr->temps[jj].ljh = degptr->temps[kk].ljh;       // known labjack, copy the parameters
	  degptr->temps[jj].ljnum = degptr->temps[kk].ljnum;   // known labjack, copy the parameters
	  kflag=1;                                             // change flag to show this one is known	  
	}
      }
      if (kflag == 0){                               // unknown labjack since kflag still 0...set it up
	degptr->temps[jj].ljnum = ljnum;             // store labjack index no. for relation to specific lj calibration
	labjackSetup(lj,jj,ljnum);                   // set up the labjack
	ljnum++;                                     // keep track of number of labjacks used
      }
    }
    num++;               // number of channels in conf file to use
  }
  degptr->maxchan = num;

/*
  printf ("Num = %i\n",num);
  
  int nn=0;
  for (nn=0; nn<num; nn++){
    printf (" %i \n", degptr->temps[nn].model);
    printf ("      %i \n", degptr->temps[nn].ljnum);
    printf ("     %li \n", degptr->temps[nn].lj);
    printf ("     %li \n", degptr->temps[nn].ljh);
    printf ("      %i \n", degptr->temps[nn].chan);
    printf ("      %i \n", degptr->temps[nn].range);
    printf ("      %i \n", degptr->temps[nn].onoff);
    printf ("      %i \n", degptr->temps[nn].unit);
    printf ("      %i \n", degptr->temps[nn].para);
    printf ("      %i \n", degptr->temps[nn].data);
    printf ("     %lf \n", degptr->temps[nn].degree);
  }
*/

/*  
  Setup times based on first read of thermometers
*/
  curtime = time(NULL);
  degptr->time0 = curtime;     // program start time ...store in memory

  init=1;
  for (kk=0; kk<degptr->maxchan; kk++){                       // initialize each ADC - needed for U3 labjacks
    degptr->temps[kk].degree = tempCKF(kk,init);
    degptr->temps[kk].data = (int) (10* degptr->temps[kk].degree); 
  }

  printf ("ADCs are all initialized  \n"); 

  for (kk=0; kk<degptr->maxchan; kk++){                       // initialize each ADC - needed for U3 labjacks
    printf ("%i temp %hi = %.1lf \n", kk, degptr->temps[kk].unit,degptr->temps[kk].degree);
  }

  return;
}

/***********************************************************/
double tempCKF(int ii, long int init) {

/*
  reads temperature with command and returns in ckf usits:
     fck = 0 - Celcius
           1 - Kelvin
           2 - Farenheit

long eAIN(HANDLE Handle, 
          u3CalibrationInfo *CalibrationInfo, 
          long ConfigIO,    // =1 configure and initialize; =0 after 
          long *DAC1Enable, // =1 TRUE...always true except for hardware rev 1.20, 1.21
          long ChannelP, 
          long ChannelN,    // 31 for single ended values, other eAIN for differential, 32 for extended voltage range
          double *Voltage,  // ADC value read
          long Range, 
          long Resolution, 
          long Settling, 
          long Binary, 
          long Reserved1, long Reserved2)
*/
  long int error=0, DAC1Enable=1;
  double involts=0.0, deg=0.0, degC=0.0, degK=0.0, degF=0.0;

  error = eAIN (degptr->temps[ii].ljh, 
		&labj[degptr->temps[ii].ljnum].caliInfo, 
		init, 
		&DAC1Enable, 
		degptr->temps[ii].chan, 
		degptr->temps[ii].range, 
		&involts, 
		0, 0, 0, 0, 0, 0);
 
  if (error != 0){
    printf("%li - %s\n", error, errormsg[error]);
    closeUSBConnection(degptr->temps[ii].ljh);
    return 0;
  }
/*   
     temperature probe EI-1034,1022, Pt-100 and U3-HV 
*/
  if (degptr->temps[ii].model == 1034) {
      degF = involts*100.0;
      degK = (55.56*involts) + 255.37;
      degC = degK - 273.15;
  }
  if (degptr->temps[ii].model == 1022){      
      degK = involts*100.0;
      degC = degK - 273.15;
      degF = degC*1.8 + 32;
  }
  if (degptr->temps[ii].model == 100){      
      degC = (involts*2.5-1)/0.003911;
      degK = degC + 273.15;
      degF = degC*1.8 + 32;
  }
  if (degptr->temps[ii].model == 1000){      
      degC = (involts)*1000;
      degK = degC;
      degF = degC;
  }
  
  deg = degC;
  if (degptr->temps[ii].unit == 1) deg = degK;
  if (degptr->temps[ii].unit == 2) deg = degF;

  return (deg);
}

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

//#include <stdio.h>
//#include <stdlib.h>
//#include <time.h>

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
/******************************************************************************/

/**************************************************************/
/*void shmSetup() {
  //   file is probably: include/deg.conf
  printf("Setting up shared memory...\n");

  shmKey = ftok(deg_conf,'b');                // key unique identifier for shared memory, other programs use 'LN' tag
  //  shmKey = ftok("include/deg.conf",'b');  // key unique identifier for shared memory, other programs use 'LN' tag

  shmid = shmget(shmKey, sizeof (struct thermometer), 0666 | IPC_CREAT);   // get ID of shared memory, size, permissions, create if necessary
  if (shmid == -1){
    printf("shmid=%li, shmKey=%i, size=%li\n",
	   shmid, shmKey, sizeof (struct thermometer));
    perror("shmget");
    exit(EXIT_FAILURE);
  }
  printf("May need to remove shmid: ipcrm -m %li  \n",shmid);
  degptr =  (struct thermometer*) shmat (shmid, (void *)0, 0);   // link area used; struct lnfill pointer char *lnptr

  if (degptr == (struct thermometer *)(-1)){                                  // check for errors
    printf("number 2a error\n");
    perror("shmat");
    exit(EXIT_FAILURE);
  }

  degptr->shm = shmid;   // this is the number of the shared memory segment

  return;
}
*/
/**************************************************************/
