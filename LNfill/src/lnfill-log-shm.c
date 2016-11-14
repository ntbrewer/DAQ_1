/*********************************************************************/
/* Program lnfill-log is used to write a log file of the             */
/*    LabJack temperatures and fill status from the experiment       */
/*                                                                   */
/*                                                                   */
/* NT Brewer, October  2016                                          */
/*********************************************************************/

//#include "../include/u6.h"        // location may also be: 
#include "../../include/lnfill-shm.h" 

#define INTERVAL 300

//int  mmapSetup();
void printoutHead();
void printoutBody();
void openLogFile();
char *GetDate();
long int GetTime();

FILE *fileLN;
time_t time0 = 0, time1 = 0;
//long int activemax=0;

/***********************************************************/
int main(int argc, char **argv){

  int mapLNfill, ii=0;
/*
  Shared memory creation and attachment
  the segment number is stored in lnptr->pid
*/
  shmKey = ftok("../conf/lnfill.conf",'b');  // key unique identifier for shared memory, other programs use include this
  shmid = shmget(shmKey, sizeof (struct lnfill), 0666); // gets the ID of shared memory, size, permissions, create if necessary by changing to  0666 | IPC_CREAT)
  lnptr = shmat (shmid, (void *)0, 0);                              // now link to area so it can be used; struct lnfill pointer char *lnptr
  if (lnptr == (struct lnfill *)(-1)){                              // check for errors
    perror("shmat");
    exit(EXIT_FAILURE);
  }
  
  openLogFile();                  // open log file...based on name
  printoutHead();                 // print file headings in first 2 lines
  //time0 = lnptr->secRunning;      // load time based on what is in shared memory
  time0 = GetTime();      // load time based on what is in shared memory
//    printf("%li time  ",time1);
/*
  Set logging flag on and print out data to file
*/
  //degptr->com2 = 1;               // set daq logging flag on (writing to SHM from this program at start/stop only)
  lnptr->com2 = 5;               // set daq logging flag on (writing to SHM from this program at start/stop only) CAREFUL! com2=1 is shutdown using 5/6 for logging.
  while (lnptr->com2 == 5 ){
    time1 = GetTime()-time0;
    if (time1 % INTERVAL == 0) {
      printoutBody();
//      time0 += INTERVAL;           // keep the time internal to this program..ie, INTERVAL s 
    }
    sleep(1);
  }
  fclose(fileLN);              // close file on disk
/*
   Wrap up the program ending the lnfill-log
*/  
  lnptr->com2 = 6;               // set daq logging flag off (writing to SHM from this program at start/stop only)
  kill(lnptr->pid,SIGALRM);

  shmdt(lnptr); 
  /* Un-mmaping doesn't close the file, so we still need to do that.
   */
  close(mapLNfill);

  return 0;
}
/**************************************************************/
/*int mmapSetup() {
  int fd=0;//, result=0, ii=0;   // mapped file descriptor

  /* Open a file for writing.
   *  - Creating the file if it doesn't exist.
   *  - Truncating it to 0 size if it already exists. (not really needed)
   *
   * Note: "O_WRONLY" mode is not sufficient when mmaping.
   
 //fd = open(LNFILLDATAPATH, O_RDWR, (mode_t)0600);
  fd = open(LNFILLDATAPATH, O_RDWR , (mode_t)0600);
  if (fd == -1) {
    perror("Error opening file path for writing");
    exit(EXIT_FAILURE);
  }
  printf("%s dp, %i fd", LNFILLDATAPATH, fd);  
  // Now the file is ready to be mmapped.
  lnptr = (struct lnfill*) mmap(0, LNFILLDATASIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  //printf("%i pid", lnptr->pid);
  if (lnptr == MAP_FAILED) {
    close(fd);
    perror("Error mmapping the file");
    exit(EXIT_FAILURE);
  }
  // Don't forget to free the mmapped memory usually at end of main
    
  return (fd);
}*/

/******************************************************************************/
void openLogFile(){

  long int ii=0, jj=0, kk=0;
  char file_name[200]="log/lnfill-", tt[50]="\0";
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
  strcat(file_name,".log\0");
/*
    Open file
*/
 if (( fileLN = fopen (file_name,"a+") ) == NULL){
   printf ("*** File on disk could not be opened \n");
   exit (EXIT_FAILURE);
 }
  printf("LNfill logfile opened: %s\n",file_name);             

  return;
}

/******************************************************************************/
void printoutHead() {
 // long int ii=0;
/*
  Write to file
*/
  fprintf (fileLN,"---------------------------------------------------\n");
  fprintf (fileLN,"  Seconds    Name    Temperature  Status: Ge,Tank,Com    Time up");
  fprintf (fileLN,"\n-----------------------------------------------------\n");
  fflush(fileLN);
/*
  Write to screen
*/
  printf ("--------------------------------------------------------\n");
  printf ("  Seconds    Name    Temperature  Status: Ge,Fill,Com    Time up");
  printf ("\n--------------------------------------------------------\n");

  return;
}


/******************************************************************************/
void printoutBody() {
  long int ii=0;
/*
  Write to file
*/
  for (ii = 0; ii < 20; ii++){
    if (lnptr->ge[ii].onoff == 1){
      fprintf (fileLN,"%8li      %s        %0.1lf      %s  %s  %s   %8li\n",time1,lnptr->ge[ii].name,lnptr->ge[ii].rtd,
						lnptr->ge[ii].status,lnptr->tank.status,lnptr->comStatus,lnptr->secRunning);
    }
  }
  fflush(fileLN);
/*
  Write to screen
*/
//  printf (" %8li ",(degptr->tim.time1 - degptr->time0));
  for (ii = 0; ii <20; ii++){
    if (lnptr->ge[ii].onoff == 1){
      printf ("%8li      %s        %0.1lf      %s  %s  %s   %8li\n",time1,lnptr->ge[ii].name,lnptr->ge[ii].rtd,
						lnptr->ge[ii].status,lnptr->tank.status,lnptr->comStatus,lnptr->secRunning);
    }
  }

  return;
}

/**************************************************************/

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
long int GetTime() {
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

    return (long int ) myClock;
}
