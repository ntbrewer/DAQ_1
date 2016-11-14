/*
  Program lnfill-u6 to handle LN filling and monitoring of Ge
  when RMS system is not available.

  Will put relevant data into shared memory where other programs
  can access it and decide actions such as turning off HV.

  Program will be based on LabJack U6 USB daq, dataport probe
  power strip with remote relay control.

*/

#include "../../include/lnfill-shm.h"
void updateRTD();
void inactiveList();
int activeList();
void saveSetup();
void menu();
void addresses();
void parametersGe(int ii);
void parametersTank();
//int mmapSetup();                 // sets up the memory map

/***********************************************************/
int main(int argc, char **argv){

  int ii=0, ans=0, nn=0;
  int det=0;
  char yn[100]="\0";
  int mapLNfill;
  
  //shmSetup();
//  mapLNfill = mmapSetup();
  if (mapLNfill == -1) return 0;

/*
  Shared memory creation and attachment
*/ 
//  shmKey = ftok("/Users/c4g/src/LNfill/include/lnfill.conf",'b');       // key unique identifier for shared memory, other programs use 'LN' tag
//  shmKey = ftok("/Users/c4g/src/LNfill/include/lnfill.conf",'b');  // key unique identifier for shared memory, other programs use include this
  shmKey = ftok("include/lnfill.conf",'b');  // key unique identifier for shared memory, other programs use include this
  //  shmKey = ftok("SHM_PATH",'b');                                   // key unique identifier for shared memory, other programs use 'LN' tag
  //  shmKey = ftok("/Users/gross/Desktop/LNfill/LNdata",'b');         // key unique identifier for shared memory, other programs use 'LN' tag
  shmid = shmget(shmKey, sizeof (struct lnfill), 0666); // gets the ID of shared memory, size, permissions, create if necessary by changing to  0666 | IPC_CREAT)
  lnptr = shmat (shmid, (void *)0, 0);                              // now link to area so it can be used; struct lnfill pointer char *lnptr
  if (lnptr == (struct lnfill *)(-1)){                              // check for errors
    perror("shmat");
    exit(EXIT_FAILURE);
  }


  //  printf ("%li = %li\n",shmKey, shmid);
  /*  

  */
  //  printf ("shm size = %li\n",sizeof (struct lnfill) );
  //  printf ("pid = %li\n", lnptr->pid);

  while (ans != 100){
    menu();
    scanf ("%i", &ans);          // read in ans (pointer is indicated)     
 
    switch (ans){
    case 0:                   // end program but not lnfill
      if (munmap(lnptr, sizeof (struct lnfill*)) == -1) {
	perror("Error un-mmapping the file");
      }
      //shmdt(lnptr);           // detach from shared memory segment
      return 0;
      break;

    case 1:                      // display temps and limits
      if (lnptr->command == 8 || lnptr->command == 7 || lnptr->command == 18) break;  // don't force an update
      lnptr->command = 1;        // command (3) stored in SHM so lnfill can do something
      kill(lnptr->pid,SIGALRM);  // send an alarm to let lnfill know it needs to do command (1)
      break;

    case 2:                    // Change limit and timing parameters
      if (lnptr->command == 8 || lnptr->command == 7 || lnptr->command == 18) break;  // don't force an update
      printf (" Which detector do you want to change? 1-20, outside this range to do nothing \n");
      scanf("%i",&det);
      if ((det > 0) && (det < 21)){
      parametersGe(det);
      }
      /*

	printf("Filling interval ? (usually 28800 s (8 hrs.)) \n");
	scanf ("%lf",&zz.interval);
	printf("Filling time max ? (usually 420 s) \n");
	scanf ("%lf",&zz.max);
	printf("Filling time min ? (usually 150 s) \n");
	scanf ("%lf",&zz.min);
	printf("RTD temperature limit ? (usually ~90 or 6 K above coldest value) \n");
	scanf ("%lf",&zz.limit);
	printf("Outlet temperature limit ? (usually ~90 or few degrees below value) \n");
	scanf ("%lf",&zz.limit);

	lnptr->ge[det].interval = zz.interval;
	lnptr->ge[det].max = zz.max;
	lnptr->ge[det].min = zz.min;
	lnptr->ge[det].limit = zz.limit;
	lnptr->ge[det].olimit = zz.olimit;
	lnptr->ge[det].next = time(NULL) + lnptr->ge[det].interval;

	printf("Detector next fill time has been set to %.0lf s from now \n",lnptr->ge[det].interval);

      }
      */
      break;

    case 3:                   // Add or remove detector from system
      activeList();
      break;
/*
      activemax = activeList();
      printf (" Which detector do you want to change? 1-20, outside this range to do nothing \n");
      scanf("%i",&det);
      if ((det > 0) && (det < 21)){
	if (lnptr->ge[det].onoff == 0) {
	  printf("RTD is which U6 channel? (0-13) \n");
	  scanf ("%i",&zz.chanRTD);
	  printf("Overflo is which U6 channel? (0-13) \n");
	  scanf ("%i",&zz.chanOFLO);
	  printf("Filling interval ? (usually 28800 s (8 hrs.)) \n");
	  scanf ("%i",&zz.interval);
	  //	  scanf ("%i",&zz.interval);
	  printf("Filling time max ? (usually 420 s) \n");
	  scanf ("%i",&zz.max);
	  printf("Filling time min ? (usually 150 s) \n");
	  scanf ("%i",&zz.min);
	  printf("RTD temperature limit ? (usually ~90 or 6 K above coldest value) \n");
	  scanf ("%lf",&zz.limit);
	  printf("Outlet temperature limit ? (usually ~90 or few degrees below detector value) \n");
	  scanf ("%lf",&zz.olimit);
	  printf(" %i %i %lf %lf \n", zz.chanRTD, zz.chanOFLO, zz.limit,zz.olimit);
	  zz.onoff = 1;
	  strcpy(zz.status,"OK");
	  lnptr->ge[det] = zz;
	  lnptr->ge[det].next = time(NULL) + lnptr->ge[det].interval;
	  printf("Detector next fill time has been set to %.0lf s from now \n",lnptr->ge[det].interval);

	} 
	else {
	  lnptr->ge[det].onoff = 0;       // turn channel off
	  lnptr->ge[det].chanOFLO = -1;   // set U6 channels to -1
	  lnptr->ge[det].chanRTD = -1;    // set U6 channels to -1
	  strcpy(lnptr->ge[det].status,"OFF");
	}
      }
      //      lnptr->command = 3;        // command (3) stored in SHM so lnfill can do something not needed...user can force a fill

      // do I force an RTD read here or wait up to a minute for the natural read?
      break;
*/
    case 4:                   // display temps and limits
      inactiveList();
      break;

    case 5:                   // Save setup into lnfill.conf
      printf ("Saving current setup into lnfill.conf (hope you made a copy of the old one) ....\n");
      saveSetup();
      break;

    case 7:                   // force fill 1 detector
      if (lnptr->command == 8) {
	printf("Already started a fill ALL ..... returning to menu \n");
	break;
      }
      if (lnptr->command == 7) {
	printf("Already filling a detector .... should have done a fill ALL ..... returning to menu \n");
	break;
      }
      printf ("Which detector do you wish to fill ....  <0 do nothing\n");
      printf ("Det Name  iBar  \n");
      printf ("--- ----  ----  \n");
      for (ii=1; ii < 21; ii++){
	if (lnptr->ge[ii-1].chanIbar >=0) printf ("%2i   %3s   %2i  \n",ii, lnptr->ge[ii-1].name, lnptr->ge[ii-1].chanIbar);
      }
      scanf("%i",&nn);
      if (nn < 0) break;
      lnptr->com1 = nn-1;
      lnptr->command = 8;                         // command (8) stored in SHM to start a fill
      kill(lnptr->pid,SIGALRM);                   // send an alarm to let lnfill know it needs to do command (8)
      // put in detector and value questions
      break;

    case 8:                   // force fill all detectors
      printf ("force fill ALL detectors here....\n");
      if (lnptr->command == 8 || lnptr->command == 7) {
	printf("Already started a fill ALL ..... returning to menu \n");
	break;
      }
      lnptr->com1 = -1;          // fill all flag
      lnptr->command = 8;        // command (8) stored in SHM so lnfill can do something
      kill(lnptr->pid,SIGALRM);  // send an alarm to let lnfill know it needs to do command (8)
      break;

    case 9:                   // do initial cool down of a detector
      printf ("Begin the initial cool down of detectors with next fill 1 hour....\n");
      printf ("Which detector do you wish to fill .... <0 do nothing \n");
      printf ("Det  Name  iBar  \n");
      printf ("---  ----  ----  \n");
      printf (" 0   ALL    --   \n");
      for (ii=1; ii < 21; ii++){
	if (lnptr->ge[ii-1].chanIbar >=0) printf ("%2i   %3s   %2i  \n",ii, lnptr->ge[ii-1].name, lnptr->ge[ii-1].chanIbar);
      }
      scanf("%i",&nn);
      if (nn < 0 || nn > 6) break;
      //      if (nn == 0) lnptr->com1 = -1;
      //      else 
	lnptr->com1 = nn-1;
      lnptr->command = 18;      
      kill(lnptr->pid,SIGALRM);   // send an alarm to let lnfill know it needs to do command (3)
      // put in detector and value questions
      break;

    case 10:                   // Close all valves
      printf ("Closing valves and opening manifold ....\n");
      lnptr->command = ans;  
      kill(lnptr->pid,SIGALRM);  // send an alarm to let lnfill know it needs to do command (10)
      break;

    case 11:                   // Close tanl valve
      printf ("Closing tank ....\n");
      lnptr->command = ans;  
      kill(lnptr->pid,SIGALRM);  // send an alarm to let lnfill know it needs to do command (10)
      break;

    case 12:                   // Open tank valve
      printf ("Opening tank ....\n");
      lnptr->command = ans;  
      kill(lnptr->pid,SIGALRM);  // send an alarm to let lnfill know it needs to do command (10)
      break;

    case 13:                   // Close manifold
      printf ("Closing manifold ....\n");
      lnptr->command = ans;  
      kill(lnptr->pid,SIGALRM);  // send an alarm to let lnfill know it needs to do command (10)
      break;

    case 14:                   // Open manifold
      printf ("Opening manifold ....\n");
      lnptr->command = ans;  
      kill(lnptr->pid,SIGALRM);  // send an alarm to let lnfill know it needs to do command (10)
      break;

    case 15:                   // Close a detector valve
      printf ("Which detector valve do you wish to close ? (1-20 from 1st column) \n");
      for (ii=1; ii < 21; ii++){
	if (lnptr->ge[ii-1].chanIbar >=0) printf ("%2i   %3s   %2i  \n",ii, lnptr->ge[ii-1].name, lnptr->ge[ii-1].chanIbar);
      }
      scanf("%i",&nn);
      lnptr->command = 20 + nn;
      kill(lnptr->pid,SIGALRM);  // send an alarm to let lnfill know it needs to do command (10)
      break;

    case 16:                   // Open a detector valve
      printf ("Which detector valve do you wish to open ? (1-20 from 1st column) \n");
      for (ii=1; ii < 21; ii++){
	if (lnptr->ge[ii-1].chanIbar >=0) printf ("%2i   %3s   %2i  \n",ii, lnptr->ge[ii-1].name, lnptr->ge[ii-1].chanIbar);
      }
      scanf("%i",&nn);
      lnptr->command = 40 + nn;
      kill(lnptr->pid,SIGALRM);  // send an alarm to let lnfill know it needs to do command (10)
      break;

    case 17:                    // Get valve status directly from ibootbar 
      printf ("ibootBar outlet status  \n");
      lnptr->command = 17;
      kill(lnptr->pid,SIGALRM);  // send an alarm to let lnfill know it needs to do command (10)
      break;

    case 18:                   // do initial cool down of a detector

      printf (" Do you really want to reset all alarms?  y/n \n");
      scanf("%s",yn);
      if (strcmp(yn,"Y") == 0 || strcmp(yn,"y") == 0) { 
	//      if (yn == 'Y' || yn == 'y'){
	for (ii=0; ii < 20; ii++){
	  if (lnptr->ge[det].onoff == 1) strcpy(lnptr->ge[ii].status,"OK");
	  else strcpy(lnptr->ge[ii].status,"OFF");
	}
      } else printf ("No alarms were reset");
      /*
      scanf("%1s",&zzz);
      if (strcmp(zzz,"Y") == 0 || strcmp(zzz,"y") == 0) { 
	for (ii=0; ii < 20; ii++){
	  if (lnptr->ge[det].onoff == 1) strcpy(lnptr->ge[ii].status,"OK");
	  else strcpy(lnptr->ge[ii].status,"OFF");
	}
      }
      */
      break;

    case 19:
      if (lnptr->com2 == 0) {
	printf (" Do you really want to toggle the LN HV Emergency Shutdown ON?  y/n \n");
     	scanf("%s",yn);
	if (strcmp(yn,"Y") == 0 || strcmp(yn,"y") == 0) { 
	//	if (yn == 'Y' || yn == 'y'){
	//	scanf("%1s",&zzz);
	  printf (" Turning emergency shutdown ON \n If shutdown occurs you must run bash script to clear events and shutdown!\n");
	  lnptr->com2 = 1;
	  printf (" Emergency shutdown is ON \n");
	}
	else {
	  printf (" Emergency shutdown is staying OFF \n");
	}
      }
      else {
	printf (" Do you really want to toggle the LN Emergency Shutdown OFF?  y/n \n");
     	scanf("%s",yn);
	if (strcmp(yn,"Y") == 0 || strcmp(yn,"y") == 0) { 
	//	char yn = getchar();
	  //	if (yn == 'Y' || yn == 'y'){
	  //	scanf("%1s",&zzz);
	  printf (" Turning emergency shutdown OFF \n You better know that the Ge detectors are COLD! \n And remember to turn it back on when done !\n");
	  lnptr->com2 = 0;
	  printf (" Emergency shutdown is OFF \n");
	}
	else {
	  printf (" Emergency shutdown is staying ON \n");
	}
      }
      break;
	
    case 61:                    // Get valve status directly from ibootbar 
      printf ("Input email addresses  \n");
      addresses();
      if (lnptr->command == 61) kill(lnptr->pid,SIGALRM);
      break;

    case 100:                 // End ALL lnfill programs
      break;

    default:                  // Do nothing and go back to the list of options
      ans = 0;
      break;      
    }  
  }

/*
   Wrap up the program ending the lnfill-u6, detaching and getting rid of the shared memory segment
*/  
  lnptr->command=-1;
  kill(lnptr->pid,SIGALRM);

/*
   Release the shared memory and close the U3

*/
/*  if (munmap(lnptr, sizeof (struct lnfill*)) == -1) {
    perror("Error un-mmapping the file");
// Decide here whether to close(fd) and exit() or not. Depends... 
  }
  close(mapLNfill);
*/
  shmdt(lnptr);                      // detach from shared memory segment
  shmctl(shmid, IPC_RMID, NULL);     // remove the shared memory segment hopefully forever

  return 0;
}
/**************************************************************/
int mmapSetup() {
  int fd=0;     // mapped file descriptor

  /* Open a file for writing.
   *  - Creating the file if it doesn't exist.
   *  - Truncating it to 0 size if it already exists. (not really needed)
   *
   * Note: "O_WRONLY" mode is not sufficient when mmaping.
   */
  fd = open(LNFILLDATAPATH, O_RDWR, (mode_t)0600);
  if (fd == -1) {
    perror("Error opening file path for writing");
    exit(EXIT_FAILURE);
  }
        
  /* Now the file is ready to be mmapped.
   */
  lnptr = (struct lnfill*) mmap(0, LNFILLDATASIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (lnptr == MAP_FAILED) {
    close(fd);
    perror("Error mmapping the file");
    exit(EXIT_FAILURE);
  }
  /* Don't forget to free the mmapped memory ... usually at end of main
   */
   
  return (fd);
}

/***********************************************************/
void updateRTD(){
  long int ii=0;
/*
  display  RTDs and tank RTD and Pressure...not used...info in Menu now
*/
  printf("\n Tank \n");
  printf(" RTD    PRESSURE    \n");
  printf("-----   ---------   \n");
  printf("%.0lf     %.1lf      \n", lnptr->tank.rtd, lnptr->tank.pressure);

  printf("Active detectors\n");
  printf("Det    RTD    RTD-LIMIT    STATUS     \n");
  printf("---   -----   ---------   ---------   \n");
  for (ii=0; ii<20; ii++){
    if (lnptr->ge[ii].onoff == 1){
      printf(" %.2li   %4.0lf     %4.0lf        %s \n", ii, lnptr->ge[ii].rtd, lnptr->ge[ii].limit, lnptr->ge[ii].status);
    }
  }
  return;
}

/***********************************************************/

int activeList(){
  long int ii=0, activemax=0;

  printf("\n Active detectors\n");
  printf("Det    RTD    RTD-LIMIT    STATUS    LAST FILL(s)   NEXT FILL(s)  \n");
  printf("---   -----   ---------   ---------  ------------   -------------\n");
  for (ii=0; ii<20; ii++){
    if (lnptr->ge[ii].onoff == 1){
      activemax++;
      printf(" %.2li   %4.0lf     %4.0lf        %6s         %.0lf        %6.0lf \n", ii, lnptr->ge[ii].rtd, lnptr->ge[ii].limit, lnptr->ge[ii].status,lnptr->ge[ii].last,lnptr->ge[ii].next-time(NULL));
      printf(" %.2li   %4.0lf     %4.0lf        %6s         %.0lf        %6.0lf \n", ii, lnptr->ge[ii].rtd, lnptr->ge[ii].limit, lnptr->ge[ii].status,lnptr->ge[ii].last,lnptr->ge[ii].next);
    }
  }

  return (activemax);
}
/***********************************************************/


void inactiveList(){
  long int ii=0;

  printf("\n Inactive detectors\n");
  printf("Det    RTD    RTD-LIMIT    STATUS     \n");
  printf("---   -----   ---------   ---------   \n");
  for (ii=0; ii<20; ii++){
    if (lnptr->ge[ii].onoff == 0){
      printf(" %.2li   %4.0lf     %4.0lf        %s \n", ii, lnptr->ge[ii].rtd, lnptr->ge[ii].limit, lnptr->ge[ii].status);
    }
  }

  return;
}

/***********************************************************/
void saveSetup(){
/*
  write out the lnfill.conf file from current shared memory
*/

  FILE *ofile;
  //  char lnfill_conf[200]="/Users/c4g/src/LNfill/include/lnfill.conf";   //see define statement in lnfill.h 
  char lnfill_conf[200]="include/lnfill.conf";   //see define statement in lnfill.h 
  int ii=0;
  //  char line[200]="\0";
  //  int onoff=0, chanRTD=0, chanOFLO=0;
  //  char name[10]="\0";
  //  double interval=0.0, max=0.0, min=0.0, limit=0.0;

  //  struct dewar ge;

/*
   Read configuration file
*/  

  if ( ( ofile = fopen (lnfill_conf,"wr") ) == NULL)
    {
      printf ("*** File on disk (%s) could not be opened: \n",lnfill_conf);
      printf ("===> %s \n",lnfill_conf);
      exit (EXIT_FAILURE);
    }

/*
 Should be positioned to read file
*/
  fprintf(ofile,"Num   Name   OnOff    Fill   dFMax   dFMin  U6RTD   U6OFLO   LIMIT    OLIM    Ibar    Ibar \n");
  fprintf(ofile,"---   ----   -----   -----   -----   -----  -----   ------   -----    ----    ----    ---- \n");

  for (ii=0; ii<20; ii++){

    fprintf(ofile,"%2i    %3s      %i     %0.lf    %3.0lf     %3.0lf     %2i      %2i      %3.0lf      %3.0lf     %3i \n", ii+1, 
	    lnptr->ge[ii].name, lnptr->ge[ii].onoff,   lnptr->ge[ii].interval, lnptr->ge[ii].max,    
	    lnptr->ge[ii].min,  lnptr->ge[ii].chanRTD, lnptr->ge[ii].chanOFLO, lnptr->ge[ii].limit,
	    lnptr->ge[ii].olimit, lnptr->ge[ii].chanIbar);
  }
  fprintf(ofile,"21    TANK     1       0      %3.0lf       0     %2i      %2i      %3.0lf      %3.0lf     %3i      %3i \n", 
	  lnptr->tank.cooltime,    lnptr->tank.chanRTD, lnptr->tank.chanPRES, lnptr->tank.limit, lnptr->tank.olimit, lnptr->tank.chanIbar, lnptr->tank.chanMani);

  fclose(ofile);

  return;
}

/***********************************************************/

void menu(){
  int ii=0, active[6], activemax=0;
  time_t curtime;

  for (ii=0; ii<20; ii++){
    if (lnptr->ge[ii].onoff == 1){
      active[activemax++] = ii;
    }
  }
  ii = 0;
  curtime = time(NULL);
  //  xtime = (double) curtime;
  printf("activemax %i\n",activemax);
  printf ("---------------------------------------------------------------------------------------\n");
  printf ("     Detectors       |         Manual          | Det - RTD/LIM -OFLO/OLIM- Next fill   \n");
  printf ("---------------------------------------------------------------------------------------\n");
  if (activemax > 0) //  - %.0lf/%.0lf - %.0lf/%.0lf - %li - %s\n",
    printf ("1 - status           | 10 - close all & vent   | %3s - %.0lf/%.0lf - %.0lf/%.0lf - %.0lf - %s\n",
	    lnptr->ge[active[0]].name,lnptr->ge[active[0]].rtd,lnptr->ge[active[0]].limit,lnptr->ge[active[0]].oflo,lnptr->ge[active[0]].olimit,lnptr->ge[active[0]].next-curtime,lnptr->ge[active[0]].status);
  //	    lnptr->ge[active[0]].name,lnptr->ge[active[0]].rtd,lnptr->ge[active[0]].limit,lnptr->ge[active[0]].oflo,lnptr->ge[active[0]].olimit,lnptr->ge[active[0]].next-curtime);//,lnptr->ge[active[0]].status);
	    //	    lnptr->ge[active[0]].name,lnptr->ge[active[0]].rtd,lnptr->ge[active[0]].limit,lnptr->ge[active[0]].oflo,lnptr->ge[active[0]].olimit,lnptr->ge[active[0]].next-xtime,lnptr->ge[active[0]].status);
  else
    printf ("1 - status           | 10 - close all & vent   |         \n");

  if (activemax > 1) 
    printf ("2 - alter parameters | 11 - close tank         | %3s - %.0lf/%.0lf - %.0lf/%.0lf - %.0lf - %s\n",
	    lnptr->ge[active[1]].name,lnptr->ge[active[1]].rtd,lnptr->ge[active[1]].limit,lnptr->ge[active[1]].oflo,lnptr->ge[active[1]].olimit,lnptr->ge[active[1]].next-curtime,lnptr->ge[active[1]].status);
  else
    printf ("2 - alter parameters | 11 - close tank         |         \n");

  if (activemax > 2) 
    printf ("3 - list active      | 12 - open tank          | %3s - %.0lf/%.0lf - %.0lf/%.0lf - %.0lf - %s\n",
	    lnptr->ge[active[2]].name,lnptr->ge[active[2]].rtd,lnptr->ge[active[2]].limit,lnptr->ge[active[2]].oflo,lnptr->ge[active[2]].olimit,lnptr->ge[active[2]].next-curtime,lnptr->ge[active[2]].status);
  else
    printf ("3 - list active      | 12 - open tank          |          \n");

  if (activemax > 3) 
    printf ("4 - list inactive    | 13 - close manifold     | %3s - %.0lf/%.0lf - %.0lf/%.0lf - %.0lf - %s\n",
	    lnptr->ge[active[3]].name,lnptr->ge[active[3]].rtd,lnptr->ge[active[3]].limit,lnptr->ge[active[3]].oflo,lnptr->ge[active[3]].olimit,lnptr->ge[active[3]].next-curtime,lnptr->ge[active[3]].status);
  else
    printf ("4 - list inactive    | 13 - close manifold     |          \n");

  if (activemax > 4) 
    printf ("5 - save parameters  | 14 - open manifold      | %3s - %.0lf/%.0lf - %.0lf/%.0lf - %.0lf - %s\n",
	    lnptr->ge[active[4]].name,lnptr->ge[active[4]].rtd,lnptr->ge[active[4]].limit,lnptr->ge[active[4]].oflo,lnptr->ge[active[4]].olimit,lnptr->ge[active[4]].next-curtime,lnptr->ge[active[4]].status);
  else
    printf ("5 - save parameters  | 14 - open manifold      |             \n");
  if (activemax > 5) 
    printf ("                     | 15 - close a valve      | %3s - %.0lf/%.0lf - %.0lf/%.0lf - %.0lf - %s\n",
	    lnptr->ge[active[5]].name,lnptr->ge[active[5]].rtd,lnptr->ge[active[5]].limit,lnptr->ge[active[5]].oflo,lnptr->ge[active[5]].olimit,lnptr->ge[active[5]].next-curtime,lnptr->ge[active[5]].status);
  else
    printf ("                     | 15 - close a valve      |          \n");

  printf ("7 - fill 1 detector  | 16 - open a valve       |                      \n");
  printf ("8 - fill ALL         |                         |                      \n");
  printf ("9 - initial fill     | 17 - hardware status    | TANK - %.1lf psi - %.0lf/%.0lf - %s \n",lnptr->tank.pressure,lnptr->tank.rtd,lnptr->tank.olimit,lnptr->tank.status);
  printf ("---------------------------------------------------------------------------------------\n");
  printf ("18 - reset alarms    | %s  | Time up - %li s            \n",lnptr->bitstatus,lnptr->secRunning);
  if (lnptr->com2 == 1) {
    printf ("19 - Shutdown HV     |                         |   is ON          \n");
  }
  else {
    printf ("19 - Shutdown HV     |                         |   is OFF       \n");
  }
  printf ("---------------------------------------------------------------------------------------\n");
  printf ("61 - email addresses | Number addresses = %2i   | %s \n", lnptr->maxAddress,lnptr->comStatus);
  printf ("---------------------------------------------------------------------------------------\n");
  printf ("0 - end this program | iBootbar IP address     | %s \n", lnptr->ibootbar_IP);
  printf ("100 - end all LNfill | MPOD     IP address     | %s \n", lnptr->mpod_IP);
  printf ("---------------------------------------------------------------------------------------\n");
  //  printf ("0 - end this program | 100 - end all LNfill    |                  \n");
  //  printf ("---------------------------------------------------------------------------------------\n");

  //  printf ("1:%c 2:%c 3:%c 4:%c 5:%c 6:%c 7:%c 8:%c \n",lnptr->bitstatus);

return;
}

/***********************************************************/

void addresses(){
  int ii=0;
  char ans[40]="\n";

  //  for (ii=0; ii< 10; ii++){
  printf ("Stored email addresses:\n");
  for (ii=0; ii< lnptr->maxAddress; ii++){
    printf("%i) %s\n",ii,lnptr->ge[ii].email);
  }
  printf ("Modify addresses ? y/n \n");
  scanf("%s",ans);
  if ( strcmp(ans,"y") == 0 || strcmp(ans,"Y") == 0){
    printf ("Number (<0 for no change)\n");
    scanf("%i",&ii);
    printf ("Type in each address one at a time (<cr> = no change) \n");
    scanf("%s",ans);
    strcpy(lnptr->ge[ii].email, ans);
  }
  
  printf ("Add addresses ? y/n \n");
  scanf("%s",ans);
  if ( strcmp(ans,"y") == 0 || strcmp(ans,"Y") == 0){
    printf ("Type in each address one at a time (ends if < 4 characters inputted) \n");
    for (ii=lnptr->maxAddress; ii< 20; ii++){
      scanf("%s",ans);
      if (strlen(ans) < 4) break;
      strcpy(lnptr->ge[ii].email, ans);
    }
  }
  lnptr->maxAddress=ii;

  printf ("Send test emails ? y/n \n");
  scanf("%s",ans);
  if ( strcmp(ans,"y") == 0 || strcmp(ans,"Y") == 0) lnptr->command = 61;

  return;
}
/***********************************************************/

/*
    printf (" --- Commands --- (type number) \n");
    printf (" 1 - show temperatures and limits  \n");
    printf (" 2 - change limit and timing parameters  \n");
    printf (" 3 - toggle on/off (turning on involves setup parameters)  \n");
    printf ("\n");

    printf (" 5 - save setup into lnfill.conf  \n");

    printf ("\n");
    printf (" 7 - force fill 1 detector  \n");
    printf (" 8 - force fill ALL detectors  \n");

    printf ("\n");
    printf (" 10 - close all valves (open manifold vent)   \n");

    printf (" 11 - close tank valve        \n");
    printf (" 12 - open tank valve        \n");

    printf (" 13 - close manifold valve    \n");
    printf (" 14 - open manifold valve     \n");

    printf (" 15 - close a valve           \n");
    printf (" 16 - open a valve            \n");

    printf ("\n");
    printf (" 18 - initial cool down   \n");
    printf ("\n");
    printf (" 0 - end this program  \n");
    printf ("\n");
    printf (" 100 - end all lnfill programs  \n");
*/

/*
  printf ("----------------------------------------------------------------------------\n");
  printf ("     Detectors       |         Manual          |   Last known status   \n");
  printf ("----------------------------------------------------------------------------\n");
  printf ("1 - status           | 10 - close all & vent   | Grow-in     = %li ms\n",igrow);
  printf ("2 - parameters       | 11 - close tank         | Decay-out   = %li ms\n",idecay);
  printf ("3 - on/off + setup   | 12 - open tank          | MTC move    = %li ms \n",mtc);
  printf ("                     | 13 - close manifold     | Ampl. (P-P) = %li mV\n",amplitude);
  printf ("5 - save parameters  | 14 - open manifold      | Ampl. (P-P) = %li mV\n",amplitude);
  printf ("                     | 15 - close a valv       | Ampl. (P-P) = %li mV\n",amplitude);
  printf ("7 - fill 1 detector  | 16 - open a valve       | Ampl. (P-P) = %li mV\n",amplitude);
  printf ("8 - fill ALL         |                         | Ampl. (P-P) = %li mV\n",amplitude);
  printf ("----------------------------------------------------------------------------\n");
  printf ("18 - initial fill    |                         | Ampl. (P-P) = %li mV\n",amplitude);
  printf ("----------------------------------------------------------------------------\n");
  printf ("0 - end this program | 100 - end all LNfill    | Ampl. (P-P) = %li mV\n",amplitude);
  printf ("----------------------------------------------------------------------------\n");
*/


/***********************************************************/
void parametersGe(int ii){
  int jj=0, kk=0;//, active[6], activemax=0;
  time_t curtime;
  char zz[100]="\0";
  double xx=0.;
  long int pp=0;


  while (jj != 99){

    printf ("----------------------------------------------------------------------------\n");
    printf ("  Software parameter   |  Value    | Hardware parameter  |  Value \n");
    printf ("----------------------------------------------------------------------------\n");
    printf ("1 - on/off toggle      | %3i       | 11 - RTD chan       | %i \n",lnptr->ge[ii].onoff,lnptr->ge[ii].chanRTD);
    printf ("2 - name               | %3s       | 12 - OFLO chan      | %i \n",lnptr->ge[ii].name,lnptr->ge[ii].chanOFLO);
    printf ("3 - fill interval      | %0.lf       | 13 - Ibar chan      | %i \n",lnptr->ge[ii].interval,lnptr->ge[ii].chanIbar + 1);
    printf ("4 - max fill interval  | %6.0lf    | 14 - next fill      | %0.lf \n",lnptr->ge[ii].max,lnptr->ge[ii].next-time(NULL));
    printf ("5 - min fill interval  | %6.0lf    | 15 -  \n",lnptr->ge[ii].min);
    printf ("6 - detector RTD limit | %6.0lf    | 16 -  \n",lnptr->ge[ii].limit);
    printf ("7 - overflow RTD limit | %6.0lf    | 17 - change TANK parameters  \n",lnptr->ge[ii].olimit);
    printf ("----------------------------------------------------------------------------\n");
    printf (" Type 99 to exit        | Type -1 to change to different detector (1-20) \n");
    printf ("----------------------------------------------------------------------------\n");
  

    scanf ("%s", zz);          // read in ans (pointer is indicated)   
    jj = atoi(zz);
    if (jj == 99) return;

    if (abs(jj) < 15){
      
      switch (jj){
      case -1:
	printf ("Detector (1-20)? \n");
	scanf ("%s", zz);          // read in ans (pointer is indicated)     
	kk = atoi(zz);
	if (kk > 0 && kk < 21) ii = kk;
	break;

      case 1:
	kk = activeList();
	if (kk < 6) {
	  if (lnptr->ge[ii].onoff == 0) lnptr->ge[ii].onoff = 1;
	  else lnptr->ge[ii].onoff = 0;
	}
	else if (kk >= 6 && lnptr->ge[ii].onoff == 1) lnptr->ge[ii].onoff = 0;
	else {
	  printf("Can't!!! You must turn off another detector before turning this on!\n");
	  break;
	}

	break;

      case 2:
	printf("New name ?  \n");
	scanf ("%s", zz);          // read in ans (pointer is indicated)     
	kk = strlen(zz);
	if (kk > 4) kk=4;
	strncpy(lnptr->ge[ii].name,zz,kk);

      case 3:
	printf("Filling interval in seconds ? (28800 = 8 hrs., < 0.99 do nothing) \n");
	scanf ("%s", zz);          // read in ans (pointer is indicated)     
	xx = atof(zz);
	if (xx >= 1) lnptr->ge[ii].interval = xx;
	break;
      case 4:
	printf("Maximum fill time before error ? (typ. 420 s, < 0.99 do nothing) \n");
	scanf ("%s", zz);          // read in ans (pointer is indicated)     
	xx = atof(zz);
	if (xx >= 1) lnptr->ge[ii].max = xx;
	break;
      case 5:
	printf("Minimum fill time before error ? (typ. 150 s, < 0.99 do nothing) \n");
	scanf ("%s", zz);          // read in ans (pointer is indicated)     
	xx = atof(zz);
	if (xx >= 1) lnptr->ge[ii].min = xx;
	break;
      case 6:
	updateRTD();
	printf("Maximum RTD reading before error ? (typ. 6 K above detector baseline) \n");
	printf("Current RTD/LIM  = %.0lf /%.0lf and OFLO/OLIM = %.0lf/%.0lf \n",lnptr->ge[ii].rtd,lnptr->ge[ii].limit,lnptr->ge[ii].oflo,lnptr->ge[ii].olimit);
	scanf ("%s", zz);          // read in ans (pointer is indicated)     
	xx = atof(zz);
	if (xx >= 1) lnptr->ge[ii].limit = xx;
	break;

      case 7:
	updateRTD();
	printf("Maximum overflow RTD reading before error ? (typ. 6 K above detector baseline) \n");
	printf("Current RTD/LIM  = %.0lf /%.0lf and OFLO/OLIM = %.0lf/%.0lf \n",lnptr->ge[ii].rtd,lnptr->ge[ii].limit,lnptr->ge[ii].oflo,lnptr->ge[ii].olimit);
	scanf ("%s", zz);          // read in ans (pointer is indicated)     
	xx = atof(zz);
	if (xx >= 1) lnptr->ge[ii].olimit = xx;
	break;

      case 11:
	printf("LabJack channel for detector RTD readings ? (range 0-13, <0 do nothing) \n");
	scanf ("%s", zz);          // read in ans (pointer is indicated)     
	kk = atoi(zz);
	if (kk >= 0 && kk < 15) lnptr->ge[ii].chanRTD = kk;
	break;

      case 12:
	printf("LabJack channel for detector OFLO readings ? (range 0-13, <0 do nothing) \n");
	scanf ("%s", zz);          // read in ans (pointer is indicated)     
	kk = atoi(zz);
	if (kk >= 0 && kk < 15) lnptr->ge[ii].chanOFLO = kk;
	break;

      case 13:
	printf("IBootBar channel for detector RTD readings ? (range 1-8, <0 do nothing) \n");
	scanf ("%s", zz);          // read in ans (pointer is indicated)     
	kk = atoi(zz);
	kk = kk - 1;
	if (kk >= 0 && kk < 8) lnptr->ge[ii].chanIbar = kk;
	break;

      case 14:
	curtime=time(NULL);
	printf("Next fill occurs in ~%.0lf seconds ? (< 0.99 do nothing) \n",lnptr->ge[ii].next-(double)curtime);
	printf("x hr in Y s: 1=3600, 2=7200, 3=10800, 4=14400, 5=18000, 6=21600, 7=25200, 8=28800  \n");
	scanf ("%s", zz);          // read in ans (pointer is indicated)     
	pp = atol(zz);
	if (pp >= 1) {
	  //	  lnptr->ge[ii].next = xx;
	  lnptr->ge[ii].next = curtime + pp;
	}
	break;

      case 17:
	parametersTank();
	break;

      case 99:
	jj=0;
	break;

      default:

	break;

      }
    }
  }
  
  return;
}

/***************************************************************/
void parametersTank(){
  int jj=0, kk=0;//, active[6], activemax=0;
  char zz[100]="\0";
  double xx=0.;

  while (jj !=99) {

    printf ("----------------------------------------------------------------------------\n");
    printf ("  Software parameter   |  Value  | Hardware parameter       |  Value \n");
    printf ("----------------------------------------------------------------------------\n");
    printf ("1 - Manifold RTD chan  | %3i     | 4 - Manifold RTD limit  | %.0lf \n",lnptr->tank.chanRTD,lnptr->tank.limit);
    printf ("                       |         | 5 - Tank Pressure       | %i \n",lnptr->tank.chanPRES);
    printf ("3 - Tank valve chan    | %3i     | 6 - Manifold valve chan | %i \n",lnptr->tank.chanIbar + 1,lnptr->tank.chanMani + 1);
    printf ("----------------------------------------------------------------------------\n");
    printf (" Type 99 to exit        |                                                  \n");
    printf ("----------------------------------------------------------------------------\n");

    scanf ("%s", zz);          // read in ans (pointer is indicated)   
    jj = atoi(zz);
    if (jj == 99) return;

    if (jj < 7){
      switch (jj){

      case 1:
	printf("LabJack channel for manifold RTD readings ? (range 0-13, <0 do nothing) \n");
	scanf ("%s", zz);          // read in ans (pointer is indicated)     
	kk = atoi(zz);
	if (kk >= 0 && kk < 15) lnptr->tank.chanRTD = kk;
	break;

      case 3:
	printf("IBootBar channel for tank valve ? (range 1-8, <0 do nothing) \n");
	scanf ("%s", zz);          // read in ans (pointer is indicated)     
	kk = atoi(zz);
	kk = kk - 1;
	if (kk >= 0 && kk < 8) lnptr->tank.chanIbar = kk;
	break;

      case 4:
	updateRTD();
	printf("Minimum overflow manifold RTD reading before closing ? (typ. a little below detector baseline) \n");
	printf("Current RTD/LIM  = %.0lf /%.0lf \n",lnptr->tank.rtd,lnptr->tank.olimit);
	scanf ("%s", zz);          // read in ans (pointer is indicated)     
	xx = atof(zz);
	if (xx >= 1) lnptr->tank.olimit = xx;
	break;

      case 5:
	printf("LabJack channel for tank pressure readings ? (range 0-13, <0 do nothing) \n");
	scanf ("%s", zz);          // read in ans (pointer is indicated)     
	kk = atoi(zz);
	if (kk >= 0 && kk < 15) lnptr->tank.chanPRES = kk;
	break;

      case 6:
	printf("IBootBar channel for manifold valve ? (range 1-8, <0 do nothing) \n");
	scanf ("%s", zz);          // read in ans (pointer is indicated)     
	kk = atoi(zz);
	kk = kk - 1;
	if (kk >= 0 && kk < 8) lnptr->tank.chanMani = kk;
	break;

      default:
	break;

      }
    }

  }
  return;
  }

/*
	  printf("RTD is which U6 channel? (0-13) \n");
	  scanf ("%i",&zz.chanRTD);
	  printf("Overflo is which U6 channel? (0-13) \n");
	  scanf ("%i",&zz.chanOFLO);
	  printf("Filling interval ? (usually 28800 s (8 hrs.)) \n");
	  scanf ("%i",&zz.interval);
	  //	  scanf ("%i",&zz.interval);
	  printf("Filling time max ? (usually 420 s) \n");
	  scanf ("%i",&zz.max);
	  printf("Filling time min ? (usually 150 s) \n");
	  scanf ("%i",&zz.min);
	  printf("RTD temperature limit ? (usually ~90 or 6 K above coldest value) \n");
	  scanf ("%lf",&zz.limit);
	  printf("Outlet temperature limit ? (usually ~90 or few degrees below detector value) \n");
	  scanf ("%lf",&zz.olimit);
	  printf(" %i %i %lf %lf \n", zz.chanRTD, zz.chanOFLO, zz.limit,zz.olimit);
	  zz.onoff = 1;
	  strcpy(zz.status,"OK");
	  lnptr->ge[det] = zz;
	  lnptr->ge[det].next = time(NULL) + lnptr->ge[det].interval;
	  printf("Detector next fill time has been set to %.0lf s from now \n",lnptr->ge[det].interval);
*/
