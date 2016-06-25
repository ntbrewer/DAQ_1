/*
  Program hv-caen to monitor detector High Voltage
  Understands CAEN x527

  To be compiled with:                                           
     gcc -Wall -lcaenhvwrapper -lpthread -ldl -lm -o hv-caen hv-caen.c
     gcc -Wall -lm -o hv-caen-read hv-caen-read.c
*/

#include "../include/hv-caen.h"

int mmapSetup();                  // sets up the memory map
void menu();
void menu1();
int scan2int (); 
void printOut();
void fprintOut();
void printActive();
void printDeactive();
void clone();
//void hvOnAll();
//void hvOffAll();
void saveSetup();
void openSaveFile();
char *GetDate();
void detParam();
void statusStats();

FILE *fileSave;

int stats[16];
 
/***********************************************************/
int main(int argc, char **argv){
/*

*/
  int ii=0;
  int ans=0;
  int mapHvcaen;
  //  pid_t pid;
  /*
  printf("Working directory: %s\n",getcwd(path,PATH_MAX+1));
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
  Scroll through options
*/
  while (ans != 100){
    statusStats();
    menu1();
    //    scanf ("%i", &ans);          // read in ans (pointer is indicated)     
    ans = scan2int ();
    
    switch (ans){
    case 0:                    // end program but not kelvin
      ans = 100;
      break;
    case 1:                      // display temps and limits do not communicate to kelvin
      printOut();
      break;

    case 2:                      // display temps and limits
      printOut();
      hvcptr->com0 = 2;           // force a read of the ssystem
      kill(hvcptr->pid,SIGALRM);  // send an alarm to let hvcaen know it needs to do command (1)
      sleep(1);                  // sleep for a second to allow kelvin to read new values
      printOut();
  
      break;
/*
    case 3:                      // display temps and limits
      printf ("What interval in integer seconds ?\n");
      kk = scan2int ();
      //      scanf ("%li", &kk);          // read in ans (pointer is indicated)     
      hvcptr->interval = kk;
      hvcptr->com0 = 3;            // command (3) stored in SHM so kelvin can do something
      kill(hvcptr->pid,SIGALRM);   // send an alarm to let kelvin know it needs to do command (1)
      break;
*/
    case 4:                      // HV On
      printf("HV on...\n");
            hvcptr->com0 = 4;
      kill(hvcptr->pid,SIGALRM);  // send an alarm to let kelvin know it needs to do command (1)
      sleep(10);     // gives time to execute command
      break;

    case 5:                      // HV Off
      printf("HV off...\n");
      hvcptr->com0 = 5;
      kill(hvcptr->pid,SIGALRM);  // send an alarm to let kelvin know it needs to do command (1)
      sleep(10);     // gives time to execute command
      break;
    case 6:                      // save conf file
      printf("Saving file...\n");
      openSaveFile();
      fprintOut();
      fclose(fileSave);
      printf("file should be saved ...\n");
      break;

    case 7:
      hvcptr->com0 = 7;           // recover data from hardware
      kill(hvcptr->pid,SIGALRM);  // send an alarm to let kelvin know it needs to do command (1)
      printf("Recover setup from hardware ...\n");
      sleep(1);
      break;

    case 8:                      // print active detectors hvcaen does need to do anything
      printActive();
      break;

    case 9:                     // print inactive detectors hvcaen does need to do anything
      printDeactive();
      break;

    case 11:                      // clone detector parameters
      printf("Clone channel parameters...\n");
      clone();
      break;
     
    case 14:                      // turn on one channel
      printf("Which detector HV to turn ON ? (-1 to end) \n");
      printActive();
      ii = scan2int();
      if (ii > -1 && ii < hvcptr->maxchan) hvcptr->com1 = ii;
      hvcptr->com0 = 14;
      kill(hvcptr->pid,SIGALRM);   // send an alarm to let kelvin know it needs to do command (1)
      break;

    case 15:                      // turn off one channel
      printf("Which detector HV to turn OFF ? (-1 to end) \n");
      printActive();
      ii = scan2int();
      if (ii > -1 && ii < hvcptr->maxchan) hvcptr->com1 = ii;
      hvcptr->com0 = 15;
      kill(hvcptr->pid,SIGALRM);   // send an alarm to let kelvin know it needs to do command (1)
      break;

    case 16:                      // display temps and limits
      printf ("Alter parameters ?\n");
      printActive();
      detParam();
      hvcptr->com0 = 16;
      kill(hvcptr->pid,SIGALRM);   // send an alarm to let kelvin know it needs to do command (1)
      break;

    case 17:
      printf("Reading channel from file...\n");
      break;

    case 18:
      printf("Alarm resets?.....\n");
      break;

      
    case 100:                 // End ALL kelvin programs by breaking out of while statement
      hvcptr->com0=-1;                   // send command to hvcaen to break out of its while command
      hvcptr->com1=-1;                   // send command to hvcaen-log to break out of its while command
      hvcptr->com2=-1;                   // send command to hvcaen-daq to break out of its while command
      kill(hvcptr->pid,SIGALRM);
      break;

    default:                  // Do nothing and go back to the list of options
      ans = 0;
      break;      
    }  
  }
  
/*
   Release the shared memory and close the U3
*/
  if (munmap(hvcptr, sizeof (struct hvcaen*)) == -1) {
    perror("Error un-mmapping the file");
/* Decide here whether to close(fd) and exit() or not. Depends... */
  }
  close(mapHvcaen);

  return 0;
}

