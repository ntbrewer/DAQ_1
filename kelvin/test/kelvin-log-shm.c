/*********************************************************************/
/* Program deg-u3-log is used to write a log file of the             */
/*    LabJack U3-HV or U3-LV temperatures near the experiment        */
/*                                                                   */
/* To be compiled with:                                              */
/*    gcc -Wall -o kelvin-log kelvin-log.c                           */
/*                                                                   */
/* C. J. Gross, February 2015                                        */
/*********************************************************************/

#include "../include/u3.h"        // location may also be: 
#include "../include/kelvin-shm.h" 
//#include "kelvin-shm.h"

#define INTERVAL 60;

//int  mmapSetup();
void shmSetup();
void printoutHead();
void printoutBody();
void openLogFile();
char *GetDate();

FILE *fileTherm;

/***********************************************************/
int main(int argc, char **argv){

  time_t time0=0;
//  int mapKelvin=0;
/*
  Shared memory creation and attachment
  the segment number is stored in lnptr->pid
*/
//  mapKelvin = mmapSetup();
  shmSetup();
  openLogFile();                  // open log file...based on name
  printoutHead();                 // print file headings in first 2 lines
  time0 = degptr->tim.time1;      // load time based on what is in shared memory

/*
  Set logging flag on and print out thermometer data to file
*/
  degptr->com2 = 1;               // set daq logging flag on (writing to SHM from this program at start/stop only)

  while (degptr->com2 == 1){
    if (degptr->tim.time1 >= time0) {
      printoutBody();
      time0 += INTERVAL;           // keep the time internal to this program..ie, INTERVAL s 
    }
    sleep(5);
  }
  fclose(fileTherm);              // close file on disk
/*
   Wrap up the program ending the lnfill-u6, detaching and getting rid of the shared memory segment
*/  
  degptr->com2 = 1;               // set daq logging flag off (writing to SHM from this program at start/stop only)

  //if (munmap(degptr, sizeof (struct thermometer*)) == -1) {
  //  perror("Error un-mmapping the file");
    /* Decide here whether to close(fd) and exit() or not. Depends... */
  //}
    
  /* Un-mmaping doesn't close the file, so we still need to do that.
   */
  //close(mapKelvin);

  return 0;
}
/**************************************************************/
/*int mmapSetup() {
  int fd=0;   // mapped file descriptor

  * Open a file for writing.
   *  - Creating the file if it doesn't exist.
   *  - Truncating it to 0 size if it already exists. (not really needed)
   *
   * Note: "O_WRONLY" mode is not sufficient when mmaping.
   *
  fd = open(KELVINDATAPATH, O_RDWR, (mode_t)0600);
  if (fd == -1) {
    perror("Error opening file path for writing");
    exit(EXIT_FAILURE);
  }
    
  * Now the file is ready to be mmapped.
   *
  degptr = (struct thermometer*) mmap(0, KELVINDATASIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (degptr == MAP_FAILED) {
    close(fd);
    perror("Error mmapping the file");
    exit(EXIT_FAILURE);
  }
  * Don't forget to free the mmapped memory usually at end of main
   *
    
  return (fd);
}
*/
/**************************************************************/
void shmSetup() {
  //   file is probably: include/deg.conf
  printf("Setting up shared memory...\n");

  shmKey = ftok(KELVINCONF,'b');                // key unique identifier for shared memory, other programs use 'LN' tag
  //  shmKey = ftok("include/deg.conf",'b');  // key unique identifier for shared memory, other programs use 'LN' tag

  shmid = shmget(shmKey, sizeof (struct thermometer), 0666 | IPC_CREAT);   // get ID of shared memory, size, permissions, create if necessary
  if (shmid == -1){
    printf("shmid=%li, shmKey=%i, size=%u\n",
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
/******************************************************************************/
void openLogFile(){

  long int ii=0, jj=0, kk=0;
  char file_name[200]="log/therm-", tt[50]="\0";
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
 if (( fileTherm = fopen (file_name,"a+") ) == NULL){
   printf ("*** File on disk could not be opened \n");
   exit (EXIT_FAILURE);
 }
  printf("Temperature logfile opened: %s\n",file_name);             

  return;
}

/******************************************************************************/
void printoutHead() {
  long int ii=0;
/*
  Write to file
*/
  fprintf (fileTherm,"---------------------------------------------------\n");
  fprintf (fileTherm,"  Seconds  ");
  for (ii=0; ii<degptr->maxchan; ii++){
    fprintf (fileTherm,"   %s    ",degptr->temps[ii].name);
  }
  fprintf (fileTherm,"\n---------------------------------------------------\n");
  fflush(fileTherm);
/*
  Write to screen
*/
  printf ("---------------------------------------------------\n");
  printf ("  Seconds");
  for (ii=0; ii<degptr->maxchan; ii++){
    printf ("\t%s",degptr->temps[ii].name);
  }
  printf ("\n---------------------------------------------------\n");

  return;
}


/******************************************************************************/
void printoutBody() {
  long int ii=0;
/*
  Write to file
*/
  fprintf (fileTherm," %8li    ",(degptr->tim.time1 - degptr->time0));
  for (ii=0; ii<degptr->maxchan; ii++){
    fprintf (fileTherm,"\t%0.1lf",degptr->temps[ii].degree);
  }
  fprintf (fileTherm,"\n");
  fflush(fileTherm);
/*
  Write to screen
*/
  printf (" %8li ",(degptr->tim.time1 - degptr->time0));
  for (ii=0; ii<degptr->maxchan; ii++){
    printf ("   %0.1lf  ",degptr->temps[ii].degree);
  }
  printf ("\n");

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
