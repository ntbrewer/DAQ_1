/*********************************************************************/
/* Program usr2xia is used to write a binary buffer of the           */
/*    LabJack U3-HV or U3-LV temperatures near the experiment        */
/*                                                                   */
/* To be compiled with:                                              */
/*    gcc -o usr2xia usr2xia.c                                       */
/*                                                                   */
/* C. J. Gross, December 2014                                        */
/*********************************************************************/

#include <fcntl.h>
#include <sys/mman.h>
#include "../include/u3.h"                     // location: /usr/local/include (required for labjack HANDLE typedef)
#include "../include/kelvin.h"                 // location: /usr/local/include
#include "../include/lnfill.h"                 // location: /usr/local/include
#include "../include/usrdata.h"                // location: /usr/local/include
#include "../include/hv-caen.h"                // location: /usr/local/include
//#include "../include/daqcom.h"                 // location: /usr/local/include


//#define COMDATAPATH "data/daqcom.bin"             // com data for control of daq programs
//#define COMDATASIZE sizeof(struct comData)

//#define USRDATAPATH "data/usrdata.bin"             // com data for control of daq programs
//#define USRDATASIZE sizeof(usrData)

//#define KELVINDATAPATH "data/kelvin.bin"            // user data for input to xia2disk
//#define KELVINDATASIZE sizeof(struct thermometer)

#define GELNDATASIZE sizeof(struct lnfill)

#define INTERVAL 60

//int openDaqCom();
//void closeDaqCom(int mapCom);
int openTherm();
void closeTherm(int mapTherm);
void buffTherm(int begTherm, int lenTherm);

int openLN();
void closeLN(int mapLN);
void buffLN(int begLN, int lenLN);

int openHVcaen();
void closeHVcaen(int mapHVcaen);
void buffHVcaen(int begHVcaen, int lenHVcaen);

int openUserData();
void closeUserData(int mapUser);

//const int sizeData = 1024;
//int usrData[1024];        // user data buffer to contain data for thermometers, LN, HV, etc.
//int *usrptr;              // mapped array pointing to usrData

time_t timeTherm = 0;
int initTherm = 0;
time_t timeLN = 0;
int initLN = 0;
time_t timeHVcaen = 0;
int initHVcaen = 0;

