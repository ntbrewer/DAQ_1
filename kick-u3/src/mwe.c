#include "../../include/u3-err-msg.h"
#include "../../include/u3.h"
#include "../../include/labjackusb.h"
#include "../../include/kick-u3.h"

int readConf();
int mmapSetup(); 
uint8  findLJchanEIO(char *aaa);     // used by readConf
uint8  findLJchanCIO(char *aaa);     // used by readConf
long int  findLJchanFIO(char *aaa);  // used by readConf to identify digital input from the MTC controller tape faults
struct sec_us time_In_ms (struct sec_us xx);   // my timer routines and structure for times
int labjackSetup(long int lj, int num, int ljnum);           // sets up the LabJack U3
void clearParameters();           // clear parameters
void clearStats();                // clear satistics
struct labjack {
  HANDLE hand;
  long int lj;
  int ljnum;                    // number of labjack to relate this specific to a particular labjack such as calibrations
  u3CalibrationInfo caliInfo;
} labj[5];

int main() { 
int mapKick=0;
uint8 ii=0,jj=0,kk=0,ll=0; //kk (pulse width) less than 125 for 4000 us kk*32 = us
int ljmax=5;


  mapKick = mmapSetup();
  if (mapKick == -1) {
    printf(" Error on setting up memory map ... exiting \n");
    return 0;
  }

  clearParameters(); //This sets default width used by readConf();
  clearStats();
  ljmax = readConf();
  //  printf("mtc %x   %x \n",mtcptr->mtc[0],mtcptr->mtc[1]);
  ii = mtcptr->mtc[0] + mtcptr->move[0] + mtcptr->kck[0] + mtcptr->beam[2]        // move mtc with no beam  (beamOFF)
       + mtcptr->meas[2] + mtcptr->bkg[2] + mtcptr->lite[2]; //enable meas,bkg,lite all OFF
  jj = mtcptr->mtc[1] + mtcptr->move[1] + mtcptr->kck[1] + mtcptr->beam[3]
       + mtcptr->meas[3] + mtcptr->bkg[3] + mtcptr->lite[3];
  kk = mtcptr->mtc[0] | mtcptr->move[0] | mtcptr->kck[0] | mtcptr->beam[2]        // move mtc with no beam  (beamOFF)
       | mtcptr->meas[2] | mtcptr->bkg[2] | mtcptr->lite[2]; //enable meas,bkg,lite all OFF
  ll = mtcptr->mtc[1] | mtcptr->move[1] | mtcptr->kck[1] | mtcptr->beam[3]
       | mtcptr->meas[3] | mtcptr->bkg[3] | mtcptr->lite[3];

  ii += mtcptr->move[2] - mtcptr->move[0];
  jj += mtcptr->move[3] - mtcptr->move[1];

   kk = (kk | mtcptr->move[2]) & ~mtcptr->move[0];
   ll = ll | mtcptr->move[3] & ~mtcptr->move[1];
   printf(" ok: %i,%i,%i,%i \n ",ii,jj,kk,ll);//,mtcptr->move[0],mtcptr->move[1],mtcptr->move[2],mtcptr->move[3]);

   return(0);
}

/*********************************************************************************/
int  mmapSetup() {
  int fd=0, result=0, ii=0;
  /* Open a file for writing.
    *  - Creating the file if it doesn't exist.
    *  - Truncating it to 0 size if it already exists. (not really needed)
    *
    * Note: "O_WRONLY" mode is not sufficient when mmaping.
    */
   fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
   if (fd == -1) {
     perror("Error opening file for writing");
	exit(EXIT_FAILURE);
   }
  /*
   Stretch the file size to the size of the (mmapped) array of ints
  */
   for (ii=0; ii<sizeof (struct mtc_par); ii++){
     result = write(fd, "D", 1);
     if (result != 1) {
       close(fd);
       perror("Error writing last byte of the file");
       exit(EXIT_FAILURE);
     }
   };

   mtcptr = (struct mtc_par*) mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
   if (mtcptr == MAP_FAILED) {
	close(fd);
	perror("Error mmapping the file");
	exit(EXIT_FAILURE);
   }

   return (fd);
}