/***********************************************************/
void menu(){

  printf ("\nOptions ? \n");
  printf ("    1 - display current high voltage data (does not read or change interval) \n");
  printf ("    2 - force a read of current high voltage data \n");
  printf ("    3 - change measurement interval (currently %i s)\n",hvcptr->interval);
  printf (" \n");
  printf ("    4 - End log file \n");
  //  printf ("    5 - End DAQ logging \n");
  printf (" \n");
  printf ("    0 - end this program only \n");
  printf (" \n");
  printf ("  100 - end all hvcaen programs \n");

return;
}

/***********************************************************/
int mmapSetup() {
  //#define FILEPATH "data/mmapped.bin"
  //#define FILESIZE sizeof(struct thermometer)
  int fd=0;//, result=0, ii=0;
  /* Open a file for writing.
    *  - Creating the file if it doesn't exist.
    *  - Truncating it to 0 size if it already exists. (not really needed)
    *
    * Note: "O_WRONLY" mode is not sufficient when mmaping.
    */
  //   fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0644);
   fd = open(HVCAENDATAPATH, O_RDWR, (mode_t)0644);
   if (fd == -1) {
        perror("Error opening hvcaen file for writing");
        exit(EXIT_FAILURE);
   }

   /* Now the file is ready to be mmapped.
    */
   hvcptr = (struct hvcaen*) mmap(0, HVCAENDATASIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
   if (hvcptr == MAP_FAILED) {
        close(fd);
        perror("Error mmapping the hvcaen file");
        exit(EXIT_FAILURE);
   }

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

/**************************************************************/
float scan2float () {
  char sss[200];
  float xx=0;
  
  scanf("%s",sss);
  xx=atof(sss);                            // convert to base 10
  return (xx);
}

/**************************************************************/
void printOut() {
  int ii=0;
  //  time_t curtime;
/*
  union int2int4 {
    time_t ct;
    unsigned short int ust[4];
  } x;

  x.ct = degptr->tim.time1;
*/
  printf ("-----------------------------------------------------------------------------------------------------------\n");
  printf ("Time since start: %li s\n", hvcptr->secRunning);
  printf ("-----------------------------------------------------------------------------------------------------------\n");
  //       12345678901234  123456  123456  123456  123456  123456
  printf ("     Name        vSet    vMeas   iSet    iMeas  On/Off   uRamp  dRamp  SVMax  V1Set  I1Set   Trp  ITrp eTrp kill  powUp \n");
  printf ("                  (V)     (V)    (uA)     (uA)  (bool)   (V/s)  (V/s)   (V)    (V)    (uA)   (s)                        \n");
  printf ("--------------  ------  ------  ------  ------  ------  ------ ------ ------ ------ ------  ----  ---- ---- ----  ----- \n");

  for (ii=0; ii<hvcptr->maxchan; ii++){
    if (hvcptr->xx[ii].type == 0) {
      printf ("%14s  %6.1lf  %6.1lf %6.6lf %6.6lf %3i ",
	      hvcptr->xx[ii].name,hvcptr->xx[ii].vSet,hvcptr->xx[ii].vMeas,hvcptr->xx[ii].iSet,hvcptr->xx[ii].iMeas,hvcptr->xx[ii].onoff);
      //      printf ("%6.1lf %6.1lf  ",hvcptr->xx[ii].vRamp, hvcptr->xx[ii].downRamp);
      printf ("%6.1lf  ",hvcptr->xx[ii].vRamp);
      printf("\n");
    }
    if (hvcptr->xx[ii].onoff == 1 && hvcptr->xx[ii].type == 1) {
      printf ("%14s  %6.1lf  %6.1lf  %6.1lf  %6.1lf     %i   ",
	      hvcptr->xx[ii].name,hvcptr->xx[ii].vSet,hvcptr->xx[ii].vMeas,hvcptr->xx[ii].iSet,hvcptr->xx[ii].iMeas,hvcptr->xx[ii].onoff);
      printf ("%6.1lf %6.1lf  ",hvcptr->xx[ii].vRamp, hvcptr->xx[ii].downRamp);
      printf ("%6.1lf %6.1lf %6.1lf  ",hvcptr->xx[ii].vMax, hvcptr->xx[ii].v1Set,hvcptr->xx[ii].i1Set);
      printf ("%4.1lf  %3i  %3i ",hvcptr->xx[ii].trip, hvcptr->xx[ii].intTrip,hvcptr->xx[ii].extTrip);
      printf ("   %i     %i  ",hvcptr->xx[ii].kill, hvcptr->xx[ii].powUp);
      printf("\n");
    }
  }
  printf ("--------------------------------------------------------------------------------------------------------------------------\n");
  
  return;
}

/******************************************************************/
void fprintOut() {
  int ii=0;
  //  time_t curtime;
/*
  union int2int4 {
    time_t ct;
    unsigned short int ust[4];
  } x;

  x.ct = degptr->tim.time1;
*/
//  curtime = time(NULL);
//  fprintf (fileSave,"    IP address       Type Slot Chan      Name       vSet    iSet   uRamp   dRamp  SVMax  V1Set  I1Set   Trp  ITrp eTrp kill  powUp \n");
//  fprintf (fileSave,"                                                     (V)   (uA-A)  (V/s)   (V/s)   (V)    (V)   (uA-A)  (s)                        \n");
//  fprintf (fileSave,"-------------------- ---- ---- ---- -------------- ------  ------  ------ ------  ------ ------ ------  ---- ---- ---- ----  ----- \n");

  fprintf (fileSave," Type Slot Chan      Name       vSet    iSet   uRamp   dRamp  SVMax  V1Set  I1Set   Trp  ITrp eTrp kill  powUp \n");
  fprintf (fileSave,"                                 (V)   (uA-A)  (V/s)   (V/s)   (V)    (V)   (uA-A)  (s)                        \n");
  fprintf (fileSave," ---- ---- ---- -------------- ------  ------  ------ ------  ------ ------ ------  ---- ---- ---- ----  ----- \n");
  for (ii=0; ii<hvcptr->maxchan; ii++){
    if (ii == 0) fprintf (fileSave,"%20s \n",hvcptr->xx[ii].ip);
    if (hvcptr->xx[ii].onoff == 1 && hvcptr->xx[ii].type == 1) {
      //      fprintf (fileSave,"%20s %3i  %3i  %3i ",hvcptr->xx[ii].ip,hvcptr->xx[ii].type,hvcptr->xx[ii].slot,hvcptr->xx[ii].chan);
      fprintf (fileSave," %3i  %3i  %3i ",hvcptr->xx[ii].type,hvcptr->xx[ii].slot,hvcptr->xx[ii].chan);
      fprintf (fileSave,"%14s  %6.1lf  %6.1lf  ",hvcptr->xx[ii].name,hvcptr->xx[ii].vSet,hvcptr->xx[ii].iSet);
      fprintf (fileSave,"%6.1lf %6.1lf  ",hvcptr->xx[ii].vRamp, hvcptr->xx[ii].downRamp);
      fprintf (fileSave,"%6.1lf %6.1lf %6.1lf  ",hvcptr->xx[ii].vMax, hvcptr->xx[ii].v1Set,hvcptr->xx[ii].i1Set);
      fprintf (fileSave,"%4.1lf %3i  %3i ",hvcptr->xx[ii].trip, hvcptr->xx[ii].intTrip,hvcptr->xx[ii].extTrip);
      fprintf (fileSave,"   %i     %i  ",hvcptr->xx[ii].kill, hvcptr->xx[ii].powUp);
      fprintf(fileSave,"\n");
    }
  }
  //  fprintf (fileSave,"-1  -1                                               (V)   (uA-A)  (V/s)   (V/s)   (V)    (V)   (uA-A)  (s)                        \n");
  fprintf (fileSave,"-1  -1                           (V)   (uA-A)  (V/s)   (V/s)   (V)    (V)   (uA-A)  (s)                        \n");
  //  fprintf (fileSave,"-1  -1              (V)     (V)    (uA)     (uA)  (bool)   (V/s)  (V/s)   (V)    (V)    (uA)   (s)            \n");
  fprintf (fileSave,"-------------------------------------------------------------------------------------------------------------------------------------\n");
  
  return;
}

/**************************************************************/
void printActive() {
  int ii=0, tt=0;

  tt = hvcptr->maxchan/5;
  for (ii=0; ii<tt; ii++){
    printf("%4i - %10s  ",ii,hvcptr->xx[ii].name);
    printf("%4i - %10s  ",ii+tt,hvcptr->xx[ii+tt].name);
    printf("%4i - %10s  ",ii+(2*tt),hvcptr->xx[ii+(2*tt)].name);
    printf("%4i - %10s  ",ii+(3*tt),hvcptr->xx[ii+(3*tt)].name);
    printf("%4i - %10s\n",ii+(4*tt),hvcptr->xx[ii+(4*tt)].name);
  }
/*      
    if (hvcptr->xx[ii].onoff)        printf("%4i - %10s  ",ii,hvcptr->xx[ii].name);
    else printf("     -             ");
    if (hvcptr->xx[ii+tt].onoff)     printf("%4i - %10s  ",ii+tt,hvcptr->xx[ii+tt].name);
    else printf("     -             ");
    if (hvcptr->xx[ii+(2*tt)].onoff) printf("%4i - %10s  ",ii+(2*tt),hvcptr->xx[ii+(2*tt)].name);
    else printf("     -             ");
    if (hvcptr->xx[ii+(3*tt)].onoff) printf("%4i - %10s  ",ii+(3*tt),hvcptr->xx[ii+(3*tt)].name);
    else printf("     -             ");
    if (hvcptr->xx[ii+(4*tt)].onoff) printf("%4i - %10s\n",ii+(4*tt),hvcptr->xx[ii+(4*tt)].name);
    else printf("     -             \n");
  }
*/
  for (ii=5*tt; ii<hvcptr->maxchan; ii++){
    printf("%4i - %10s  ",ii,hvcptr->xx[ii].name);
    //    if (hvcptr->xx[ii].onoff) printf("%4i - %10s  ",ii,hvcptr->xx[ii].name);
    //    else printf("     -             ");
  }
  printf("\n");
  return;
}

/**************************************************************/
void printDeactive() {
  int ii=0, tt=0;

  printf("Available channels...\n");
  for (tt=0; tt<16; tt++) {
    if ((int) hvcptr->caenCrate[tt] >= 0) {
      printf("Slot %2i occupied ... available channels ",tt);
      for (ii=0;ii<24;ii++) {
	if (((hvcptr->caenCrate[tt] & (int) pow(2,ii)) >> ii)== 0) printf(" %2i",ii);
      }
    printf ("\n");
    }
  }
  /*
  tt = hvcptr->maxchan/5;

  for (ii=0; ii<tt; ii++){ 
    if (!hvcptr->xx[ii].onoff)        printf("%4i - %10s  ",ii,hvcptr->xx[ii].name);
    else printf("     -             ");
    if (!hvcptr->xx[ii+tt].onoff)     printf("%4i - %10s  ",ii+tt,hvcptr->xx[ii+tt].name);
    else printf("     -             ");
    if (!hvcptr->xx[ii+(2*tt)].onoff) printf("%4i - %10s  ",ii+(2*tt),hvcptr->xx[ii+(2*tt)].name);
    else printf("     -             ");
    if (!hvcptr->xx[ii+(3*tt)].onoff) printf("%4i - %10s  ",ii+(3*tt),hvcptr->xx[ii+(3*tt)].name);
    else printf("     -             ");
    if (!hvcptr->xx[ii+(4*tt)].onoff) printf("%4i - %10s\n",ii+(4*tt),hvcptr->xx[ii+(4*tt)].name);
    else printf("     -             \n");
  }
  for (ii=5*tt; ii<hvcptr->maxchan; ii++){ 
    if (!hvcptr->xx[ii].onoff) printf("%4i - %10s  ",ii,hvcptr->xx[ii].name);
    else printf("     -             ");
  }
  printf("\n");
  */
  return;
}

/******************************************************************/
void menu1(){
  int ii=0;
  int jj=0,kk=0;

  jj = stats[4]+stats[5]+stats[7]+stats[13];
  kk = stats[6]+stats[8]+stats[9];
  // ii = 0;
  printf ("---------------------------------------------------------------------------------------\n");
  printf ("     Global          |        Channels          |  Detector Status (bits set)  \n");
  printf ("---------------------------------------------------------------------------------------\n");
  printf ("1 - Status           | 11 - Clone channel       | %4i  On (0)          \n",stats[0]);
  printf ("2 - Force read       | 12 -                     | %4i  Ramp (1,2)      \n",stats[1]+stats[2]);
  printf ("3 -                  | 13 -                     | %4i  Verr (4,5,7,13) \n",jj);
  printf ("4 - HV on            | 14 - HV on               | %4i  Ierr (3)        \n",stats[3]);
  printf ("5 - HV off           | 15 - HV off              | %4i  Trips (6,8,9)   \n",kk);
  printf ("6 - Save conf file   | 16 - Alter parameters    | %4i  Calib (10)      \n",stats[10]);
  printf ("7 - Recover hardware | 17 - Read from file      | %4i  Fail (11,14)    \n",stats[11]+stats[14]);
  printf ("8 - Known det list   | 18 -                     | %4i  Temp (15)       \n",stats[15]);
  printf ("9 - Unknown det list | 19 -                     |  \n");
  printf ("---------------------------------------------------------------------------------------\n");
  printf ("18 - reset alarms?   | Time up - %li s            \n",hvcptr->secRunning);
  printf ("---------------------------------------------------------------------------------------\n");
  printf ("CAEN temps C: ");
  for (ii=0;ii<16;ii++) if (hvcptr->caenSlot[ii] > 0) printf ("%2.f  ",hvcptr->caenTemp[ii]);
  printf("\n");
  printf ("---------------------------------------------------------------------------------------\n");
  printf ("0 - end this program | 100 - end all Hvcaen     | \n");
  printf ("---------------------------------------------------------------------------------------\n");

return;
}

/***********************************************************/
void openSaveFile(){

  long int ii=0, jj=0, kk=0;
  char file_name[200]="hvcaen-", tt[50]="\0";
/*
    Build file name out of date and time
*/
  sprintf(tt,"%s%c",GetDate(),'\0');
  jj=0;
  jj = strlen(tt);
  ii=0;
  kk=0;
  while (ii < jj) {
    if (isspace(tt[ii]) == 0) tt[kk++] = tt[ii];
    ii++;
  }
  tt[kk]='\0';
  if (jj > 0) strcat(file_name,tt);
  strcat(file_name,".conf\0");
/*
    Open file
*/
 if (( fileSave = fopen (file_name,"a+") ) == NULL){
   printf ("*** File on disk could not be opened \n");
   exit (EXIT_FAILURE);
 }
  printf("hvcaen new config file opened: %s\n",file_name);             

  return;
}

/***********************************************************/
char *GetDate() {
  
  struct tm *timer;                                     // found in <time.h>
  time_t    myClock;                                    // found in <time.h>
  time((time_t *) &myClock);                            // get the current time
  tzset();                                              // put time in our timezone
  timer = (struct tm *) localtime((time_t *) &myClock); // convert time to local time

  return (char *) asctime(timer);                       // return it formatted
}


/******************************************************************************/
/***********************************************************/
void clone(){
  int ii=0, jj=0, kk=0, mm=0;

  printActive();
  printf("Clone from detector number ? e.g. 89 ( < 0 to quit)\n");
  ii = scan2int ();
  if (ii < 0) return;
  printDeactive();
  printf("to slot ? e.g. 2 ( < 0 to quit) \n");
  jj = scan2int ();
  if (jj < 0) return;
  printf("to channel ? e.g. 21 ( < 0 to quit) \n");
  kk = scan2int ();
  if (kk < 0) return;

  printf("Will you want detector %i removed from the set-up (HV off) ? 1 = yes 0 = no \n",ii);
  mm = scan2int ();

  if (mm == 1) {
    printf("Changing slot and channel assignment to detector %i \n",ii);
    //    hvOff(ii);
    hvcptr->caenCrate[hvcptr->xx[ii].slot] -= pow(2,hvcptr->xx[ii].chan);  // remove old occupied channel bit
    hvcptr->xx[ii].slot = jj;                                            // set new slot
    hvcptr->xx[ii].chan = kk;                                            // set new channel
    hvcptr->caenCrate[jj] += pow(2,kk);                                  // add new occupied channel bit
    printf("Go to channel mode to turn on HV \n");                      // maxchan does not change
    }
  else if (mm == 0) {
    hvcptr->xx[hvcptr->maxchan] = hvcptr->xx[ii];                          // transfer the data
    hvcptr->xx[hvcptr->maxchan].slot = jj;                                // correct to new slot
    hvcptr->xx[hvcptr->maxchan].chan = kk;                                // correct new channel
    hvcptr->caenCrate[jj] += pow(2,kk);                                  // add new occupied channel bit
    hvcptr->maxchan++;                                                   // increment maxchan
    printf("Go to channel mode to turn on HV and change appropriate values such as detector name \n");
  }
  else
    printf("Neither 0 nor 1...doing nothing.. \n");
  
  return;
}

/******************************************************************************/
void detParam() {
  int ii=0, jj=0, kk=0;
  float yy=0.0;
  char xname[30]="\0";

  printf ("Which detector number to change ?\n");
  ii = scan2int ();
      
  //       12345678901234  123456  123456  123456  123456  123456
  printf ("     Name        vSet    vMeas   iSet    iMeas  On/Off   uRamp  dRamp  SVMax  V1Set  I1Set   Trp  ITrp eTrp kill  powUp\n");
  printf ("                  (V)     (V)    (uA)     (uA)  (bool)   (V/s)  (V/s)   (V)    (V)    (uA)   (s)                       \n");
  printf ("--------------  ------  ------  ------  ------  ------  ------ ------ ------ ------ ------  ----  ---- ---- ----  -----\n");
  printf ("%14s  %6.1lf  %6.1lf  %6.1lf  %6.1lf     %i   ",
	  hvcptr->xx[ii].name,hvcptr->xx[ii].vSet,hvcptr->xx[ii].vMeas,hvcptr->xx[ii].iSet,hvcptr->xx[ii].iMeas,hvcptr->xx[ii].onoff);
  printf ("%6.1lf %6.1lf  ",hvcptr->xx[ii].vRamp, hvcptr->xx[ii].downRamp);
  printf ("%6.1lf %6.1lf %6.1lf  ",hvcptr->xx[ii].vMax, hvcptr->xx[ii].v1Set,hvcptr->xx[ii].i1Set);
  printf ("%4.1lf  %3i  %3i ",hvcptr->xx[ii].trip, hvcptr->xx[ii].intTrip,hvcptr->xx[ii].extTrip);
  printf ("   %i     %i  ",hvcptr->xx[ii].kill, hvcptr->xx[ii].powUp);
  printf("\n");
  printf ("-----------------------------------------------------------------------------------------------------------------------\n");

  printf ("Type number of quantity to change:  0 = nothing   \n");
  printf (" 1 - Voltage setting  |  5 - Voltage 1 setting   |  9 - Trip          | 13 - HV on power up  \n");
  printf (" 2 - Current setting  |  6 - Current 1 setting   | 10 - Trip internal | 14 - Name  \n");
  printf (" 3 - HV on/off        |  7 - SV Maximum setting  | 11 - Trip external | 15 -  \n");
  printf (" 4 - Voltage ramp up  |  8 - Voltage ramp down   | 12 - Kill or Ramp  | 16 -  \n");

  jj = scan2int ();
  if (jj < 1 || jj > 14) return;

  switch (jj){

  case 1:
    printf ("What voltage setting? < %6.1lf V \n",hvcptr->xx[ii].vMax);
    yy = scan2float();
    if (yy <= hvcptr->xx[ii].vMax) hvcptr->xcom3 = yy;
    else hvcptr->xcom3 = hvcptr->xx[ii].vSet;
    break;
  case 2:
    printf ("What current limit setting?  (uA) (5000 uA limit)\n");
    yy = scan2float();
    if (yy <= 5000) hvcptr->xcom3 = yy;
    else hvcptr->xcom3 = hvcptr->xx[ii].iSet;
    break;
  case 3:
    if (hvcptr->xx[ii].onoff == 1) {
      printf ("Turn HV off ? 1 = yes \n");
      kk = scan2int();
      if (kk == 1) hvcptr->com3 = 0;
    }
    else if (hvcptr->xx[ii].onoff == 0) {
      printf ("Turn HV on ? 1 = yes \n");
      kk = scan2int();
      if (kk == 1)  hvcptr->com3 = 1;
    }
    break;
  case 4:
    printf ("What voltage ramp-up setting? < 50 V/s \n");
    yy = scan2float();
    if (yy <= 50.0) hvcptr->xcom3 = yy;
    else hvcptr->xcom3 = hvcptr->xx[ii].vRamp;
    break;
  case 5:
    printf ("What Voltage 1 setting? < %6.1lf V \n",hvcptr->xx[ii].vMax);
    yy = scan2float();
    if (yy <= hvcptr->xx[ii].vMax) hvcptr->xcom3 = yy;
    else hvcptr->xcom3 = hvcptr->xx[ii].v1Set;
    break;
  case 6:
    printf ("What current 1 limit setting?  (uA) \n");
    yy = scan2float();
    if (yy <= hvcptr->xx[ii].i1Set) hvcptr->xcom3 = yy;
    else hvcptr->xcom3 = hvcptr->xx[ii].i1Set;
    break;
  case 7:
    printf ("What maximum voltage setting? < 3550 V \n");
    yy = scan2float();
    if (yy <= 3550.0) hvcptr->xcom3 = yy;
    else hvcptr->xcom3 = hvcptr->xx[ii].vMax;
    break;
  case 8:
    printf ("What voltage ramp-down setting? < 50 V/s \n");
    yy = scan2float();
    if (yy <= 50.0) hvcptr->xcom3 = yy;
    else hvcptr->xcom3 = hvcptr->xx[ii].downRamp;
    break;
  case 9:
    printf ("What trip setting? < 11 s \n");
    yy = scan2float();
    if (yy <= 10.0) hvcptr->xcom3 = yy;
    else hvcptr->xcom3 = hvcptr->xx[ii].trip;
    break;
   case 10:
    printf ("What is internal trip setting \n");
    kk = scan2int();
    if (kk <= 2) hvcptr->com3 = kk;
    else hvcptr->com3 = hvcptr->xx[ii].intTrip;
    break;
   case 11:
    printf ("What is external trip setting \n");
    kk = scan2int();
    if (kk <= 17) hvcptr->com3 = kk;
    else hvcptr->com3 = hvcptr->xx[ii].extTrip;
    break;
  
   case 12:
    printf ("Kill HV or ramp down on trip ? 0/1  (-1=do nothing)\n");
    kk = scan2int();
    if (kk == 0 || kk == 1) hvcptr->com3 = kk;
    else hvcptr->com3 = hvcptr->xx[ii].kill;
    break;

   case 13:
    printf ("What is  power on setting? 0/1 (-1=do nothing) \n");
    kk = scan2int();
    if (kk == 0 || kk == 1) hvcptr->com3 = kk;
    else hvcptr->com3 = hvcptr->xx[ii].powUp;
    break;

   case 14:
    printf ("What is new name? (less than 12 letters no spaces) \n");
    scanf("%s",xname);
    if (strlen(xname) < 12) strcpy(hvcptr->xx[ii].name,xname);
    break;

  default:
    ii=0;
    jj=0;
    break;
  }
  
  hvcptr->com1 = ii;  // send detector to control program
  hvcptr->com2 = jj;  // function to change by control program
  //   com3 and xcom3 are new values returned to the control program

  /*
  char V0Set[30]="V0Set\0";       //   V0Set  
  char I0Set[30]="I0Set\0";       //   I0Set  
  char VMon[30]="VMon\0";         //   VMon
  char IMon[30]="IMon\0";         //   IMon 
  char Pw[30]="Pw\0";             //   PW
  char V1Set[30]="V1Set\0";       //   V1Set
  char I1Set[30]="I1Set\0";       //   I1Set
  char SVMax[30]="SVMax\0";       //   V0Set  I0Set  IMon VMon
  char Trip[30]="Trip\0";         //   SVMax
  char RUp[30]="RUp\0";           //   RUp
  char RDWn[30]="RDWn\0";         //   RDWn
  char TripInt[30]="TripInt\0";   // TripInt
  char TripExt[30]="TripExt\0";   // TripExt
  */
      
  return;
}

/******************************************************************************/
void statusStats(){
  int ii=0, jj=0;
/*
  Run thru the bits to see how many are set
*/
  for (jj=0;jj<16;jj++)stats[jj]=0;   // zero all stats

  for (ii=0;ii< hvcptr->maxchan;ii++){
    for (jj=0;jj<16;jj++){
      if (hvcptr->xx[ii].status & (int)pow(2,jj))stats[jj]++;
    }
  }

  return;
}

/******************************************************************************/


/******************************************************************************/


/******************************************************************************/


/******************************************************************************/

