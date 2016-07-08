/*
  Program hvmon to monitor detector High Voltage
  Understands Wiener MPOD and CAEN x527

  To be compiled with:                                           
     gcc -Wall -lpthread -ldl -lm -o hvmon hvmon.mpod.c
*/

#include "../include/hvmon.h"

#define MPOD_MIB     "../include/WIENER-CRATE-MIB.txt"
#define CONFIGFILE   "../include/hvmon.conf"
#define INTERVAL 60  //wake up every 15 seconds 

void readConf();                  // processes configuration file
int mmapSetup();                  // sets up the memory map

//  MPOD SNMP related commands

void snmp(int setget, int ii, char *cmd, char *cmdResult);   // snmp get/set commands
int readOnOff(char *cmdResult);                              // snmp get/set results (on/off)
int readInt(char *cmdResult);                                // snmp get/set results (integer)
float readFloat(char *cmdResult);                            // snmp get/set results (floats)
unsigned int readBits(char *cmdResult);                      // snmp get/set results (bits)

//  CAENHVWrapper related commands

void setTimer();                  // sets the timer for signals
void alarm_wakeup();
struct itimerval tout_val;        // needed the timer for signals
void handlerCommand(int sig, siginfo_t *siginfo, void *context);  // main handler for signal notification and decisions
void signalBlock(int pp);
sigset_t mask;
sigset_t orig_mask;
struct sigaction act;

char         path [PATH_MAX+1];                  // PATH_MAX defined in a system libgen.h file
char     mpod_mib [PATH_MAX+100]="\0";
char ibootbar_mib [PATH_MAX+100]="\0";

void getHVmpod();
void getHVmpodChan(int ii);
void setHVmpod(int ii);
void setVolts(int ii);
void setRampUp(int ii);
void setCurrent(int ii);
void setOnOff(int ii);
void getTemp();
void changeParam();
int scan2int();
float scan2float();
/*
void getVMon(int ii);
void getIMon(int ii);
void getV0Set(int ii);
void getI0Set(int ii);
void getPw(int ii);
void getRUp(int ii);
void getV1Set(int ii);
void getI1Set(int ii);
void getSVMax(int ii);
void getRDWn(int ii);
void getTrip(int ii);
void getTripInt(int ii);
void getTripExt(int ii);
void getName(int ii);
void setV0Set(int ii);
void setI0Set(int ii);
void setPw(int ii);
void setRUp(int ii);
void setV1Set(int ii);
void setI1Set(int ii);
void setSVMax(int ii);
void setRDWn(int ii);
void setTrip(int ii);
void setTripInt(int ii);
void setTripExt(int ii);
*/
time_t time0=0, time1=0, nread;
int indexMax=0;