/*********************************************************************************/
void clearParameters(){
  
  mtcptr->onoff = 0;     // set all modes off

  mtcptr->normal = 0; 
  mtcptr->takeaway = 0;
  mtcptr->pause = 0;

  mtcptr->beammeas = 0;    // always with takeaway
  mtcptr->measbeam = 0;    // option for normal and pause

  mtcptr->trigbeam = 0;    // option: default is true
  mtcptr->laserbeam = 0;   // option for MTAS
  mtcptr->background = 0;  // option for MTAS
  mtcptr->bkgRatio = -1;    // option for MTAS; -1 = all data
  mtcptr->numMove = 2;     // option for normal/pause mode

  mtcptr->trigDT=32;     // width of trigger signal
  return;
}


/**************************************************************/
void clearStats(){
  mtcptr->cyclesData = 0;
  mtcptr->cyclesBkg = 0;
  
  return;
}

/*********************************************************************************/
struct sec_us time_In_ms (struct sec_us xx) {
  //  struct sec_us  xx;                // structure to hold ms times in sec, ms, us
  
  xx.sec = xx.ms / 1000;                      // integer division to convert ms to sec
  xx.us  = (xx.ms - (xx.sec * 1000))*1000;    // integer arithmatic to get fraction of second in ms
  
  return (xx);
}
/*********************************************************************************/
int readConf() {
  FILE *ifile;
  char line[200]="\0";

// see define statement in lnfill.h for deg_conf file name
  int ljnum=0, ljmax=0;
  int ii = 0, lj = 0, t0 = 0, jj=0, mm=0;
  char name[15]="\0", ch0[5]="\0", ch1[5]="\0";
/*
   Read configuration file
*/  

  printf ("Opening configuration file: %s \n",pulse_conf);
  if ( ( ifile = fopen (pulse_conf,"r+") ) == NULL)
    {
      printf ("*** File on disk (%s) could not be opened: \n",pulse_conf);
      printf ("===> %s \n",pulse_conf);
      exit (EXIT_FAILURE);
    }
  printf ("Opened: %s \n",pulse_conf);

  fgets(line,150,ifile);    // reads column headers
  //   printf ("%s\n",line);
  fgets(line,150,ifile);    // reads ----
  //  printf ("%s\n",line);
/*
 Should be positioned to read file
*/
  ii=0;
  while (ii!=-1) {                   // 1 = true
    fgets(line,150,ifile);
       printf ("%s\n",line);
    if (feof(ifile)) {
      mm = fclose(ifile);
      break;
    }
/*
   A line from the file is read above and processed below
 Index     Labjack SN    Parameter     Labjack parameter       Time(ms)
--------   ----------    ---------    ------------------     -----------
   1        320059423     beam-ON        eio0   eio1            2400                
   2        320059423     beam-OFF       eio2   eio3            2500                   
*/
    mm = sscanf (line,"%i %i %s %s %s %i", &ii, &lj, name, ch0,  ch1,  &t0);
    if (mm == 0) printf ("in values: %i %i %s %s %s %i \n", ii, lj, name, ch0, ch1, t0 );
/*
   Now load the data into the shared memory structure
*/
    mtcptr->pon.ms = 0;      
    /*  
    if (ii == -1) {
      mtcBreak =   findLJchanFIO(ch0);
      mtcFault =  findLJchanFIO(ch1);
      break;
    }
*/    
    //      printf ("Detected end of conf file after processing %i labjacks and %i channels \n",ljmax,num); 
    //    mtcptr->xx.ms = t0;
    mtcptr->lj = lj;
    switch (ii) {
      case -1:
      //tcBreak = findLJchanFIO(ch0);
      //mtcFault = findLJchanFIO(ch1);
      break;
    case 1:
      mtcptr->meas[0] = findLJchanEIO(ch0);           // meas ON in eio
      mtcptr->meas[1] = findLJchanCIO(ch0);           // meas ON in cio
      mtcptr->meas[2] = findLJchanEIO(ch1);	      // meas OFF in eio
      mtcptr->meas[3] = findLJchanCIO(ch1);           // meas OFF in cio
      mtcptr->bon.ms = t0;
      mtcptr->bon = time_In_ms(mtcptr->bon);     // set sec and us values in structure
      break;
     case 2:
      mtcptr->bkg[0] = findLJchanEIO(ch0);            // bkg channel
      mtcptr->bkg[1] = findLJchanCIO(ch0);            // beam ON in cio
      mtcptr->bkg[2] = findLJchanEIO(ch1);           // bkg channel
      mtcptr->bkg[3] = findLJchanCIO(ch1);           // beam ON in cio
      break;
    case 3:
      mtcptr->kck[0] = findLJchanEIO(ch0);            // kicker values
      mtcptr->kck[1] = findLJchanCIO(ch0);            // beam ON in cio
      mtcptr->move[0] = findLJchanEIO(ch1);           // send cmd to move tape
      mtcptr->move[1] = findLJchanCIO(ch1);           // same in cio
      mtcptr->boff.ms = t0;                      // load ms times into structure
      mtcptr->boff = time_In_ms(mtcptr->boff);   // set sec and us values in structure
      break;
    case 4:
      mtcptr->mtc[0] = findLJchanEIO(ch0);            // MTC move values
      mtcptr->mtc[1] = findLJchanCIO(ch0);           // beam ON in cio
      mtcptr->mtc[2] = findLJchanEIO(ch1);
      mtcptr->mtc[3] = findLJchanCIO(ch1);           // beam ON in cio
      mtcptr->tmove.ms = t0;
      mtcptr->tmove= time_In_ms(mtcptr->tmove);  // set sec and us values in structure
      break;
    case 5:
      mtcptr->trig[0] = findLJchanFIO(ch0);            // trig channel
      mtcptr->trig[1] = findLJchanCIO(ch0);           // beam ON in cio
      mtcptr->trig[2] = findLJchanFIO(ch1);            // trig channel
      mtcptr->trig[3] = findLJchanCIO(ch1);           // beam ON in cio
      mtcptr->tdt.ms = t0;
      mtcptr->tdt= time_In_ms(mtcptr->tdt);  // set sec and us values in structure
      break;
    case 6:
      mtcptr->beam[0] = findLJchanEIO(ch0);           // beam ON in eio
      mtcptr->beam[1] = findLJchanCIO(ch0);           // beam ON in cio
      mtcptr->beam[2] = findLJchanEIO(ch1);          // beam OFF in eio
      mtcptr->beam[3] = findLJchanCIO(ch1);          // beam OFF in cio
      break;
     case 7:
      mtcptr->lite[0] = findLJchanEIO(ch0);           // laser lite values
      mtcptr->lite[1] = findLJchanCIO(ch0);           // beam ON in cio
      mtcptr->lite[2] = findLJchanEIO(ch1);
      mtcptr->lite[3] = findLJchanCIO(ch1);           // beam ON in cio
      mtcptr->lon.ms = t0;
      mtcptr->lon= time_In_ms(mtcptr->lon);      // set sec and us values in structure
      break;
    case 8:                                     // timer-counter start for controlling laser pulses
      //tcStart = findLJchanFIO(ch0);
     default:
      break;
      /*
    case -1:
      mtcBreak =  findLJchanFIO(ch0);
      mtcFault =  findLJchanFIO(ch1);
      mtcptr->lon.ms = t0;
      mtcptr->lon = time_In_ms(mtcptr->lon);      // set sec and us values in structure
      break;
    case 1:
      mtcptr->pulse_e[0] =  findLJchanEIO(ch0);                                  // beam ON in eio
      mtcptr->pulse_c[0] =  findLJchanCIO(ch0);           // beam ON in cio
      mtcptr->pulse_e[1] =  findLJchanEIO(ch1);                                  // beam ON in eio
      mtcptr->pulse_c[1] =  findLJchanCIO(ch1);           // beam ON in cio
      mtcptr->tdt.ms = t0;
      mtcptr->trigDT = t0;
      mtcptr->tdt = time_In_ms(mtcptr->tdt);     // set sec and us values in structure
      break;
     case 2:
      mtcptr->pulse_e[2] =  findLJchanEIO(ch0);           // beam OFF values
      mtcptr->pulse_c[2] =  findLJchanCIO(ch0);           // beam ON in cio
      mtcptr->pulse_e[3] =  findLJchanEIO(ch1);
      mtcptr->pulse_c[3] =  findLJchanCIO(ch1);           // beam ON in cio
      break;
    case 3:
      mtcptr->pulse_e[4] =  findLJchanEIO(ch0);           // beam ON in cio
      mtcptr->pulse_c[4] =  findLJchanCIO(ch0);           // beam ON in cio
      mtcptr->pulse_e[5] =  findLJchanEIO(ch1);           // beam ON in cio
      mtcptr->pulse_c[5] =  findLJchanCIO(ch1);           // beam ON in cio
      break;
    case 4:
      mtcptr->pulse_e[6] =  findLJchanEIO(ch0);            // trig channel
      mtcptr->pulse_c[6] =  findLJchanCIO(ch0);           // beam ON in cio
      mtcptr->pulse_e[7] =  findLJchanEIO(ch1);             // bkg channel
      mtcptr->pulse_c[7] =  findLJchanCIO(ch1);           // beam ON in cio
      break;
    case 5:
      mtcptr->pulse_e[8] =  findLJchanEIO(ch0);           // laser lite values
      mtcptr->pulse_c[8] =  findLJchanCIO(ch0);           // beam ON in cio
      mtcptr->trig[0]  =  findLJchanEIO(ch1);            // trig channel
      mtcptr->trig[1]  =  findLJchanCIO(ch1);           // beam ON in cio
      mtcptr->beam[0]  =  findLJchanEIO(ch1);            // trig channel
      mtcptr->beam[1]  =  findLJchanCIO(ch1);           // beam ON in cio
      mtcptr->meas[0]  =  findLJchanEIO(ch1);            // trig channel
      mtcptr->meas[1]  =  findLJchanCIO(ch1);           // beam ON in cio
      break;
    case 6:
      mtcptr->kck[0] =  findLJchanEIO(ch0);            // kicker values
      mtcptr->kck[1] =  findLJchanCIO(ch0);            // kicker values
      mtcptr->bkg[0] =  findLJchanEIO(ch1);             // bkg channel
      mtcptr->bkg[1] =  findLJchanCIO(ch1);           // beam ON in cio
      mtcptr->bon.ms = t0;                      // load ms times into structure
      mtcptr->bon = time_In_ms(mtcptr->bon);   // set sec and us values in structure
      break;
     case 7:
      mtcptr->mtc[0] =  findLJchanEIO(ch0);            // MTC move values
      mtcptr->mtc[1] =  findLJchanCIO(ch0);
      mtcptr->lite[0] =  findLJchanEIO(ch1);
      mtcptr->lite[1] =  findLJchanCIO(ch1);           // beam ON in cio
      mtcptr->boff.ms = t0;                      // load ms times into structure
      mtcptr->boff = time_In_ms(mtcptr->boff);   // set sec and us values in structure
      break;
    case 8:                                     // timer-counter start for controlling laser pulses
      tcStart =  findLJchanFIO(ch0);            //missing fio6?
      mtcptr->tmove.ms = t0;
      mtcptr->tmove= time_In_ms(mtcptr->tmove);  // set sec and us values in structure
     default:
      break;*/
    }
  }

/*  
  Setup pause times and labjack
*/
  mtcptr->pon.ms = 0;                        // set default pause values to 0
  mtcptr->pon= time_In_ms(mtcptr->pon);      // set sec and us values in structure

  jj=0;
  mtcptr->lj = lj;
  mtcptr->ljnum = ljnum;                     // store labjack index no. for relation to specific lj calibration
  ljmax=labjackSetup(lj,jj,ljnum);           // set up the labjack of the first one found

  return(ljmax); 
}

