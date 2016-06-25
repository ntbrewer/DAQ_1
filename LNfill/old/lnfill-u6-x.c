/*
  Program lnfill-u6 to handle LN filling and monitoring of Ge
  when RMS system is not available.

  Will put relevant data into shared memory where other programs
  can access it and decide actions such as turning off HV.

  Program will be based on LabJack U6 USB daq, dataport probe
  power strip with remote relay control.

*/

#include "lnfill.h"
#include "../u6.h"
#include "../labjackusb.h"

#define INTERVAL 60
#define IBOOTBAR_IP "192.168.0.254"  //  Default from iBootBar: mask 255.255.255.0 
//#define IBOOTBAR_IP "192.168.13.33"  // ORNL address: mask 255.255.255.0   Subnet 192.168.13.0; name:powerbar0 (no phy.ornl.gov)
#define IBOOTBAR_MIB "include/ibootbar_v1.50.258.mib"

void readConf();                  // processes configuration file
void shmSetup();                  // sets up the shared memory
int labjackSetup();               // sets up the LabJack U6
double readRTD(long int ii);      // reads RTDs for temperature
void resetU6(uint8 res);          // resets the U6

void setTimer ();                 // sets the timer for signals
void alarm_wakeup ();
struct itimerval tout_val;        // needed the timer for signals

void handlerCommand(int sig, siginfo_t *siginfo, void *context);  // main handler for signal notification and decisions
void signalBlock(int pp);

HANDLE hU6;                       // LabJack stuff
u6CalibrationInfo caliInfo;       // LabJack stuff

void updateRTD();                 // read RTDs and store values into shared memory

void snmp(int setget, char *cmd, char *cmdResult);   // snmp get/set commands
int readOnOff(char *cmdResult);                      // snmp get/set results (on/off)
int readInt(char *cmdResult);                        // snmp get/set results (integer)
float readFloat(char *cmdResult);                    // snmp get/set results (floats)
unsigned int readBits(char *cmdResult);              // snmp get/set results (bits)
 
void fillGe ();
void fillAllDelete ();
void openTank();
void closeTank();
void openMani();
void closeMani();
void checkMani();
void openDet(int ii);
void closeDet(int ii);
void checkDet(int ii);
void closeAllValves ();
void outletStatus();
void sendEmail();
void readEmail();
void updateStatus (int mm, int nn);

char *errormsg[] =
   {
     "no msg code",   //0
     "no msg code",   //1
     "no msg code",   //2
	"DATA_BUFFER_OVERFLOW",   //3	
	"ADC0_BUFFER_OVERFLOW",   //4
	"FUNCTION_INVALID",   //5	
	"SWDT_TIME_INVALID",   //6	
	"XBR_CONFIG_ERROR",   //7	
        "no msg code",   //8
        "no msg code",   //9
        "no msg code",   //10
        "no msg code",   //11
        "no msg code",   //12
        "no msg code",   //13
        "no msg code",   //14
        "no msg code",   //15
	"FLASH_WRITE_FAIL",   //	16
	"FLASH_ERASE_FAIL",   //	17
	"FLASH_JMP_FAIL",   //	18
	"FLASH_PSP_TIMEOUT",   //19	
	"FLASH_ABORT_RECEIVED",   //20	
	"FLASH_PAGE_MISMATCH",   //21	
	"FLASH_BLOCK_MISMATCH",   //22	
	"FLASH_PAGE_NOT_IN_CODE_AREA",   //23	
	"MEM_ILLEGAL_ADDRESS",   //24	
	"FLASH_LOCKED",   //	25
	"INVALID_BLOCK",   //	26
	"FLASH_ILLEGAL_PAGE",   //27	
	"FLASH_TOO_MANY_BYTES",   //28	
	"FLASH_INVALID_STRING_NUM",   //29	
        "no msg code",   //30
        "no msg code",   //31
        "SMBus_inQ_overflow",   //32
        "SMBus_outQ_underflow",   //33
        "SMBus_CRC_failed",   //34
        "no msg code",   //35
        "no msg code",   //36
        "no msg code",   //37
        "no msg code",   //38
        "no msg code",   //39
	"SHT1x_COMM_TIME_OUT",   //40	
	"SHT1x_NO_ACK",   //	41
	"SHT1x_CRC_FAILED",   //	42
	"SHT1X_TOO_MANY_W_BYTES",   //43	
	"SHT1X_TOO_MANY_R_BYTES",   //44	
	"SHT1X_INVALID_MODE",   //45	
	"SHT1X_INVALID_LINE",   //46
        "no msg code",   //47	
	"STREAM_IS_ACTIVE",   //	48
	"STREAM_TABLE_INVALID",   //	49
	"STREAM_CONFIG_INVALID",   //	50
	"STREAM_BAD_TRIGGER_SOURCE",   //51	
	"STREAM_NOT_RUNNING",   //	52
	"STREAM_INVALID_TRIGGER",   //	53
	"STREAM_ADC0_BUFFER_OVERFLOW",   //54	
	"STREAM_SCAN_OVERLAP",   //	55
	"STREAM_SAMPLE_NUM_INVALID",   //56	
	"STREAM_BIPOLAR_GAIN_INVALID",   //57	
	"STREAM_SCAN_RATE_INVALID",   //	58
	"STREAM_AUTORECOVER_ACTIVE",   //59	
	"STREAM_AUTORECOVER_REPORT",   //60
        "STREAM_SOFTPWM_ON",   //61	
        "no msg code",   //62	
	"STREAM_INVALID_RESOLUTION",   //63	
	"PCA_INVALID_MODE",   //	64
	"PCA_QUADRATURE_AB_ERROR",   //65	
	"PCA_QUAD_PULSE_SEQUENCE",   //66	
	"PCA_BAD_CLOCK_SOURCE",   //67
	"PCA_STREAM_ACTIVE",   //68	
	"PCA_PWMSTOP_MODULE_ERROR",   //69	
	"PCA_SEQUENCE_ERROR",   //70	
	"PCA_LINE_SEQUENCE_ERROR",   //71	
	"PCA_SHARING_ERROR",   //72	
        "no msg code",   //73
        "no msg code",   //74
        "no msg code",   //75
        "no msg code",   //76
        "no msg code",   //77
        "no msg code",   //78
        "no msg code",   //79
	"EXT_OSC_NOT_STABLE",   //80	
	"INVALID_POWER_SETTING",   //81	
	"PLL_NOT_LOCKED",   //82	
        "no msg code",   //83
        "no msg code",   //84
        "no msg code",   //85
        "no msg code",   //86
        "no msg code",   //87
        "no msg code",   //88
        "no msg code",   //89
        "no msg code",   //90
        "no msg code",   //91
        "no msg code",   //92
        "no msg code",   //93
        "no msg code",   //94
        "no msg code",   //95
	"INVALID_PIN",   //96	
	"PIN_CONFIGURED_FOR_ANALOG",   //97	
	"PIN_CONFIGURED_FOR_DIGITAL",   //98	
	"IOTYPE_SYNCH_ERROR",   //99	
	"INVALID_OFFSET",   //100
	"IOTYPE_NOT_VALID",   //101	
	"no msg code",   //102	
        "no msg code",   //103
        "no msg code",   //104
        "no msg code",   //105
        "no msg code",   //106
        "no msg code",   //107
        "no msg code",   //108
        "no msg code",   //109
        "no msg code",   //110
        "no msg code",   //111
	"no msg code",   //112	
	"no msg code",   //113	
	"no msg code",   //114	
   };

