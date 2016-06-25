//
//  scan-map.c
//  
//
//  Created by Carl Gross on 4/1/14.
//
//

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
// put include files for memory mapping
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>           // include file for open/close/return commands
#include <time.h>
//#include <vector>
//#include <iterator>
//#include <algorithm>
#include <stdint.h>

// 64-bit unsigned integer clock in picoseconds is good for 213.5 days
// 16 F's = 64 binary 1's = 18 446 744 073 709 551 615 ps = 18 446 744 s = 5124 hr = 213.5 d
int mmapListDataInp(char *infile);                // sets up the mapped memory
int openFileInp(char *oname);                     // opens list mode file and calls mmapSetup
void closeFileInp(int ifile);                     // unmaps/closes list mode file
//uint64_t *tolptr;                                 // mapped array of ints (64 bit = 8 bytes)...
uint32_t *tolptr;                                 // mapped array of ints (64 bit = 8 bytes)...

struct evt{
    uint64_t ps;           // 64-bit clock given in picoseconds  ... this could contain 48-bit clock + CFD information
    uint16_t id;           // encoding of mod-chan ID = chan + 16*slot + 256*crate : Pixie has 16 channels max in 14-slot crates up to 16 crates
     int16_t status;       // bits set 2 = sums, 4 = qdc, 8 = trace active, 16 = pile-up, 32 = trace present
    uint16_t energy;       // energy up to 65535
    uint16_t lent;         // length of trace     // 128-bit aligned
    uint32_t sum[4];       // 128-bit aligned
    uint32_t qdc[8];       // 128-bit aligned
    uint16_t trace[4000];  // must be divisible by 8 to be 128-bit aligned
};

off_t fsize=0;

/**************************************************************************/
int main(int argc, char **argv) {
    struct stat sb;             // file status structure for fstat
    int ifile=0, ofile=0;
    char oname[190]="\0";      // name of file to open
    uint64_t x0=0, xsave;
    int ii=0, jj=0, iimax=0, tt[1000], ss=0, rr=0, pp=0, nn=0, oo=0;

  union int16int32int64 {
    uint64_t x64;           // 64-bit clock
    uint32_t x32[2];
    uint16_t x16[4];        // 32-bit clock parts from pixie
  } w,x,y,z;
  /*
  union int32int64 {
    uint64_t c64;           // 64-bit clock
    uint32_t c32[2];
  } w;
  */
   int S='S', P='P', I='I', L='L', T='T', A='A';
   int spill=0, counts=0;
   int cnt0=0, cnt1=0;
   struct evt n;
   int wallClock=0, lenUser=0, lenSpill=0;
   uint64_t lastclk=0;
/*
     Input LDF-equivalent file on command line
*/
    if (argc == 2) {
        strcpy(oname,argv[1]);
    }
    else {
        printf(" No tol file defined....input file name:\n");
        scanf ("%s",oname);
    }
    ifile=openFileInp(oname);
    //    fstat(ifile,&sb);
    //    intFilesize = sb.st_size/sizeof(int);
    //    printf ("file size in ints = %i\n",intFilesize);      // file is open and size determined
    iimax = (int) fsize/(sizeof(uint32_t));
    //    iimax = (int) fsize/(sizeof(uint64_t));
    for (ii=0;ii<1000;ii++){
      tt[ii]=0;                       // event array
    }
// change from 64 to 32 here
//    for (ii=0;ii<iimax; ii=ii+2){
    for (ii=0;ii<iimax; ii=ii+4){
      //      w.x64 = tolptr[ii];             // ps clock
      w.x32[0] = tolptr[ii];             // ps clock
      w.x32[1] = tolptr[ii+1];             // ps clock
      //      x.x64 = tolptr[ii+1];           // other header data
      x.x32[0] = tolptr[ii+2];           // other header data
      x.x32[1] = tolptr[ii+3];           // other header data
      if (w.x32[0]==S && w.x32[1]==P && x.x32[0]==I && x.x32[1]==L) {        // process spill header and user data
	//	w.x64 = tolptr[ii+2];
	w.x32[0] = tolptr[ii+4];
	w.x32[1] = tolptr[ii+5];
	x.x64 = tolptr[ii+3];           // other header data
	x.x32[0] = tolptr[ii+6];           // other header data
	x.x32[1] = tolptr[ii+7];           // other header data
	spill     = w.x32[0];
	lenSpill  = w.x32[1] - ii;
	wallClock = x.x32[0];
	lenUser   = x.x32[1];
	//	ii += 2;                                                       // another 2 gets added at end of for loop
	ii += 4;                                                       // another 2 gets added at end of for loop
	if (lenUser != 0) ii += (lenUser-1)/2;
	printf (" Spill number: %i in ii = %i and spill length: %i \n",spill, ii, w.x32[1]);
	cnt1++;  // spill counter
      }
      else {                                                                 // process pixie data
	n.ps     = w.x64;
	n.id     = x.x16[0];
	n.status = x.x16[1];  //  0x01, 02, 04, 08 = header length 4, 8, 12, 16
			      //  0x10 = pile up, 0x20 = trace taken, 0x40 = saturation bit, 0x80 = virtual bit
	n.energy = x.x16[2];
	n.lent   = x.x16[3];
	printf("n.id=%i, n.status=0x%x, n.energy=%i, n.lent=%i\n",n.id,n.status,n.energy,n.lent);

	if (n.status & 0x02 || n.status & 0x08) {   // header length 8 or 16 (4 uint32 sums in header)
	  //	  memcpy(&n.sum[0],&tolptr[ii+2],16);
	  //	  ii += 2;
	  memcpy(&n.sum[0],&tolptr[ii+4],16);
	  ii += 4;
	}
	if (n.status & 0x04 || n.status & 0x08) {  // header length 12 or 16  (16 uin16 qdcs in header)
	  //	  memcpy(&n.qdc[0],&tolptr[ii+2],32);
	  //	  ii += 4;
	  memcpy(&n.qdc[0],&tolptr[ii+4],32);
	  ii += 8;
	}

	if (n.status & 0x20 || n.lent != 0) {
	  cnt0++;
	  //	  memcpy(&n.trace[0],&tolptr[ii+2],n.lent*2);
	  //	  ii += (n.lent*2)/sizeof(uint64_t);
       	  memcpy(&n.trace[0],&tolptr[ii+4],n.lent*2);
	  ii += (n.lent*2)/sizeof(uint32_t);
	}

	if (n.id < 1000 && n.id > -1) tt[n.id]++;
	else {
	  rr++;
	  printf ("id out of range: %i \n", n.id);
	}
	if (n.id == 0){
	  printf (" chan 0 in spill %i at position %i or byte %i \n",spill,ii, ii*8);
	  printf ("x0 = %x    x1 = %x  \n",w.x64,x.x64);
	}

	if (n.ps < lastclk) ss++;      // time out of sequence
	lastclk = n.ps;

	if (n.status & 0x10) pp++;     // pile-up events
	if (n.status & 0x40) oo++;     // saturation events
	if (n.status & 0x80) nn++;     // virtual events

      }
/* 
  Check if poll might have died unnaturally leaving a file filled with zeroes at the end.
  So test if you have 128 bits of zeroes as evidence for that and set ii = iimax to end the loop
*/
      if (w.x64 == 0 && x.x64 == 0) ii = iimax; 
      //      if (spill == 400) break;
    }
    printf ("data out of time sequence  : %i\n",ss);
    printf ("id out of range            : %i\n",rr);
    printf ("piled up events            : %i\n",pp);
    printf ("saturation events          : %i\n",oo);
    printf ("virtual events             : %i\n",nn);
    printf ("events with traces         : %i\n",cnt0);
    printf ("number of spills processed : %i\n",cnt1);
    for (ii=2;ii<1000;ii++){
      if (tt[ii] != 0) {
	printf ("ii = %i, tt = %i\n",ii,tt[ii]);
	counts += tt[ii]; 
      }
    }
    printf ("total events  : %i\n",counts);
    closeFileInp(ifile);
    //    printf ("wcount=%li   scount=%li   count=%li  \n", wcount,scount,count);
    
    return EXIT_SUCCESS;
}

