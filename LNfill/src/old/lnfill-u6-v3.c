/*
  Program lnfill-u6 to handle LN filling and monitoring of Ge
  when RMS system is not available.

  Will put relevant data into shared memory where other programs
  can access it and decide actions such as turning off HV.

  Program will be based on LabJack U6 USB daq, dataport probe
  power strip with remote relay control.

*/

#include "lnfill.h"
#include "u6.h"
#include "labjackusb.h"

#define INTERVAL 60
#define IBOOTBAR_IP "192.168.0.254"  // 
//#define IBOOTBAR_IP "192.168.13.xxx"  // 
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

HANDLE hU6;                       // LabJack stuff
u6CalibrationInfo caliInfo;       // LabJack stuff

void updateRTD();                 // read RTDs and store values into shared memory

void snmp(int setget, char *cmd, char *cmdResult);   // snmp get/set commands
int readOnOff(char *cmdResult);                      // snmp get/set results (on/off)
int readInt(char *cmdResult);                        // snmp get/set results (integer)
float readFloat(char *cmdResult);                    // snmp get/set results (floats)
unsigned int readBits(char *cmdResult);              // snmp get/set results (bits)
 
void fillAll ();



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

/***********************************************************/
int main(int argc, char **argv){
/*
  key_t shmKey;
  int shmid;

  time_t curtime = -1;
  char xtime[40]="\0";
  long int size=4194304; //   = 4 MB // 131072;  //65536;
*/
  long int ii=0;
  long int adc=0;
  pid_t pid;
  int xx=0;

  struct sigaction act;

/*
  Set up LabJack U6
*/
  labjackSetup();

/*
  Shared memory creation and attachment
*/

  shmSetup();

/*  
   Get the process number for interupts when the two programs need to interact
*/
  pid = getpid();
  lnptr->pid = pid;
  printf("pid = %li \n",lnptr->pid);
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
   Read setup file on disk and load into shared memory
*/
  printf("Read conf file...\n");
  readConf();
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

/*  
  Setup monitoring loop to look for changes/minute and requests   
*/
  nread = INTERVAL;
  lnptr->command = 0;
  while(lnptr->command != -1) {
    curtime = time(NULL);               // check what time it is 
    if (curtime >= time1) {             // read RTDs every INTERVAL (usually ~60 s)
      //      time1 = time1+INTERVAL;
      time1 = time1+nread;              // read RTDs every nread (usually INTERVAL though during a fill it might be more often)
      updateRTD();
    }
/*
   Check if an RTD is too high
*/
    for (ii=0;ii<20;ii++){                               // run thru each detector possibility
      if (lnptr->ge[ii].onoff == 1) {                    // run thru each detector that is ON
	if (lnptr->ge[ii].rtd > lnptr->ge[ii].limit){    // check rtd is within limit
	  printf("SHUT DOWN !\n");
	  strcpy(lnptr->ge[ii].status,"ALARM");
	}
	if (curtime > lnptr->ge[ii].next){               // check if its time to fill a detector
	  printf("start a fill !\n");
	} 
      }
    }

    //  AFTER THIS LOOP CHECK FOR COMMANDS COMING IN FROM CONTROL PROGRAM

      /*    
    switch (lnptr->command) {
    case 0:
      //      signal(SIGTERM,alarm_wakeup);   // set the Alarm signal capture 
      sleep (1);                      // pause for time
      curtime = time(NULL);
      printf ("curtime = %li %li \n", curtime, (time1-curtime));
      if (curtime >= time1) {
	time1 = time1+INTERVAL;
	updateRTD();
      }
      break;
    case 1:
      printf ("1 selected\n");
	break;
    case 2:
      printf ("2 selected\n");
	break;
    default:
      printf ("default selected\n");
      lnptr->command=-1;
	break;
    }
      */    
    sleep (59);
  }

/*
  Alarm processing
*/

/*  
   
*/

/*
   Print out some of shared memory to show it has been accessed

  ln.ge[0].rtd = 80.2;

  for (ii=0; ii< 20; ii++){
    ln.ge[ii].rtd = ln.ge[0].rtd + (double) ii;
    lnptr->ge[ii].rtd = ln.ge[ii].rtd;
    printf ("rtd[%li] = %.2lf = %.2lf\n", ii, ln.ge[ii].rtd,lnptr->ge[ii].rtd);

    sleep (1);
  }
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
void updateRTD(){
  long int ii=0;
/*
  read only active RTDs and tank RTD and Pressure
*/
  for (ii=0; ii<20; ii++){
    if (lnptr->ge[ii].onoff == 1){
      lnptr->ge[ii].rtd = readRTD(lnptr->ge[ii].chanRTD);
      lnptr->ge[ii].oflo = readRTD(lnptr->ge[ii].chanOFLO);
      printf("Active det: %li ->  %lf   %lf\n", ii,lnptr->ge[ii].rtd,lnptr->ge[ii].oflo );
      if (lnptr->ge[ii].rtd >= lnptr->ge[ii].limit) strcpy(lnptr->ge[ii].status,"HIGH");
      if (lnptr->ge[ii].rtd <= 70) strcpy(lnptr->ge[ii].status,"NO RTD");
    }
  }
  lnptr->tank.rtd = readRTD(lnptr->tank.chanRTD);
  lnptr->tank.pressure = readRTD(lnptr->tank.chanPRES);

  printf("TANK: %lf\n", lnptr->tank.rtd);
return;
}