/***********************************************************/
int main(int argc, char **argv){
/*

*/
  //int ii=0, result=0;
  //  long int p0=0, p1=1, count=0, etime=0;
  int mapHVmon;
  pid_t pid;
  long int p0=0,p1=1;
  
  printf("Working directory: %s\n",getcwd(path,PATH_MAX+1));
  strcpy(mpod_mib,path);                  // copy path to mpod mib file
  strcat(mpod_mib,"/");                   // add back the ending directory slash
  strcat(mpod_mib,MPOD_MIB);              // tack on the mib file location
  printf("    mpod_mib file: %s\n",mpod_mib);
/*
  Memory map creation and attachment
  the segment number is stored in hvptr->pid
*/
  mapHVmon = mmapSetup();
  if (mapHVmon == -1) return 0;
  /*  
   Set up the signal capture routine for when the reading
   program wants something changed
*/
  pid = getpid();    // this gets process number of this program
  hvptr->pid= pid;   // this is the pid number for the SIGALRM signals

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
  readConf();
/*  
  Setup time of next fill based on current time and configure file
*/
  time0 = time(NULL);            // record starting time of program
  hvptr->time0=time0;
/*  
  Get the first read of the HV data
*/
  hvptr->com0 = 2;
  //getHVmpod();
  //return 0;
  //getTemp();
/*  
  Setup monitoring loop to look for changes/minute and requests   
*/
  nread = INTERVAL;
  hvptr->com0 = 2;

  while(hvptr->com0 != -1) {
    /*    for (ii=0; ii<indexMax; ii++){
          if (hvptr->xx[ii].type == 0) {   
		printf ("mon  MPOD  %i \n",ii);*/

    switch ( hvptr->com0 ) { 
    
    case  1:              // do nothing as calling program will use what is in shared memory
      hvptr->com0 = 0;    // set comand back to regular reading
      break;

    case  2:                   // do a regular or forced read...curtime is reset
      hvptr->secRunning = time(NULL) - hvptr->time0;
      signalBlock(p1);
      getHVmpod(); 
      getTemp();
      signalBlock(p0);
      hvptr->com0 = 0;    // set comand to regular reading or else we lose touch with the CAEN module
/*
      for (ii=0; ii<indexMax; ii++){
	printf("i = %i, VSet = %f,  Vred = %f, Vramp = %f, Ired = %f Imax = %f OnOFF = %i\n",ii,
	       hvptr->xx[ii].vSet, hvptr->xx[ii].vMeas, hvptr->xx[ii].vRamp, hvptr->xx[ii].iMeas,
	       hvptr->xx[ii].iSet,hvptr->xx[ii].onoff);
    }
*/
      break;
    case 3:
      signalBlock(p1);
      getTemp();
      signalBlock(p0);
      hvptr->com0 = 2;    // test if necessary
      break;
    case 4:                 // HV On all
      signalBlock(p1);
      setHVmpod(1);
      signalBlock(p0);
      hvptr->com0 = 2;    // set comand to regular reading. Test if necessary
      break;
    case 5:                // HV Off All
      signalBlock(p1);
      setHVmpod(0);
      signalBlock(p0);
      hvptr->com0 = 2;   
      break;
    case 16:
      printf(" Changing parameters of one detector....\n");
      signalBlock(p1);
      changeParam();
      signalBlock(p0);
      hvptr->com0 = 2;    // set comand to regular reading. Test if necessary
     break;
      
    default:
      hvptr->com0 = 2;
      sleep (INTERVAL);

      break;
    }  // end of switch statement    hvptr->com0 = -1;
  }  // end of while statement

  
/*
   Release the shared memory and close the U3

*/
  if (munmap(hvptr, sizeof (struct hvmon*)) == -1) {
    perror("Error un-mmapping the file");
/* Decide here whether to close(fd) and exit() or not. Depends... */
  }
  close(mapHVmon);

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

  printf ("I caught signal number %i ! \n",hvptr->com0);

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
   fd = open(HVMONDATAPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0644);
   if (fd == -1) {
        perror("Error opening hvmon file for writing");
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
   for (ii=0; ii<HVMONDATASIZE; ii++){
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
   hvptr = (struct hvmon*) mmap(0, HVMONDATASIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
   if (hvptr == MAP_FAILED) {
        close(fd);
        perror("Error mmapping the hvmon file");
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

/******************************************************************************/
void readConf() {
  FILE *ifile;
  char line[200]="\0";
  char hvmon_conf[200] ="\0";
  int ii=0, mm=0, slot=0, chan=0, onoff=0;
//int itrip=0, etrip=0;
  float volts=0.0, current=0.0, ramp=0.0;
//float trip=0.0, dramp=0.0, svmax=0.0, v1set=0.0, i1set=0.0;
  char ip[30]="\0", name[15]="\0";

  //char ipsave[10][30];
  //int kk=0,new=0;
    
  strcpy(hvmon_conf,path);
  strcat(hvmon_conf,"/");
  strcat(hvmon_conf,CONFIGFILE);
  printf("    mpod_mib file: %s\n",hvmon_conf);

/*
   Read configuration file
*/  
  if ( ( ifile = fopen (hvmon_conf,"r+") ) == NULL) {
    printf ("*** File on disk (%s) could not be opened: \n",hvmon_conf);
    printf ("===> %s \n",hvmon_conf);
    exit (EXIT_FAILURE);
  }

  fgets(line,200,ifile);    // reads column headers
  fgets(line,200,ifile);    // reads column headers units
  fgets(line,200,ifile);    // reads ----
/*
 Should be positioned to read file
*/
  indexMax = 0;
  while (1) {                   // 1 = true
    fgets(line,200,ifile);
    if (feof(ifile)) {
      mm = fclose(ifile);
      if (mm != 0) printf("File not closed\n");
      break;
    }
/*
   A line from the file is read above and processed below
*/
    mm = sscanf (line,"%i", &ii);  // read ip and type to determine CAEN or MPOD
    printf (" type = %i\n", ii);
    onoff = -1;
    chan = -1;
    slot = -1;
    volts = 0.0;
    ramp = 0.0;
    current = 0.0;
    strcpy (name,"\0");
    /*dramp = 0.0;
    svmax = 0.0;
    v1set = 0.0;
    i1set = 0.0;
    trip = 0.0;
    itrip = 0;
    etrip = 0;*/
/*
  Process MPOD and other SNMP data
*/    
    if (ii == 0) {
      mm = sscanf (line,"%i %s %u %u %14s %f %f %f %i",&ii, ip, &chan, &slot, name, &volts, &current, &ramp, &onoff);  // MPOD data
      hvptr->xx[indexMax].type = ii;
      hvptr->xx[indexMax].slot = slot;
      hvptr->xx[indexMax].chan = chan;
      strcpy(hvptr->xx[indexMax].name,name);
      strcpy(hvptr->xx[indexMax].ip,ip);

      getHVmpodChan(indexMax);
      if (hvptr->xx[indexMax].vSet != volts)
      {
        hvptr->xx[indexMax].vSet = volts;
        setVolts(indexMax);
      }
      if (hvptr->xx[indexMax].vRamp != ramp)
      {
        hvptr->xx[indexMax].vRamp = ramp;
        setRampUp(indexMax);
      }
      if (hvptr->xx[indexMax].iSet != current)
      {
        hvptr->xx[indexMax].iSet = current;
        setCurrent(indexMax);
      }
      /*if getTempChan(indexMax) > allowed
        sprintf(cmd, "outputSwitch.u%i i %i",chan, 0);
        snmp(1,indexMax,cmd,cmdRes);
        hvptr->xx[indexMax].onOff = 0; 
      else */
      if (hvptr->xx[indexMax].onoff != onoff) 
      {
        hvptr->xx[indexMax].onoff = onoff;
        setOnOff(indexMax);
      }
      /*
      mm = sscanf (line,"%i %s %u     %f", &ii, ip, &chan, &volts);  // MPOD data
      hvptr->xx[indexMax].type = ii;
      hvptr->xx[indexMax].chan = chan;
      hvptr->xx[indexMax].vSet = volts;
      hvptr->xx[indexMax].slot = slot;
      strcpy(hvptr->xx[indexMax].ip,ip);
      */
      printf ("MPOD = %s %3u    %7.1f %i\n", hvptr->xx[indexMax].ip, hvptr->xx[indexMax].chan, hvptr->xx[indexMax].vSet,hvptr->xx[indexMax].type);
    }
/*
    Reached the end of the configuration file data
*/
    if (ii == -1) {
      mm = fclose(ifile);
      if (mm != 0) printf("File not closed\n");
      break;
    }
    
    indexMax++;            // increment indexes 
  }   // end of while statement
  hvptr->maxchan = indexMax;
  printf ("%i HV entries found on MPODs\n",hvptr->maxchan);

  return;
}

/******************************************************************************/
float readFloat(char *cmdResult) {
  int kk;
  char *jj;             //pointer 
  float zz;
  char ss[140];

  jj = strstr(cmdResult,"Float:"); 
  strcpy(ss,(jj+7));
  kk = strlen (ss);
  if (kk > 2) zz=strtod(ss,&jj);            //mm = sscanf(ss,"%f %s", zz, uu);

  return (zz);
}

/******************************************************************************/
int readInt(char *cmdResult) {
  char *jj;             //pointer 
  char ss[140];
  int nn;

  jj = strstr(cmdResult,"INTEGER:"); 
  strcpy(ss,(jj+9));
  nn=strtol(ss,&jj,10);            //mm = sscanf(ss,"%f %s", zz, uu);

  return (nn);
}

/******************************************************************************/
int readOnOff(char *cmdResult) {
  char *jj, *gg;             //pointer 
  char ss[140];
  int nn;

  jj = strstr(cmdResult,"("); 
  gg = strstr(cmdResult,")"); 
  strcpy(ss,(jj+1));

  ss[gg-jj-1]='\0';     // 0, 1 on/off; 10 = clear flags

  nn=atoi(ss);

  return (nn);
}

/******************************************************************************/
void getHVmpod() {
  int ii=0;
  //char cmd[150]="\0", cmdResult[140]="\0";

  //printf (" Getting MPOD data ....");  
  for (ii=0; ii<indexMax; ii++){
    getHVmpodChan(ii);
  }
  printf (".... finished \n");  
  return;
}
/******************************************************************************/
void getHVmpodChan(int ii) {
  int setget=0;
  char cmd[150]="\0", cmdResult[140]="\0";

  printf (" Getting MPOD data ....");  

    if (hvptr->xx[ii].type == 0) {
      sprintf(cmd,"outputVoltage.u%i",hvptr->xx[ii].chan); 
      snmp(setget,ii,cmd,cmdResult);     //mpodGETguru(cmd, cmdResult);     // read the set voltage
      printf(cmdResult);
      hvptr->xx[ii].vSet = readFloat(cmdResult);

      sprintf(cmd,"outputMeasurementSenseVoltage.u%i",hvptr->xx[ii].chan);
      snmp(setget,ii,cmd,cmdResult);     //mpodGETguru(cmd, cmdResult);     // read the measured voltage
      hvptr->xx[ii].vMeas = readFloat(cmdResult);

      sprintf(cmd,"outputVoltageRiseRate.u%i",hvptr->xx[ii].chan);
      snmp(setget,ii,cmd,cmdResult);    //mpodGETguru(cmd, cmdResult);     // read the voltage ramp rate
      hvptr->xx[ii].vRamp = readFloat(cmdResult);

      sprintf(cmd,"outputMeasurementCurrent.u%i",hvptr->xx[ii].chan);
      snmp(setget,ii,cmd,cmdResult);   //mpodGETguru(cmd, cmdResult);     // read the measured current
      hvptr->xx[ii].iMeas = readFloat(cmdResult);

      sprintf(cmd,"outputCurrent.u%i",hvptr->xx[ii].chan);
      snmp(setget,ii,cmd,cmdResult);   //mpodGETguru(cmd, cmdResult);     // read the max current
      hvptr->xx[ii].iSet = readFloat(cmdResult);

      sprintf(cmd,"outputSwitch.u%i",hvptr->xx[ii].chan);
      snmp(setget,ii,cmd,cmdResult);   //mpodGETguru(cmd, cmdResult);     // read the max current
      hvptr->xx[ii].onoff = readOnOff(cmdResult);

    }
  printf("current settings are: V=%f, Vm=%f, Vup=%f, I=%f, Im=%f, 1/0=%i",
          hvptr->xx[ii].vSet, hvptr->xx[ii].vMeas,hvptr->xx[ii].vRamp, 
          hvptr->xx[ii].iSet, hvptr->xx[ii].iMeas, hvptr->xx[ii].onoff);
  //printf (".... finished \n");  
  return;
}
/******************************************************************************/
void setHVmpod(int nf) {
  int ii=0;
  //char cmd[150]="\0", cmdResult[140]="\0";

  printf (" Getting MPOD data ....");  
  for (ii=0; ii<indexMax; ii++){

    if (hvptr->xx[ii].type == 0) {
      hvptr->xx[ii].onoff = nf;      
      setOnOff(ii);    //mpodGETguru(cmd, cmdResult);     // read the set voltage
    }
  }
  printf (".... finished \n");  
  return;
}
/******************************************************************************/
void snmp(int setget, int ii, char *cmd, char *cmdResult) {
  //  pid_t wait(int *stat_loc);
  FILE *fp;
  int status;
  char com[150];
  char res[200];
  res[0] = '\0';

/*
   setget flag chooses if this is a 
    0=read (snmpget) or
    1=write (snmpset) or 
    2-shutdown with MPOD MIB and IP from HV control
    
    snmpset -OqvU -v 2c -M /usr/share/snmp/mibs -m +WIENER-CRATE-MIB -c guru 192.168.13.231 outputSwitch.u7 i 0
*/
  if (setget == 1) {
     //printf("ok.");
    //    sprintf (com,"snmpset -v 2c -L o -m %s -c guru %s ",MPOD_MIB,MPOD_IP);  //guru (MPOD)-private-public (iBOOTBAR); versions 1 (iBOOTBAR) and 2c (MPOD)
    //sprintf (com,"snmpset -v 2c -L o -m %s -c guru %s ",MPOD_MIB,hvptr->xx[ii].ip);  //guru (MPOD)-private-public (iBOOTBAR); versions 1 (iBOOTBAR) and 2c (MPOD)
    sprintf (com,"snmpset -v 2c -M /usr/share/snmp/mibs -m +%s -c guru %s ",MPOD_MIB,hvptr->xx[ii].ip);  //guru (MPOD)-private-public (iBOOTBAR); versions 1 (iBOOTBAR) and 2c (MPOD)
  } 
  else if (setget == 0){
    sprintf (com,"snmpget -v 2c -M /usr/share/snmp/mibs -m %s -c guru %s ",MPOD_MIB,hvptr->xx[ii].ip);
    //sprintf (com,"snmpget -v 2c -L o -m %s -c guru  %s ",MPOD_MIB,hvptr->xx[ii].ip);
    //    sprintf (com,"snmpget -v 2c -L o -m %s -c guru  %s ",MPOD_MIB,MPOD_IP);
  }
  else if (setget == 2){              // if setget =2 then issue HV shutdown
    sprintf (com,"snmpset -OqvU -v 2c -M /usr/share/snmp/mibs -m +%s -c guru %s i 0",MPOD_MIB,hvptr->xx[ii].ip);
  }
  strcat(com,cmd);                             // add command to snmp system call
  //  printf("%s\n",com);

  fp = popen(com, "r");   // open child process - call to system; choosing this way

  if (fp == NULL){                             // so that we get the immediate results
    printf("<br/>Could not open to shell!\n");
    strncat(cmdResult,"popen error",39);
    return;
  }

  // process the results from the child process    
    while (fgets(res,200,fp) != NULL){
      fflush(fp);  
    } 

  // close the child process    
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

/******************************************************************************/
/*NEEDS REDONE TWO FUNCS BELOW GETTEMP -> get temps from kelvin and change parms 
needs to communicate to MPOD
*******************************************************************************/
void getTemp(){
  /* Should talk to either log file or binary
     with checks that it updates... */
  return;
}

/******************************************************************************/
void changeParam(){
  int ii = 0, ans;
  /*
 control variables:
  com0 = 16        selects this function
  com1 = detector  selects detector to change
  com2 = parameter selects the parameter to change.....
  hvptr->xx[hvptr->com1].parameter contains the new parameter
   */

  ii = hvptr->com1;
  switch (hvptr->com2){

  case 1:
    if (hvptr->xx[ii].vSet == hvptr->xcom3)
    {  return; 
    } else 
    {
      hvptr->xx[ii].vSet = hvptr->xcom3;
      setVolts(ii);  
    }
    hvptr->com2=20;

  case 2:
    if (hvptr->xx[ii].vSet == hvptr->xcom3)
    {  return; 
    } else 
    {
     hvptr->xx[ii].iSet = hvptr->xcom3;
     setCurrent(ii);
    }
    hvptr->com2=20;

  case 3:
    if (hvptr->xx[ii].onoff == hvptr->xcom3)
    {  return; 
    } else 
    {
     hvptr->xx[ii].onoff = hvptr->xcom3;
     setOnOff(ii);
    }
    hvptr->com2=20;

  case 4:
    if (hvptr->xx[ii].vRamp == hvptr->xcom3)
    {  return; 
    } else 
    {
     hvptr->xx[ii].vRamp = hvptr->xcom3;
     setRampUp(ii);
    }
    hvptr->com2=20;

  case 5:
    //setV1Set(ii);
    hvptr->com2=20;

  case 6:
    //setI1Set(ii);
    hvptr->com2=20;

  case 7:
    //setSVMax(ii);
    hvptr->com2=20;

  case 8:
    //setRDWn(ii);
    hvptr->com2=20;

  case 9:
    //setTrip(ii);
    hvptr->com2=20;

  case 10:
    //setTripInt(ii);
    hvptr->com2=20;

   case 11:
    //setTripExt(ii);
    hvptr->com2=20;
  case 20:
    printf("Are you finished with this detector? (1=yes, 0=no)");
    ans = scan2int();
    if (ans == 1) 
    {
      break;
    } else if (ans == 0) 
    {
      printf ("Which detector number to change ?\n");
      ans = scan2int ();
      hvptr->com2= ans;
     } else 
     {
       printf("N/A");
       break;
     }
  default:
    break;
  }
 

  return;
}
//******************************************************************/
int scan2int () {
  char sss[200];
  long int ii=0;
  
  scanf("%s",sss);
  ii=strtol(sss,NULL,10);                            // convert to base 10
  if ((ii == 0) && (strcmp(sss,"0") != 0)) ii = -1;  // check if 0 result is 0 or because input is not number
  return ((int) ii);
}

/**************************************************************/
float scan2float () {
  char sss[200];
  float ff=0;
  
  scanf("%s",sss);
  ff=atof(sss);                            // convert to base 10
  return (ff);
}
/**************************************************************/
void setVolts(int ii) {
  char cmd[140]="\0", cmdRes[140]="\0";
  sprintf(cmd, "outputVoltage.u%i F %f",  hvptr->xx[ii].chan,hvptr->xx[ii].vSet);
  snmp(1,indexMax,cmd,cmdRes);   
}
/**************************************************************/ 
void setRampUp(int ii) {
  char cmd[140]="\0", cmdRes[140]="\0";
  sprintf(cmd, "outputVoltageRiseRate.u%i F %f", hvptr->xx[ii].chan,hvptr->xx[ii].vRamp);
  snmp(1,indexMax,cmd,cmdRes);    
}      
/**************************************************************/
void setCurrent(int ii) {
  char cmd[140]="\0", cmdRes[140]="\0";
  sprintf(cmd, "outputCurrent.u%i F %f",hvptr->xx[ii].chan,hvptr->xx[ii].iSet);
  snmp(1,indexMax,cmd,cmdRes);  
}
/**************************************************************/
void setOnOff(int ii) {
  char cmd[140]="\0", cmdRes[140]="\0";
      /*if getTempChan(indexMax) > allowed
        sprintf(cmd, "outputSwitch.u%i i %i",chan, 0);
        snmp(1,indexMax,cmd,cmdRes);
        hvptr->xx[indexMax].onOff = 0; 
      else */
  sprintf(cmd, "outputSwitch.u%i i %i",hvptr->xx[ii].chan,hvptr->xx[ii].onoff);
  snmp(1,indexMax,cmd,cmdRes);
}
/**************************************************************/