long int time0=0, time1=0, nread;

sigset_t mask;
sigset_t orig_mask;
struct sigaction act;

/***********************************************************/
int main(int argc, char **argv){
/*
  key_t shmKey;
  int shmid;

  time_t curtime = -1;
  char xtime[40]="\0";
  long int size=4194304; //   = 4 MB // 131072;  //65536;
*/
  long int ii=0, p0=0, p1=1, count=0, etime=0;
  long int adc=0;
  pid_t pid;
  int xx=0;

/*
  Set up LabJack U6  ***PUT NOTHING BEFORE THIS AND SHARED MEMORY
*/
  labjackSetup();
/*
  Shared memory creation and attachment
  the segment number is stored in lnptr->pid
*/
  shmSetup();
/*  
   Set up the signal capture routine for when the reading
   program wants something changed
*/
  pid = getpid();    // this gets process number of this program
  lnptr->pid= pid;   // this is the pid number for the SIGALRM signals

  memset (&act,'\0', sizeof(act));
  act.sa_sigaction = &handlerCommand;
  act.sa_flags = SA_SIGINFO;
  if (sigaction(SIGALRM,&act,NULL) < 0){
    perror("sigaction");
    return 1;
  }
/*  
   Read setup file on disk and load into shared memory
*/
  printf("Read conf and email file...\n");
  readConf();
  readEmail();
  printf(" ... Conf file read \n");
/*
  Go out and read RTDs and get everything needed loaded into shared memory
*/

  updateRTD();
  /*
  for (ii=0; ii<20; ii++){
    if (lnptr->ge[ii].onoff == 1){
      lnptr->ge[ii].rtd = readRTD(lnptr->ge[ii].chanRTD);
      lnptr->ge[ii].oflo = readRTD(lnptr->ge[ii].chanOFLO);
      printf("Active det: %li ->  %lf   %lf\n", ii,lnptr->ge[ii].rtd,lnptr->ge[ii].oflo );
    }
  }
  lnptr->tank.rtd = readRTD(lnptr->tank.chanRTD);
  lnptr->tank.pressure = readRTD(lnptr->tank.chanPRES);

  printf("TANK: %lf\n", lnptr->tank.rtd);
  */
/*  
  Setup time of next fill based on current time and configure file
*/
  curtime = time(NULL);
  time0 = curtime;               // record starting time of program
  time1 = time0 + INTERVAL;      // set up next read of RTD...usually every minute

  for (ii=0; ii<20; ii++){
    if (lnptr->ge[ii].onoff == 1){
      lnptr->ge[ii].next = time0 + lnptr->ge[ii].interval;
    }
    else {
      lnptr->ge[ii].next = 0;
    }
  }

/*  
  Setup monitoring loop to look for changes and requests   
*/
//  setTimer();

//  outletStatus();
/*  
  Setup monitoring loop to look for changes/minute and requests   
*/
  nread = INTERVAL;
  lnptr->command = 0;

  while(lnptr->command != -1) {
    if (strcmp(lnptr->comStatus,"FILL ALL INTERRUPTED") == 0){
      if (++count > 10) strcpy(lnptr->comStatus,"NORMAL");
    }
    else {
      strcpy(lnptr->comStatus,"NORMAL");
      count=0;
    }


    //    curtime = time(NULL);                   // check what time it is - confirm still running
    //    lnptr->secRunning = curtime - time0;    // record time running (lets user now it it is still up)
    updateRTD();                            // read and update the RTD information including lnptr->secRunning 
    //    outletStatus ();                        // read status of outlets
/*
    if (curtime >= time1) {                 // read RTDs every INTERVAL (usually ~60 s)
      //      time1 = time1+INTERVAL;
    /     time1 = time1+nread;              // read RTDs every nread (usually INTERVAL though during a fill it might be more often)
      updateRTD();
    }

   Check if an RTD is too high
*/
    for (ii=0;ii<20;ii++){                               // run thru each detector possibility
      if (lnptr->ge[ii].onoff == 1) {                    // run thru each detector that is ON
	if (lnptr->ge[ii].rtd > lnptr->ge[ii].limit){    // check rtd is within limit
	  //	  printf("SHUT DOWN !\n");
	  //	  strcpy(lnptr->ge[ii].status,"ALARM");
	  if (curtime > etime + 900 && (strcmp(lnptr->ge[ii].status,"ALARM") == 0 ||strcmp(lnptr->ge[ii].status,"NO RTD") == 0)) {  // sending email on HIGH TEMP or LONG FILL TIME
	    sendEmail();
	    etime = curtime;
	  }
	}
	if (curtime > lnptr->ge[ii].next){               // check if its time to fill a detector
	  printf("start a fill ...set command to 8 = Fill All! \n");
	  lnptr->command = 8;
	} 
      }
    }

    //  AFTER THIS LOOP CHECK FOR COMMANDS COMING IN FROM CONTROL PROGRAM

    if (lnptr->command > 20 &&   lnptr->command <= 40){
      closeDet(lnptr->command-21);                      // way to close individual valves
      lnptr->command=0;
    }
    else if (lnptr->command > 40 &&  lnptr->command <= 60) {
      openDet(lnptr->command-41);                       // way to open individual valves
      lnptr->command=0;
    }
    else {
      switch (lnptr->command) {
      case 0:
/*
      //      signal(SIGTERM,alarm_wakeup);   // set the Alarm signal capture 
      sleep (1);                      // pause for time
      curtime = time(NULL);
      printf ("curtime = %li %li \n", curtime, (time1-curtime));
      if (curtime >= time1) {
	time1 = time1+INTERVAL;
	updateRTD();
      }
*/
	break;
      case 1:
	printf ("1 selected\n");
	updateRTD();
	lnptr->command=0;
	break;
      case 2:
	printf ("2 selected\n");
	lnptr->command=0;
	break;
      case 3:
	printf ("3 selected\n");
	lnptr->command=0;
	break;

      case 7:
	printf ("7 selected\n");   // fill 1 detector
	signalBlock(p1);
	lnptr->command=8;  // set to 8 in order to get thru fill routine
	fillGe();
	signalBlock(p0);
	lnptr->command=0;
	break;
      case 8:
	printf ("8 selected\n");    // fill All
	signalBlock(p1);
	fillGe();
	signalBlock(p0);
	lnptr->command=0;
	break;
      case 9:
	printf ("9 selected\n");     // initial cooldown
	signalBlock(p1);
	fillGe();
	signalBlock(p0);
	lnptr->command=0;
	break;

      case 10:
	printf ("10 selected\n");
	closeAllValves();             // close all valves including tank but open manifold
	lnptr->command=0;
	break;
      case 11:
	printf ("11 selected\n");
	closeTank();                  // open tank
	lnptr->command=0;
	break;
      case 12:
	printf ("12 selected\n");
	openTank();                  // close Tank
	lnptr->command=0;
	break;
      case 13:
	printf ("13 selected\n");
	closeMani();                  // close Manifold
	lnptr->command=0;
	break;
      case 14:
	printf ("14 selected\n");
	openMani();                  // open Manifold
	lnptr->command=0;
	break;
      case 17:
	signalBlock(p1);
	printf ("17 selected\n");     // initial cooldown
	outletStatus ();
	lnptr->command=0;
	signalBlock(p0);

	break;
      case 18:
	printf ("18 selected\n");     // initial cooldown
	lnptr->command=0;
	break;

      case 61:
	printf ("Sending emails\n");     // initial cooldown
	sendEmail();
	lnptr->command=0;
	break;

      default:
	printf ("default selected\n");
	lnptr->command=0;
	break;
      }
    }  

    sleep (nread);
  }


/*
   Release the shared memory and close the U6

*/
  shmdt(lnptr);                     // detach from shared memory segment
  printf("detached from SHM\n");

  shmctl(shmid, IPC_RMID, NULL);    // remove the shared memory segment hopefully forever
  printf("removed from SHM\n");

  closeUSBConnection(hU6);
  printf("USB closed\n");


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
void updateRTD(){
  long int ii=0;
/*
  read only active RTDs and tank RTD and Pressure
  should only be 4 status conditions
 - OK
 - OK-FILL
 - NO RTD
 - ALARM
*/
  curtime = time(NULL);
  lnptr->secRunning = curtime - time0; 

  for (ii=0; ii<20; ii++){
    if (lnptr->ge[ii].onoff == 1){
      lnptr->ge[ii].rtd = readRTD(lnptr->ge[ii].chanRTD);
      lnptr->ge[ii].oflo = readRTD(lnptr->ge[ii].chanOFLO);
      //      printf("Active det: %li ->  %lf   %lf\n", ii,lnptr->ge[ii].rtd,lnptr->ge[ii].oflo );
      if (lnptr->ge[ii].rtd >= lnptr->ge[ii].limit) strcpy(lnptr->ge[ii].status,"ALARM");
      else if (lnptr->ge[ii].rtd <= 70) strcpy(lnptr->ge[ii].status,"NO RTD");
      else if (strcmp(lnptr->ge[ii].status,"LONG") == 0) break;
      else {
	strcpy(lnptr->ge[ii].status,"OK");
	if (lnptr->command == 7 || lnptr->command == 8) strcpy(lnptr->ge[ii].status,"FILL");
      }
    }
  }
  lnptr->tank.rtd = readRTD(lnptr->tank.chanRTD);
  lnptr->tank.pressure = readRTD(lnptr->tank.chanPRES);

  //  printf("TANK: %lf\n", lnptr->tank.rtd);
  return;
}

/***********************************************************/
void handlerCommand(int sig, siginfo_t *siginfo, void *context){
/*
  Handlers are delicate and don't have all protections the rest
  of the user code has, so get in and out quickly...RLV recommendation!
*/

  printf ("I caught signal number %i ! \n",lnptr->command);

  return;
}

/***********************************************************/
void setTimer () {
/*
  Set timer to check status every second
*/

  tout_val.it_interval.tv_sec = 0;
  tout_val.it_interval.tv_usec = 0;

  tout_val.it_value.tv_sec = INTERVAL; // set timer for "INTERVAL (1) seconds at top of file
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

  tout_val.it_value.tv_sec = INTERVAL; // set timer for "INTERVAL (10) seconds 

  setitimer(ITIMER_REAL, &tout_val,0);   // this starts the timer which will issue the alarm signal....needed to restart

    //  signal(SIGALRM,alarm_wakeup);   // set the Alarm signal capture ... may not be needed 

  lnptr->ge[0].rtd = readRTD(lnptr->ge[0].chanRTD);
  lnptr->ge[0].oflo = readRTD(lnptr->ge[0].chanOFLO);
  printf("Alarm_wakeup: 1,1 ->  %lf   %lf\n", lnptr->ge[0].rtd,lnptr->ge[0].oflo );

  return;
 }

/**************************************************************/

void shmSetup() {

  printf("Setting up shared memory...\n");                              ///Users/c4g/src/LNfill/include/lnfill.conf
  shmKey = ftok(file_shm,'b');                                          // key unique identifier for shared memory, other programs use 'LN' tag
  shmid = shmget(shmKey, sizeof (struct lnfill), 0666 | IPC_CREAT);     // gets ID of shared memory, size, permissions, create if necessary
  lnptr = shmat (shmid, (void *)0, 0);                                  // now link to area so it can be used; struct lnfill pointer char *lnptr
  if (lnptr == (struct lnfill *)(-1)){                                  // check for errors
    perror("shmat");
    exit;
  }

  lnptr->shm = shmid;   // this is the number of the shared memory segment

  //  pid = getpid();    // this gets process number of this program
  //  lnptr->pid= pid;   // this is the pid number for the SIGALRM signals

/*
  printf("pid = %li \n",(long int)lnptr->pid);
  printf ("shm size = %li\n",sizeof (struct lnfill) );
  printf("... set shared memory...\n");
*/
  return;
}

/**************************************************************/

void readConf() {
  FILE *ifile;
  char line[200]="\0";
  //  char lnfill_conf[200]="/Users/c4g/src/Labjack/LNfill/include/lnfill.conf";   //see define statement in lnfill.h 
  long int ii=0, mm=0;
  int onoff=0, chanRTD=0, chanOFLO=0, Ibar=0, mani=0;
  char name[10]="\0";
  double interval=0.0, max=0.0, min=0.0, limit=0.0, olimit=0.0;

  struct dewar ge;

/*
   Read configuration file
*/  

  if ( ( ifile = fopen (lnfill_conf,"r+") ) == NULL)
    {
      printf ("*** File on disk (%s) could not be opened: \n",lnfill_conf);
      printf ("===> %s \n",lnfill_conf);
      exit (EXIT_FAILURE);
    }

  fgets(line,150,ifile);    // reads column headers
  fgets(line,150,ifile);    // reads ----

/*
 Should be positioned to read file
*/
  while (1) {                   // 1 = true
    fgets(line,150,ifile);
    if (feof(ifile)) {
      mm = fclose(ifile);
      break;
    }
/*
   A line from the file is read above and processed below
*/
    mm = sscanf (line,"%li %s %i %lf %lf %lf %i %i %lf %lf %i", &ii, name, &onoff, &interval, &max, &min, &chanRTD, &chanOFLO, &limit, &olimit, &Ibar);
//    printf("\n %li %s %i %lf %lf %lf %i %i %lf \n", ii, name, onoff, interval, max, min, chanRTD, chanOFLO, limit);
    if (ii == 21) {
      mm = sscanf (line,"%li %s %i %lf %lf %lf %i %i %lf %lf %i %i", &ii, name, &onoff, &interval, &max, &min, &chanRTD, &chanOFLO, &limit, &olimit, &Ibar,&mani);
    }
/*
   Now load the data into the shared memory structure
*/

    if (ii < 21) {
      strcpy(lnptr->ge[ii-1].name,name);
      lnptr->ge[ii-1].onoff = onoff;
      lnptr->ge[ii-1].interval = interval;
      lnptr->ge[ii-1].max = max;
      lnptr->ge[ii-1].min = min;
      lnptr->ge[ii-1].chanRTD = chanRTD;
      lnptr->ge[ii-1].chanOFLO = chanOFLO;
      lnptr->ge[ii-1].limit = limit;
      lnptr->ge[ii-1].olimit = olimit;
      lnptr->ge[ii-1].chanIbar = Ibar;
    } else {
      lnptr->tank.chanRTD = chanRTD;
      lnptr->tank.chanPRES = chanOFLO;
      lnptr->tank.timeout = max;
      lnptr->tank.limit = limit;
      lnptr->tank.olimit = olimit;
      lnptr->tank.chanIbar = Ibar;
      lnptr->tank.chanMani = mani;
    }
  }
   
  for (ii=0; ii<20; ii++){
    printf("%2li %3s   %i   %0.lf    %0.lf    %0.lf    %4.lf    %4.lf   %i  \n",ii,lnptr->ge[ii].name,lnptr->ge[ii].onoff,lnptr->ge[ii].interval,lnptr->ge[ii].max,lnptr->ge[ii].min,lnptr->ge[ii].limit,lnptr->ge[ii].olimit,lnptr->ge[ii].chanIbar);
  }

  printf("21    TANK     1       0      %3.0lf       0     %2i      %2i      %3.0lf     %3.0lf      %i      %i \n", 
	  lnptr->tank.cooltime,    lnptr->tank.chanRTD, lnptr->tank.chanPRES, lnptr->tank.limit, lnptr->tank.olimit, lnptr->tank.chanIbar, lnptr->tank.chanMani);

  //  printf("%2li %3s   %i   %0.lf    %0.lf    %0.lf    %4.lf   %i     %i\n",ii,lnptr->tank.name,lnptr->tank.onoff,lnptr->tank.interval,lnptr->tank.max,lnptr->tank.min,lnptr->tank.limit,lnptr->tank.chanIbar,lnptr->tank.chanMani);
 
  return;
}

/**************************************************************/
double readRTD (long int adc) {
  long int error=0;
  double involts=0.0, deg=0.0;
  double ohms=0.0, degRTD=273.15, amps=0.004167; //I=V/R=4.92/1080 = 4.555 mA ; 200 - 50 uA
/*
  Read the ADC from the U6
*/

  error = eAIN(hU6, &caliInfo, adc, 0, &involts, LJ_rgBIP1V, 8, 0, 0, 0, 0);
  //  error = eAIN(hU3, &caliInfo, 0, &DAC1Enable, adc, 31, &involts, 0, 0, 0, 0, 0, 0);
  if (error != 0){
    printf("%li - %s\n", error, errormsg[error]);
    closeUSBConnection(hU6);
    return 0;
  }
/*
  Convert the voltage read to Kelvin using a two point linear calibration
  close enough for our purposes .. google pt100 resistance vs temperatur table
*/

    ohms =  involts / amps;
    degRTD = 2.453687 * ohms + 27.781;
    deg = degRTD;  // voltage value in mV

    //    printf("%lf= %lf\n",involts,deg);

  return (deg);
}


/**************************************************************/
int labjackSetup(){
  long int localID=-1;
  long int count=0;
  long int error=0;

/*
  Open first found U6 over USB
*/
  printf("opening usb .... ");

  while (count < 5) {
    if( (hU6 = openUSBConnection(localID)) == NULL){
      count++;
      printf("Opening failed; reseting and attempting %li of 5 more times \n",count);
      printf ("....U6 device reset \n");
      resetU6(0x01);                       // 0x00 = soft reset; 0x01 = reboot
      sleep (2);
      resetU6(0x01);                       // 0x00 = soft reset; 0x01 = reboot
      sleep (2);
      if (count > 5) return 0;
    } 
    else {
      count = 5;
    }
  }

  printf("opened usb\n");
/*
  Get calibration information from U6
*/
  printf("getting calib .... ");
  error = getCalibrationInfo(hU6, &caliInfo);
  if(error != 0){
    printf("\n%li - %s\n",error, errormsg[error]);
    closeUSBConnection(hU6);
    return 0;
  } 
  printf("got calib \n");

  return;
}


/***********************************************************/
void resetU6(uint8 res){
/*
  Resets U3
*/
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

  if( (error = LJUSB_BulkWrite(hU6, U6_PIPE_EP1_OUT, resetIn, 4)) < 4){
    LJUSB_BulkRead(hU6, U6_PIPE_EP2_IN, resetOut, 4); 
    printf("U6 Reset error: %s\n", errormsg[(int)resetOut[3]]);
    closeUSBConnection(hU6);
    return;
  }
  printf ("....U6 device reset \n");
  return;
}
/***********************************************************/


void snmp(int setget, char *cmd, char *cmdResult) {
  //  pid_t wait(int *stat_loc);
  FILE *fp;
  int status;
  char com[150];
  char res[200];
  res[0] = '\0';

/*
   setget flag chooses if this is a 0=read (snmpget) or 1=write (snmpset)
*/
  if (setget == 1) {
    sprintf (com,"snmpset -v 1 -L o -m %s -c private %s ",IBOOTBAR_MIB,IBOOTBAR_IP);
  } 
  else {
    sprintf (com,"snmpget -v 1 -L o -m %s -c public  %s ",IBOOTBAR_MIB,IBOOTBAR_IP);
  }
  strcat(com,cmd);                             // add command to snmp system call
  //  printf("%s\n",com);

  fp = popen(com, "r");   // open child process - call to system; choosing this way

  if (fp == NULL){                             // so that we get the immediate results
    printf("<br/>Could not open to shell!\n");
    strncat(cmdResult,"popen error",39);
    return;
  }

  /* process the results from the child process  */  
    while (fgets(res,200,fp) != NULL){
      fflush(fp);  
    } 

  /* close the child process  */  
  status = pclose(fp);

  if (status == -1) {
    printf("<p>Could not issue snmp command to shell or to close shell!</p>\n");
    strncat(cmdResult,"pclose error",139);
    return;
  } 
  else {
    //     wait();
     strncpy(cmdResult,res,139);   // cutoff results so no array overflows
  }

  return;
}

/*************************************************************/
void updateStatus (int mm, int nn){
  int ii=0;
  char *xxxx;

  xxxx=lnptr->bitstatus;
  ii = nn;
  ii = 6+2*ii;                      // find correct field
  if (mm == 1) *(xxxx+ii) = '1';
  else *(xxxx+ii) = '0';

}
/*************************************************************/
void openTank (){
/*  
  Open tank valve
*/
  int mm=0, set=1, get=0;
  char cmd[150], cmdResult[140];

  while (mm == 0){
    sprintf(cmd,"outletCommand.%i i 1",lnptr->tank.chanIbar);
    snmp(set, cmd, cmdResult);       // set the outlet on tank overflow and tank valve
    mm = readOnOff(cmdResult);
    sleep(5);
    
    sprintf(cmd,"outletStatus.%i",lnptr->tank.chanIbar);
    snmp(get, cmd, cmdResult);       // set the outlet on tank overflow and tank valve
    mm = readOnOff(cmdResult);       // when mm=1 valve opened
    updateStatus(mm,lnptr->tank.chanIbar);
  }

  strcpy(lnptr->tank.status,"OPEN");   

  return;
}

/*************************************************************/
void closeTank (){
/*  
  Close tank valve
*/
  int mm=1, set=1, get=0;
  char cmd[150], cmdResult[140];

  printf (" Tank valve closing \n");   
  while (mm == 1){
    sprintf(cmd,"outletCommand.%i i 0",lnptr->tank.chanIbar);
    snmp(set, cmd, cmdResult);       // set the outlet on tank overflow and tank valve
    mm = readOnOff(cmdResult);
    sleep(5);
    
    sprintf(cmd,"outletStatus.%i",lnptr->tank.chanIbar);
    snmp(get, cmd, cmdResult);       // set the outlet on tank overflow and tank valve
    mm = readOnOff(cmdResult);       // when mm=0 valve closed
    updateStatus(mm,lnptr->tank.chanIbar);

    }
  strcpy(lnptr->tank.status,"CLOSED");   

  //    printf (" Tank valve closed  : mm = %i\n", mm);
    
    return;
}

/*************************************************************/
void openMani (){
/*  
  Open manifold valve
*/
  int mm=0, set=1, get=0;
  char cmd[150], cmdResult[140];
  printf (" Tank mani opening \n");       
  while (mm == 0){
    sprintf(cmd,"outletCommand.%i i 1",lnptr->tank.chanMani);
    snmp(set, cmd, cmdResult);       // set the outlet on tank overflow and tank valve
    mm = readOnOff(cmdResult);
    sleep(5);
        
    sprintf(cmd,"outletStatus.%i",lnptr->tank.chanMani);
    snmp(get, cmd, cmdResult);       // set the outlet on tank overflow and tank valve
    mm = readOnOff(cmdResult);       // when mm=1 manifold valve open
    updateStatus(mm,lnptr->tank.chanMani);
    //        printf (" mm = %i\n", mm);
  }
    //    strcpy(lnptr->tank.status,"Cool/Vent");   
    //    printf (" Manifold valve opened  : mm = %i\n", mm);
    
  return;
}
/*************************************************************/
void closeMani (){
/*  
  Close manifold valve
*/

    int mm=1, set=1, get=0;
    char cmd[150], cmdResult[140];

    printf (" Tank mani closing \n");       
    while (mm == 1){
        sprintf(cmd,"outletCommand.%i i 0",lnptr->tank.chanMani);
        snmp(set, cmd, cmdResult);       // set the outlet on tank overflow and tank valve
        mm = readOnOff(cmdResult);
        sleep(5);
        
        sprintf(cmd,"outletStatus.%i",lnptr->tank.chanMani);
        snmp(get, cmd, cmdResult);       // set the outlet on tank overflow and tank valve
        mm = readOnOff(cmdResult);       // when mm=0 manifold valve closed
	updateStatus(mm,lnptr->tank.chanMani);
	//        printf (" mm = %i\n", mm);
    }
    //    strcpy(lnptr->tank.status,"Vented");   
    //    printf (" Manifold valve closed  : mm = %i\n", mm);
    
    return;
}
/*************************************************************/
void openDet (int ii){
/*  
  Open detetctor valve
*/

  int mm=0, set=1, get=0;
  char cmd[150], cmdResult[140];
  
  printf (" Detectors opening \n");      
  while (mm == 0){
    sprintf(cmd,"outletCommand.%i i 1",lnptr->ge[ii].chanIbar);
    snmp(set, cmd, cmdResult);       // set the outlet on
    mm = readOnOff(cmdResult);
    sleep(5);

    sprintf(cmd,"outletStatus.%i",lnptr->ge[ii].chanIbar);
    snmp(get, cmd, cmdResult);       // set the outlet on tank overflow and tank valve
    mm = readOnOff(cmdResult);
    //        printf (" mm = %i\n", mm);
    updateStatus(mm,lnptr->ge[ii].chanIbar);
  }
  strcpy(lnptr->ge[ii].status,"FILL");
  
    //    printf (" Detector valve opened  : mm = %i\n", mm);
    
  return;
}
/*************************************************************/
void closeDet (int ii){
/*  
  Close detetctor valve
*/

  int mm=1, set=1, get=0;
  char cmd[150], cmdResult[140];
    
  printf (" Detectors closing \n");  
  while (mm == 1){
    sprintf(cmd,"outletCommand.%i i 0",lnptr->ge[ii].chanIbar);
    snmp(set, cmd, cmdResult);       // set the outlet on
    mm = readOnOff(cmdResult);
    sleep(5);

    sprintf(cmd,"outletStatus.%i",lnptr->ge[ii].chanIbar);
    snmp(get, cmd, cmdResult);       // set the outlet on tank overflow and tank valve
    mm = readOnOff(cmdResult);
    updateStatus(mm,lnptr->ge[ii].chanIbar);
	//        printf (" mm = %i\n", mm);
  }
  strcpy(lnptr->ge[ii].status,"OK");
    
    //    printf (" Detector valve %i closed  : mm = %i\n", ii,mm);
    
  return;
}

/*************************************************************/
void fillGe (){
/*
     fill all the detctors
*/
  int ii=0, jj=0, kk=0, mm=0, init=0;
  int active[6], activemax=0, actkk[6];
  time_t xtime=0;

    /*
    printf("Block signals attempt\n");
    sigemptyset(&mask);
    sigaddset (&mask,SIGALRM);

    if (sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0) {
      perror ("sigprocmask");
      return;
    }
    */
  if (lnptr->command == 9) {   // initial fill set next fill to 60 minutes
    init=1;
    lnptr->command = 8;         // change to fillGe command
  }
  for (ii=0;ii<6;ii++){
    active[ii]=0;
    actkk[ii]=0;    
  }
  activemax = 0;
  if (lnptr->com1 == -1){
    strcpy(lnptr->comStatus,"FILLING ALL");              // all detector fill
    jj = 0;
    for (ii=0; ii<20; ii++){
      if (lnptr->ge[ii].onoff == 1){
	active[jj++] = ii;
	activemax++;
      }
    }
  }
  else {
    sprintf(lnptr->comStatus,"FILLING Ge %i",lnptr->com1);     // single detector fill
    activemax = 1;
    active[0] = lnptr->com1;
    lnptr->com1-1;
  }
  
  //    printf("Activemax = %i\n",activemax);
  if (activemax==0) return;
  
  openTank();
  openMani();
  updateRTD();                                    // read RTDs
  
  kk = 0;
  xtime = time(NULL) + lnptr->tank.timeout;
  //    while (time[NULL] <= xtime) {    // while counter until timeout occurs
  while (kk < 4){                                 // while counter of number of times checked            
    updateRTD();                                // read RTDs
    lnptr->tank.olimit = 80;                       // set it low to pass the test
    if(lnptr->tank.rtd < lnptr->tank.olimit){    // see if below limit
      kk++;                                     // increment counter
      if (kk > 3){                              // if cold on 4 consecutive reads its filled 
	closeMani();                            // close detector
	break;        //break out of while loop?
      }
    }
    else {
      kk=0;                                     // reset counter to 0...false readings occur from splashes of LN
    }
    printf("Mani test kk = %i\n",kk);
    sleep(5);
    if (lnptr->command != 8) {
      strcpy(lnptr->comStatus,"FILL ALL INTERRUPTED");
      return;     // another interupt signal was sent ...go to users needs to manual get things right
    }
  }
  
  //    closeMani();
/*
   Fill detector(s)
*/

  for (ii=0; ii<activemax; ii++){
    openDet(active[ii]);
    if (lnptr->command != 8) {
      strcpy(lnptr->comStatus,"FILL ALL INTERRUPTED");
      return;     // another interupt signal was sent ...go to users needs to manual get things right
    }  
  }
  
  //    updateRTD();
  
  xtime = time(NULL);
  mm = activemax;
  for (ii=0; ii<activemax; ii++){
    actkk[ii] = 0;                               // set up counter for times manifold is cold enough (suppresses splashes)
  }
  
  while (mm > 0){
    updateRTD();                                 // read RTDs
    /*
      lnptr->ge[active[0]].oflo = 80;              // fake rtd values
      lnptr->ge[active[1]].oflo = 80;
      lnptr->ge[active[2]].oflo = 80;
      lnptr->ge[active[3]].oflo = 80;
      lnptr->ge[active[4]].oflo = 80;
      lnptr->ge[active[5]].oflo = 80;
    */
    for (ii=0; ii<activemax; ii++){                 // go thru active detectors
      curtime = time(NULL);
      if (actkk[ii] != 9){
	if (actkk[ii] <= 4) {       // set to 9 when detector finished filling
	  if(lnptr->ge[active[ii]].oflo < lnptr->ge[active[ii]].olimit){   // check if oflo cold
	    actkk[ii] = actkk[ii] + 1;            // record that oflo was colder
	    if (actkk[ii] > 3) {                  // read overflo values 4 times to be sure they are cold
	      mm--;                               // reduce the while count by 1
	      closeDet(active[ii]);               // close the detector that is filled
	      actkk[ii] = 9;                      // set the counter high so it doesn't get reset to 0
	      strcpy(lnptr->ge[active[ii]].status, "OK");                              // Set detector status OK
	      lnptr->ge[active[ii]].last = curtime - xtime;                            // record fill duration
	      if (init == 0) lnptr->ge[active[ii]].next = curtime + lnptr->ge[active[ii]].interval; // set next filling time to conf setting
	      else lnptr->ge[active[ii]].next = curtime + 3600;   // set next filling time to 1 hour
	    }                                     // closeDet has a sleep period - one path thru loop
	    else {
	      sleep (5);                          // sleep until next rtd check - one path thru loop
	    }
	  }
	  else {                                
	    actkk[ii] = 0;                        // reset the counter -- not a full LN stream
	    sleep (5);                            // sleep until next rtd check - one path thru loop
	  }
	}
	if (curtime > (xtime + lnptr->ge[ii].max)) {               // check for TOO LONG fill
	  mm--;                                                    // reduce the while count by 1
	  closeDet(active[ii]);                                    // close the detector that is filled
	  actkk[ii] = 9;                                           // set the counter high so it doesn't get reset to 0
	  if (init == 0) lnptr->ge[active[ii]].next = curtime + lnptr->ge[active[ii]].interval; // set next filling time to conf setting
	  else lnptr->ge[active[ii]].next = curtime + 3600;        // set next filling time to 1 hour
	  strcpy(lnptr->ge[active[ii]].status, "LONG");            // this also has a sleep so 
	}
      }
    }

    for (ii=0; ii<activemax; ii++){                             // go thru active detectors send email if any took too long
      if (strcmp(lnptr->ge[ii].status,"LONG") == 0) sendEmail;
    }
/*
    Filling has finished....
*/    
    if (lnptr->command != 8) {
      strcpy(lnptr->comStatus,"FILL ALL INTERRUPTED");
      return;     // another interupt signal was sent ...go to users needs to manual get things right
    }
  }  // end of while statement
  
  for (ii=0; ii<activemax; ii++){   // reset all the variables
    actkk[ii] =0;
  }
  mm=0;
  activemax=0;
  init = 0;
  
  closeTank();
  openMani();
/*

*/
  return;
}


/*************************************************************/
void closeAllValves (){
  int ii=0;

  closeTank();
  openMani();

  for (ii=0; ii<20; ii++){
    if (lnptr->ge[ii].onoff == 1) closeDet(ii); 
  }


  return;
}

/*************************************************************/
void outletStatus (){
  int ii=0, kk=0, mm=0, get=0;
  char cmd[150], cmdResult[140];
  char cc[24]="\0";


  sprintf(cc,"outl: ");

  for (ii=0; ii<8; ii++){
    sprintf(cmd,"outletStatus.%i",ii);
    snmp(get, cmd, cmdResult);       // set the outlet on tank overflow and tank valve
    mm = readOnOff(cmdResult);       // when mm=0 manifold valve closed

    if (mm == 0) strcat(cc,"0 ");
    else strcat(cc,"1 ");
  }

  strcpy(lnptr->bitstatus,cc);

  return;
}

/******************************************************************************/

float readFloat(char *cmdResult) {
  int ii,kk;
  char *jj;             //pointer 
  float zz;
  char ss[140], uu[130];

  ii = strlen(cmdResult);
  jj = strstr(cmdResult,"Float:"); 
  strcpy(ss,(jj+7));
  kk = strlen (ss);
  if (kk > 2) zz=strtod(ss,&jj);            //mm = sscanf(ss,"%f %s", zz, uu);

  return (zz);
}

/******************************************************************************/

unsigned int readBits(char *cmdResult) {
  int ii,kk, a, b, c, d;
  char *jj;             //pointer 
  char ss[140];
  unsigned int ff;
  char sa[2]="\0", sb[2]="\0", sc[2]="\0", sd[2]="\0";

  ii = strlen(cmdResult);
  jj = strstr(cmdResult,"BITS:"); 
  strcpy(ss,(jj+6));

  snprintf (sa,1,"%c",ss[0]); 
  snprintf (sb,1,"%c",ss[1]); 
  snprintf (sc,1,"%c",ss[3]); 
  snprintf (sd,1,"%c",ss[4]); 
  
  a = atoi(sa);
  b = atoi(sb);
  c = atoi(sc);
  d = atoi(sd);
  /*
  */
  ff = a*4096 + b*256 + c*16 + d;

  return (ff);
}

/******************************************************************************/

int readInt(char *cmdResult) {
  int ii,kk;
  char *jj;             //pointer 
  char ss[140];
  int nn;

  ii = strlen(cmdResult);
  jj = strstr(cmdResult,"INTEGER:"); 
  strcpy(ss,(jj+9));
  kk = strlen (ss);
  nn=strtol(ss,&jj,10);            //mm = sscanf(ss,"%f %s", zz, uu);
  //printf("this int: %i %i =%s=  v==>%i =%s\n",ii,kk,ss,nn);
  return (nn);
}
/******************************************************************************/

int readOnOff(char *cmdResult) {
  long int ii,kk;
  char *jj, *gg;             //pointer 
  char ss[140];
  int nn;

  ii = strlen(cmdResult);
  jj = strstr(cmdResult,"("); 
  gg = strstr(cmdResult,")"); 
  strcpy(ss,(jj+1));

  ss[gg-jj-1]='\0';     // 0, 1 on/off; 10 = clear flags

  nn=atoi(ss);

  return (nn);
}
/******************************************************************************/
void sendEmail(){
  FILE *fp;
  int status;
  char com[150];
  char res[200];
  int ii=0,jj=0;
  /*
  for (ii=0; ii< 20; ii++){
    //    if (strlen(lnptr->email[ii]) > 4)lnptr->maxAddress++;
    if (strlen(lnptr->ge[maxAddress].email) > 4)lnptr->maxAddress++;
    }
  */
  printf("%i\n",lnptr->maxAddress);
  for (jj=0; jj < lnptr->maxAddress; jj++){
    printf("loop is: %i %i\n",jj,(int)lnptr->maxAddress);
    //    sprintf (com,"mail -s \"Liquid nitrogen filling failure\" %s < include/fill-failed.txt",lnptr->ge[ii].email);
    sprintf (com,"mail -s \"LN\" %s",lnptr->ge[jj].email);
    //                     1         2         3         4         5         6         7         8
    //            12345678901234567890123456789012345678901234567890123456789012345678901234567890 + 40 for email addresses

    printf("%s\n",com);

    fp = popen(com, "r+");   // open child process - call to system; choosing this way
    if (fp == NULL){                             // so that we get the immediate results
      printf("<br/>Could not open to shell!\n");
    //    strncat(cmdResult,"popen error",39);
      return;
    }
    //    fprintf(fp,"Detector status:\n");
    for (ii=0; ii< 20; ii++){
      if (lnptr->ge[ii].onoff == 1){
	fprintf(fp,"%i-%s-%s T/L=%.1f/%.0f\n",ii,lnptr->ge[ii].name,lnptr->ge[ii].status,lnptr->ge[ii].rtd,lnptr->ge[ii].limit);
      }
    }
        fprintf(fp,".");
    fflush(fp);  
  /* process the results from the child process    
    while (fgets(res,200,fp) != NULL){
      printf("sendEmail: %s\n");
      fflush(fp);  
    } 
 */  
  /* close the child process  */  
    status = pclose(fp);
    printf("status = %li\n",status);
  }

  return;
}

/******************************************************************************/

void readEmail() {
  FILE *ifile;
  char line[200]="\0";
  //  char lnfill_mail[200]="/Users/c4g/src/Labjack/LNfill/include/lnfill-email.txt";   //see define statement in lnfill.h 
  int ii=0, mm=0, jj=0, maxAddress=0;
  char ans[40];
/*
   Read configuration file
*/  
  if ( ( ifile = fopen (lnfill_mail,"r+") ) == NULL) {
    printf ("*** File on disk (%s) could not be opened: \n",lnfill_mail);
    printf ("===> %s \n",lnfill_mail);
    exit (EXIT_FAILURE);
  }
/*
 Should be positioned to read file
*/
  lnptr->maxAddress=0;
  while (1) {                   // 1 = true
    fgets(line,150,ifile);
    printf("%s",line);

    if (feof(ifile)) {
      mm = fclose(ifile);
      break;
    }
/*
   A line from the file is read above and processed below
*/
//    sprintf(lnptr->email[0],"Hi Carl");
    mm = sscanf (line,"%s",ans);
    jj = strlen(ans);

    if (jj > 3 && strchr(ans,'#') == NULL) {
      strcpy(lnptr->ge[lnptr->maxAddress].email, ans);
      lnptr->maxAddress++;
      printf("%i\n",(int)lnptr->maxAddress);
    }
  }
  return;
}


/******************************************************************************/
//  ii = strlen(cmdResult);
//  jj = strstr(cmdResult,"INTEGER:"); 