/***********************************************************/
void handlerCommand(int sig, siginfo_t *siginfo, void *context){


  printf ("I caught signal number %li ! \n",lnptr->command);
  if (lnptr->command == 7) {
    nread = INTERVAL/12;
    tout_val.it_value.tv_sec = nread    ;  // set timer for "INTERVAL (1) seconds at top of file
    //    setitimer(ITIMER_REAL, &tout_val,0);   // start the timer
    //    lnptr->command = 0;
  }
  if (lnptr->command == 1) {
    tout_val.it_value.tv_sec = INTERVAL;   // set timer for "INTERVAL (1) seconds at top of file
    //    setitimer(ITIMER_REAL, &tout_val,0);   // start the timer
    lnptr->command = 0;
    nread = INTERVAL;
  }
  if (lnptr->command == 8) {
    lnptr->command = 8;
    fillAll();

    nread = INTERVAL/12;
    tout_val.it_value.tv_sec = nread;       // set timer for "INTERVAL (1) seconds at top of file
    //    setitimer(ITIMER_REAL, &tout_val,0);    // start the timer
  }

  //  char cmd[150],  cmdResult[140];

  //  lnptr->command=0;

  /*
  switch (lnptr->command) {
  case 0:
    //    signal(SIGALRM,alarm_wakeup);   // set the Alarm signal capture 
    sleep (5);                      // pause for time
    curtime = time(NULL);
    printf ("curtime = %li, time0 = %li\n",curtime,time0);

    break;
  case 1:
    printf ("1 selected\n");
    break;
  case 2:
    printf ("2 selected\n");
    break;
  default:
    printf ("default selected\n");
    lnptr->command=-1;
    break;
  }
  */
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

  printf("Setting up shared memory...\n");///Users/c4g/src/LNfill/include/lnfill.conf
  shmKey = ftok("/Users/c4g/src/LNfill/include/lnfill.conf",'b');       // key unique identifier for shared memory, other programs use 'LN' tag
  //  shmKey = ftok("SHM_PATH",'b');                                    // key unique identifier for shared memory, other programs use 'LN' tag
  shmid = shmget(shmKey, sizeof (struct lnfill), 0666 | IPC_CREAT);     // gets ID of shared memory, size, permissions, create if necessary
  lnptr = shmat (shmid, (void *)0, 0);                                  // now link to area so it can be used; struct lnfill pointer char *lnptr
  if (lnptr == (struct lnfill *)(-1)){                                  // check for errors
    perror("shmat");
    exit;
  }
  printf ("shm size = %li\n",sizeof (struct lnfill) );

  printf("... set shared memory...\n");

  return;
}

