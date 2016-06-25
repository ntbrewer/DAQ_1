/*
  Program lnfill-u6 to handle LN filling and monitoring of Ge
  when RMS system is not available.

  Will put relevant data into shared memory where other programs
  can access it and decide actions such as turning off HV.

  Program will be based on LabJack U6 USB daq, dataport probe
  power strip with remote relay control.

*/

#include "lnfill.h"
#include "include/hv.h"
#define MPOD_MIB "include/WIENER-CRATE-MIB.txt"

void shmSetup();                  // sets up the shared memory with LN
void confRead();                  // processes configuration file
void mpodIP();                    // known MPOD IP lists
void snmp(int setget, char *cmd, char *cmdResult);    // snmp(setget,cmd,cmdResult);
int menu();
char *GetDate();

int readOnOff(char *cmdResult);
int readInt(char *cmdResult); 
unsigned int readBits(char *cmdResult);
float vCheck(int ii, int jj, int kk, float zz);
float readFloat(char *cmdResult);  

void confLoadSet();
void mtasStrucLoad();
void detStatus(int ii, int jj, int kk);
void reportStatus();
void loadShutdown();

void OnAll();
void OffAll();
void ResetAll();
void ItripOffAll(); 
void ItripOnAll();
void ImaxAll();
void VmaxAll();
void VrampAll();
void VsetAll();

void DetGet();
void adjustImax();
void adjustVmax();
void adjustVramp();
void adjustVset();
void resetDet();
void adjustItrip();
void adjustOnOff();

void mtasWriteNew();
//void enterItrip();
void tempShutdown(int index);

mode_t file_protect=00660; /* leading 0 defines octal format */
float xval=0;


/******************************************************************************/
int main( int argc, char *argv[] ){
  double zzz;
  int ans=0, ans2=0;
/*
  Shared memory creation and attachment
  the segment number is stored in lnptr->pid
*/
  shmSetup();
/*  
   Set up the signal capture routine for when the reading
   program wants something changed
*/
/*  
   Read setup file on disk and load into shared memory
*/
  mpodIP();  
  printf("Need to read conf to know which detectors are available...\n");
  printf("Read conf and email file...\n");
  confRead();                         // Read conf file
  printf(" ... Conf file read \n");
  if (lnptr->com2 == 1) {
    printf("Loading current configuration...\n");    // need to read config file to know what detectors are available
    mtasStrucLoad();
  }

/*
  Scroll through options
*/
  while (ans != 100){
    reportStatus();
    ans = menu();
    //    scanf ("%i", &ans);          // read in ans (pointer is indicated)     

    switch (ans){
    case 0:                    // end program but not deg-u3
      if (lnptr->shm > 0) shmdt(lnptr);           // detach from shared memory segment
      return 0;                // end this program
      break;

    case 1:                    
      confLoadSet();           // set up all detectors (voltages, trip, etc.) based on config. file
      lnptr->com2 = 1;         //set flag in shared memory that HV is ON
      return 0;                // end this program
      break;

    case 2:
      VsetAll();
      break;

    case 3:
      VmaxAll();
      break;

    case 4:
       VrampAll();
      break;

    case 5:
      printf ("1 = on     0 = off   \n");
      scanf("%i", &ans2);
      if (ans2 == 1){
	ItripOnAll();
      } else {
	ItripOffAll();
      }

      break;

    case 6:
      ImaxAll();
      break;

    case 7:
      printf ("1 = on     0 = off   \n");
      scanf("%i", &ans2);
      if (ans2 == 1){
	OnAll();
	lnptr->com2 = 1;    //set flag in shared memory that HV is ON
      } else {
	OffAll();
	lnptr->com2 = -1;   //set flag in shared memory that HV is OFF
      }
      break;

    case 8:
      loadShutdown();       // load shutdown message into LNfill
      break;

    case 9:                    
      ResetAll();               // resets all MPOD errors 
      break;

    case 12:
      iisave=-1;
      jjsave=-1;
      kksave=-1;
      adjustVset();             // manually adjust voltage

    case 13:
      iisave=-1;
      jjsave=-1;
      kksave=-1;
      adjustVmax();             // manually adjust voltage maximum

    case 14:
      iisave=-1;
      jjsave=-1;
      kksave=-1;
      adjustVramp();             // manually adjust voltage ramp

    case 15:
      iisave=-1;
      jjsave=-1;
      kksave=-1;
      adjustItrip();             // manually adjust current trip
      break;

    case 16:
      iisave=-1;
      jjsave=-1;
      kksave=-1;
      adjustImax();             // manually adjust current maximum
      break;

    case 17:
      iisave=-1;
      jjsave=-1;
      kksave=-1;
      adjustOnOff();             // manually adjust HV on or off
      break;

    default:                  // Do nothing and go back to the list of options
      ans = 0;
      break;      
    }  
  }

/*    sprintf(lnptr->ge[pp].shutdown,"snmpset -v 2c -L o -m %s -c guru %s outputSwitch.u%i i 3"",MPOD_MIB,MPOD_IP,index);

   Wrap up the program ending the deg-u3 detaching the shared memory segment
   shmctl(shmid, IPC_RMID, NULL);  // remove the shared memory segment hopefully forever (needs to be in the main program)
*/  

  kill(lnptr->pid,SIGALRM);

  shmdt(lnptr);                      // detach from shared memory segment
  //  shmctl(shmid, IPC_RMID, NULL);      // remove the shared memory segment if the last attached


  return 0;
}

/**************************************************************/
void loadShutdown(){
  int ii=0, jj=0, kk=0, index=0, mm=0, nn=0;
  char txt[100]="\0";
  printf("Entering load Shutdown\n");

  for(mm=0;mm<20;mm++){        // for each detector known to the LNfill
    nn=0;
    index=0;
    for (ii=0; ii< 1; ii++){   // scroll through all possible indexes known in hv-ln
      for (jj=0; jj<4; jj++){
	for (kk=0; kk<16; kk++){
	  if (strncmp(crate[ii].slot[jj].chan[kk].name,"xxx",3) ){   // identify each named detectors in hv-ln
	    index = 1000*ii + 100*jj + kk;

	    if (strcmp(lnptr->ge[mm].name, crate[ii].slot[jj].chan[kk].LNname) == 0){ // found a match between HV name and LNfill name
	      // sprintf(lnptr->ge[mm].shutdown[nn],"snmpset -v 2c -L o -m %s -c guru %s outputSwitch.u%i i 3",MPOD_MIB,MPOD_IP,index);  //load shutdown message
	      sprintf(txt,"snmpset -v 2c -L o -m %s -c guru %s outputSwitch.u%i i 3",MPOD_MIB,MPOD_IP,index);  //load shutdown message
	      strcpy(lnptr->ge[mm].shutdown[nn],txt);
	      strcpy(txt,"\0");
	      if (lnptr->ge[mm].onoff !=1) printf ("\nLNFILL not on for (%s,%s) *** DANGER *** \n",lnptr->ge[mm].name, crate[ii].slot[jj].chan[kk].name); 
	      //	      printf("mm = %i,  nn = %i %s\n", mm,nn,lnptr->ge[mm].shutdown[nn]);  //load shutdown message
	      nn++;
	      crate[ii].slot[jj].chan[kk].SDloaded = 1;   // shutdown command loaded into SHM
	    }
	  }
	}
      }
    }
  }

  for(mm=0;mm<20;mm++){         // for each detector known to the LNfill
    for(nn=0;nn<4;nn++) if (lnptr->ge[mm].onoff == 1) printf("Shutdwon for det: %s => %s\n",lnptr->ge[mm].name,lnptr->ge[mm].shutdown[nn]);  //load shutdown message
  }
  printf ("\n*** CAUTION IF ANY SHUTDOWN MESSAGE IS MISSING ABOVE ***\n"); 

  return;
}