/*********************************************************************************/

/*********************************************************************************/
int labjackSetup(long int lj, int num, int ljnum){
  //  long int localID=-1;
  long int count=0;
  long int error=0;
  HANDLE ljh;
  u3CalibrationInfo caliInfo;
  int kk=0, ljmax = 0;
/*
  Open first found U6 over USB
*/
  printf("opening usb .... ");

  /*while (count < 5) {
    //  if( (degptr->temps[num].ljh = openUSBConnection(localID)) == NULL){
    if( (ljh = openUSBConnection(lj)) == NULL){
      count++;
      printf("Opening failed; reseting and attempting %li of 5 more times \n",count);
      printf ("....U3 device reset twice \n");
      LJUSB_ResetConnection(ljh);
      sleep(2);
      LJUSB_ResetConnection(ljh);
      sleep(2);
    } 
    else {
      count = 5;
    }
  }*/

  printf("opened usb\n");

  /*kk = ljnum;

  labj[kk].hand = ljh;            // use later in labjack structure
  labj[kk].lj = lj;
  mtcptr->ljh = ljh;                      // file opened...load pointer and use later in channel structure
  mtcptr->lj = lj;      */                 // file opened...load pointer and use later in channel structure
/*
  Get calibration information from U6
*/
  printf("getting calib .... ");
  //error = getCalibrationInfo(mtcptr->ljh, &caliInfo);

  /*if(error != 0){
    printf("\n%li - %s\n",error, errormsg[error]);
    closeUSBConnection(mtcptr->ljh);
    return (0);
  } 
  else {
    labj[kk].caliInfo = caliInfo;                // load calibration info in struct model
    printf("got calib \n");
    ljmax++;                          // number of labjacks successfully set up
  }*/

  printf("Completed setup of LabJack SN %li \n",mtcptr->lj);

  return (ljmax);
}