/**************************************************************/

void readConf() {
  FILE *ifile;
  char line[200]="\0";
  char lnfill_conf[200]="/Users/c4g/src/LNfill/include/lnfill.conf";   //see define statement in lnfill.h 
  long int ii=0, mm=0;
  int onoff=0, chanRTD=0, chanOFLO=0, Ibar;
  char name[10]="\0";
  double interval=0.0, max=0.0, min=0.0, limit=0.0;

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
    mm = sscanf (line,"%li %s %i %lf %lf %lf %i %i %lf %i", &ii, name, &onoff, &interval, &max, &min, &chanRTD, &chanOFLO, &limit, &Ibar);
//    printf("\n %li %s %i %lf %lf %lf %i %i %lf \n", ii, name, onoff, interval, max, min, chanRTD, chanOFLO, limit);
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
      lnptr->ge[ii-1].chanIbar = Ibar;
    } else {
      lnptr->tank.chanRTD = chanRTD;
      lnptr->tank.chanPRES = chanOFLO;
      lnptr->tank.timeout = max;
      lnptr->tank.limit = limit;
      lnptr->tank.chanIbar = Ibar;
    }
  }
   
  for (ii=0; ii<20; ii++){
    printf("%2li %3s   %i   %0.lf    %0.lf    %0.lf    %4.lf   %i  \n",ii,lnptr->ge[ii].name,lnptr->ge[ii].onoff,lnptr->ge[ii].interval,lnptr->ge[ii].max,lnptr->ge[ii].min,lnptr->ge[ii].limit,lnptr->ge[ii].chanIbar);
  }
 
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
  printf("%s\n",com);

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
void fillAll (){
/*
   fill all the detctors
*/
  int ii=0, jj=0, mm=0, set=1, get=0;
  char cmd[150], cmdResult[140];

/*
   Open tank and cool off the line and manifold
*/

  for (ii=0; ii<2; ii++) {
    jj= ii + lnptr->tank.chanIbar;       // tank overflow and tank valve channels on iBar must be sequential
    mm = 0;
    while (mm == 0){
      sprintf(cmd,"outletCommand.%i i 1",jj);
      snmp(set, cmd, cmdResult);       // set the outlet on tank overflow and tank valve
      mm = readOnOff(cmdResult);
      sleep(5);
      sprintf(cmd,"outletStatus.%i",jj);
      snmp(get, cmd, cmdResult);       // set the outlet on tank overflow and tank valve
      mm = readOnOff(cmdResult);
      printf (" mm = %i\n", mm);
    }
    strcpy(lnptr->tank.status,"COOLING");
  }
  printf (" Valves opened  : mm = %i\n", mm);

/*
   Read the RTDs and when tank.RTD is at LN temp, open detector valves and close the manifold overflow
*/


//  updateRTD();  // this might not be needed if the interupt changes work the way I hope
  mm=0;
  while (lnptr->tank.rtd > lnptr->tank.limit) {
    printf (" tank rtd/limit : %lf = %lf\n", lnptr->tank.rtd, lnptr->tank.limit);
    sleep(INTERVAL/10);
    mm++;
    if (mm > 6){
      printf("Faking tank limit \n");
      lnptr->tank.rtd = 80;    //    updateRTD();       // this might not be needed
    }
  }
/*
   Close tank overflow 
*/
  jj = 1 + lnptr->tank.chanIbar;
  mm = 1;
  printf("Close tank OFLO; tank valve still open \n");
  while (mm == 1){
    sprintf(cmd,"outletCommand.%i i 0", jj);
    snmp(set, cmd, cmdResult);           // set the outlet off on tank overflow 
    mm = readOnOff(cmdResult);
    sleep(5);
    sprintf(cmd,"outletStatus.%i",jj);
    snmp(get, cmd, cmdResult);       // set the outlet on tank overflow and tank valve
    mm = readOnOff(cmdResult);
    printf (" mm = %i\n", mm);
  }
/*
   Open to the detectors
*/

  printf("Open to detectors - tank valve still open \n");

  for (ii=0; ii<20; ii++){
    if (lnptr->ge[ii].onoff == 1){
      mm = 0;
      while (mm == 0){
	sprintf(cmd,"outletCommand.%i i 1",lnptr->ge[ii].chanIbar);
	snmp(set, cmd, cmdResult);       // set the outlet on
	mm = readOnOff(cmdResult);
	sleep(5);
	sprintf(cmd,"outletStatus.%i",lnptr->ge[ii].chanIbar);
	snmp(get, cmd, cmdResult);       // set the outlet on tank overflow and tank valve
	mm = readOnOff(cmdResult);
	printf (" mm = %i\n", mm);
      }
      strcpy(lnptr->ge[ii].status,"FILLING");
    }
  }
/*
   Close to the detectors when OFLOs are cooled
*/
  for (ii=0; ii<20; ii++){
    if (lnptr->ge[ii].onoff == 1){
      lnptr->ge[ii].oflo = lnptr->ge[ii].rtd -5;
      mm = 1;
      while (mm == 1){
	sprintf(cmd,"outletCommand.%i i 0",lnptr->ge[ii].chanIbar);
	snmp(set, cmd, cmdResult);       // set the outlet on
	mm = readOnOff(cmdResult);
	sleep(5);
	sprintf(cmd,"outletStatus.%i",lnptr->ge[ii].chanIbar);
	snmp(get, cmd, cmdResult);       // set the outlet on tank overflow and tank valve
	mm = readOnOff(cmdResult);
	printf (" mm = %i\n", mm);
      }
    }
  }
/*
   After final detector filled, close tank and open tank OFLO valve
*/
    jj= lnptr->tank.chanIbar;        // tank overflow first
    mm = 1;
    while (mm == 1){
      sprintf(cmd,"outletCommand.%i i 0",jj);
      snmp(set, cmd, cmdResult);       // set the tank overflow
      mm = readOnOff(cmdResult);
      sleep(5);
      sprintf(cmd,"outletStatus.%i",jj);
      snmp(get, cmd, cmdResult);       // get the result of the action
      mm = readOnOff(cmdResult);
      printf (" mm = %i\n", mm);
    }
    jj= 1+ lnptr->tank.chanIbar;        // tank valve
    mm = 0;
    while (mm == 0){
      sprintf(cmd,"outletCommand.%i i 1",jj);
      snmp(set, cmd, cmdResult);       // set the outlet on tank overflow and tank valve
      mm = readOnOff(cmdResult);
      sleep(5);
      sprintf(cmd,"outletStatus.%i",jj);
      snmp(get, cmd, cmdResult);       // set the outlet on tank overflow and tank valve
      mm = readOnOff(cmdResult);
      printf (" mm = %i\n", mm);
    }
    lnptr->command = 0;

/*
   count active detectors and count down as RTDs on overflow indicate detector is filled.
*/

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

  sprintf (sa,"%c\0",ss[0]); 
  sprintf (sb,"%c\0",ss[1]); 
  sprintf (sc,"%c\0",ss[3]); 
  sprintf (sd,"%c\0",ss[4]); 
  
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

/******************************************************************************/
/*
float vCheck(int ii, int jj, int kk, float zz){
  float VMAXCENT=1000.;
  float VMAXHEX=1500.;
  char out[2]="O", mid[2]="M", inn[2]="I", cen[2]="C";

  if (crate[ii].slot[jj].chan[kk].name[0]==cen[0]) {
    if (zz > VMAXCENT) zz = VMAXCENT;
  }
  if (crate[ii].slot[jj].chan[kk].name[0]==out[0] ||
      crate[ii].slot[jj].chan[kk].name[0]==mid[0] ||
      crate[ii].slot[jj].chan[kk].name[0]==inn[0]) {
    if (zz > VMAXHEX) zz = VMAXHEX;
  }



  return (zz);
}
*/