/**************************************************************/
void reportStatus(){
  int ii=0, jj=0, kk=0;
  int index=0, mm=0;
  char sdloaded[5] ="\0";
/*
    Get rtd temperature from lnfill
*/
  for(mm=0;mm<20;mm++){        // for each detector known to the LNfill
    //    nn=0;
    index=0;
    for (ii=0; ii< 1; ii++){   // scroll through all possible indexes known in hv-ln
      for (jj=0; jj<4; jj++){
	for (kk=0; kk<16; kk++){
	  if (strncmp(crate[ii].slot[jj].chan[kk].name,"xxx",3) ){   // identify each named detectors in hv-ln
	    index = 1000*ii + 100*jj + kk;

	    if (strcmp(lnptr->ge[mm].name, crate[ii].slot[jj].chan[kk].LNname) == 0){ // found a match between HV name and LNfill name
	      crate[ii].slot[jj].chan[kk].ktemp = (int) lnptr->ge[mm].rtd;
	      //	      printf("k = %3i \n",crate[ii].slot[jj].chan[kk].ktemp);
	    }
	  }
	}
      }
    }
  }
/*
    Output detector data
*/
  printf ("--------------------------------------------------------------------------------\n");
  printf (" Det   MPOD   OnOff Itrip  Vset    Vmeas    Vmax   Imax   SDload  Temp   Status      \n");
  printf ("--------------------------------------------------------------------------------\n");
  for (ii=0; ii< 1; ii++){
    for (jj=0; jj<4; jj++){
      for (kk=0; kk<16; kk++){
	if (strncmp(crate[ii].slot[jj].chan[kk].name,"xxx",3) ){
	  index = 1000*ii + 100*jj + kk;
	  if (crate[ii].slot[jj].chan[kk].SDloaded == 1) strcpy(sdloaded,"YES");
	  else  strcpy(sdloaded,"NO");
	  printf ("%4s   %s    %i     %i   %5.1lf   %5.1lf   %5.1lf   %3.0lf    %3s   %4i    %4s \n",
		  crate[ii].slot[jj].chan[kk].name,
		  crate[ii].slot[jj].chan[kk].uname,
		  crate[ii].slot[jj].chan[kk].onoff,
		  crate[ii].slot[jj].chan[kk].Itrip,
		  crate[ii].slot[jj].chan[kk].Vset,
		  crate[ii].slot[jj].chan[kk].Vmeas,
		  crate[ii].slot[jj].chan[kk].Vmax,
		  crate[ii].slot[jj].chan[kk].Imax*pow(10,6),
		  sdloaded,
		  crate[ii].slot[jj].chan[kk].ktemp,
		  crate[ii].slot[jj].chan[kk].color
		  );
	}   // end if if over name=xxx
      }
    }
  }

 return;
}

/**************************************************************/
int menu(){
  int ans;

  printf ("------------------------------------------------------------------------------\n");
  printf (" All Detectors       | Individual Detectors    |    Current settings   \n");
  printf ("------------------------------------------------------------------------------\n");
  printf ("1 - All (conf file)  | 11 -                    | mpod IP = %s  \n",MPOD_IP);
  printf ("2 - Voltages         | 12 - Voltage            |         = \n");//%li ms \n",mtc);
  printf ("3 - Max voltages     | 13 - Max Voltage        | SHM PID = %li \n",lnptr->shm);
  printf ("4 - Ramp voltages    | 14 - Ramp Voltage       |         = \n");//%li ms \n",mtc);
  printf ("5 - Current trips    | 15 - Current trip       |         = \n");//%li mV\n",amplitude);
  printf ("6 - Max currents     | 16 - Max current        |         = \n");//%li mV\n",amplitude);
  printf ("7 - On/Off           | 17 - On/Off             |         = \n");//%li mV\n",amplitude);
  printf ("8 - LNfill shutdowns | 18 - LNfill shutdown    |         = \n");//%li mV\n",amplitude);
  printf ("9 - Reset alarms     | 19 - Reset alarm        |         = \n");//%li mV\n",amplitude);
  printf ("------------------------------------------------------------------------------\n");
  printf ("0 - End program      | 20 -                    |         = \n");//%li mV\n",amplitude);


  scanf ("%i", &ans);          /* reads in ans (pointer is indicated */

  return (ans);
}

