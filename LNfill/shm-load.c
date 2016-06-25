/*
  Program lnfill-u6 to handle LN filling and monitoring of Ge
  when RMS system is not available.

  Will put relevant data into shared memory where other programs
  can access it and decide actions such as turning off HV.

  Program will be based on LabJack U6 USB daq, dataport probe
  power strip with remote relay control.

*/
#include <unistd.h>  /* UNIX standard function definitions */
#include <time.h>    /* Time definitions */
//#include <gtk/gtk.h>
//#include <cairo.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <ctype.h>
#include <math.h>    /* Math definitions */
#include <signal.h>  /* Signal interrupt definitions */
//#include <stddef.h>
//#include <limits.h>
//#include <float.h>
//#include <termios.h> /* POSIX terminal control definitions */
//#include <errno.h>   /* Error number definitions */
//#include <fcntl.h>   /* File control definitions */
#include <sys/types.h> /* for shared memory stuff */
#include <sys/ipc.h>   /* for shared memory stuff */
#include <sys/shm.h>   /* for shared memory stuff */

/***********************************************************/
int main(int argc, char **argv){
  key_t shmKey;
  int shmid;
  char *LNdata;

  time_t curtime = -1;
  char xtime[40]="\0";
  long int ii=0;
  long int size=4194304; //   = 4 MB // 131072;  //65536;

  struct clover {
    int onoff;    //0=off, 1=on
    char name[10];
    double rtd;
    double filltime;
    double filltimemax;
    double filltimemin;
    double filllast;
    double fillnext;
  } ge[20];
  
  struct clover *geptr; 

  struct tank {
    double pressure;
    double rtd;
    double cooltime;
    double timeout;
  } ln; 

  long int geptr_offset, ge_offset, shm_offset;

/*
  Shared memory creation and attachment
*/

  shmKey = ftok("/Users/c4g/src/LNdata",'b');      // key unique identifier for shared memory, other programs use 'LN' tag
  shmid = shmget(shmKey, size, 0666 | IPC_CREAT);  // gets the ID of shared memory, size, permissions, create if necessary
  geptr = shmat (shmid, (void *)0, 0);             // now link to area so it can be used; defined to be characters by pointer char *LNdata
  if (geptr == (struct clover *)(-1)){             // check for errors
    perror("shmat");
    exit;
  }
  
   geptr_offset = sizeof (struct clover);
   ge_offset = sizeof ge;
   shm_offset = sizeof geptr;
   printf("\n geptr size= %li, ge size = %li, shm size = %li \n",geptr_offset,ge_offset,shm_offset);

   //  geptr = ge.onoff;
  //geptr = (struct clover *)ge;

  geptr[0].rtd = 80.2;
  geptr[1].rtd = 90.2;
  geptr[2].rtd = 100.2;
  //  memcpy(geptr,ge,sizeof(ge[20]));

  for (ii=0; ii< 20; ii++){
    geptr[ii].rtd = geptr[0].rtd + (double) ii;
    //    geptr[1].rtd = geptr[1].rtd + (double) ii;
    //    geptr[2].rtd = geptr[2].rtd + (double) ii;
    //    ge[1].rtd = ge[1].rtd + (double) ii;
    //    ge[2].rtd = ge[2].rtd + (double) ii;

    printf ("ii=%li, rtd[0] = %.2lf, rtd[1] = %.2lf; rtd[2] = %.2lf\n", ii, geptr[0].rtd, geptr[1].rtd, geptr[2].rtd);
    //    printf ("rtd[0] = %.2lf, rtd[1] = %.2lf; rtd[2] = %.2lf\n\n",ge[0].rtd, ge[1].rtd, ge[2].rtd);
    //    printf ("rtd[0] = %.2lf, rtd[1] = %.2lf; rtd[2] = %.2lf\n",geptr[0].rtd, ge[1].rtd, geptr[2].rtd);

    sleep (1);
  }


  shmdt(geptr);                     // detach from shared memory segment
  //  shmctl(shmid, IPC_RMID, NULL);     // remove the shared memory segment hopefully forever

  return 0;
}
