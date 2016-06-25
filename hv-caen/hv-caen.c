/*
  Program hvcaen to monitor detector High Voltage
  Understands Wiener MPOD and CAEN x527

  To be compiled with:                                           
     gcc -Wall -lcaenhvwrapper -lpthread -ldl -lm -o hv-caen hv-caen.c
*/

#include "../include/hv-caen.h"
#include "../include/CAENHVWrapper.h"

#define CONFIGFILE   "../include/hv-caen.conf"
#define INTERVAL 15  //wake up every 15 seconds 

void readConf();                  // processes configuration file
int mmapSetup();                  // sets up the memory map

//  MPOD SNMP related commands

int readOnOff(char *cmdResult);                              // snmp get/set results (on/off)
int readInt(char *cmdResult);                                // snmp get/set results (integer)
float readFloat(char *cmdResult);                            // snmp get/set results (floats)
unsigned int readBits(char *cmdResult);                      // snmp get/set results (bits)
int scan2int();                           // reads in ascii and makes it an int

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
//char ibootbar_mib [PATH_MAX+100]="\0";

void caenGet();
void caenSet();
void caenCrate();
void caenGetBySlot();
void hvOnAll();
void hvOffAll();
void recoverCAEN();
void getTemp();
void changeParam();
void getVMon(int ii);
void getIMon(int ii);
void getV0Set(int ii);
void getI0Set(int ii);
void getPw(int ii);
void getPOn(int ii);
void getPDwn(int ii);
void getStatus(int ii);
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
void setPOn(int ii);
void setPDwn(int ii);
void setRUp(int ii);
void setV1Set(int ii);
void setI1Set(int ii);
void setSVMax(int ii);
void setRDWn(int ii);
void setTrip(int ii);
void setTripInt(int ii);
void setTripExt(int ii);
void setName(int ii);
void setAllParam();
void getAllParam();

time_t time0=0, time1=0, nread;
int indexMax=0;