/*********************************************************************************/
uint8  findLJchanEIO(char *aaa){
  uint8 ii;
  ii=0;                           // set = 0 as default and in case its a cio channel
  if (strcmp(aaa,"eio0\0") == 0) ii = 0x01;
  if (strstr(aaa,"eio1\0") != NULL) ii = 0x02;
  if (strstr(aaa,"eio2\0") != NULL) ii = 0x04;
  if (strstr(aaa,"eio3\0") != NULL) ii = 0x08;
  if (strstr(aaa,"eio4\0") != NULL) ii = 0x10;
  if (strstr(aaa,"eio5\0") != NULL) ii = 0x20;
  if (strstr(aaa,"eio6\0") != NULL) ii = 0x40;
  if (strstr(aaa,"eio7\0") != NULL) ii = 0x80;
  //printf ("lj eio chan = %x\n",ii);
  return (ii);
}
/*********************************************************************************/
uint8  findLJchanCIO(char *aaa){
  uint8 ii;
  ii=0;                        // set = 0 as default and in case its a eio channel
  if (strstr(aaa,"cio0\0") != NULL) ii = 0x01;
  if (strstr(aaa,"cio1\0") != NULL) ii = 0x02;
  if (strstr(aaa,"cio2\0") != NULL) ii = 0x04;
  if (strstr(aaa,"cio3\0") != NULL) ii = 0x08;

  //printf ("lj cio chan = %x\n",ii);
  return (ii);
}

/*********************************************************************************/
long int  findLJchanFIO(char *aaa){                   // used to id digital i/o signals
  long int ii;
  ii=0;                        // set = 0 as default and in case its a eio channel
  if (strstr(aaa,"fio0\0") != NULL) ii = 0;
  if (strstr(aaa,"fio1\0") != NULL) ii = 1;
  if (strstr(aaa,"fio2\0") != NULL) ii = 2;
  if (strstr(aaa,"fio3\0") != NULL) ii = 3;
  if (strstr(aaa,"fio4\0") != NULL) ii = 4;
  if (strstr(aaa,"fio5\0") != NULL) ii = 5;
  if (strstr(aaa,"fio6\0") != NULL) ii = 6;
  if (strstr(aaa,"fio7\0") != NULL) ii = 7;

  //  printf ("lj cio chan = %x\n",ii);
  return (ii);
}
/*********************************************************************************/