/***********************************************************/
int main(int argc, char **argv){
  int mapUser=-1;
  int mapTherm=-1, begTherm=0, lenTherm=0;               // positions for thermometer data
  int mapLN=-1, begLN=0, lenLN=0;                        // positions for LN data
  int mapHVcaen=-1, begHVcaen=0, lenHVcaen=0;            // positions for HVcaen data
  int lenData = 1;             // length of usr data in usrBuf lenBuf =1 to ensure at least length of user data is counted even if its 0
  //  time_t curtime=0, time0=0;

/*
  Memory creation setup and attachment for usrBuf that holds mapped data for sharing
  Memory setup and attachment - see main program for creation and closing
*/
  mapUser = openUserData();           // memory mapped user data file
    //    mapCom = openDaqCom();          // memory mapped daq command file 
/*
    printf ("daq file: %s\n",daqptr->daqfile);
    printf ("daq: %i\n",daqptr->daq);
    printf ("usr2daq: %i\n",daqptr->usr2daq);
    printf ("kelvin: %i\n",daqptr->kelvin);
*/
  printf ("Setting up data from program kelvin\n");
  mapTherm = openTherm();                   // Open mapped file on disk file descriptor mapTherm = -1 if failure
  if (mapTherm != -1 ) {
    timeTherm = degptr->tim.time1;          // load time based on what is in shared memory
    degptr->com3 = 1;                       // set daq logging flag on (only writing to map from this program)
    begTherm = lenData;                     // start of therm data in user buf => [0] will be actual lenData
    lenTherm = degptr->maxchan + 3;         // length of therm data in user buff (data length, header, time, sensor data)
    lenData += lenTherm;                    // total data that will be in user data
    printf ("timeTherm=%i\n",(int) timeTherm);
  }
  
  printf ("Setting up data from program hv-caen\n");
  mapHVcaen = openHVcaen();                  // Open mapped file on disk file descriptor mapTherm = -1 if failure
  if (mapHVcaen != -1 ) {
    timeHVcaen = hvcptr->secRunning;         // load time based on what is in shared memory
    hvcptr->com4 = 1;                        // set daq logging flag on (only writing to map from this program)
    begHVcaen = lenData;                     // start of therm data in user buf => [0] will be actual lenData
    lenHVcaen = hvcptr->maxchan + 3;         // length of therm data in user buff (data length, header, time, sensor data)
    lenData += lenHVcaen;                    // total data that will be in user data
    printf ("timeHVcaen=%i\n",(int) timeHVcaen);
  }

    /*
      printf ("Setting up data from program lnfill  \n");
  mapLN= openLN();                          // Open mapped file on disk file descriptor mapLN = -1 if failure
  if (mapLN != -1 ) {
    timeLN = lnptr->tim.time1;              // load time based on what is in shared memory
    lnptr->com3 = 1;                        // set daq logging flag on (only writing to map from this program)
    begLN = lenData;                        // start of therm data in user buf
    lenLN = lnptr->maxdet + 3;              // length of therm data in user buff
    lenData += lenLN;                       // total data that will be in user data
  }
*/
/*
  length of user data in first array element followed by each user data group and ending on 128-bit boundary 
  => for example, 3 thermometers + time + length of thermometer data = 5 so we need 8 elements to end on 128-bits for 32-bit words
  if no user data available so set lenData for usrBuf[0] = 0;
*/
  if (lenData == 1) lenData = 0;            // no user data available so set lenData = 0 for usrBuf[0] = 0;
  usrptr->data[0] = 1 + lenData + (4 - (lenData % 4)); 
  //      if (data[0] == 1) lenData = 0;            // no user data available so set lenData for usrBuf[0] = 0;
  printf("data[0] +1 for the length of all user data = %i \n",usrptr->data[0]);
  
/******************************** Loop here **********************************************
   Below: check if data has changed in each program every 5 seconds
*/
  time0 = time(NULL);
  sleep(5);
  while (usrptr->com0 != 2){                  // could be that if endData != 0 is sufficient   // loop over loading user data       
    
    if (usrptr->kelvin){
      if (degptr->tim.time1 >= timeTherm) {      // check if its been 60 s since last thermometer update
	buffTherm(begTherm, lenTherm);           // load data to thermometer data to buffer
	usrptr->kelvinTime = degptr->tim.time1;          // load time into usrptr for kelvin time
      }                    
    }
    if (usrptr->geLN){
      if (lnptr->secRunning >= timeLN) {      // check if its been 60 s since last thermometer update
	buffLN(begLN, lenLN);                         // load data to thermometer data to buffer
      }
    }
    if (usrptr->caenHV){
      if (hvcptr->time1 >= timeHVcaen) {         // check if its been 60 s since last thermometer update
	buffHVcaen(begHVcaen, lenHVcaen);                         // load data to thermometer data to buffer
      }
    }
    
    // repeat above for LN, HV or other user data coming in per minute with an || on while statement
    
    usrptr->time = time(NULL);                // time of last check ...proof program is running
    //	memcpy(usrptr,(void *) usrptr->data[0], (sizeof(int)*lenData));   // copy data to added user data buffer
    sleep(5);           // sleep a short amount so we are not burning cycles...
  }
  
/*
   Wrap up the program ending the program
*/
  if (mapTherm != -1 ) closeTherm(mapTherm);           // if thermometers active, unmap and close
  if (mapLN != -1 ) closeLN(mapLN);           // if thermometers active, unmap and close

  if (mapUser != -1) closeUserData(mapUser);
  return 0;
}

/*************************************************************************************************************************************************/
/*************************************************************************************************************************************************/
/**************************************** Room Temperature Monitoring Data Below *****************************************************************/