/***********************************************************/
int main(int argc, char **argv){
/*

*/
  int ii=0, result=0;
  int mapHvcaen;
  pid_t pid;
  long int p0=0,p1=1;

  printf("Working directory: %s\n",getcwd(path,PATH_MAX+1));
/*  
  strcpy(mpod_mib,path);                  // copy path to mpod mib file
  strcat(mpod_mib,"/");                   // add back the ending directory slash
  strcat(mpod_mib,MPOD_MIB);              // tack on the mib file location
  printf("    mpod_mib file: %s\n",mpod_mib);
*/
/*
  Memory map creation and attachment
  the segment number is stored in hvcptr->pid
*/
  mapHvcaen = mmapSetup();
  if (mapHvcaen == -1) return 0;
  /*  
   Set up the signal capture routine for when the reading
   program wants something changed
*/
  pid = getpid();    // this gets process number of this program
  hvcptr->pid= pid;   // this is the pid number for the SIGALRM signals

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
//  readConf();
  readConf();
/*  
  Setup time of next fill based on current time and configure file
*/
  time0 = time(NULL);            // record starting time of program
  hvcptr->time0=time0;
  //  time1 = time0 + INTERVAL;      // set up next read of voltages...usually every minute
/*  
  Get the first read of the HV data
*/
  nread = INTERVAL;
  hvcptr->com0 = 2;

  printf ("1 - Load parameters from configuration file\n");
  printf ("2 - Recover parameters from CAEN crate (only on channels will be recovered)\n");
  printf ("0 - end file \n");
  ii = scan2int();
  if (ii == 1) {    // sets all parameters except HV on or off
    setAllParam();
    getAllParam();  // gets all the parameters
  }
  else if (ii == 2) hvcptr->com0 = 7;
  else hvcptr->com0 =-1;

/*  
  Setup monitoring loop to look for changes/minute and requests   
*/
   while(hvcptr->com0 != -1) {
    switch ( hvcptr->com0 ) { 
    case  1:              // do nothing as calling program will use what is in shared memory
      hvcptr->com0 = 0;    // set comand back to regular reading
      break;

    case  2:                   // do a regular or forced read...curtime is reset
      hvcptr->time1 = time(NULL);
      hvcptr->secRunning = hvcptr->time1 - hvcptr->time0;
      signalBlock(p1);
      caenGet();
      getTemp();
      getAllParam();      // gets all the parameters
      signalBlock(p0);
      hvcptr->com0 = 0;    // set comand to regular reading or else we lose touch with the CAEN module
      break;

    case 3:
      hvcptr->com0 = 2;    // set comand to regular reading or else we lose touch with the CAEN module
      break;

    case 4:                 // HV On all
      signalBlock(p1);
      hvOnAll();
      signalBlock(p0);
      hvcptr->com0 = 2;    // things by slots...may not need
      break;

    case 5:                // HV Off All
      signalBlock(p1);
      hvOffAll();
      signalBlock(p0);
   
      hvcptr->com0 = 2;    // things by 
      break;
      
    case 7:                 // Recover from hardware...
      printf(" Recovering data from hardware....\n");
      signalBlock(p1);
      recoverCAEN();
      signalBlock(p0);
      hvcptr->com0 = 2;    // set comand to regular reading or else we lose touch with the CAEN module
      //      sleep(10);
      break;

    case 14:                  // turn on detector
      signalBlock(p1);
      hvcptr->com3 = 1;
      ii = hvcptr->com1;
      setPw(ii);
      sleep(1);
      getPw(ii);
      signalBlock(p0);
   
      hvcptr->com0 = 0;    // things by 
      break;

    case 15:               // turn off detector
      signalBlock(p1);
      hvcptr->com3 = 0; 
      ii = hvcptr->com1;
      setPw(ii);
      sleep(1);
      getPw(ii);
      signalBlock(p0);
   
      hvcptr->com0 = 0;    // things by 
      break;

    case 16:
      printf(" Changing parameters of one detector....\n");
      signalBlock(p1);
      changeParam();
      signalBlock(p0);
      hvcptr->com0 = 2;    // set comand to regular reading or else we lose touch with the CAEN module
     break;
      
    default:
      hvcptr->com0 = 2;
      sleep (INTERVAL);
      //      hvcptr->com0 = 2;
      break;
    }  // end of switch statement    hvcptr->com0 = -1;
  }  // end of while statement

  // CLOSE THE CAEN SYSTEMS HERE!!!!
  for (ii=0; ii<hvcptr->maxchan; ii++){
    if (hvcptr->xx[ii].type == 1) {
      result = CAENHV_DeinitSystem(hvcptr->xx[ii].caenH);
      if (result != 0) printf("Error deattaching to CAEN system %s \n",hvcptr->xx[indexMax].ip);
      else {
	printf ("Successful logout of CAEN crate!\n");
	break;
      }
    }
  }
  
/*
   Release the shared memory and close the U3

*/
  hvcptr->com4 = 0;
  if (munmap(hvcptr, sizeof (struct hvcaen*)) == -1) {
    perror("Error un-mmapping the file");
/* Decide here whether to close(fd) and exit() or not. Depends... */
  }
  close(mapHvcaen);

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
    //       printf("Block signals attempt\n");
    
    if (sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0) {
      perror ("sigprocmask");
      return;
    }  
  }
  else {    
    //       printf("release block signals ....\n");
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

  printf ("I caught signal number %i ! \n",hvcptr->com0);

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
   fd = open(HVCAENDATAPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0644);
   if (fd == -1) {
        perror("Error opening hvcaen file for writing");
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
   for (ii=0; ii<HVCAENDATASIZE; ii++){
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
   hvcptr = (struct hvcaen*) mmap(0, HVCAENDATASIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
   if (hvcptr == MAP_FAILED) {
        close(fd);
        perror("Error mmapping the hvcaen file");
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

/******************************************************************/
int scan2int () {
  char sss[200];
  long int ii=0;
  
  scanf("%s",sss);
  ii=strtol(sss,NULL,10);                            // convert to base 10
  if ((ii == 0) && (strcmp(sss,"0") != 0)) ii = -1;  // check if 0 result is 0 or because input is not number
  return ((int) ii);
}

/******************************************************************************/
void readConf() {
  FILE *ifile;
  char line[200]="\0";
  char hvcaen_conf[200] ="\0";
  int ii=0, mm=0, slot=0, chan=0, onoff=0, itrip=0, etrip=0, powup=0, kill=0;
  float volts=0.0, current=0.0, ramp=0.0, trip=0.0, dramp=0.0, svmax=0.0, v1set=0.0, i1set=0.0;
  char ip[30]="\0", name[15]="\0";
  char ipsave[10][30];
  int kk=0,new=0, maxIP=0;
    
  strcpy(hvcaen_conf,path);
  strcat(hvcaen_conf,"/");
  strcat(hvcaen_conf,CONFIGFILE);
  //  printf("    mpod_mib file: %s\n",hvcaen_conf);

/*
   Read configuration file
*/  
  if ( ( ifile = fopen (hvcaen_conf,"r+") ) == NULL) {
    printf ("*** File on disk (%s) could not be opened: \n",hvcaen_conf);
    printf ("===> %s \n",hvcaen_conf);
    exit (EXIT_FAILURE);
  }

  fgets(line,200,ifile);    // reads column headers
  fgets(line,200,ifile);    // reads column headers units
  fgets(line,200,ifile);    // reads ----
  fgets(line,200,ifile);    // reads ip address
  mm = sscanf (line,"%s", ip);  // copy ip address into string ip

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
    //    mm = sscanf (line,"%s %i", ip, &ii);  // read ip and type to determine CAEN or MPOD
    //    printf (" ip=%s , type = %i\n", ip, ii);
//    ii = 0;
    onoff = -1;
    chan = -1;
    slot = -1;
    volts = 0.0;
    ramp = 0.0;
    current = 0.0;
    strcpy (name,"\0");
    dramp = 0.0;
    svmax = 0.0;
    v1set = 0.0;
    i1set = 0.0;
    trip = 0.0;
    itrip = 0;
    etrip = 0;
    kill = 0;
    powup = 0;

/*
  Process CAEN 1527, 2527, ... HV supplies
*/    
    if (ii == 1) {
      mm = sscanf (line,"%i %u %u %14s %f %f %f %f %f %f %f %f %i %i %i %i", &ii, &slot, &chan, name, &volts, &current, &ramp,
		   &dramp, &svmax, &v1set, &i1set, &trip, &itrip, &etrip, &kill, &powup);  // CAEN data  
      hvcptr->xx[indexMax].type = ii;
      hvcptr->xx[indexMax].chan = chan;
      hvcptr->xx[indexMax].slot = slot;
      hvcptr->xx[indexMax].vSet = volts;
      hvcptr->xx[indexMax].vRamp = ramp;
      hvcptr->xx[indexMax].iSet = current;
      strcpy(hvcptr->xx[indexMax].name,name);
      strcpy(hvcptr->xx[indexMax].ip,ip);
      hvcptr->xx[indexMax].downRamp = dramp;
      hvcptr->xx[indexMax].vMax = svmax;
      hvcptr->xx[indexMax].v1Set = v1set;
      hvcptr->xx[indexMax].i1Set = i1set;
      hvcptr->xx[indexMax].trip = trip;
      hvcptr->xx[indexMax].intTrip = itrip;    // probably should get rid of this junk (if one channel trips others trip)
      hvcptr->xx[indexMax].extTrip = etrip;    // probably should get rid of this junk (if one channel trips others trip)
      hvcptr->xx[indexMax].kill = kill;        // probably should get rid of this junk (if one channel trips others trip)
      hvcptr->xx[indexMax].powUp = powup;      // probably should get rid of this junk (if one channel trips others trip)

      new = 0;
      for (kk=0; kk<maxIP; kk++) {                         // check if multiple CAEN units; new = a new IP found
	if (strcmp(ipsave[kk],ip) == 0) new = 1;              
      }
      if (new == 0) {
	strcpy(ipsave[maxIP],ip);      //...save and connect to get the handle: ceanH
      	printf("new IP: maxcaen = %i, IP = %s \n",maxIP,ipsave[maxIP]);
	maxIP++;
	caenCrate();
      }
      else hvcptr->xx[indexMax].caenH = hvcptr->xx[indexMax-1].caenH;   // copy previous caen Handle to this entry
    
      hvcptr->caenCrate[hvcptr->xx[indexMax].slot] += pow(2,hvcptr->xx[indexMax].chan);    // record active chan in bit pattern

    printf ("CAEN = %s %3u %3u %7.1f %7.1f %7.1f %i %s \n",hvcptr->xx[indexMax].ip, hvcptr->xx[indexMax].chan,
	      hvcptr->xx[indexMax].slot, hvcptr->xx[indexMax].vSet, hvcptr->xx[indexMax].vRamp, hvcptr->xx[indexMax].iSet,
	      hvcptr->xx[indexMax].type, hvcptr->xx[indexMax].name);
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
  hvcptr->maxchan = indexMax;
  printf ("%i HV entries found on MPODs and/or %i CAEN HV supplies\n",hvcptr->maxchan, maxIP);

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
void caenCrate() {
  char *model, *descModel, *m, *d;
  unsigned short numOfSlots;
  unsigned short *numOfChan, *serNumList;
  unsigned char  *firmwareMin, *firmwareMax;
  int result=0, ii=0;

  result = CAENHV_InitSystem(SY1527, 0, hvcptr->xx[indexMax].ip, "admin", "admin", &hvcptr->xx[indexMax].caenH);   // only opens 1527 CAEN systems
  if (result != 0) printf("Error attaching to CAEN system %s \n",hvcptr->xx[indexMax].ip);
  else {
    printf ("Successful login!...now map it \n");
    result = CAENHV_GetCrateMap(hvcptr->xx[indexMax].caenH, &numOfSlots, &numOfChan, &model, &descModel, &serNumList, &firmwareMin, &firmwareMax);   // need to get the crate info in order to access
    if (result != 0) printf("Error mapping CAEN system %s \n",hvcptr->xx[indexMax].ip);
    else {
      printf ("Successful Map\n\n");
      m = model;
      d = descModel;
      for (ii=0; ii<numOfSlots; ii++, m += strlen(m) + 1, d += strlen(d) + 1 ){
	if( *m == '\0' ){
	  printf("Board %2d: Not Present\n", ii);
	  hvcptr->caenCrate[ii]=-1;    //  bits above 24 are also set so -1 is OK
	  hvcptr->caenSlot[ii]=0;      //  number of channels
	}
	else {
	  printf("Board %2d: %s %s  Nr. Ch: %d  Ser. %d   Rel. %d.%d \n",
		    ii, m, d, numOfChan[ii], serNumList[ii], firmwareMax[ii],firmwareMin[ii]);
	  hvcptr->caenCrate[ii]=0;           //  bits = 0 above 24 are also set so -1 is OK
	  hvcptr->caenSlot[ii]=(unsigned int) numOfChan[ii];    //  number of channels
	  printf ("number of channels in slot %i = %i\n",ii,hvcptr->caenSlot[ii]);
	}
      }
    }
    printf ("\n");
  }
  free(serNumList), free(model), free(descModel), free(firmwareMin), free(firmwareMax), free(numOfChan);

  return;
}

/******************************************************************************
void caenGetBySlot() {

//   Get parameters from CAEN crates   

  int ii=0, jj=0, result =0;
  unsigned short c24 = 24, slot=0;
  char V0Set[30]="V0Set\0";   //   V0Set  I0Set  IMon VMon
  char I0Set[30]="I0Set\0";   //   V0Set  I0Set  IMon VMon
  char VMon[30]="VMon\0";     //   V0Set  I0Set  IMon VMon
  char IMon[30]="IMon\0";     //   V0Set  I0Set  IMon VMon
  char Pw[30]="Pw\0";         //   PW
  float	         *floatVal  = NULL;
  unsigned int   *unsignVal = NULL;
  unsigned short *ChList;

  ChList =    malloc(c24*sizeof(unsigned short));
  floatVal =  malloc(c24*sizeof(float));               // if multiple channels use numChan instead of one
  unsignVal = malloc(c24*sizeof(unsigned int));      // if multiple channels use numChan instead of one

  //  printf (" Getting CAEN data ....");  
  for (ii=0; ii< c24; ii++){
    ChList[ii]=ii;
  }
  slot=5;
  ii = 10;
  //  for (ii=0; ii<indexMax; ii++){
  //    if (hvcptr->xx[ii].type == 1) {

  result = CAENHV_GetChParam(hvcptr->xx[ii].caenH, slot, VMon, c24, ChList, floatVal);
  if (result != 0) printf("Error getting VMON from CAEN system %s handle %i\n",hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  else {
    //      floatVal[0] = 0;
    for (ii=0; ii<c24; ii++){
      printf ("chan %u = %7.1f volts\n", ChList[ii],floatVal[ii]);
    }
  }
  result = CAENHV_GetChParam(hvcptr->xx[ii].caenH, slot, Pw, c24, ChList, unsignVal);
  if (result != 0) printf("Error getting VMON from CAEN system %s handle %i\n",hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  else {
    //      floatVal[0] = 0;
    for (ii=0; ii<c24; ii++){
      printf ("chan %u = %u volts\n", ChList[ii],unsignVal[ii]);
    }
  }

  for (ii=0; ii <hvcptr->maxchan; ii++){
    if ( (hvcptr->xx[ii].caenH == hvcptr->xx[ii].caenH) && (hvcptr->xx[ii].slot == slot)) {
      for (jj=0;jj<c24;jj++){
	if ((hvcptr->xx[ii].chan == ChList[jj]) && (hvcptr->xx[ii].onoff == 1)){
	  printf ("chan %u = %7.1f volts\n", ChList[jj],floatVal[jj]);
	  hvcptr->xx[ii].vMeas = floatVal[jj];
	}
      }
    }
  }

  free(floatVal);  free(ChList);  free(unsignVal);
  printf (".... finished\n");  
  return;
}*/

/******************************************************************************/
void recoverCAEN() {
/*
   Get parameters from CAEN crates by processing each board..ie., all channels are read once per field
*/
  int ii=0, jj=0, kk=0, nn=0, type1=0, result=0, pp=0;
  int handle=0;
  char ipaddr[30];
  
  char V0Set[30]="V0Set\0";       //   V0Set  
  char I0Set[30]="I0Set\0";       //   I0Set  
  //  char VMon[30]="VMon\0";         //   VMon
  //  char IMon[30]="IMon\0";         //   IMon 
  char Pw[30]="Pw\0";             //   PW
  char V1Set[30]="V1Set\0";       //   V1Set
  char I1Set[30]="I1Set\0";       //   I1Set
  char SVMax[30]="SVMax\0";       //   V0Set  I0Set  IMon VMon
  char Trip[30]="Trip\0";         //   SVMax
  char RUp[30]="RUp\0";           //   RUp
  char RDWn[30]="RDWn\0";         //   RDWn
  char TripInt[30]="TripInt\0";   // TripInt
  char TripExt[30]="TripExt\0";   // TripExt
  char POn[30]="POn\0";       //   Turn on at power up or not
  char PDwn[30]="PDwn\0";       // Kill or ramp down
  char Status[30]="Status\0";       // Status

  struct hvchan yy[1000];

  float	         *floatVal  = NULL;
  unsigned int   *unsignVal = NULL;
  unsigned short *ChList;
  char  (*charVal)[MAX_CH_NAME];
  /*

*/
  printf (" Getting CAEN data ....\n ");  

  ii=0;
  //  while (++ii < 1000) {
  while (++ii < 1000) {
    handle = hvcptr->xx[ii].caenH;
    strcpy(ipaddr, hvcptr->xx[ii].ip);
    type1=ii;
    break;
  }

  hvcptr->maxchan =0;             // 0 maxchan to reload it....but what about MPOD?
  nn=0;
  for (kk=0; kk<16; kk++){              // loop over slots getting data by module...ie., get info from all channels
    //    printf ("processing slot %i\n",kk);
    if (hvcptr->caenSlot[kk] != 0) {     // will sort at end based on those that have onoff = 1.
      //     printf ("processing loaded slot %i\n",kk);
      ChList =    malloc(hvcptr->caenSlot[kk]*sizeof(unsigned short));
      floatVal =  malloc(hvcptr->caenSlot[kk]*sizeof(float));               // if multiple channels use numChan instead of one
      unsignVal = malloc(hvcptr->caenSlot[kk]*sizeof(unsigned int));      // if multiple channels use numChan instead of one
      charVal =   malloc(hvcptr->caenSlot[kk]*sizeof(MAX_CH_NAME));
  
      hvcptr->caenCrate[kk] =0;                      // zero the channel bit pattern....reset based on Pw =1 below
      for (jj=0; jj< hvcptr->caenSlot[kk]; jj++){    // zero the return data
	ChList[jj]=jj;
      }

      result = CAENHV_GetChParam(handle, kk, V0Set, hvcptr->caenSlot[kk], ChList, floatVal);
      if (result != 0) printf("Error getting V0Set from CAEN system %s handle %i\n",ipaddr,handle);
      else {
	for (ii=nn; ii < nn+hvcptr->caenSlot[kk]; ii++){
	  yy[ii].vSet = floatVal[ii-nn];
	}
      }

      result = CAENHV_GetChParam(handle, kk, I0Set, hvcptr->caenSlot[kk], ChList, floatVal);
      if (result != 0) printf("Error getting I0Set from CAEN system %s handle %i\n",ipaddr,handle);
      else {
	for (ii=nn; ii < nn+hvcptr->caenSlot[kk]; ii++){
	  yy[ii].iSet = floatVal[ii-nn];
	}
      }

      result = CAENHV_GetChParam(handle, kk, I1Set, hvcptr->caenSlot[kk], ChList, floatVal);
      if (result != 0) printf("Error getting I1Set from CAEN system %s handle %i\n",ipaddr,handle);
      else {
	for (ii=nn; ii < nn+hvcptr->caenSlot[kk]; ii++){
	  yy[ii].i1Set = floatVal[ii-nn];
	}
      }

      result = CAENHV_GetChParam(handle, kk, V1Set, hvcptr->caenSlot[kk], ChList, floatVal);
      if (result != 0) printf("Error getting V1Set from CAEN system %s handle %i\n",ipaddr,handle);
      else {
	for (ii=nn; ii < nn+hvcptr->caenSlot[kk]; ii++){
	  yy[ii].v1Set = floatVal[ii-nn];
	}
      }

      result = CAENHV_GetChParam(handle, kk, SVMax, hvcptr->caenSlot[kk], ChList, floatVal);
      if (result != 0) printf("Error getting SVMax from CAEN system %s handle %i\n",ipaddr,handle);
      else {
	for (ii=nn; ii < nn+hvcptr->caenSlot[kk]; ii++){
	  yy[ii].vMax = floatVal[ii-nn];
	}
      }

      result = CAENHV_GetChParam(handle, kk, RUp, hvcptr->caenSlot[kk], ChList, floatVal);
      if (result != 0) printf("Error getting vRamp from CAEN system %s handle %i\n",ipaddr,handle);
      else {
	for (ii=nn; ii < nn+hvcptr->caenSlot[kk]; ii++){
	  yy[ii].vRamp = floatVal[ii-nn];
	}
      }

      result = CAENHV_GetChParam(handle, kk, RDWn, hvcptr->caenSlot[kk], ChList, floatVal);
      if (result != 0) printf("Error getting RDWn from CAEN system %s handle %i\n",ipaddr,handle);
      else {
	for (ii=nn; ii < nn+hvcptr->caenSlot[kk]; ii++){
	  yy[ii].downRamp = floatVal[ii-nn];
	}
      }

      result = CAENHV_GetChParam(handle, kk, Trip, hvcptr->caenSlot[kk], ChList, floatVal);
      if (result != 0) printf("Error getting Trip from CAEN system %s handle %i\n",ipaddr,handle);
      else {
	for (ii=nn; ii < nn+hvcptr->caenSlot[kk]; ii++){
	  yy[ii].trip = floatVal[ii-nn];
	}
      }

      result = CAENHV_GetChParam(handle, kk, Pw, hvcptr->caenSlot[kk], ChList, unsignVal);
      if (result != 0) printf("Error getting PW (onoff) from CAEN system %s handle %i\n",ipaddr,handle);
      else {
	for (ii=nn; ii < nn+hvcptr->caenSlot[kk]; ii++){
	  yy[ii].onoff = (int) unsignVal[ii-nn];   // (float)val;           // get power PW as onoff
	  if ( yy[ii].onoff == 1) {
	    yy[ii].type = 1;                           // set type to caen crate
	    hvcptr->caenCrate[kk] += pow(2,ii-nn);      // load channel bit pattern
	    yy[ii].chan=ii-nn;                         // load crannel
	    yy[ii].slot=kk;                            // load slot
	    yy[ii].caenH=handle;                       // load handle
	    strcpy(yy[ii].ip,ipaddr);                  // load ip address
	    pp++;
	    //	    printf("last pp = %i\n",pp);
	  }
	}
      }

      result = CAENHV_GetChParam(handle, kk, TripInt, hvcptr->caenSlot[kk], ChList, unsignVal);
      if (result != 0) printf("Error getting TripInt from CAEN system %s handle %i\n",ipaddr,handle);
      else {
	for (ii=nn; ii < nn+hvcptr->caenSlot[kk]; ii++){
	  yy[ii].intTrip = (int) unsignVal[ii-nn];   // (float)val;           // get power PW as onoff
	}
      }

      result = CAENHV_GetChParam(handle, kk, TripExt, hvcptr->caenSlot[kk], ChList, unsignVal);
      if (result != 0) printf("Error getting TripExt from CAEN system %s handle %i\n",ipaddr,handle);
      else {
	for (ii=nn; ii < nn+hvcptr->caenSlot[kk]; ii++){
	  yy[ii].extTrip = (int) unsignVal[ii-nn];   // (float)val;           // get power PW as onoff
	}
      }

      result = CAENHV_GetChParam(handle, kk, PDwn, hvcptr->caenSlot[kk], ChList, unsignVal);
      if (result != 0) printf("Error getting PDwn from CAEN system %s handle %i\n",ipaddr,handle);
      else {
	for (ii=nn; ii < nn+hvcptr->caenSlot[kk]; ii++){
	  yy[ii].kill = (int) unsignVal[ii-nn];   // (float)val;           // get power PW as onoff
	}
      }

      result = CAENHV_GetChParam(handle, kk, POn, hvcptr->caenSlot[kk], ChList, unsignVal);
      if (result != 0) printf("Error getting POn from CAEN system %s handle %i\n",ipaddr,handle);
      else {
	for (ii=nn; ii < nn+hvcptr->caenSlot[kk]; ii++){
	  yy[ii].powUp = (int) unsignVal[ii-nn];   // (float)val;           // get power PW as onoff
	}
      }

       result = CAENHV_GetChParam(handle, kk, Status, hvcptr->caenSlot[kk], ChList, unsignVal);
      if (result != 0) printf("Error getting POn from CAEN system %s handle %i\n",ipaddr,handle);
      else {
	for (ii=nn; ii < nn+hvcptr->caenSlot[kk]; ii++){
	  yy[ii].status = (int) unsignVal[ii-nn];   // (float)val;           // get power PW as onoff
	}
      }

 // get names naturally from next read
      /*
      result = CAENHV_GetChName(handle, kk, hvcptr->caenSlot[kk], ChList, charVal);
      if (result != 0) printf("Error getting NAME from CAEN system %s handle %i\n",ipaddr, handle);
      else {

	strcpy(hvcptr->xx[ii].name,charVal[0]);   // (float)val;           // get power PW as onoff
      }
      
      result = CAENHV_GetChName(handle, kk, hvcptr->caenSlot[kk], ChList, charVal);
      if (result != 0) printf("Error getting Name from CAEN system %s handle %i\n",ipaddr,handle);
      else {
	printf("Name string length %li\n",strlen(charVal[ii-nn]));
	for (ii=nn; ii < nn+hvcptr->caenSlot[kk]; ii++){
	  //	  strcpy(hvcptr->xx[ii].name,charVal[ii]);   // (float)val;           // get power PW as onoff
	  strcpy(yy[ii].name,charVal[ii-nn]);   // (float)val;           // get power PW as onoff
	}
      }
      */
      nn += hvcptr->caenSlot[kk];
      //      printf("nn = %i  pp = %i\n",nn,pp);
      free(floatVal);  free(ChList);  free(unsignVal); free(charVal);
    }
  }
/*
  Have loaded all information into the arrays; now consolodate to maxchan.
*/
  kk = type1;                  // first CAEN module
  //  printf("last nn = %i\n",nn);
  for (ii=0; ii<nn; ii++) {
    if (yy[ii].onoff == 1) {                         // scroll through those found with HV on
      hvcptr->xx[kk++] = yy[ii];                      // copy into the CAEN data AFTER the MPOD
      //      printf("processing channel with HV on %i\n",kk);
    }
  }
  hvcptr->maxchan = kk;     // record maxchan
  indexMax = hvcptr->maxchan;
  
  printf (".... finished...found %i CAEN channels that are on\n", hvcptr->maxchan-type1);  
  return;
}
	
/******************************************************************************/
void caenGet() {
/*
   Get parameters from CAEN crates   
*/
  int ii=0;
  
  printf (" Getting CAEN data ....");  
  for (ii=0; ii<hvcptr->maxchan; ii++){
    if (hvcptr->xx[ii].type == 1) {
      getVMon(ii);
      getIMon(ii);
    }
  }

  printf (".... finished\n");  
  return;
}

/******************************************************************************/
void hvOnAll(){
  int ii=0;//, result =0, one = 1;
  //  char Pw[30]="Pw\0";                 //   PW
  //  unsigned int *unsignVal = NULL;

  //  unsignVal = malloc(sizeof(unsigned int));      // if multiple channels use numChan instead of one

  for (ii=0; ii<hvcptr->maxchan; ii++){
    hvcptr->com3 = 1;
    setPw(ii);
  }
  sleep(1);       // give time to allow settings to happen before getting status
  for (ii=0; ii<hvcptr->maxchan; ii++){
    getPw(ii);
  }
/*
    unsignVal[0] = 1;
    result = CAENHV_SetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, Pw, one, &hvcptr->xx[ii].chan, unsignVal);
    if (result != 0) printf("Error = 0x%x setting PW (onoff) for %s from CAEN system %s handle %i\n",
			    result,hvcptr->xx[ii].name,hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
    else {
      //      printf("Successful setting PW %i \n", ii); 
      hvcptr->xx[ii].onoff = 1;
    }
*/

//  }
  
//  free(unsignVal);
  return;
}

/******************************************************************************/
void hvOffAll(){
  int ii=0;//, result =0, one = 1;
  //  char Pw[30]="Pw\0";                 //   PW
  //  unsigned int *unsignVal = NULL;


  for (ii=0; ii<hvcptr->maxchan; ii++){
    hvcptr->com3 = 0;
    setPw(ii);
  }
  sleep(1);       // give time to allow settings to happen before getting status
  for (ii=0; ii<hvcptr->maxchan; ii++){
    getPw(ii);
  }
  /*

  unsignVal = malloc(sizeof(unsigned int));      // if multiple channels use numChan instead of one

  for (ii=0; ii<hvcptr->maxchan; ii++){
    unsignVal[0] = 0;
    result = CAENHV_SetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, Pw, one, &hvcptr->xx[ii].chan, unsignVal);
    if (result != 0) printf("Error = 0x%x setting PW (onoff) for %s from CAEN system %s handle %i\n",
			    result,hvcptr->xx[ii].name,hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
    else {
      //      printf("Successful setting PW %i \n", ii); 
      hvcptr->xx[ii].onoff = 0;
    }
  }
  
  free(unsignVal);
*/
  return;
}

/******************************************************************************/
void getTemp(){
  int result=0, one=1;
  int handle=0;
  short unsigned int ii=0;
  char Temp[30]="Temp\0";                 //   Temperature
  unsigned int   *unsignVal = NULL;
  unsigned short *slotList;
  float	         *floatVal  = NULL;
  
  slotList  = malloc(sizeof(unsigned short));
  unsignVal = malloc(sizeof(unsigned int));      // if multiple channels use numChan instead of one
  floatVal = malloc(sizeof(float));      // if multiple channels use numChan instead of one
  ii=-1;
  while (++ii < 1000) {
    if (hvcptr->xx[ii].type == 1) {
      handle = hvcptr->xx[ii].caenH;
      break;
    }
  }
  
  for (ii=0; ii<16; ii++){
    if (hvcptr->caenSlot[ii] > 0){
      result = CAENHV_GetBdParam(handle, one, &ii, Temp, floatVal);
      if (result != 0) printf("Error = %i (0x%x) getting temperature \n", result,result);
      else hvcptr->caenTemp[ii] = floatVal[0];
    }
  }
  free(unsignVal); free(slotList); free(floatVal);
  
  return;
}

/******************************************************************************/
void setAllParam(){
  int ii=0;
  // does not turn HV on

  printf("Setting all parameters ...  \n");
  for (ii=0; ii< hvcptr->maxchan; ii++){
    hvcptr->xcom3 = hvcptr->xx[ii].vSet;
    setV0Set(ii);
	
    hvcptr->xcom3 = hvcptr->xx[ii].iSet;
    setI0Set(ii);
 
    hvcptr->xcom3 = hvcptr->xx[ii].vRamp;
    setRUp(ii);
 
    hvcptr->xcom3 = hvcptr->xx[ii].v1Set;
    setV1Set(ii);

    hvcptr->xcom3 = hvcptr->xx[ii].i1Set;
    setI1Set(ii);
 
    hvcptr->xcom3 = hvcptr->xx[ii].vMax;
    setSVMax(ii);

    hvcptr->xcom3 = hvcptr->xx[ii].downRamp;
    setRDWn(ii);

    hvcptr->xcom3 = hvcptr->xx[ii].trip;
    setTrip(ii);

    hvcptr->com3 = hvcptr->xx[ii].intTrip;
    setTripInt(ii);

    hvcptr->com3 = hvcptr->xx[ii].extTrip;
    setTripExt(ii);
    /*
    hvcptr->com3 = hvcptr->xx[ii].kill;
    setPDwn(ii);

    hvcptr->com3 = hvcptr->xx[ii].powUp;
    setPOn(ii);
*/
    setName(ii);
	
  }
  printf(" done setting all parameters  \n");

  return;
}
/******************************************************************************/
void getAllParam(){
  int ii=0;

  for (ii=0; ii< hvcptr->maxchan; ii++){
    getV0Set(ii);
    getI0Set(ii);
    getRUp(ii);
    getV1Set(ii);
    getI1Set(ii);
    getSVMax(ii);
    getRDWn(ii);
    getTrip(ii);
    getTripInt(ii);
    getTripExt(ii);
    getPOn(ii);
    getPDwn(ii);
    getStatus(ii);
    getName(ii);
    //    printf ("%i - on/off = %i kill = %i enable = %i status = %i name = %s\n",ii, hvcptr->xx[ii].onoff, hvcptr->xx[ii].kill, hvcptr->xx[ii].powUp, hvcptr->xx[ii].status, hvcptr->xx[ii].name);
  }

  return;
}

/******************************************************************************/
void changeParam(){
  int ii = 0;

/*
 control variables:
  com0 = 16        selects this function
  com1 = detector  selects detector to change
  com2 = parameter selects the parameter to chang
  com3, xcom3 = new value  .....
*/

  ii = hvcptr->com1;
  switch (hvcptr->com2){

  case 1:
    if (hvcptr->xcom3 == hvcptr->xx[ii].vSet) break;  // nothing to do
    setV0Set(ii);
    getV0Set(ii);
    break;

  case 2:
    if (hvcptr->xcom3 == hvcptr->xx[ii].iSet) break;  // nothing to do
    setI0Set(ii);
    getI0Set(ii);
    break;

  case 3:
    if (hvcptr->com3 == hvcptr->xx[ii].onoff) break;  // nothing to do
    setPw(ii);
    getPw(ii);
    break;

  case 4:
    if (hvcptr->xcom3 == hvcptr->xx[ii].vRamp) break;  // nothing to do
    setRUp(ii);
    getRUp(ii);
    break;

  case 5:
    if (hvcptr->xcom3 == hvcptr->xx[ii].v1Set) break;  // nothing to do
    setV1Set(ii);
    getV1Set(ii);
    break;

  case 6:
    if (hvcptr->xcom3 == hvcptr->xx[ii].i1Set) break;  // nothing to do
    setI1Set(ii);
    getI1Set(ii);
    break;

  case 7:
    if (hvcptr->xcom3 == hvcptr->xx[ii].vMax) break;  // nothing to do
    setSVMax(ii);
    getSVMax(ii);
    break;

  case 8:
    if (hvcptr->xcom3 == hvcptr->xx[ii].downRamp) break;  // nothing to do
    setRDWn(ii);
    getRDWn(ii);
    break;

  case 9:
    if (hvcptr->xcom3 == hvcptr->xx[ii].trip) break;  // nothing to do
    setTrip(ii);
    getTrip(ii);
    break;

  case 10:
    if (hvcptr->com3 == hvcptr->xx[ii].intTrip) break;  // nothing to do
    setTripInt(ii);
    getTripInt(ii);
    break;

   case 11:
    if (hvcptr->com3 == hvcptr->xx[ii].extTrip) break;  // nothing to do
    setTripExt(ii);
    getTripExt(ii);
    break;

   case 12:
    if (hvcptr->com3 == hvcptr->xx[ii].kill) break;  // nothing to do
    setPDwn(ii);
    getPDwn(ii);
    break;

   case 13:
    if (hvcptr->com3 == hvcptr->xx[ii].powUp) break;  // nothing to do
    setPOn(ii);
    getPOn(ii);
    break;

   case 14:
    setName(ii);
    getName(ii);
    break;
  
  default:
    break;
  }
  hvcptr->xcom3 = -1;

  return;
}
/******************************************************************************/
void getVMon(int ii){
  char VMon[30]="VMon\0";       //   V0Set  
  int one=1;
  int result=0;
  float	         *floatVal  = NULL;

  floatVal =  malloc(one * sizeof(float));               // if multiple channels use numChan instead of one

  result = CAENHV_GetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, VMon, one, &hvcptr->xx[ii].chan, floatVal);
  if (result != 0) printf("Error getting VMon for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  else hvcptr->xx[ii].vMeas = floatVal[0];   // (float)val;           // set voltage

  free (floatVal);

  return;
}

/******************************************************************************/
void getIMon(int ii){
  char IMon[30]="IMon\0";       //   V0Set  
  int one=1;
  int result=0;
  float	         *floatVal  = NULL;

  floatVal =  malloc(one * sizeof(float));               // if multiple channels use numChan instead of one

  result = CAENHV_GetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, IMon, one, &hvcptr->xx[ii].chan, floatVal);
  if (result != 0) printf("Error getting IMon for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  else hvcptr->xx[ii].iMeas = floatVal[0];   // (float)val;           // set voltage

  free (floatVal);

  return;
}

/******************************************************************************/
void getV0Set(int ii){
  char V0Set[30]="V0Set\0";       //   V0Set  
  int one=1;
  int result=0;
  float	         *floatVal  = NULL;

  floatVal =  malloc(one * sizeof(float));               // if multiple channels use numChan instead of one

  result = CAENHV_GetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, V0Set, one, &hvcptr->xx[ii].chan, floatVal);
  if (result != 0) printf("Error getting V0Set for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  else hvcptr->xx[ii].vSet = floatVal[0];   // (float)val;           // set voltage

  free (floatVal);

  return;
}

/******************************************************************************/
void setV0Set(int ii){
  char V0Set[30]="V0Set\0";       //   V0Set  
  int one=1;
  int result=0;
  float	         *floatVal  = NULL;

  //  if (hvcptr->xx[ii].vSet == hvcptr->xcom3) return;
  
  floatVal =  malloc(one * sizeof(float));               // if multiple channels use numChan instead of one

  floatVal[0] = hvcptr->xcom3;
  result = CAENHV_SetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, V0Set, one, &hvcptr->xx[ii].chan, floatVal);
  if (result != 0) printf("Error setting V0Set for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  //  else printf("Success V0\n");            // set voltage
  free (floatVal);

  return;
}

/******************************************************************************/
void getI0Set(int ii){
  char I0Set[30]="I0Set\0";       //   V0Set  
  int one=1;
  int result=0;
  float	         *floatVal  = NULL;

  floatVal =  malloc(one * sizeof(float));               // if multiple channels use numChan instead of one

  result = CAENHV_GetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, I0Set, one, &hvcptr->xx[ii].chan, floatVal);
  if (result != 0) printf("Error getting I0Set for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  else hvcptr->xx[ii].iSet = floatVal[0];   // (float)val;           // set voltage

  free (floatVal);

  return;
}

/******************************************************************************/
void setI0Set(int ii){
  char I0Set[30]="I0Set\0";       //   I0Set
  int one=1;
  int result=0;
  float	         *floatVal  = NULL;

  //  if (hvcptr->xx[ii].iSet == hvcptr->xcom3) return;
  
  floatVal =  malloc(one * sizeof(float));               // if multiple channels use numChan instead of one
  floatVal[0] = hvcptr->xcom3;
  result = CAENHV_SetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, I0Set, one, &hvcptr->xx[ii].chan, floatVal);
  if (result != 0) printf("Error setting I0Set for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  //  else printf("Success\n");            // set current
  free (floatVal);

  return;
}

/******************************************************************************/
void getV1Set(int ii){
  char V1Set[30]="V1Set\0";       //   V0Set  
  int one=1;
  int result=0;
  float	         *floatVal  = NULL;

  floatVal =  malloc(one * sizeof(float));               // if multiple channels use numChan instead of one

  result = CAENHV_GetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, V1Set, one, &hvcptr->xx[ii].chan, floatVal);
  if (result != 0) printf("Error getting V1Set for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  else hvcptr->xx[ii].v1Set = floatVal[0];   // (float)val;           // set voltage

  free (floatVal);

  return;
}

/******************************************************************************/
void setV1Set(int ii){
  char V1Set[30]="V1Set\0";       //   I0Set
  int one=1;
  int result=0;
  float	         *floatVal  = NULL;

  //  if (hvcptr->xx[ii].v1Set == hvcptr->xcom3) return;

  floatVal =  malloc(one * sizeof(float));               // if multiple channels use numChan instead of one
  floatVal[0] = hvcptr->xcom3;
  result = CAENHV_SetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, V1Set, one, &hvcptr->xx[ii].chan, floatVal);
  if (result != 0) printf("Error setting V1Set for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  //  else printf("Success\n");            // set current
  free (floatVal);

  return;
}
/******************************************************************************/
void getI1Set(int ii){
  char I1Set[30]="I1Set\0";       //   V0Set  
  int one=1;
  int result=0;
  float	         *floatVal  = NULL;

  floatVal =  malloc(one * sizeof(float));               // if multiple channels use numChan instead of one

  result = CAENHV_GetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, I1Set, one, &hvcptr->xx[ii].chan, floatVal);
  if (result != 0) printf("Error getting I1Set for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  else hvcptr->xx[ii].i1Set = floatVal[0];   // (float)val;           // set voltage

  free (floatVal);

  return;
}

/******************************************************************************/
void setI1Set(int ii){
  char I1Set[30]="I1Set\0";       //   I0Set
  int one=1;
  int result=0;
  float	         *floatVal  = NULL;

  //  if (hvcptr->xx[ii].i1Set == hvcptr->xcom3) return;

  floatVal =  malloc(one * sizeof(float));               // if multiple channels use numChan instead of one
  floatVal[0] = hvcptr->xcom3;
  result = CAENHV_SetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, I1Set, one, &hvcptr->xx[ii].chan, floatVal);
  if (result != 0) printf("Error setting I1Set for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  //  else printf("Success\n");            // set current
  free (floatVal);

  return;
}
/******************************************************************************/
void getSVMax(int ii){
  char SVMax[30]="SVMax\0";       //   V0Set  
  int one=1;
  int result=0;
  float	         *floatVal  = NULL;

  floatVal =  malloc(one * sizeof(float));               // if multiple channels use numChan instead of one

  result = CAENHV_GetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, SVMax, one, &hvcptr->xx[ii].chan, floatVal);
  if (result != 0) printf("Error getting SVMax for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  else hvcptr->xx[ii].vMax = floatVal[0];   // (float)val;           // set voltage

  free (floatVal);

  return;
}

/******************************************************************************/
void setSVMax(int ii){
  char SVMax[30]="SVMax\0";       //   I0Set
  int one=1;
  int result=0;
  float	         *floatVal  = NULL;

  //  if (hvcptr->xx[ii].vMax == hvcptr->xcom3) return;

  floatVal =  malloc(one * sizeof(float));               // if multiple channels use numChan instead of one
  floatVal[0] = hvcptr->xcom3;
  result = CAENHV_SetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, SVMax, one, &hvcptr->xx[ii].chan, floatVal);
  if (result != 0) printf("Error setting SVMax for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  //  else printf("Success\n");            // set current
  free (floatVal);

  return;
}

/******************************************************************************/
void getRUp(int ii){
  char RUp[30]="RUp\0";       //   V0Set  
  int one=1;
  int result=0;
  float	         *floatVal  = NULL;

  floatVal =  malloc(one * sizeof(float));               // if multiple channels use numChan instead of one

  result = CAENHV_GetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, RUp, one, &hvcptr->xx[ii].chan, floatVal);
  if (result != 0) printf("Error getting RUp for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  else hvcptr->xx[ii].vRamp = floatVal[0];   // (float)val;           // set voltage

  free (floatVal);

  return;
}

/******************************************************************************/
void setRUp(int ii){
  char RUp[30]="RUp\0";       //   I0Set
  //  char RUp[30]="Ramp-Up\0";       //   I0Set
  int one=1;
  int result=0;
  float	         *floatVal  = NULL;

  //  if (hvcptr->xx[ii].vRamp == hvcptr->xcom3) return;

  floatVal =  malloc(one * sizeof(float));               // if multiple channels use numChan instead of one
  floatVal[0] = hvcptr->xcom3;

  result = CAENHV_SetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, RUp, one, &hvcptr->xx[ii].chan, floatVal);
  if (result != 0) printf("Error 0x%x setting RUp for det %i from CAEN system %s handle %i\n",result, ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  //  else printf("Success\n");            // set current

  free (floatVal);

  return;
}

/******************************************************************************/
void getRDWn(int ii){
  char RDWn[30]="RDWn\0";       //   V0Set  
  int one=1;
  int result=0;
  float	         *floatVal  = NULL;

  floatVal =  malloc(one * sizeof(float));               // if multiple channels use numChan instead of one

  result = CAENHV_GetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, RDWn, one, &hvcptr->xx[ii].chan, floatVal);
  if (result != 0) printf("Error getting RDWn for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  else hvcptr->xx[ii].downRamp = floatVal[0];   // (float)val;           // set voltage

  free (floatVal);

  return;
}

/******************************************************************************/
void setRDWn(int ii){
  char RDWn[30]="RDWn\0";       //   I0Set
  int one=1;
  int result=0;
  float	         *floatVal  = NULL;

  //  if (hvcptr->xx[ii].downRamp == hvcptr->xcom3) return;

  floatVal =  malloc(one * sizeof(float));               // if multiple channels use numChan instead of one
  floatVal[0] = hvcptr->xcom3;
  result = CAENHV_SetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, RDWn, one, &hvcptr->xx[ii].chan, floatVal);
  if (result != 0) printf("Error setting RDWn for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  //  else printf("Success\n");            // set current
  free (floatVal);

  return;
}

/******************************************************************************/
void getTrip(int ii){
  char Trip[30]="Trip\0";       //   V0Set  
  int one=1;
  int result=0;
  float	         *floatVal  = NULL;

  floatVal =  malloc(one * sizeof(float));               // if multiple channels use numChan instead of one

  result = CAENHV_GetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, Trip, one, &hvcptr->xx[ii].chan, floatVal);
  if (result != 0) printf("Error getting Trip for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  else hvcptr->xx[ii].trip = floatVal[0];   // (float)val;           // set voltage

  free (floatVal);

  return;
}

/******************************************************************************/
void setTrip(int ii){
  char Trip[30]="Trip\0";       //   I0Set
  int one=1;
  int result=0;
  float	         *floatVal  = NULL;

  //  if (hvcptr->xx[ii].trip == hvcptr->xcom3) return;

  floatVal =  malloc(one * sizeof(float));               // if multiple channels use numChan instead of one
  floatVal[0] = hvcptr->xcom3;
  result = CAENHV_SetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, Trip, one, &hvcptr->xx[ii].chan, floatVal);
  if (result != 0) printf("Error setting Trip for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  //  else printf("Success\n");            // set current
  free (floatVal);

  return;
}

/******************************************************************************/
void getTripInt(int ii){
  char TripInt[30]="TripInt\0";       //   I0Set
  int one=1;
  int result=0;
  unsigned int   *unsignVal = NULL;

  unsignVal =  malloc(one * sizeof(unsigned int));               // if multiple channels use numChan instead of one

  result = CAENHV_GetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, TripInt, one, &hvcptr->xx[ii].chan, unsignVal);
  if (result != 0) printf("Error setting TripInt for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  else hvcptr->xx[ii].intTrip = unsignVal[0];   // (float)val;           // set voltage
  free (unsignVal);

  return;
}

/******************************************************************************/
void setTripInt(int ii){
  char TripInt[30]="TripInt\0";       //   I0Set
  int one=1;
  int result=0;
  unsigned int   *unsignVal = NULL;

  //  if (hvcptr->xx[ii].intTrip == hvcptr->com3) return;

  unsignVal =  malloc(one * sizeof(unsigned int));               // if multiple channels use numChan instead of one
  unsignVal[0] = hvcptr->com3;
  result = CAENHV_SetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, TripInt, one, &hvcptr->xx[ii].chan, unsignVal);
  if (result != 0) printf("Error setting TripInt for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  //  else printf("Success\n");            // set internal trip
  free (unsignVal);

  return;
}

/******************************************************************************/
void getTripExt(int ii){
  char TripExt[30]="TripExt\0";       //   I0Set
  int one=1;
  int result=0;
  unsigned int   *unsignVal = NULL;

  unsignVal =  malloc(one * sizeof(unsigned int));               // if multiple channels use numChan instead of one

  result = CAENHV_GetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, TripExt, one, &hvcptr->xx[ii].chan, unsignVal);
  if (result != 0) printf("Error setting TripExt for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  else hvcptr->xx[ii].extTrip = unsignVal[0];   // (float)val;           // set voltage
  free (unsignVal);

  return;
}

/******************************************************************************/
void setTripExt(int ii){
  char TripExt[30]="TripExt\0";       //   I0Set
  int one=1;
  int result=0;
  unsigned int   *unsignVal = NULL;

  //  if (hvcptr->xx[ii].extTrip == hvcptr->com3) return;

  unsignVal =  malloc(one * sizeof(unsigned int));               // if multiple channels use numChan instead of one
  unsignVal[0] = hvcptr->com3;
  result = CAENHV_SetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, TripExt, one, &hvcptr->xx[ii].chan, unsignVal);
  if (result != 0) printf("Error setting TripExt for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  //  else printf("Success\n");            // set internal trip
  free (unsignVal);

  return;
}

/******************************************************************************/
void getPw(int ii){
  char Pw[30]="Pw\0";       //   I0Set
  int one=1;
  int result=0;
  unsigned int   *unsignVal = NULL;

  unsignVal =  malloc(one * sizeof(unsigned int));               // if multiple channels use numChan instead of one

  result = CAENHV_GetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, Pw, one, &hvcptr->xx[ii].chan, unsignVal);
  if (result != 0) printf("Error setting Pw for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  else hvcptr->xx[ii].onoff = unsignVal[0];   // (float)val;           // set voltage
  free (unsignVal);

  return;
}

/******************************************************************************/
void setPw(int ii){
  char Pw[30]="Pw\0";       //   I0Set
  int one=1;
  int result=0;
  unsigned int   *unsignVal = NULL;


  //  printf("ii=%i, com3 = %i\n",ii,hvcptr->com3);
  unsignVal =  malloc(one * sizeof(unsigned int));               // if multiple channels use numChan instead of one
  unsignVal[0] = hvcptr->com3;
  result = CAENHV_SetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, Pw, one, &hvcptr->xx[ii].chan, unsignVal);
  if (result != 0) printf("Error setting HV ON/OFF for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  //  else printf("Success\n");            // set internal trip
  free (unsignVal);

  return;
}

/******************************************************************************/
void getPDwn(int ii){
  char PDwn[30]="PDwn\0";       //   I0Set
  int one=1;
  int result=0;
  unsigned int   *unsignVal = NULL;

  unsignVal =  malloc(one * sizeof(unsigned int));               // if multiple channels use numChan instead of one

  result = CAENHV_GetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, PDwn, one, &hvcptr->xx[ii].chan, unsignVal);
  if (result != 0) printf("Error getting PDwn for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  else hvcptr->xx[ii].kill = unsignVal[0];   // (float)val;           // set voltage
  free (unsignVal);

  return;
}

/******************************************************************************/
void setPDwn(int ii){
  char PDwn[30]="PDwn\0";       //   I0Set
  int one=1;
  int result=0;
  unsigned int   *unsignVal = NULL;

  //  if (hvcptr->xx[ii].onoff == hvcptr->com3) return;

  unsignVal =  malloc(one * sizeof(unsigned int));               // if multiple channels use numChan instead of one
  unsignVal[0] = hvcptr->com3;
  result = CAENHV_SetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, PDwn, one, &hvcptr->xx[ii].chan, unsignVal);
  if (result != 0) printf("Error setting PDwn for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  //  else printf("Success\n");            // set internal trip
  free (unsignVal);

  return;
}

/******************************************************************************/
void getPOn(int ii){
  char POn[30]="POn\0";       //   I0Set
  int one=1;
  int result=0;
  unsigned int   *unsignVal = NULL;

  unsignVal =  malloc(one * sizeof(unsigned int));               // if multiple channels use numChan instead of one

  result = CAENHV_GetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, POn, one, &hvcptr->xx[ii].chan, unsignVal);
  if (result != 0) printf("Error getting POn for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  else hvcptr->xx[ii].powUp = unsignVal[0];   // (float)val;           // set voltage
  free (unsignVal);

  return;
}

/******************************************************************************/
void setPOn(int ii){
  char POn[30]="POn\0";       //   I0Set
  int one=1;
  int result=0;
  unsigned int   *unsignVal = NULL;

  //  if (hvcptr->xx[ii].onoff == hvcptr->com3) return;

  unsignVal =  malloc(one * sizeof(unsigned int));               // if multiple channels use numChan instead of one
  unsignVal[0] = hvcptr->com3;
  result = CAENHV_SetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, POn, one, &hvcptr->xx[ii].chan, unsignVal);
  if (result != 0) printf("Error setting POn for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  //  else printf("Success\n");            // set internal trip
  free (unsignVal);

  return;
}
/******************************************************************************/
void getStatus(int ii){
  char Status[30]="Status\0";       //   I0Set
  int one=1;
  int result=0;
  unsigned int   *unsignVal = NULL;

  unsignVal =  malloc(one * sizeof(unsigned int));               // if multiple channels use numChan instead of one

  result = CAENHV_GetChParam(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, Status, one, &hvcptr->xx[ii].chan, unsignVal);
  if (result != 0) printf("Error getting status for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  else hvcptr->xx[ii].status = unsignVal[0];   // (float)val;           // set voltage
  free (unsignVal);

  return;
}

/******************************************************************************/
void getName(int ii){
  int one=1;
  int result=0;
  char  (*charVal)[MAX_CH_NAME];

  charVal =   malloc(sizeof(MAX_CH_NAME));

  result = CAENHV_GetChName(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, one, &hvcptr->xx[ii].chan, charVal);
  if (result != 0) printf("Error getting NAME for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  else strcpy(hvcptr->xx[ii].name,charVal[0]);   // (float)val;           // get power PW as onoff

  free (charVal);

  return;
}

/******************************************************************************/
void setName(int ii){
  int one=1;
  int result=0;
  char  (*charVal)[MAX_CH_NAME];

  charVal =   malloc(sizeof(MAX_CH_NAME));

  strcpy(charVal[0],hvcptr->xx[ii].name);

  result = CAENHV_SetChName(hvcptr->xx[ii].caenH, hvcptr->xx[ii].slot, one, &hvcptr->xx[ii].chan, charVal[0]);
  if (result != 0) printf("Error setting NAME for det %i from CAEN system %s handle %i\n",ii, hvcptr->xx[ii].ip,hvcptr->xx[ii].caenH);
  //  else printf("Success\n");   

  free (charVal);

  return;
}
/******************************************************************************/