/**************************************************************/
void shmSetup() {

  printf("Setting up shared memory...\n");///Users/c4g/src/LNfill/include/lnfill.conf
  //  shmKey = ftok("/Users/c4g/src/LNfill/include/lnfill.conf",'b');       // key unique identifier for shared memory, other programs use 'LN' tag
  shmKey = ftok("include/lnfill.conf",'b');       // key unique identifier for shared memory, other programs use 'LN' tag
  //  shmKey = ftok("SHM_PATH",'b');                                    // key unique identifier for shared memory, other programs use 'LN' tag
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
/******************************************************************************/
void mpodIP() {
  int pp=0, mm=0, ans=0;
  char mpod[50][20], text[20], line[150];

  if ( ( input_file = fopen (mpod_conf,"r+") ) == NULL)
    {
      printf ("*** File on disk (%s) could not be opened: \n",mtas_conf);
      printf ("===> %s \n",mtas_conf);
      exit (EXIT_FAILURE);
    }

  fgets(line,150,input_file);    // reads date in .conf file
  fgets(line,150,input_file);    // reads column headers
/*
 Should be positioned to read file
*/
  printf("Choose number with MPOD IP address\n");
  while (1) {                   // 1 = true
    fgets(line,150,input_file);
    //    printf("........................%i.....line = %s \n",count,line);
    if (feof(input_file)) {
      //      mm = fclose(input_file);
      //      printf ("<br>*** File closing: %i\n",count);
      break;
    }
/*
   A line from the file is read above and processed below
*/
//    mm = sscanf (line,"%s",&xname);
//    if (strcmp(xname,"MTAS") != 0) {
    printf("%s\n",line);
    mm = sscanf (line,"%i %s",&pp,text);
    strcpy(mpod[pp-1],text);
  }

  printf("%i ----- New adress \n", ++pp);
  scanf ("%i", &ans);
  strcpy(text,"\n");
  if (ans == pp) {
    printf("Input new address (www.xxx.yyy.zzz)...");
    scanf("%s",text);
    strcpy(mpod[ans-1],text);
    fprintf(input_file,"%i   %s    \n",pp,mpod[ans-1]);
    printf("****** Edit %s to add identifier string to new address ******\n\n",mpod_conf);
  }

  strcpy(MPOD_IP,mpod[ans-1]);
  mm = fclose(input_file);

  printf("mpodIP = %s\n\n",MPOD_IP);
  return;
}


/******************************************************************************/
void confRead() {

  int i, fd;
  int ii, jj, kk, mm, pp;
  FILE *input_file;
  char file_name[100];
  char line[200]="\0";
  int count, crslch, icrate, islot, ichan;
  char u, xname[10], LNname[10];
  float volts, vrmp, vmx, imx;
  int itrp;
  char nameu[10];//, c, n, a, aa;
  //  char win_message[20000]="\0";

/*
   Go through all possible mpod combinations loading in  xxx's and 0's for empty channels
   Then read in new data
*/
  for (ii=0; ii< 1; ii++){
    for (jj=0; jj<4; jj++){
      for (kk=0; kk<16; kk++){
	if (kk<10) sprintf(nameu,"u%i%i0%i",ii,jj,kk);
	if (kk>9)  sprintf(nameu,"u%i%i%i",ii,jj,kk);

	strcpy(crate[ii].slot[jj].chan[kk].uname,nameu);                  // default parameters
	strcpy(crate[ii].slot[jj].chan[kk].name,"xxx");
	strcpy(crate[ii].slot[jj].chan[kk].color,"Off");
	crate[ii].slot[jj].chan[kk].Vset = 0.0;
	crate[ii].slot[jj].chan[kk].Vmax = vCheck(ii,jj,kk,3000.0);
	crate[ii].slot[jj].chan[kk].Imax = 0.000500;
	crate[ii].slot[jj].chan[kk].Itrip = 1;
	crate[ii].slot[jj].chan[kk].ktemp = 0;
	crate[ii].slot[jj].chan[kk].klimit = 10;
	strcpy(crate[ii].slot[jj].chan[kk].LNname,"\0");	
	crate[ii].slot[jj].chan[kk].SDloaded = 0;
      }
    }
  }

/*
   Read configuration file
*/  
//  printf ("opening file\n");

  if ( ( input_file = fopen (mtas_conf,"r+") ) == NULL)  // a+ for append, r+ for reading
    {
      printf ("*** File on disk (%s) could not be opened: \n",mtas_conf);
      printf ("===> %s \n",mtas_conf);
      exit (EXIT_FAILURE);
    }

  fgets(line,150,input_file);    // reads date in .conf file
  //  printf ("%s",line);
  fgets(line,150,input_file);    // reads column headers
  //  printf ("%s",line);
/*
 Should be positioned to read file
*/
  while (1) {                   // 1 = true
    fgets(line,150,input_file);
    //       printf("........................%i.....line = %s \n",count,line);
    if (feof(input_file)) {
      mm = fclose(input_file);
      //      printf ("<br>*** File closing: %i\n",count);
      break;
    }
/*
   A line from the file is read above and processed below
*/
//    mm = sscanf (line,"%s",&xname);
//    if (strcmp(xname,"MTAS") != 0) {
    mm = sscanf (line,"%i %s %c %i %f %f %f %f %i %s",&pp,&xname,&u,&crslch,&volts,&vmx,&imx,&vrmp,&itrp,&LNname);
    //    printf ("mm = %i, pp = %i\n",mm,pp);
    
    if (pp == 99) {
      mm = fclose(input_file);
      //      printf ("<br>*** File closing: %i\n",count);
      break;
    }
    
    //      mm = sscanf (line,"%c %i %f %f %f %f %i",&u,&crslch,&volts,&vmax,&imax,&vramp,&itrip);
	  //    mm = sscanf (line,"%s %c %i %f",&xname,&u,&crslch,&volts);
     //     printf("%s\n",line);
     icrate=crslch/1000;
     islot=(crslch-(icrate*1000))/100;
     ichan=crslch-(icrate*1000)-(islot*100);
     strcpy(crate[icrate].slot[islot].chan[ichan].name,xname);       // record xname   now the uXXX and C1F detector notation linked
     crate[icrate].slot[islot].chan[ichan].Vramp=vrmp;               // record voltage ramp
     crate[icrate].slot[islot].chan[ichan].Vmax=vmx;                 // record voltage max
     crate[icrate].slot[islot].chan[ichan].Imax=imx*pow(10,-6);      // record current max - convert uA to A
     crate[icrate].slot[islot].chan[ichan].Itrip=1;                      // default is set to current trip
     if (itrp == 0) crate[icrate].slot[islot].chan[ichan].Itrip=itrp;    // set to no trip if desired
     crate[icrate].slot[islot].chan[ichan].Vset=vCheck(icrate,islot,ichan,volts);       // check that requested voltage is below maximum allowed
     crate[icrate].slot[islot].chan[ichan].Vmax = vCheck(icrate,islot,ichan,crate[icrate].slot[islot].chan[ichan].Vmax);
     strcpy(crate[icrate].slot[islot].chan[ichan].LNname,LNname);       // record xname   now the uXXX and C1F detector notation linked
/*
     printf("%s %s %.1f %.0f %.0f %.0f %i %s\n",
	    crate[icrate].slot[islot].chan[ichan].name,
	    crate[icrate].slot[islot].chan[ichan].uname,
	    crate[icrate].slot[islot].chan[ichan].Vset,
	    crate[icrate].slot[islot].chan[ichan].Vmax,
	    crate[icrate].slot[islot].chan[ichan].Imax,
	    crate[icrate].slot[islot].chan[ichan].Vramp,
	    crate[icrate].slot[islot].chan[ichan].Itrip,
	    crate[icrate].slot[islot].chan[ichan].LNname
	    );

     count++;
*/
    //    strcat(win_message,"<tt>");
     //    strcat(win_message,line);   // positioned here to record entire file
    //    strcat(win_message,"</tt>");
  }

  //  mess1(win_message);    // scroll message of what was in config file

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
   setget flag chooses if this is a 
    0=read (snmpget) or
    1=write (snmpset) or 
    2-shutdown with MPOD MIB and IP from HV control
*/
  if (setget == 1) {
    sprintf (com,"snmpset -v 2c -L o -m %s -c guru %s ",MPOD_MIB,MPOD_IP);  //guru (MPOD)-private-public (iBOOTBAR); versions 1 (iBOOTBAR) and 2c (MPOD)
  } 
  else if (setget == 0){
    sprintf (com,"snmpget -v 2c -L o -m %s -c guru  %s ",MPOD_MIB,MPOD_IP);
  }
  //  else if (setget == 2){              // if setget =2 then issue HV shutdown
  //}
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

/******************************************************************************/
void ResetAll() {
  int ii, jj, kk;
  char cmd[150], ttt[20], cmdResult[140];
  int index, setget=1;
  float zz;

  // snmpget -v 2c -m +WIENER-CRATE-MIB -c guru 10.0.0.0 groupsSwitch.64.u0
  //sprintf ("snmpget -v 2c -m %s  -c guru %s outputSwitch.u",MPOD_MIB,MPOD_IP);
  for (ii=0; ii< 1; ii++){
    for (jj=0; jj<4; jj++){
      for (kk=0; kk<16; kk++){
	if (strncmp(crate[ii].slot[jj].chan[kk].name,"xxx",3) ){
	index = 1000*ii + 100*jj + kk;

	sprintf(cmd,"outputSwitch.u%i i 2",index);        // resets emergency off
	snmp(setget,cmd,cmdResult);    //mpodSETguru(cmd,cmdResult);
	sprintf(cmd,"outputSwitch.u%i i 10",index);       // resets errors
	snmp(setget,cmd,cmdResult);     //mpodSETguru(cmd,cmdResult);

	crate[ii].slot[jj].chan[kk].onoff = atoi(cmdResult);

	sprintf(cmd,"outputVoltage.u%i F %f",index,crate[ii].slot[jj].chan[kk].Vset); //field.value[ii+kk]);
	snmp(setget,cmd,cmdResult);     //mpodSETguru(cmd, cmdResult);     // set the set voltage
	zz = readFloat(cmdResult);
	crate[ii].slot[jj].chan[kk].Vset = vCheck(ii,jj,kk,zz);
	}
      }
    }
  }
  mtasStrucLoad();
  return;
}
//******************************************************************************/

void OnAll() {
  int ii, jj, kk;
  char cmd[150], ttt[20], cmdResult[140];
  int index, setget=1;
  float zz;

  // snmpget -v 2c -m +WIENER-CRATE-MIB -c guru 10.0.0.0 groupsSwitch.64.u0
  //sprintf ("snmpget -v 2c -m %s  -c guru %s outputSwitch.u",MPOD_MIB,MPOD_IP);
  for (ii=0; ii< 1; ii++){
    for (jj=0; jj<4; jj++){
      for (kk=0; kk<16; kk++){
	if (strncmp(crate[ii].slot[jj].chan[kk].name,"xxx",3) ){
	index = 1000*ii + 100*jj + kk;

	sprintf(cmd,"outputSwitch.u%i i 1",index);       // turns on HV
	snmp(setget,cmd,cmdResult);    //mpodSETguru(cmd,cmdResult);
	crate[ii].slot[jj].chan[kk].onoff = atoi(cmdResult);

	sprintf(cmd,"outputVoltage.u%i F %f",index,crate[ii].slot[jj].chan[kk].Vset); //field.value[ii+kk]);
	snmp(setget,cmd,cmdResult);      //mpodSETguru(cmd, cmdResult);     // set the set voltage
	zz = readFloat(cmdResult);
	crate[ii].slot[jj].chan[kk].Vset = vCheck(ii,jj,kk,zz);
	}
      }
    }
  }
  mtasStrucLoad();
  return;
}
/******************************************************************************/

void OffAll() {
  int ii, jj, kk;
  char cmd[150], cmdResult[140], ttt[20];
  int index, setget=1;

  for (ii=0; ii< 1; ii++){
    for (jj=0; jj<4; jj++){
      for (kk=0; kk<16; kk++){
	if (strncmp(crate[ii].slot[jj].chan[kk].name,"xxx",3) ){
	index = 1000*ii + 100*jj + kk;
	sprintf(cmd,"outputSwitch.u%i i 0",index);       // turns off HV
	snmp(setget,cmd,cmdResult);    //mpodSETguru(cmd,cmdResult);
	crate[ii].slot[jj].chan[kk].onoff = atoi(cmdResult);
	}
      }
    }
  }
  mtasStrucLoad();
  return;
}

/*****************************************************************************
void enterItrip() {       // IS THIS ROUTINE NECESSARY???
  char value[25];
  int itripall;

  itripall=atoi(value);
  if (itripall==0 || itripall==1){
    printf("%s =>%i\n",value,itripall);
    //  mtasItripEnableAll(itripall); 
    // detStatusAllSvg(); 
  } 
  else {
    printf("Not boolean -> do nothing value = %i\n",itripall);
  }

  return;
}
*/
/*****************************************************************************/
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
float vCheck(int ii, int jj, int kk, float zz){
  float VMAXCENT=4000.;
  float VMAXHEX=4000.;
  char out[2]="O", mid[2]="M", inn[2]="I", cen[2]="C";

  if (zz > crate[ii].slot[jj].chan[kk].Vmax) zz = crate[ii].slot[jj].chan[kk].Vmax;
  /*
  if (crate[ii].slot[jj].chan[kk].name[0]==cen[0]) {
    if (zz > VMAXCENT) zz = VMAXCENT;
  }
  if (crate[ii].slot[jj].chan[kk].name[0]==out[0] ||
      crate[ii].slot[jj].chan[kk].name[0]==mid[0] ||
      crate[ii].slot[jj].chan[kk].name[0]==inn[0]) {
    if (zz > VMAXHEX) zz = VMAXHEX;
  }


  }
  if ((strncmp(crate[ii].slot[jj].chan[kk].name,"C",1) )){
    if (zz > VMAXHEX) zz = VMAXHEX;
  }
  //  else { 
  //  if (zz > VMAXCENT) zz = VMAXCENT;
  // }
  */
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

void tempShutdown(int index) {
  char cmd[150]="\0", cmdResult[140]="\0";
  int setget=2;
/*
  Currently this is only called from mtasStrucLoad which will verify the
  results when it checks the status bits
*/

  sprintf(cmd,"outputSwitch.u%i i 3",index);       // Emergency off
  snmp(setget,cmd,cmdResult);    //mpodSETguru(cmd,cmdResult);

	  
  return;
}

/******************************************************************************/

void mtasStrucLoad() {
  int ii,jj,kk,mm,nn;
  int iimax=1,jjmax=4,kkmax=16;

  /* failed progress bar crap
  float frac=0.;
  int numerator,denominator;
  */
  float zz, tot;
  int index, setget=0;   //setget = 0 = get data, 1 = set data
  char cmd[150]="\0", cmdResult[140]="\0";

  //  denominator = iimax*jjmax*kkmax;
  //  printf("2 denominator = %i - frac =%f\n",denominator, frac);

  for (ii=0; ii< iimax; ii++){
    for (jj=0; jj<jjmax; jj++){
      for (kk=0; kk<kkmax; kk++){
	if (strncmp(crate[ii].slot[jj].chan[kk].name,"xxx",3) ){

	  //	  numerator = kk + jj*kkmax + ii*(jjmax*kkmax);  // for progress bar
	  index = (1000*ii)+(100*jj)+kk;

	  if (crate[ii].slot[jj].chan[kk].ktemp > crate[ii].slot[jj].chan[kk].klimit){
	    tempShutdown(index) ;
	  }// tempShutdown(index) ;

	  sprintf(cmd,"outputSwitch.u%i",index);
	  snmp(setget,cmd,cmdResult);   //mpodGETguru(cmd, cmdResult);     // read the HV on
	  mm = readOnOff(cmdResult);
	  crate[ii].slot[jj].chan[kk].onoff = mm;
	  

	  sprintf(cmd,"outputVoltage.u%i",index);
	  snmp(setget,cmd,cmdResult);     //mpodGETguru(cmd, cmdResult);     // read the set voltage
	  zz = readFloat(cmdResult);
	  crate[ii].slot[jj].chan[kk].Vset = vCheck(ii,jj,kk,zz);;

	  sprintf(cmd,"outputMeasurementSenseVoltage.u%i",index);
	  snmp(setget,cmd,cmdResult);     //mpodGETguru(cmd, cmdResult);     // read the measured voltage
	  zz = readFloat(cmdResult);
	  crate[ii].slot[jj].chan[kk].Vmeas = zz;

	  sprintf(cmd,"outputVoltageRiseRate.u%i",index);
	  snmp(setget,cmd,cmdResult);    //mpodGETguru(cmd, cmdResult);     // read the voltage ramp rate
	  zz = readFloat(cmdResult);
	  crate[ii].slot[jj].chan[kk].Vramp = zz;

	  //sprintf(cmd,"outputSupervisionMaxTerminalVoltage.u%i",index);
	  //mpodGETguru(cmd, cmdResult);     // read the Vmax..doesn't work
	  //zz = readFloat(cmdResult);
	  //if (strncmp(crate[ii].slot[jj].chan[kk].name,"C",1)) {
	    //crate[ii].slot[jj].chan[kk].Vmax = zz;
	  //}

	  sprintf(cmd,"outputMeasurementCurrent.u%i",index);
	  snmp(setget,cmd,cmdResult);   //mpodGETguru(cmd, cmdResult);     // read the measured current
	  zz = readFloat(cmdResult);
	  crate[ii].slot[jj].chan[kk].Imeas = zz;

	  sprintf(cmd,"outputCurrent.u%i",index);
	  snmp(setget,cmd,cmdResult);   //mpodGETguru(cmd, cmdResult);     // read the max current
	  zz = readFloat(cmdResult);
	  crate[ii].slot[jj].chan[kk].Imax = zz;

	  detStatus(ii,jj,kk);  // read and process status bits loading Itrip, onoff, etc

	  //frac = (float)numerator/(float)denominator;
	  //gtk_progress_bar_update(GTK_PROGRESS_BAR(progress),frac);
	}
      }
    }
  }
  return;

}
/******************************************************************************/
void mtasWriteNew(){
  int ii, jj, kk;

  int i, fd;
  FILE *ofile;
  char file_name[100]="cards-", ss[10]="\0", tt[50]="\0";
  int c,n,a,index;
  char *ind;

  //  fprintf(output_file,"<HTML> %s<P>%c",GetDate(),10);strcat(file_name,"mtas-%s"
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
  printf("%s \n",tt);
  //  if (strcmp(field.name[2],"ofile") == 0) strcat(file_name,field.value[2]);
  if (jj > 0) strcat(file_name,tt);
  strcat(file_name,".conf\0");

 if (( ofile = fopen (file_name,"a+") ) == NULL){
   printf ("*** File on disk could not be opened \n");
   exit (EXIT_FAILURE);
 }

 fprintf(ofile,"%s",GetDate());               // outputs carriage return so do not need \n
 fprintf(ofile, "MTAS  MPOD   Vset    Vmax   Imax    Vramp  Itrip  \n");

  for (ii=0; ii< 1; ii++){
    for (jj=0; jj<4; jj++){
      for (kk=0; kk<16; kk++){
	if (strncmp(crate[ii].slot[jj].chan[kk].name,"xxx",3)){
	  ind=strstr(crate[ii].slot[jj].chan[kk].uname,"u");
	  strcpy(ss,(ind+1));
	  index=atoi(ss);
	  //	  n=
          //a=
	  //index= (c*1000)+(n*100)+a;
	  fprintf(ofile, " %s  %c%-4i %6.1f %6.0f %6.0f  %6.0f    %2i \n",crate[ii].slot[jj].chan[kk].name,crate[ii].slot[jj].chan[kk].uname[0],index,crate[ii].slot[jj].chan[kk].Vset,crate[ii].slot[jj].chan[kk].Vmax,crate[ii].slot[jj].chan[kk].Imax*pow(10,6),crate[ii].slot[jj].chan[kk].Vramp,crate[ii].slot[jj].chan[kk].Itrip);
	}
      }
    }
  }
  //  fprintf(ofile, "MTAS  MPOD   Vset    Vmax   Imax    Vramp  Itrip  \n");
  //  fprintf(ofile,"%s%c",GetDate(),10);

  fclose(ofile);
  //  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wid_write), FALSE);
/*
                   user/group  owner   group   others
                   ----------  -----   -----   ------
   protection is:     0110      0000    0111    0000 
*/

 chmod(file_name,file_protect); 
 // strcat(html_file,file_name);
 // strcat(html_file,".html\0");  
 //fd = rename(file_name,html_file);
 return;
}

/******************************************************************************/
void DetGet(){
  int ii,jj,kk;

  for (ii=0; ii< 1; ii++){
    for (jj=0; jj<4; jj++){
      for (kk=0; kk<16; kk++){
	if (strcmp(det_edit,crate[ii].slot[jj].chan[kk].name) == 0){
	  iisave=ii;
	  jjsave=jj;
	  kksave=kk;
	}
      }
    }
  }
  return;
}

/******************************************************************************/
void detStatus(int ii, int jj, int kk){
  unsigned int stat;
  char cmd[150]="\0", cmdResult[140]="\0";
  int index, setget=0;
  char lime[10] = "lime";
  char white[10] = "white";
  char red[10] = "red";
  char yellow[10] = "yellow";
  char cyan[10] = "cyan";
  char orange[10] = "orange";
  char grey[10] = "grey";
  char pink[10] = "pink";
  char hotpink[10] = "hotpink";
  char beige[10] = "beige";
  char wheat[10] = "wheat";
  char greenyellow[15] = "greenyellow";

  /* highest order
     0 = 0x8000 = output on                     beige or white or lime (at Vset)
     1 = 0x4000 = output inhibit                grey
     2 = 0x2000 = failure min sense voltage     cyan
     3 = 0x1000 = failure max sense voltage     cyan
     4 = 0x0800 = failure max terminal voltage  cyan
     5 = 0x0400 = failure max current           red
     6 = 0x0200 = failure max temperature       hot pink
     7 = 0x0100 = failure max power             hot pink
     8 = 0x0080 = not used
     9 = 0x0040 = failure time out              pink
    10 = 0x0020 = current limited               wheat 
    11 = 0x0010 = ramp up                       yellow
    12 = 0x0008 = ramp down                     orange
    13 = 0x0004 = Enable kill            
    14 = 0x0002 = Emergency off                 red
    15 = 0x0001 = not used                      
   lowest order */

  index=(ii*1000)+(jj*100)+kk;

  sprintf(cmd,"outputStatus.u%i",index);       // reads status bits
  snmp(setget,cmd,cmdResult);      //mpodGETguru(cmd, cmdResult);                 // read the status bits
  stat = readBits(cmdResult);
  crate[ii].slot[jj].chan[kk].Sbits = stat;

/*
  Now do the two settings contained in status
*/

  if (stat & 0x8000){ 
    crate[ii].slot[jj].chan[kk].onoff = 1;

    if (crate[ii].slot[jj].chan[kk].Vmeas > 10) {
      if ((crate[ii].slot[jj].chan[kk].Vmeas > 0.999*crate[ii].slot[jj].chan[kk].Vset) &&
	  (crate[ii].slot[jj].chan[kk].Vmeas < 1.001*crate[ii].slot[jj].chan[kk].Vset)){
	strcpy(crate[ii].slot[jj].chan[kk].color,"HV on = Vset");          // HV on and within 0.1% of Vset
      }
      else {
	strcpy(crate[ii].slot[jj].chan[kk].color,"HV on != Vset");   // HV on not within 0.1% of Vset
      }
    }
    else {
	strcpy(crate[ii].slot[jj].chan[kk].color,"HV on <10 V");   // HV on but at less than 10 V
    }

  } else {
    crate[ii].slot[jj].chan[kk].onoff = 0;
    strcpy(crate[ii].slot[jj].chan[kk].color,"Voltage off");     // HV off (white)
  }

  if (stat & 0x0004){ 
    crate[ii].slot[jj].chan[kk].Itrip = 1;               // current trip on or off
  } else {
    crate[ii].slot[jj].chan[kk].Itrip = 0;
  }
/*
  Now do color codes on failures
*/
  if (stat & 0x4000){ 
    strcpy(crate[ii].slot[jj].chan[kk].color,"Inhibit on");      // inhibit on (grey)
  } 
  else if (stat & 0x2000){ 
    strcpy(crate[ii].slot[jj].chan[kk].color,"Min sense voltage"); // min sense voltage (cyan)
  }
  else if (stat & 0x1000){ 
    strcpy(crate[ii].slot[jj].chan[kk].color,"Max sense voltage"); // max sense voltage (cyan)
  } 
  else if (stat & 0x0800){ 
    strcpy(crate[ii].slot[jj].chan[kk].color,"Max term. voltage"); // max terminal voltage (cyan)
  }
  else if (stat & 0x0400){ 
    strcpy(crate[ii].slot[jj].chan[kk].color,"Max current");       // max current (red)
  }
  else if (stat & 0x0200){ 
    strcpy(crate[ii].slot[jj].chan[kk].color,"Max crate temp");    // max temperature (hot pink)
  }
  else if (stat & 0x0100){ 
    strcpy(crate[ii].slot[jj].chan[kk].color,"Max crate power");    // max power (hot pink)
  }

  else if (stat & 0x0040){ 
    strcpy(crate[ii].slot[jj].chan[kk].color,"Failed time out");       // failure time out (pink)
  }
  else if (stat & 0x0020){ 
    strcpy(crate[ii].slot[jj].chan[kk].color,"Current limited");      // current limited (wheat)
  }
  else if (stat & 0x0010){ 
    strcpy(crate[ii].slot[jj].chan[kk].color,"Ramping up");    // ramping up (yellow)
  }
  else if (stat & 0x0008){ 
    strcpy(crate[ii].slot[jj].chan[kk].color,"Rampind down");    // ramping down (orange)
  }
  else if (stat & 0x0002){ 
    strcpy(crate[ii].slot[jj].chan[kk].color,"EMERGENCY OFF");       // emergency off (red)
  }
  else {
    }

  return;
}
/****************************************************************************/

void adjustVset(){
  char cmd[150]="\0", cmdResult[140]="\0";
  float vset, zz;
  int index, setget =1;

  while (iisave == -1 || jjsave == -1 || kksave == -1) {                // go thru while statement until a valid detector is found
    reportStatus();
    printf ("Which detector name to modify (case sensitive, xxx = do nothing) ... \n");
    scanf("%s", &det_edit);
    if (strcmp(det_edit,"xxx") == 0) return; 
    DetGet();
  }

  printf ("Modify det %s: Present Vmax = %.1lf  - New Vset (V) ? \n",crate[iisave].slot[jjsave].chan[kksave].name,crate[iisave].slot[jjsave].chan[kksave].Vset);
  scanf("%lf", &vset);

  //  vset = xval;   //atof(gtk_entry_get_text(GTK_ENTRY(entryVset)));                        // change data string to float
  vset = vCheck(iisave,jjsave,kksave,vset);

  //  printf("Entry contents: %f \n", vset);
  //  printf ("%i %i %i\n",iisave,jjsave,kksave);

  index = 1000*iisave + 100*jjsave + kksave;                                 // get the uname
  sprintf(cmd,"outputVoltage.u%i F %.1f",index,vset);                               // create the cmd to send to MPOD
  snmp(setget,cmd,cmdResult);   //mpodSETguru(cmd, cmdResult);                                               // set the max voltage in MPOD
  zz = readFloat(cmdResult);                                                 // convert MPOD output to float value
  crate[iisave].slot[jjsave].chan[kksave].Vset = zz;                        // store new value in data structure in A
  //  printf("cmd => %s\n", cmd);
  //  printf("Entry: %f => MPOD: %f \n", vset,zz);
  sleep(1);                           // pause to let system settle before loading in parameters
  mtasStrucLoad();                    // make sure currect data loaded into structure
  //  gtk_widget_queue_draw_area(fixed,0,0,XPLOT,YPLOT);    // redraw entire window to prevent over writing

  return;
}

/****************************************************************************/
void adjustVmax(){
  char cmd[150]="\0", cmdResult[140]="\0";
  float vmax, zz;
  int index, setget=1;

  while (iisave == -1 || jjsave == -1 || kksave == -1) {                // go thru while statement until a valid detector is found
    reportStatus();
    printf ("Which detector name to modify (case sensitive, xxx = do nothing) ... \n");
    scanf("%s", &det_edit);
    if (strcmp(det_edit,"xxx") == 0) return; 
    DetGet();
  }

  printf ("Modify det %s: Present Vmax = %.1lf  - New Vmax (V) ? \n",crate[iisave].slot[jjsave].chan[kksave].name,crate[iisave].slot[jjsave].chan[kksave].Vmax);
  scanf("%lf", &vmax);

  //  vmax = xval; //atof(gtk_entry_get_text(GTK_ENTRY(entryVmax)));                        // change data string to float
  //  printf("Entry contents: %f \n", vmax);
  index = 1000*iisave + 100*jjsave + kksave;                                 // get the uname
  sprintf(cmd,"outputSupervisionMaxTerminalVoltage.u%i F %.0f",index,vmax);                               // create the cmd to send to MPOD
  //  mpodSETguru(cmd, cmdResult);                                               // set the max current in MPOD
  zz = readFloat(cmdResult);                                                 // convert MPOD output to float value
  crate[iisave].slot[jjsave].chan[kksave].Vmax = zz;                        // store new value in data structure in A
  //  printf("Entry: %f => MPOD: %f \n", vmax,zz);
  sleep(1);                           // pause to let system settle before loading in parameters
  mtasStrucLoad();                    // make sure currect data loaded into structure
  //  gtk_widget_queue_draw_area(fixed,0,0,XPLOT,YPLOT);    // redraw entire window to prevent over writing

  return;
}

/****************************************************************************/

void adjustVramp(){
  char cmd[150]="\0", cmdResult[140]="\0";
  float vramp, zz;
  int index, setget=1;

  while (iisave == -1 || jjsave == -1 || kksave == -1) {                // go thru while statement until a valid detector is found
    reportStatus();
    printf ("Which detector name to modify (case sensitive, xxx = do nothing) ... \n");
    scanf("%s", &det_edit);
    if (strcmp(det_edit,"xxx") == 0) return; 
    DetGet();
  }

  printf ("Modify det %s: Present Vramp = %.1lf  - New Vramp (<= 100 V/s) ? \n",crate[iisave].slot[jjsave].chan[kksave].name,crate[iisave].slot[jjsave].chan[kksave].Vramp);
  scanf("%lf", &vramp);

  //  vramp = xval; // atof(gtk_entry_get_text(GTK_ENTRY(entryVramp)));                        // change data string to float
  //  printf("Entry contents: %f \n", vramp);
  index = 1000*iisave + 100*jjsave + kksave;                                 // get the uname
  sprintf(cmd,"outputVoltageRiseRate.u%i F %.0f",index,vramp);                               // create the cmd to send to MPOD
  snmp(setget,cmd,cmdResult);   //mpodSETguru(cmd, cmdResult);                                               // set the max current in MPOD
  zz = readFloat(cmdResult);                                                 // convert MPOD output to float value
  crate[iisave].slot[jjsave].chan[kksave].Vramp = zz;                        // store new value in data structure in A
  //  printf("%s\n Entry: %f => MPOD: %f \n",cmd, vramp,zz);
  sleep(1);                           // pause to let system settle before loading in parameters
  mtasStrucLoad();                    // make sure currect data loaded into structure
  //  gtk_widget_queue_draw_area(fixed,0,0,XPLOT,YPLOT);    // redraw entire window to prevent over writing

  return;
}

/****************************************************************************/
void adjustImax(){
  char cmd[150]="\0", cmdResult[140]="\0";
  float imax, zz;
  int index, setget=1;

  while (iisave == -1 || jjsave == -1 || kksave == -1) {                // go thru while statement until a valid detector is found
    reportStatus();
    printf ("Which detector name to modify (case sensitive, xxx = do nothing) ... \n");
    scanf("%s", &det_edit);
    if (strcmp(det_edit,"xxx") == 0) return; 
    DetGet();
  }

  printf ("Modify det %s: Present Imax = %.1lf  - New Imax (uA) ? \n",crate[iisave].slot[jjsave].chan[kksave].name,crate[iisave].slot[jjsave].chan[kksave].Imax);
  scanf("%lf", &imax);

  //  imax = xval;   //atof(gtk_entry_get_text(GTK_ENTRY(entryImax)));                        // change data string to float
  imax = imax*pow(10,-6);                                                    // convert uA to A
  //  printf("Entry contents: %f \n", imax);
  index = 1000*iisave + 100*jjsave + kksave;                                 // get the uname
  sprintf(cmd,"outputCurrent.u%i F %.6f",index,imax);                                // create the cmd to send to MPOD
  snmp(setget,cmd,cmdResult);    //mpodSETguru(cmd, cmdResult);                                               // set the max current in MPOD
  zz = readFloat(cmdResult);                                                 // convert MPOD output to float value
  crate[iisave].slot[jjsave].chan[kksave].Imax = zz;                         // store new value in data structure in A
  //  printf("Entry: %.0f => MPOD: %.0f \n", imax*pow(10,6),zz*pow(10,6));
  sleep(1);                           // pause to let system settle before loading in parameters
  mtasStrucLoad();                    // make sure currect data loaded into structure
  //  gtk_widget_queue_draw_area(fixed,0,0,XPLOT,YPLOT);    // redraw entire window to prevent over writing

  return;
}

/****************************************************************************/
void adjustOnOff(){
  char cmd[150], cmdResult[140];
  int index, setget=1;
  char yesno = 'n';
  //  float zz;

  while (iisave == -1 || jjsave == -1 || kksave == -1) {                // go thru while statement until a valid detector is found
    reportStatus();
    printf ("Which detector name to modify (case sensitive, xxx = do nothing) ... \n");
    scanf("%s", &det_edit);
    if (strcmp(det_edit,"xxx") == 0) return; 
    DetGet();
  }
  //  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(hv_on))){
  //  if (crate[iisave].slot[jjsave].chan[kksave].onoff == 1){
    index = 1000*iisave + 100*jjsave + kksave;

  if (crate[iisave].slot[jjsave].chan[kksave].onoff == 1) {
    printf ("Modify det %s: Turn HV Off (y/n) ? \n",crate[iisave].slot[jjsave].chan[kksave].name);
    scanf("%c", yesno);
    tolower(yesno);
    if (strcmp(&yesno,"y") != 0) return; 
    sprintf(cmd,"outputSwitch.u%i i 1",index);       // turns on HV
    snmp(setget,cmd,cmdResult);    //mpodSETguru(cmd,cmdResult);
    crate[iisave].slot[jjsave].chan[kksave].onoff = atoi(cmdResult);
    printf("   HV should be on \n");

  }
  else {
    printf ("Modify det %s: Turn HV On (y/n) ? \n",crate[iisave].slot[jjsave].chan[kksave].name);
    scanf("%c", yesno);
    tolower(yesno);
    if (strcmp(&yesno,"y") != 0) return; 
    sprintf(cmd,"outputSwitch.u%i i 0",index);       // turns on HV
    snmp(setget,cmd,cmdResult);     //mpodSETguru(cmd,cmdResult);
    crate[iisave].slot[jjsave].chan[kksave].onoff = atoi(cmdResult);
    printf("   HV should be off \n");
  }

  mtasStrucLoad();                    // make sure currect data loaded into structure

  return;
}
/****************************************************************************/
void adjustItrip(){
  char cmd[150], cmdResult[140];
  int index, setget=1;
  int mm;
  char yesno='n';
/*
  Toggle detector ON/OFF
*/
  while (iisave == -1 || jjsave == -1 || kksave == -1) {                // go thru while statement until a valid detector is found
    reportStatus();
    printf ("Which detector name to modify (case sensitive, xxx = do nothing) ... \n");
    scanf("%s", &det_edit);
    if (strcmp(det_edit,"xxx") == 0) return; 
    DetGet();
  }
  index = 1000*iisave + 100*jjsave + kksave;

  //  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(itrip_on))){
  if (crate[iisave].slot[jjsave].chan[kksave].Itrip == 1){
    printf ("Modify det %s: Turn current trip Off (y/n) ? \n",crate[iisave].slot[jjsave].chan[kksave].name);
    scanf("%c", yesno);
    tolower(yesno);
    if (strcmp(&yesno,"y") != 0) return; 
   
    sprintf(cmd,"outputSupervisionBehavior.u%i i 1",index); 
    snmp(setget,cmd,cmdResult);     //mpodSETguru(cmd, cmdResult);     // set the set voltage
    mm = readInt(cmdResult);
    crate[iisave].slot[jjsave].chan[kksave].Itrip = mm;
    printf("   Current trip should be on \n");
  }
  else {
    printf ("Modify det %s: Turn current trip On (y/n) ? \n",crate[iisave].slot[jjsave].chan[kksave].name);
    scanf("%c", yesno);
    tolower(yesno);
    if (strcmp(&yesno,"y") != 0) return; 
    sprintf(cmd,"outputSupervisionBehavior.u%i i 0",index); 
    snmp(setget,cmd,cmdResult);     //mpodSETguru(cmd, cmdResult);     // set the set voltage
    mm = readInt(cmdResult);
    crate[iisave].slot[jjsave].chan[kksave].Itrip = mm;
    printf("   Current trip should be off \n");
  }

  mtasStrucLoad();                    // make sure currect data loaded into structure

  return;
}


/******************************************************************************/
void ItripOnAll() {
  int ii, jj, kk;
  char cmd[150], cmdResult[140];
  int index, setget=1;
  int mm;

  //  itrip = atof(gtk_entry_get_text(GTK_ENTRY(entry))); 
  for (ii=0; ii< 1; ii++){
    for (jj=0; jj<4; jj++){
      for (kk=0; kk<16; kk++){
	if (strncmp(crate[ii].slot[jj].chan[kk].name,"xxx",3) ){
	index = 1000*ii + 100*jj + kk;
	sprintf(cmd,"outputSupervisionBehavior.u%i i 1",index); 
     	snmp(setget,cmd,cmdResult);   //mpodSETguru(cmd, cmdResult);     // set the set voltage
	mm = readInt(cmdResult);
	crate[ii].slot[jj].chan[kk].Itrip = mm;
	}
      }
    }
  }
  mtasStrucLoad();
  return;
}
/******************************************************************************/

void ItripOffAll() {
  int ii, jj, kk;
  char cmd[150], cmdResult[140];
  int index, setget=1;
  int mm;

  //  itrip = atof(gtk_entry_get_text(GTK_ENTRY(entry))); 
  for (ii=0; ii< 1; ii++){
    for (jj=0; jj<4; jj++){
      for (kk=0; kk<16; kk++){
	if (strncmp(crate[ii].slot[jj].chan[kk].name,"xxx",3) ){
	index = 1000*ii + 100*jj + kk;
	sprintf(cmd,"outputSupervisionBehavior.u%i i 0",index); 
     	snmp(setget,cmd,cmdResult);    //mpodSETguru(cmd, cmdResult);     // set the set voltage
	mm = readInt(cmdResult);
	crate[ii].slot[jj].chan[kk].Itrip = mm;
	}
      }
    }
  }
  mtasStrucLoad();
  return;
}

/*****************************************************************************
void enterVset(GtkWidget *widget, gpointer entry) {
  float vset;

  vset = atof(gtk_entry_get_text(GTK_ENTRY(entry)));       
  printf("vset=>%f\n",vset);
  if (vset > 4000.0) vset = 4000.0;
  printf ("after Vset: %f\n",vset);
  //  mtasImaxAll(); 

  return;
}
*/

/****************************************************************************/
void ImaxAll(){
  char cmd[150]="\0", cmdResult[140]="\0";
  float imax, zz;
  int index, setget=1;

  printf ("Set all current maximums to (uA) ... \n");
  scanf("%lf", &imax);
  imax = imax*pow(10,-6);                                                    // convert uA to A

  index = 1000*iisave + 100*jjsave + kksave;                                 // get the uname
  sprintf(cmd,"outputCurrent.u%i F %.6f",index,imax);                                // create the cmd to send to MPOD
  snmp(setget,cmd,cmdResult);    //mpodSETguru(cmd, cmdResult);                                               // set the max current in MPOD
  zz = readFloat(cmdResult);                                                 // convert MPOD output to float value
  crate[iisave].slot[jjsave].chan[kksave].Imax = zz;                         // store new value in data structure in A
  //  printf("Entry: %.0f => MPOD: %.0f \n", imax*pow(10,6),zz*pow(10,6));
  sleep(1);                           // pause to let system settle before loading in parameters
  mtasStrucLoad();                    // make sure currect data loaded into structure
  //  gtk_widget_queue_draw_area(fixed,0,0,XPLOT,YPLOT);    // redraw entire window to prevent over writing

  return;
}

/******************************************************************************/
void VsetAll(){ //float vconst) {
  int ii, jj, kk;
  char cmd[150], cmdResult[140];
  int index, setget=1;
  float vset,zz;

  printf ("Set all voltages (V) to ... \n");
  scanf("%lf", &vset);

  for (ii=0; ii< 1; ii++){
    for (jj=0; jj<4; jj++){
      for (kk=0; kk<16; kk++){
	if (strncmp(crate[ii].slot[jj].chan[kk].name,"xxx",3) ){
	index = 1000*ii + 100*jj + kk;
	sprintf(cmd,"outputVoltage.u%i F %f",index,vCheck(ii,jj,kk,vset)); //field.value[ii+kk]);
	snmp(setget,cmd,cmdResult);    //mpodSETguru(cmd, cmdResult);     // set the set voltage
	zz = readFloat(cmdResult);
	crate[ii].slot[jj].chan[kk].Vset = vCheck(ii,jj,kk,zz);
	}
      }
    }
  }
  mtasStrucLoad();
  return;
}

/******************************************************************************/
void VmaxAll() {
  int ii, jj, kk;
  char cmd[150], cmdResult[140];
  int index;  // setget=1;
  float zz;
  float vmax;

  printf ("Set all voltages (V) to ... \n");
  scanf("%lf", &vmax);

  if (vmax > 4000.) vmax = 4000.0;

  for (ii=0; ii< 1; ii++){
    for (jj=0; jj<4; jj++){
      for (kk=0; kk<16; kk++){
	if (strncmp(crate[ii].slot[jj].chan[kk].name,"xxx",3) ){
	index = 1000*ii + 100*jj + kk;
	sprintf(cmd,"outputSupervisionMaxTerminalVoltage.u%i F %f",index,vmax); //field.value[ii+kk]);
	//	mpodSETguru(cmd, cmdResult);     // set the set voltage
	zz = readFloat(cmdResult);
	crate[ii].slot[jj].chan[kk].Vmax = vCheck(ii,jj,kk,zz);
	}
      }
    }
  }
  mtasStrucLoad();
  return;
}

/******************************************************************************/
void VrampAll() {
  float vramp;
  int ii, jj, kk;
  char cmd[150], cmdResult[140];
  int index, setget=1;
  float zz;

  printf ("Set all voltages (V) to ... (100 V max) \n");
  scanf("%lf", &vramp);

  if (vramp > 100.) vramp = 100.0;

  for (ii=0; ii< 1; ii++){
    for (jj=0; jj<4; jj++){
      for (kk=0; kk<16; kk++){
	if (strncmp(crate[ii].slot[jj].chan[kk].name,"xxx",3) ){
	index = 1000*ii + 100*jj + kk;
	sprintf(cmd,"outputVoltageRiseRate.u%i F %f",index,vramp); //field.value[ii+kk]);
	snmp(setget,cmd,cmdResult);   //mpodSETguru(cmd, cmdResult);     // set the set voltage
	zz = readFloat(cmdResult);
	crate[ii].slot[jj].chan[kk].Vramp = zz;
	}
      }
    }
  }
  mtasStrucLoad();
  return;
}

/******************************************************************************
void ImaxAll() {
  float imax;
  int ii, jj, kk;
  char cmd[150], cmdResult[140];
  int index, setget=1;
  float zz;

  imax = xval;      //atof(gtk_entry_get_text(GTK_ENTRY(entry)));     // get the text value   
  printf("imax=>%f\n",imax);
  if (imax > 500.) imax = 500.0;
  imax=imax/pow(10,6);                   //convert uA to A

  for (ii=0; ii< 1; ii++){
    for (jj=0; jj<4; jj++){
      for (kk=0; kk<16; kk++){
	if (strncmp(crate[ii].slot[jj].chan[kk].name,"xxx",3) ){
	index = 1000*ii + 100*jj + kk;
	sprintf(cmd,"outputCurrent.u%i F %f",index,imax); 
	snmp(setget,cmd,cmdResult);    //mpodSETguru(cmd, cmdResult);     // set the max current
	zz = readFloat(cmdResult);
	crate[ii].slot[jj].chan[kk].Imax = zz;
	}
      }
    }
  }
  mtasStrucLoad();
  return;
}
*/
/******************************************************************************/
void confLoadSet() {
  int ii, jj, kk;
  char cmd[150], ttt[20], cmdResult[140];
  int mm,index, setget=1;
  float zz;
  char ss[10]="\0";
  char *ind;

  // snmpget -v 2c -m +WIENER-CRATE-MIB -c guru 10.0.0.0 groupsSwitch.64.u0
  //sprintf ("snmpget -v 2c -m %s  -c guru %s outputSwitch.u",MPOD_MIB,MPOD_IP);
  for (ii=0; ii< 1; ii++){
    for (jj=0; jj<4; jj++){
      for (kk=0; kk<16; kk++){
	if (strncmp(crate[ii].slot[jj].chan[kk].name,"xxx",3) ){
	  //	index = 1000*ii + 100*jj + kk;
	  
	  printf("%s\n",crate[ii].slot[jj].chan[kk].uname);
	  ind=strstr(crate[ii].slot[jj].chan[kk].uname,"u");
	  strcpy(ss,(ind+1));
	  index=atoi(ss);
	  //	  	  printf("%i\n",index);
		  
	sprintf(cmd,"outputSupervisionBehavior.u%i i %i",index,crate[ii].slot[jj].chan[kk].Itrip); // set current trip
     	snmp(setget,cmd,cmdResult);   //mpodSETguru(cmd, cmdResult);     // set the set voltage
	//		printf("%s\n",cmdResult);
	mm = readInt(cmdResult);
	crate[ii].slot[jj].chan[kk].Itrip = mm;
	
	usleep(100);		     
	sprintf(cmd,"outputCurrent.u%i F %.6f",index,crate[ii].slot[jj].chan[kk].Imax); //   imax);        // create the cmd to send to MPOD
	//		  	  printf("%s\n",cmd);
	snmp(setget,cmd,cmdResult);   //mpodSETguru(cmd, cmdResult);                                               // set the max current in MPOD
	//		printf("%s\n",cmdResult);
	zz = readFloat(cmdResult);                                                 // convert MPOD output to float value
	crate[iisave].slot[jjsave].chan[kksave].Imax = zz;                         // store new value in data structure in A

	usleep(100);		     
	sprintf(cmd,"outputSupervisionMaxTerminalVoltage.u%i F %.0f",index,crate[ii].slot[jj].chan[kk].Vmax); //set max voltage
	//		  	  printf("%s\n",cmd);
      		  //	mpodSETguru(cmd, cmdResult);     // set the set voltage
	zz = readFloat(cmdResult);
	crate[ii].slot[jj].chan[kk].Vmax = zz;

	usleep(100);		     
	sprintf(cmd,"outputVoltageRiseRate.u%i F %.0f",index,crate[ii].slot[jj].chan[kk].Vramp); //set voltage ramp
	//		  	  printf("%s\n",cmd);
	snmp(setget,cmd,cmdResult);    //mpodSETguru(cmd, cmdResult);     // set the set voltage
	zz = readFloat(cmdResult);
	crate[ii].slot[jj].chan[kk].Vramp = zz;//vCheck(ii,jj,kk,zz);

	usleep(100);		     
	sprintf(cmd,"outputVoltage.u%i F %.0f",index,crate[ii].slot[jj].chan[kk].Vset); //set voltage
	//		  	  printf("%s\n",cmd);
	snmp(setget,cmd,cmdResult);    //mpodSETguru(cmd, cmdResult);     // set the set voltage
	zz = readFloat(cmdResult);
	crate[ii].slot[jj].chan[kk].Vset = vCheck(ii,jj,kk,zz);

	usleep(100);		     
	sprintf(cmd,"outputSwitch.u%i i 1",index);       // turns on HV
	//		  	  printf("%s\n",cmd);
	snmp(setget,cmd,cmdResult);     //mpodSETguru(cmd,cmdResult);
	crate[ii].slot[jj].chan[kk].onoff = atoi(cmdResult);
	}
      }
    }
  }
  //  mtasStrucLoad();


  return;
}

/****************************************************************************/
void resetDet(){
  char cmd[150], cmdResult[140];
  int index, setget=1;
  float zz;

  index = 1000*iisave + 100*jjsave + kksave;

  sprintf(cmd,"outputSwitch.u%i i 2",index);        // resets emergency off
  snmp(setget,cmd,cmdResult);    //mpodSETguru(cmd,cmdResult);
  sprintf(cmd,"outputSwitch.u%i i 10",index);       // resets trips
  snmp(setget,cmd,cmdResult);    //mpodSETguru(cmd,cmdResult);
  crate[iisave].slot[jjsave].chan[kksave].onoff = atoi(cmdResult);

  sprintf(cmd,"outputVoltage.u%i F %f",index,crate[iisave].slot[jjsave].chan[kksave].Vset); 
  snmp(setget,cmd,cmdResult);     //mpodSETguru(cmd, cmdResult);     // set the set voltage
  zz = readFloat(cmdResult);
  crate[iisave].slot[jjsave].chan[kksave].Vset = vCheck(iisave,jjsave,kksave,zz);
  mtasStrucLoad();

  return;
}

/****************************************************************************/




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


