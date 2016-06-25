#define _POSIX_C_SOURCE 200809L
#include <unistd.h>  /* UNIX standard function definitions */
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>    /* Time definitions */

#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <ctype.h>


/***********************************************************/
int main(int argc, char **argv){
  /*
  struct timeval{
    time_t   tv_sec;
    suseconds_t tv_nsec;
  } xt;
*/
  struct timespec xt;
  //  int clock_gettime(clockid_t clk_id, struct timespec *xt);
  double t0,t1;
  //  char yn;
  int ans=0;
  scanf ("%i", &ans);          // read in ans (pointer is indicated
  printf ("ans int %i \n",ans);
  
  char yn = getchar();          // need another getchar to get new line from previous input
  if (yn == 'Y' || yn == 'y'){
    printf("success\n");
    
  } else printf ("nope\n");

  //  gettimeofday();
  clock_gettime(CLOCK_MONOTONIC,&xt);
  t0 = (double) xt.tv_sec + (double) xt.tv_nsec/1.e6;
  sleep (2);
  clock_gettime(CLOCK_REALTIME,&xt);
  //  gettimeofday();
  t1 = (double) xt.tv_sec + (double) xt.tv_nsec/1.e6;
  printf ("time diff = %.lf \n", (t1-t0));
  
  return(0);
}