/**************************************************************/
int openFileInp(char *oname) {
    int fd;
    char infile[200]="\0";
/*
     Function to open and map list mode output file
     See poll for how file names auto incremented and limied to 1 GB in size
*/
    strcpy(infile,oname);  // require full name to allow skipping already processed files
    fd = mmapListDataInp(infile);     // call file open and map the array pointer tolptr
    
    return (fd);
}

/**************************************************************/
void closeFileInp(int ifile) {
    /*
     Function to unmap and close list mode output file
     See poll for how file is opened, setup, truncated at end and closed
     */
    munmap(tolptr,sizeof (*tolptr));
    close(ifile);                                  // close in file
    printf("Input file unmapped and closed...\n");
    
    return;
}

/**************************************************************/
/**************************************************************/
int mmapListDataInp(char *infile) {
/*
    Function to open and memory map the list mode file
*/
    int fd=0;                // mapped file descriptor
    ssize_t result=0;
    //    off_t fsize=0;
    
    printf("Setting up memory mapped file: %s\n", infile);
/*
     Open a file for writing.
     - Creating the file if it doesn't exist.
     - Truncating it to 0 size if it already exists. (not really needed)
     Note: "O_WRONLY" mode is not sufficient when mmaping.
*/
    //printf("Setting up memory mapped file: %s\n", (char *)FILEPATH);
    // read/write/create/overwrite if exists/truncate to 0 size      u:rw, g: o:
    //    fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    //
    fd = open(infile, O_RDWR, (mode_t)0644);
    if (fd == -1) {
        perror("Error opening file path for writing");
        exit(EXIT_FAILURE);
    }
    fsize = lseek(fd,0,SEEK_END);
    printf ("File size: %i bytes\n",(int)fsize);
    /*
     Now the file is ready to be mmapped.
*/
//	xxxptr = (int*) mmap(0, LISTDATA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    tolptr = (uint32_t*) mmap(0, fsize, PROT_READ, MAP_SHARED, fd, 0);          // cannot write to file (I hope!)
    //        tolptr = (uint64_t*) mmap(0, fsize, PROT_READ, MAP_SHARED, fd, 0);          // cannot write to file (I hope!)
    if (tolptr == MAP_FAILED) {
        close(fd);
        perror("Error mmapping the input file");
        exit(EXIT_FAILURE);
    }
    
/*
     Don't forget to free the mmapped memory usually at end of main
*/
    return (fd);
}