int openTherm() {
  int fd;                 // mapped file descriptor
  //  char fileTherm[200] = "data/mmapped.bin";
/* Open a file for writing.
     *  - Creating the file if it doesn't exist.
     *  - Truncating it to 0 size if it already exists. (not really needed)
     *
     * Note: "O_WRONLY" mode is not sufficient when mmaping.
*/
  fd = open(KELVINDATAPATH, O_RDWR, (mode_t)0600);
  if (fd == -1) {
    perror("Error opening for writing ");
    printf (" %s \n",KELVINDATAPATH);
    exit(EXIT_FAILURE);
  }
        
/* Now the file is ready to be mmapped.
*/
  degptr = (struct thermometer*) mmap(0,KELVINDATASIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (degptr == MAP_FAILED) {
    close(fd);
    perror("Error mmapping the file kelvin data");
    exit(EXIT_FAILURE);
  }
/* Don't forget to free the mmapped memory ... usually at end of main
*/
    
  return (fd);
}


/**************************************************************/
void closeTherm(int mapTherm){

  if (mapTherm != -1 ) {            // if thermometers active
    degptr->com3 = 0;               // set daq logging flag off (writing to SHM from this program at start/stop only)
    if (munmap(degptr, sizeof (struct thermometer*)) == -1) {   // unmap memory
      perror("Error un-mmapping the thermometer file");
    }
    close(mapTherm);                // close thermometer file
  }

  return;
}

/**************************************************************/
void buffTherm(int begTherm, int lenTherm){
  int ii=0;
  unsigned int buff[32];       // 16 pairs of id,data
  union bit16bit32 {
    unsigned int valu;                 // 32-bit computer clock
    unsigned short int s[2];        // two 16-bit computer clock
  } x;
  union charbit32 {
    unsigned int KELV;                 // 32-bit computer clock
    char c[4];        // two 16-bit computer clock
  } name;
  
  name.c[3]='K';
  name.c[2]='E';
  name.c[1]='L';
  name.c[0]='V';

  printf ("name: %u \n",name.KELV);
  if (initTherm == 0){
    for (ii=0; ii<32; ii++) buff[ii] = 0;    // set entire buffer to 0      
    initTherm = 1;
  }
/*
     Load clock into buffer
*/
  buff[0] = degptr->maxchan + 3;   // length of temperature data including this one and time below
  buff[1] = name.KELV;             // name of data type.....KELV for kelvin data
  buff[2] = degptr->tim.time1;     // 32-bit wall clock  => used to check program is still active
/*
     Load thermometer data into buffer
*/
  for (ii=0; ii< degptr->maxchan; ii++){
    x.s[0] = degptr->temps[ii].data;           // data written at 32-bits so x.s[0] occurs before x.s[1]
    x.s[1] = degptr->temps[ii].para;
    buff[ii+3] = x.valu;
    //    printf(" data=%i   %i\n",x.s[1],x.s[0]);
  }

  timeTherm = degptr->tim.time1 + INTERVAL;    // time to next read ..ie, last read time + INTERVAL +/- sleep time below 
  memcpy(&usrptr->data[begTherm],(void *) buff, (sizeof(int)*lenTherm));   // copy data to added user data buffer

  //  for (ii=0; ii< usrptr->data[0]+1; ii++) printf ("%i => %x    %i \n",ii, usrptr->data[ii], usrptr->data[ii]);

  return;
}

/****************************************************************************************************************************************************/
/*********************************************** Liquid Nitrogen Data Handling Below ****************************************************************/

int openLN() {
  int fd;                 // mapped file descriptor
  char fileLN[200] = "../data/lnfill.bin";
/* Open a file for writing.
     *  - Creating the file if it doesn't exist.
     *  - Truncating it to 0 size if it already exists. (not really needed)
     *
     * Note: "O_WRONLY" mode is not sufficient when mmaping.
*/
  fd = open(fileLN, O_RDWR, (mode_t)0600);
  if (fd == -1) {
    perror("Error opening for writing ");
    printf (" %s \n",fileLN);
    exit(EXIT_FAILURE);
  }
        
/* Now the file is ready to be mmapped.
*/
  lnptr = (struct lnfill*) mmap(0, GELNDATASIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (lnptr == MAP_FAILED) {
    close(fd);
    perror("Error mmapping the file");
    exit(EXIT_FAILURE);
  }
/* Don't forget to free the mmapped memory ... usually at end of main
*/
    
  return (fd);
}

/**************************************************************/
void closeLN(int mapLN){

  if (mapLN != -1 ) {            // if LN active
    lnptr->com2 = 0;               // set daq logging flag off (writing to SHM from this program at start/stop only)
    if (munmap(lnptr, sizeof (struct lnfill*)) == -1) {   // unmap memory
      perror("Error un-mmapping the LN file");
    }
    close(mapLN);                // close LN file
  }

  return;
}
/**************************************************************/
void buffLN(int begLN, int lenLN){
  int ii=0;
  unsigned int buff[30];       // header + maximum 1000 HV channels pairs of id,data
  union bit16bit32 {
    unsigned int valu;                 // 32-bit computer clock
    unsigned short int s[2];        // two 16-bit computer clock
  } x;
  union charbit32 {
    unsigned int LNFI;                 // 32-bit computer clock
    char c[4];        // two 16-bit computer clock
  } name;
  
  name.c[3]='L';
  name.c[2]='N';
  name.c[1]='F';
  name.c[0]='I';

  printf ("name: %u \n",name.LNFI);
  if (initLN == 0){
    for (ii=0; ii<30; ii++) buff[ii] = 0;    // set entire buffer to 0      
    initLN = 1;
  }
/*
     Load clock into buffer
*/
  buff[0] = 3;                    // length of temperature data including this one and time below
  buff[1] = name.LNFI;            // name of data type.....KELV for kelvin data
  buff[2] = lnptr->time1;         // 32-bit wall clock  => used to check program is still active
/*
     Load thermometer data into buffer
*/
  for (ii=0; ii< 20; ii++){    // maximum HV channels is hardcoded to 1000 channels
    if (lnptr->ge[ii].onoff == 1) {
      x.s[0] = (unsigned short int)(lnptr->ge[ii].rtd*10);         // data written at 32-bits so x.s[0] occurs before x.s[1]
      x.s[1] = (unsigned short int)(lnptr->ge[ii].u1);
      buff[ii+2] = x.valu;
      buff[0]++;
    //    printf(" data=%i   %i\n",x.s[1],x.s[0]);
    }
  }

  timeLN = lnptr->secRunning + INTERVAL;    // time to next read ..ie, last read time + INTERVAL +/- sleep time below 
  memcpy(&usrptr->data[begLN],(void *) buff, (sizeof(int)*lenLN));   // copy data to added user data buffer

  //  for (ii=0; ii< usrptr->data[0]+1; ii++) printf ("%i => %x    %i \n",ii, usrptr->data[ii], usrptr->data[ii]);

  return;
}

/****************************************************************************************************************************************************/
/********************************************** High Voltage CAEN Data Handling Below ***************************************************************/

int openHVcaen() {
  int fd;                 // mapped file descriptor
  char fileHVcaen[200] = "../data/hv-caen.bin";
/* Open a file for writing.
     *  - Creating the file if it doesn't exist.
     *  - Truncating it to 0 size if it already exists. (not really needed)
     *
     * Note: "O_WRONLY" mode is not sufficient when mmaping.
*/
  fd = open(fileHVcaen, O_RDWR, (mode_t)0600);
  if (fd == -1) {
    perror("Error opening for writing ");
    printf (" %s \n",fileHVcaen);
    exit(EXIT_FAILURE);
  }
        
/* Now the file is ready to be mmapped.
*/
  hvcptr = (struct hvcaen*) mmap(0, HVCAENDATASIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (hvcptr == MAP_FAILED) {
    close(fd);
    perror("Error mmapping the file");
    exit(EXIT_FAILURE);
  }
/* Don't forget to free the mmapped memory ... usually at end of main
*/
    
  return (fd);
}

/**************************************************************/
void closeHVcaen(int mapHVcaen){

  if (mapHVcaen != -1 ) {            // if HVcaen active
    hvcptr->com4 = 0;               // set daq logging flag off (writing to SHM from this program at start/stop only)
    if (munmap(hvcptr, sizeof (struct hvcaen*)) == -1) {   // unmap memory
      perror("Error un-mmapping the LN file");
    }
    close(mapHVcaen);                // close HV caen file
  }

  return;
}

/**************************************************************/
/**************************************************************/
void buffHVcaen(int begHVcaen, int lenHVcaen){
  int ii=0, jj=0;
  unsigned int buff[3003];       // header + maximum 1000 HV channels pairs of id,data
  union bit16bit32 {
    unsigned int valu;                 // 32-bit computer clock
    unsigned short int s[2];        // two 16-bit computer clock
  } x;
  union charbit32 {
    unsigned int CAEN;                 // 32-bit computer clock
    char c[4];        // two 16-bit computer clock
  } name;
  
  name.c[3]='C';
  name.c[2]='A';
  name.c[1]='E';
  name.c[0]='N';

  printf ("name: %u \n",name.CAEN);
  if (initHVcaen == 0){
    for (ii=0; ii<3000; ii++) buff[ii] = 0;    // set entire buffer to 0      
    initHVcaen = 1;
  }
/*
     Load clock into buffer
*/
  buff[0] = hvcptr->maxchan + 3;   // length of temperature data including this one and time below
  buff[1] = name.CAEN;             // name of data type.....KELV for kelvin data
  buff[2] = hvcptr->time1;         // 32-bit wall clock  => used to check program is still active
/*
     Load thermometer data into buffer
*/
  jj=3;
  for (ii=0; ii< hvcptr->maxchan; ii++){    // maximum HV channels is hardcoded to 1000 channels
    x.s[0] = hvcptr->xx[ii].chan;           // data written at 32-bits so x.s[0] occurs before x.s[1]
    x.s[1] = hvcptr->xx[ii].slot;
    buff[jj++] = x.valu;
    buff[jj++] = (unsigned int)(hvcptr->xx[ii].vMeas*10);     // measured voltage *10 to get tenth of volts
    buff[jj++] = (unsigned int)(hvcptr->xx[ii].iMeas);      // measured current in uAmps
    //    printf(" data=%i   %i\n",x.s[1],x.s[0]);
  }

  timeHVcaen = hvcptr->secRunning + INTERVAL;    // time to next read ..ie, last read time + INTERVAL +/- sleep time below 
  memcpy(&usrptr->data[begHVcaen],(void *) buff, (sizeof(int)*lenHVcaen));   // copy data to added user data buffer

  //  for (ii=0; ii< usrptr->data[0]+1; ii++) printf ("%i => %x    %i \n",ii, usrptr->data[ii], usrptr->data[ii]);

  return;
}

/****************************************************************************************************************************************************/
/*****************************************************************************************************************************************************/
/****************************************** User Data Section Below **********************************************************************************/
int openUserData() {
    int fd=0;                // mapped file descriptor
    //    char fileData[200] = "data/usrdata.bin";
    //    int zero=0;
    ssize_t result=0;
    /*
     Open a file for writing.
     - Creating the file if it doesn't exist.
     - read/write/create/fail if exists/truncate to 0 size      u:rw, g:r o:r
     - Truncating it to 0 size if it already exists. (not really needed)
     - include O_RDWR | O_CREAT | O_EXCL | O_TRUNC if you don't want to overwrite existing data
     Note: "O_WRONLY" mode is not sufficient when mmaping.
     */
    
    printf("Setting up memory mapped file: %s\n", USRDATAPATH);
    fd = open(USRDATAPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0644);
    if (fd == -1) {
        perror("Error opening usrdata for writing ");
        printf (" %s \n",USRDATAPATH);
        exit(EXIT_FAILURE);
    }
    /*
     Stretch the file size to the size of the (mmapped) array/structure/etc.
     We can choose to write to the entire file or seek the end and writing 1 word
     */
    printf ("going to end of file\n");
    //    result = lseek(fd, sizeData-1, SEEK_SET);       //268435455 - effectively zeros the array according to man lseek
    result = lseek(fd, USRDATASIZE-1, SEEK_SET);       //268435455 - effectively zeros the array according to man lseek
    if (result == -1) {
        close(fd);
        perror("Error calling lseek() to 'stretch' the file");
        exit(EXIT_FAILURE);
    }
    
    /*    for (int ii=LISTDATA_INTS; ii>0; ii--){
    result = write(fd, "\0", 1);  // just write anything
    if (result != 1) {
        close(fd);
        perror("Error writing last element to be \"0\" to the file");
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
    
     result = write(fd, "\0", 1);
     if (result != 1) {
     close(fd);
     perror("Error writing last byte of the file");
     exit(EXIT_FAILURE);
     }
    
    /*
     Now the file is ready to be mmapped.
     */
     //     usrptr = (int*) mmap(0, sizeData, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  // might be uint32!
     usrptr = (struct usrdat*) mmap(0, USRDATASIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  // might be uint32!
     if (usrptr == MAP_FAILED) {
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }
    
    /*
     Don't forget to free the mmapped memory usually at end of main
     */
    return (fd);
}

/**************************************************************/
void closeUserData(int mapUser){
    
  if (mapUser != -1 ) {                                  // if file is active
    if (munmap(usrptr, sizeof (struct usrptr*)) == -1) {      // unmap memory
      perror("Error un-mmapping the user data file");
    }
    close(mapUser);                                    // close usrData file
  }
  
  return;
}

/**************************************************************/


