/******************************************************************/
/* Program kicker is used to control the BNC-632 Waveform         */
/*    Generator to control the beam to LeRIBSS                    */
/*                                                                */
/* C. J. Gross, February 2009                                     */
/******************************************************************/

#include <stddef.h>
#include <limits.h>
#include <string.h>  /* String function definitions */
#include <float.h>
#include <ctype.h>
#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <math.h>    /* Math definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <signal.h>  /* Signal interrupt definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <errno.h>   /* Error number definitions */
#include <fcntl.h>   /* File control definitions */
#include <time.h>    /* Time definitions */
#include <sys/select.h>
#include <sys/time.h>

typedef void (*sig_t) (int);
sig_t signal(int sig, sig_t func);

int flag;
char *GetDate(void);
void MotorRun(long int loop, float duration, float motor);
void Looper();
void SignalHandle(int sig);

int writeport(int fd, char *data);
int writeport2(int fd, char data[]);
int readport(int fd, char *data);
int readport100(int fd, char data[]);
int getbaud(int fd);
int SerialOpen();

int BNCMode();
int BNCIO(int command);
int BNCFunction();
int BNCSine();
int BNCResetSine();  // manual recommends initializing BNC-632 here (sets offset to 0 V)
int BNCPulse();
int BNCArbitrary();
int BNCStatus();
int BNCFrontOff();
int BNCFrontOn();
int BNCFrontOnOff();
int BNCLCDEchoOn();
int BNCLCDEchoOff();
int BNCLCDEchoOnOff();
int BNC_0();
int BNC_1();
int BNC_2();
int BNC_3();
int BNC_4();
int BNC_5();
int BNC_6();
int BNC_7();
int BNC_8();
int BNC_9();
int BNCMHz();
int BNCkHz();
int BNCHz();
int BNCOffset();
int BNCTrigger();
int BNCDot();
int BNCMinus();
int BNCField();
int BNCFieldLeft();
int BNCFieldRight();
int BNCDigitUp();
int BNCDigitDown();
int BNCDigitLeft();
int BNCDigitRight();
char BNCCommand(int command);
int Manual();
int PulseF1();
int PulseF2(int duty);
int PulseF3(int trig);
int PulseF5(char mvolt[]);
int ArbF3();
int BNCParameters();

int amplitude=4000, offset=0;
int fd;
long int loop=1,num;
long int lcount=0,lrun=0,rcount=0,rrun=0;
double dur,run;
int dev;

/******************************************************************/
int main () {

  char com1;
  long int type=0;
  //  float duration, motor;
  //  int command, command1;
  long int ii=0,jj,kk;
  char data;
  int stat;
  int x1,x2,x3,x4,x5,x6;

  printf ("Welcome to program kicker \n\n");
  printf ("  This program remotely controls a BNC-632 Waveform Generator via RS-232 \n\n");
  printf ("  You will need the following information \n");
  printf ("    - which type of measurement \n");
  printf ("    - time duration of each stage of measurement \n");
  printf ("    - MTC to provide tape move signal (TTL) \n");
  printf ("    - control-c is used to escape out of any loops \n");
  printf ("    - control-\\ is used to emergency quit the program \n");
  printf (" \n ");

  printf (" Opening serial port ...    ");
  ii=SerialOpen();
  if (ii==1) exit (EXIT_FAILURE);  // end on failed open of serial port
  printf (" ... successful !\n ");
  printf (" Check status of BNC-632 generator ... \n ");
	
//	BNCIO(22);
	
  BNCFrontOff();                // Turn off front panel
  BNCIO(30);                    // Reset device in to known state (sine wave)
  //  BNCStatus();
  /* Test section of code */
  //  com1=BNCCommand(46);
  //writeport(fd,&com1);      // 
  //readport(fd,&data);
  //BNCField();
  //BNC_8();
  //BNC_4();
  //  com1=BNCCommand(1);       // 
  //writeport(fd,&com1);      // 
  //readport(fd,&data);
  /* Test section of code */
  
  //  usleep(500000);           // sleep for 50 ms
  //  usleep(1000);           // sleep for 50 ms
  //if(data == 0x00)stat=99;  // successful outcome, no message
  //if(data == 0x01)stat=1;   // unsuccessful outcome


/*
Serial port opened and settings set to:
  8-bit, no parity, 1 stop bit, baud rate 9600
*/	

  while (flag==0){
    signal(SIGHUP,SignalHandle);
    printf ("What do you want to do ? \n");
    printf ("        1 --- grow-in/decay-out \n");
    printf ("        2 --- multiple kicks per MTC cycle \n");
    printf ("        3 --- single short burst of beam \n");
    printf ("        4 --- stop    beam electrostatically \n");
    printf ("        5 --- release beam electrostatically \n");

    printf ("        6 --- Turn off front panel input (default) \n");
    printf ("        7 --- Turn  on front panel input \n");
    printf ("        8 --- Current status \n");

    printf ("        9 --- manual operation \n");
    printf ("       10 --- read serial port to debug \n"); 

    printf ("       11 --- amplitude,offset parameters \n"); 
	  
    printf ("\n");
    printf ("        0 --- end \n");
    scanf ("%li", &type);          /* reads in ans (pointer is indicated */
    if (0 > type || type > 15) {
	printf("Number out of range \n");
        type = 99;
    }      
    switch ( type ) { 
    case 1:
      printf (" Entering grow-in/decay-out mode ...  \n ");
      GrowDecay();
     break;
    case 2:
      printf (" Entering multiple kick mode ...  \n ");
      break;
    case 3:
      printf (" Entering burst mode ... USE CAREFULLY ... \n ");
      break;
    case 4:
      printf (" Turn beam off \n ");
      break;
    case 5:
      printf (" Turn beam on \n ");
      break;
    case 6: BNCFrontOff();  break;
    case 7:  BNCFrontOn();  break;
    case 8:   BNCStatus();  break;
    case 9: kk = Manual(); break;
    case 10: 
      //      readport(fd,&data);  
      printf("data = %c\n",data);
      break;
	case 11: BNCParameters(); break;
	case 0:
      printf("Kicker ending ....\n",type);
      close(fd);
      exit (EXIT_FAILURE);
      
//    case 11: 
//      printf ("Amplitude voltage in millivolts (integer input) \n");
//      scanf ("%li", &amplitude);     /* reads in ans (pointer is indicated */
      //      printf ("Offset voltage in millivolts (integer input) \n");
      //      scanf ("%li", &offset);     /* reads in ans (pointer is indicated */
      //      break;

    default:
      break;
    }
  }
  return 0;
}


/******************************************************************/
void SignalHandle(int sig) {

  printf("Sig = %i\n",sig);
  if (sig == 2) flag = 2;
  if (sig == 20) printf("reseting signals?\n");
  return;

}

/******************************************************************/
int initport(int fd) {
  struct termios options;
  // Get the current options for the port...
  tcgetattr(fd, &options);
  printf ("Original serial port settings: \n");
  printf ("Serial port configuration c_cflag = %x \n",options.c_cflag);
  printf ("Serial port configuration c_iflag = %x \n",options.c_iflag);
  printf ("Serial port configuration c_oflag = %x \n",options.c_oflag);
  printf ("Serial port configuration c_lflag = %x \n",options.c_lflag);
  /*
  cfmakeraw disables all input/output processing to raw I/O.  effectively
  this zeroes c_iflag, c_oflag, c_lflag in the structure options which sets 
  up the serial port.  I had trouble restarting this program after the
  computer was rebooted after a power outage.  I needed to add these 
  lines with cfmakeraw and checking c_iflag, oflag, and lflag.  Minicom
  was used to help debug the problem as the program could run with minicom
  configuring the port.
  */
  cfmakeraw(&options);
  /*
  options.c_iflag=1;
  options.c_oflag=0;
  options.c_lflag=0;
  */

  // Set the baud rates to 9600...*/
  cfsetispeed(&options, B9600);
  cfsetospeed(&options, B9600);
  // Enable the receiver and set local mode (ignore modem status lines)...
  options.c_cflag |= (CLOCAL | CREAD);
  // below are bit-wise operations; complement sign  ~ 
  options.c_cflag &= ~PARENB;  /* disable parity*/
  options.c_cflag &= ~CSTOPB;  /* disable 2 stop bit*/
  options.c_cflag &= ~CSIZE;   /* disable character mask size*/
  options.c_cflag |= CS8;      /* set 8 bits */
  options.c_cflag &= ~CRTSCTS; /* disables hardware flow control */


  // Set the new options for the port...
  tcsetattr(fd, TCSANOW, &options);  //TCSANOW = change immediately
  tcgetattr(fd, &options);
  printf ("Flip-flop's serial port settings: \n");
  printf ("Serial port configuration c_cflag = %x \n",options.c_cflag);
  printf ("Serial port configuration c_iflag = %x \n",options.c_iflag);
  printf ("Serial port configuration c_oflag = %x \n",options.c_oflag);
  printf ("Serial port configuration c_lflag = %x \n",options.c_lflag);
  return 1;
}

/******************************************************************/
int getbaud(int fd) {
  struct termios termAttr;
  int inputSpeed = -1;
  speed_t baudRate;
  tcgetattr(fd, &termAttr);
  /* Get the input speed.                              */
  baudRate = cfgetispeed(&termAttr);
  switch (baudRate) {
  case B0:      inputSpeed =    0; break;
  case B50:     inputSpeed =   50; break;
  case B110:    inputSpeed =   110; break;
  case B134:    inputSpeed =   134; break;
  case B150:    inputSpeed =   150; break;
  case B200:    inputSpeed =   200; break;
  case B300:    inputSpeed =   300; break;
  case B600:    inputSpeed =   600; break;
  case B1200:   inputSpeed =  1200; break;
  case B1800:   inputSpeed =  1800; break;
  case B2400:   inputSpeed =  2400; break;
  case B4800:   inputSpeed =  4800; break;
  case B9600:   inputSpeed =  9600; break;
  case B19200:  inputSpeed = 19200; break;
  case B38400:  inputSpeed = 38400; break;
  }
  return inputSpeed;
}

/*****************************************************************/
int SerialOpen(){
  fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd == -1) {
    perror("\nopen_port: Unable to open /dev/ttyS0 - \n");
    return 1;
  } else {
    fcntl(fd, F_SETFL, 0);
  }
  
  printf("baud=%d\n", getbaud(fd));
  initport(fd);
  printf("baud=%d\n", getbaud(fd));
  return;
}

/******************************************************************/
int portflush(int fd) {
	int len=1, n=0;
	
	n = 1;
	while (n != 0){
		n = tcflush(fd,TCIFLUSH);
	}
	if (n == 0) printf("successful buffer flush \n");

	return 1;
}

/******************************************************************/
int portwrite1(int fd, char *data) {
	int len=1, n=0;
	
	n = write(fd, data, len);
	usleep(500000);   // add a pause (0.5 sec) to allow module to process/respond
	if (n < 0) {
		fputs("write failed!\n", stderr);
		return 0;
	}
	
	return 1;
}

/******************************************************************/
int portwrite2(int fd, char data2[]) {
	int len=2, n=0;
	
	n = write(fd, data2, len);
	usleep(500000);   // add a pause (0.5 sec) to allow module to process/respond
	if (n < 0) {
		fputs("write failed!\n", stderr);
		return 0;
	}
	
	return 1;
}

/******************************************************************/
int portread(int fd, char *data) {
	int n=0,len=1;
	int numport=0;
	char com1;
	fd_set fds;                      // type definition for select
	struct timeval timeout;          // structure defining timeout for select

	*data='?';                       // set data to known value to ensure it changes

/* FD_SET must be in line above select!   */
	timeout.tv_sec=0;                // time out in seconds
	timeout.tv_usec=300000;          // time out in microseconds
	FD_ZERO(&fds);
	FD_SET(fd,&fds);
	numport = select(FD_SETSIZE,&fds,(fd_set*)NULL,(fd_set*)NULL,&timeout);

// printf("numport = %i\n",numport);
	
	if (numport > 0) {
		n = read(fd, data, len);
	}
	if (n < 0) {
		if (errno == EAGAIN) {
			printf("SERIAL EAGAIN ERROR\n");
			return 0;
		} 
		else {
			printf("SERIAL read error %d %s\n", errno, strerror(errno));
			return 0;
		}
	}                    
	return 1;
}
/******************************************************************/
int portread2(int fd, char *data) {
	int n=0,len=1;
	int numport=0;
	char com1;
	fd_set fds;                      // type definition for select
	struct timeval timeout;          // structure defining timeout for select
/*
   This is to be called after we've picked up the first data character of 
   a multi-character read
*/
	*data='?';                       // set data to known value to ensure it changes
	n = read(fd, data, len);
	
	if (n < 0) {
		if (errno == EAGAIN) {
			printf("SERIAL EAGAIN ERROR\n");
			return 0;
		} 
		else {
			printf("SERIAL read error %d %s\n", errno, strerror(errno));
			return 0;
		}
	}                    
	return 1;
}

/******************************************************************/
int writeport(int fd, char *data) {
  int len=1, n=0;

  n = 1;
  while (n != 0){
    n = tcflush(fd,TCIFLUSH);
  }
  if (n == 0) printf("successful buffer flush \n");
  usleep(5000);           // add a pause (0.5 ms) to allow buffers to purge
  n = write(fd, data, len);
  usleep(500000);   // add a pause (0.5 sec) to allow module to process/respond
  if (n < 0) {
    fputs("write failed!\n", stderr);
    return 0;
  }
  return 1;
}

/******************************************************************/
int readport(int fd, char *data) {
  int n=0,len=1;
  int numbyte=0;
  char com1;
  fd_set fds;                      // type definition for select
  struct timeval timeout;          // structure defining timeout for select

  timeout.tv_sec=3;       // time out in seconds
  timeout.tv_usec=300000; // time out in microseconds

  /* FD_SET must be in line above select!   */
  FD_ZERO(&fds);
  FD_SET(fd,&fds);
  numbyte = select(FD_SETSIZE,&fds,(fd_set*)NULL,(fd_set*)NULL,&timeout);
  // printf("numbyte = %i\n",numbyte);

  *data='?';
  if (numbyte > 0) {
    n = read(fd, data, len);
  }
  /* */

 //  n = read(fd, data, len);

  if (n < 0) {
    if (errno == EAGAIN) {
      printf("SERIAL EAGAIN ERROR\n");
      return 0;
    } else {
      printf("SERIAL read error %d %s\n", errno, strerror(errno));
      return 0;
    }
  }                    
  return 1;
}
/******************************************************************/
int readport110(int fd, char *data110) {
  int n=0,len=107;
  int numbyte=0;
  char com1;

  fd_set fds;                      // type definition for select
  struct timeval timeout;          // structure defining timeout for select

  FD_ZERO(&fds);
  FD_SET(fd,&fds);

  timeout.tv_sec=0;       // time out in seconds
  timeout.tv_usec=300000; // time out in microseconds
  /**/
  numbyte = select(FD_SETSIZE,&fds,(fd_set*)NULL,(fd_set*)NULL,&timeout);
  printf("numbyte = %i\n",numbyte);

  //  *data=BNCCommand(35);
  if (numbyte > 0) {
    n = read(fd, data110, len);
    printf("readport110 n = %i\n",n);
    data110[109]='\0';
    printf("data110 = %s\n",data110);
  }
  /* */

 //  n = read(fd, data, len);

  if (n < 0) {
    if (errno == EAGAIN) {
      printf("SERIAL EAGAIN ERROR\n");
      return 0;
    } else {
      printf("SERIAL read error %d %s\n", errno, strerror(errno));
      return 0;
    }
  }                    
  return 1;
}

/******************************************************************/

/*
  Ltime=dur;
  Rtime=dur;
  Mtime=run;
  printf ("   Loop       Dev     Left    Mot   Right   Mot   \n");
  printf ("-----------  -----  -------   ---  ------   ---   \n");
  printf ("\r%10i   %3i     %4i      %2i   %4i    %2i      starting\n ",loop,dev,lcount,lrun,rcount,rrun);
  //  results(case6(ii));       //step a  switch to +V
  printf ("\r%10i   %3i     %4i      %2i   %4i    %2i      positioned\n ",loop,dev,lcount,lrun,rcount,rrun);
  printf("%s\n",GetDate());
  for (ii=0;ii<num;ii++){
    printf ("\r%10i   %3i     %4i      %2i   %4i    %2i      looping ",loop++,dev,lcount,lrun,rcount,rrun);
    fflush(stdout);
    results(case6(jj));     //step f  switch to +V and run motor
    //    results(case3());       //step b  motor on +V
    timerMR(Mtime);         //step c  motor running
    results(case2());       //step d  motor off
    timerSR(Rtime);         //step e  measure
    results(case6(kk));     //step f  switch to -V and motor on -V
    timerML(Mtime);         //step g  motor running
    results(case2());       //step h  motor off
    timerSL(Ltime);         //step i  measure
    //    results(case0());       //step j  switch to +V
  }
  printf ("\r%10i   %3i     %4i      %2i   %4i    %2i      ending \n",loop,dev,lcount,lrun,rcount,rrun);
  printf("%s\n",GetDate());

}
*/  

/******************************************************************/
char BNCCommand(int command) {

  char bnc[50];

/*
    Command list below; each commend returns '>' = 0x3e
*/
  bnc[0] = 0x30;  // Number 0 or power measure
  bnc[1] = 0x31;  // Number 1 or AM
  bnc[2] = 0x32;  // Number 2 or FM
  bnc[3] = 0x33;  // Number 3 or phase modulated
  bnc[4] = 0x34;  // Number 4 or sweep
  bnc[5] = 0x35;  // Number 5 or FSK
  bnc[6] = 0x36;  // Number 6 or burst
  bnc[7] = 0x37;  // Number 7 or SSB
  bnc[8] = 0x38;  // Number 8 or DTMF generator
  bnc[9] = 0x39;  // Number 9 or DTMF detection

  bnc[10] = 0x7A;  // z - MHz,   dBm
  bnc[11] = 0x79;  // y - kHz,  Vp-p, Sec
  bnc[12] = 0x78;  // x -  Hz, mvp-p,  ms
  bnc[13] = 0x6f;  // o - Offset
  bnc[14] = 0x74;  // t - Trigger
  bnc[15] = 0x03;  // 
  bnc[16] = 0x2E;  // . - decimal point or * 
  bnc[17] = 0x2D;  // - - minus or # sign
  bnc[18] = 0x03;  // 
  bnc[19] = 0x03;  // 

  bnc[20] = 0x6D;  // m - Mode
  bnc[21] = 0x6A;  // j - Function generator (not v as stated in manual)
  bnc[22] = 0x62;  // b - Sinewave
  bnc[23] = 0x71;  // q - Pulse
  bnc[24] = 0x67;  // g - Arbritary
  bnc[25] = 0x03;  // 
  bnc[26] = 0x03;  // 
  bnc[27] = 0x03;  // 
  bnc[28] = 0x03;  // 
  bnc[29] = 0x03;  // 

  bnc[30] = 0x61;  // a - reset to sinewave
  bnc[31] = 0x76;  // v - report hardware and software versions
  bnc[32] = 0x6B;  // k - enable/disable front panel/knob (1/0)
  bnc[33] = 0x65;  // e - enable/disable LCD echo to terminal (1/0)
  bnc[34] = 0x66;  // f - move cursor by field number (0-9)
  bnc[35] = 0x3F;  // ? - help
  bnc[36] = 0x68;  // h - help
  bnc[37] = 0x05;  // ^E - control
  bnc[38] = 0x65;  // e - echo on/off front panel LCD (1/0)
  bnc[39] = 0x6b;  // k - turn on/off front panel control (1/0)

  bnc[40] = 0x70;  // p - Move field left
  bnc[41] = 0x6E;  // n - Move field right
  bnc[42] = 0x75;  // u - Move digit up
  bnc[43] = 0x64;  // d - Move digit down
  bnc[44] = 0x6C;  // l - Move digit position left
  bnc[45] = 0x72;  // r - Move digit position right
  bnc[46] = 0x66;  // f - Field (coupled with number 0-9) 
  bnc[47] = 0x03;  // 
  bnc[48] = 0x73;  // Store or recall
  bnc[49] = 0x43;  // Clear or other

  return (bnc[command]);
}

/******************************************************************/
int BNCParameters() {
	char amp[128],off[128];
	int ii,jj,kk;
	
	printf("Amplitude in millivolts");
	scanf("%s",amp);
	printf("Offset in millivolts");
	scanf("%s",off);

	jj=0;
	kk=0;
	for (ii=0;ii<4;ii++){
		if (isdigit( amp[ii] )) jj++; 
		if (isdigit( off[ii] )) kk++; 
	}
	amplitude = atoi(amp);
	offset = atoi(off);
	printf (" amplitude = %s = %i, jj = %i \n", amp, amplitude, jj);
	printf (" offset = %s = %i, kk = %i \n",off, offset, kk);
	
	
	return 0;
}
/******************************************************************/
int BNCIO( int command){
	
  int ii, stat=99;
  char com1;
	
  com1=BNCCommand(command);      // get the proper command code
  printf("%c\n",com1);
  //  portflush(fd);             // skip the flush and read
  portwrite1(fd,&com1);          // write to the device 
  usleep(300000);                // add sleep to prevent lost data writes
  //  portread (fd,&com1);        //read response of BNC

// old stuff below
//  com1=BNCCommand(command);      // Turn off front panel
//  printf("%c\n",com1);
//  writeport(fd,&com1);      // 
//  readport(fd,&com1);        //read response of BNC
	
  return(stat);
}

/******************************************************************/
int BNCIO2( int ii, int jj){
	
	int stat=99;
	char data[2];
	
	data[0]=BNCCommand(ii);      // Turn off front panel
	data[1]=BNCCommand(jj);      // Turn off front panel
	printf("%s\n",data);
	if (ii==33 && jj==0){
	  portflush(fd);            // flush before reading LCD panel
	  portwrite2(fd,data);      // 
	  portread2 (fd,data);      //read response of BNC
	} else {
	  portwrite2(fd,data);      // just write to device
	}
	// old stuff below
	//  com1=BNCCommand(command);      // Turn off front panel
	//  printf("%c\n",com1);
	//  writeport(fd,&com1);      // 
	//  readport(fd,&com1);        //read response of BNC
	
	return(stat);
}

/******************************************************************/
int BNCResetSine(){

  int ii, stat=99;
  char com1, data;

  BNCIO(30);
	  
  return(stat);
}

/******************************************************************/
int BNCFunction(){

  int ii, stat=99;
  char com1, data;

  BNCIO(21);

  return(stat);
}

/******************************************************************/
int BNCSine(){

  int ii, stat=99;
  char com1, data;

  BNCIO(22);

  return(stat);
}

/******************************************************************/
int BNCPulse(){

  int ii, stat=99;
  char com1, data;

  BNCIO(23);
	  
  return(stat);
}

/******************************************************************/
int BNCFrontOff(){

  int ii, stat=99;
  char com1, data;

  BNCIO2(32,0);
  //  BNCIO(0);
  
  return(stat);
}
/******************************************************************/
int BNCFrontOn(){

  int ii, stat=99;
  char com1, data;

  BNCIO2(32,1);
  //  BNCIO(1);

  return(stat);
}
/******************************************************************/
int BNCFrontOnOff(){

  int ii, stat=99;
  char com1, data;

  printf("Front panel on/off=1/0 \n");
  scanf ("%i", &ii);  

  BNCIO2(32,ii);
  //  BNCIO(ii);
	  
  return(stat);
}
/******************************************************************/
int BNCLCDEchoOn(){

  int ii, stat=99;
  char com1, data;

  BNCIO2(33,1);
  //  BNCIO(1);

  return(stat);
}
/******************************************************************/
int BNCLCDEchoOff(){

  int ii, stat=99;
  char com1, data;

  BNCIO2(33,0);
  //  BNCIO(0);

  return(stat);
}
/******************************************************************/
int BNCLCDEchoOnOff(){

  int ii, stat=99;
  char com1, data;

  printf("LCD echo on/off=1/0 \n");
  printf("Beware of this as you may screw up the current status display of this program! \n");
  scanf ("%i", &ii);  

  BNCIO2(33,ii);
  //  BNCIO(ii);

  return(stat);
}
/******************************************************************/
int BNC_0(){

  int ii, stat=99;
  char com1, data;

  BNCIO(0);

  return(stat);
}
/******************************************************************/
int BNC_1(){

  int ii, stat=99;
  char com1, data;

  BNCIO(1);  

  return(stat);
}
/******************************************************************/
int BNC_2(){

  int ii, stat=99;
  char com1, data;

  BNCIO(2);  

  return(stat);
}
/******************************************************************/
int BNC_3(){

  int ii, stat=99;
  char com1, data;

  BNCIO(3);  

  return(stat);
}
/******************************************************************/
int BNC_4(){

  int ii, stat=99;
  char com1, data;

  BNCIO(4);  

  return(stat);
}
/******************************************************************/
int BNC_5(){

  int ii, stat=99;
  char com1, data;

  BNCIO(5);  

  return(stat);
}
/******************************************************************/
int BNC_6(){

  int ii, stat=99;
  char com1, data;

  BNCIO(6);  

  return(stat);
}
/******************************************************************/
int BNC_7(){

  int ii, stat=99;
  char com1, data;

  BNCIO(7);  

  return(stat);
}
/******************************************************************/
int BNC_8(){

  int ii, stat=99;
  char com1, data;

  BNCIO(8);  

  return(stat);
}
/******************************************************************/
int BNC_9(){

  int ii, stat=99;
  char com1, data;

  BNCIO(9);  

  return(stat);
}
/******************************************************************/
int BNCMHz(){

  int ii, stat=99;
  char com1, data;

  BNCIO(10);  

  return(stat);
}

/******************************************************************/
int BNCkHz(){

  int ii, stat=99;
  char com1, data;

  BNCIO(11);  

  return(stat);
}

/******************************************************************/
int BNCHz(){

  int ii, stat=99;
  char com1, data;

  BNCIO(12);  

  return(stat);
}

/******************************************************************/
int BNCOffset(){

  int ii, stat=99;
  char com1, data;

  BNCIO(13);  

  return(stat);
}

/******************************************************************/
int BNCTrigger(){

  int ii, stat=99;
  char com1, data;

  BNCIO(14);  

  return(stat);
}

/******************************************************************/
int BNCDot(){

  int ii, stat=99;
  char com1, data;

  BNCIO(16);  

  return(stat);
}

/******************************************************************/
int BNCMinus(){

  int ii, stat=99;
  char com1, data;

  BNCIO(17);  

  return(stat);
}

/******************************************************************/
int BNCFieldLeft(){

  int ii, stat=99;
  char com1, data;

  BNCIO(40);  

  return(stat);
}

/******************************************************************/
int BNCFieldRight(){

  int ii, stat=99;
  char com1, data;

  BNCIO(41);  

  return(stat);
}

/******************************************************************/
int BNCDigitUp(){

  int ii, stat=99;
  char com1, data;

  BNCIO(42);  

  return(stat);
}

/******************************************************************/
int BNCDigitDown(){

  int ii, stat=99;
  char com1, data;

  BNCIO(43);  

  return(stat);
}

/******************************************************************/
int BNCDigitLeft(){

  int ii, stat=99;
  char com1, data;

  BNCIO(44);  

  return(stat);
}

/******************************************************************/
int BNCDigitRight(){

  int ii, stat=99;
  char com1, data;

  BNCIO(45);  
	  
  return(stat);
}

/******************************************************************/
int BNCField(){

  int ii, jj=46, stat=99;
  char com1, data;
/*
Need to ask field first before sending command to device
as it looks like it times out waiting for the field value.
*/
  printf ("Field number (0-9), 0=turns curser off ?\n");
  scanf ("%i", &ii);

  BNCIO(jj);  
	  BNCIO2(jj,ii);  
	
  switch(ii){
  case  0: BNC_0(); break;
  case  1: BNC_1(); break;
  case  2: BNC_2(); break;
  case  3: BNC_3(); break;
  case  4: BNC_4(); break;
  case  5: BNC_5(); break;
  case  6: BNC_6(); break;
  case  7: BNC_7(); break;
  case  8: BNC_8(); break;
  case  9: BNC_9(); break;
  default: BNC_0(); break;

  }

  return(stat);
}

/******************************************************************/
int BNCArbitrary(){

  int ii, stat=99;
  char com1, data;

  BNCIO(24);  

  printf ("Expand this section to use for short beam release mode ?\n");

  return(stat);
}

/******************************************************************/
int GrowDecay(){

  int  stat=99;
  long int ii, jj, kk, mm, iduty, igrow, idecay, msec, clockfreq;
  double duty, freq;
  char com1, data, grow[128],decay[128];
  char posonly='Y';   // positive only signals - max voltage is half indicated
  int trig=1;         // self-triggered or continuous mode; 0 = external trigger
  char mvolt[128]="5000";

  for (ii=0;ii<127;ii++){
    grow[ii]=0;
    decay[ii]=0;
  }

  //  mvolt="5000";

  printf("Integer input  - actual time may be slightly different due to hardware\n");
  printf("   Fixed hardware setup:\n");
  printf("     - power supply amplifier is x1000 \n");
  printf("     - Positive only signals \n");
  printf("     - Voltage (mV) set to half that shown on LCD  \n");  
  printf("     - Self-triggered mode   \n");  

  printf("Time for grow-in (ms) \n");       
  scanf("%s",grow);
  printf("Time for decay-out (ms) \n");    
  scanf("%s",decay);

  jj=0;
  kk=0;
  for (ii=0;ii<20;ii++){
    if (isdigit(  grow[ii] )) jj++; 
    if (isdigit( decay[ii] )) kk++; 
  }
  igrow = atoi(grow);
  idecay = atoi(decay);
  printf (" igrow = %s = %i, jj = %i \n", grow,  igrow, jj);
  printf ("idecay = %s = %i, kk = %i \n",decay, idecay, kk);
/*
  calculate waveform duration (msec) and duty cycle (iduty) in integer percentage
  to go to freguencies << 1 Hz need to which to arbitrary mode to set clock frequency
  as suggested in manual.
*/
  msec = igrow+idecay;              // cycle time
  iduty = idecay*100/msec;          // time signal is high - need 100 to get percent
  freq = 1000./(double) msec;       // frequency
  clockfreq = msec * 16;            // hardware determined to go to low sub Hz frequencies

  printf ("    Cycle time = %.2lf Hz\n", freq);
  printf (" Beam  on time = %i ms\n", msec);
  printf (" Beam off time = %i \% \n", iduty);
  printf ("    Clock time = %i per 16000 points\n", clockfreq);

/*
   Now configure BNC-632
    Go to pulse mode
*/
  BNCPulse();
 
/* do field 1 commands - positive signals */
  PulseF1();
	
/* do field 2 commands - duty cycle */
  PulseF2(iduty); 

/* do field 3 commands - trigger */
  PulseF3(trig); 
	
/* do field 5 commands - voltage */
  PulseF5(mvolt);

/* 
  do field 4 commands - frequency - last to allow arbitrary function
  option to fine tune repitition rate.
*/
  PulseF4(freq); 

/* Return to Field 0 to turn off cursor */
  BNCIO(46);      // go to field
  BNCIO(0);       // select field 0
  

/* 
    still need to explore arb. mode for << 1 Hz pulses) 
	still need to check if pulse mode can have < 1 Hz values
*/
  return(stat);
}

/******************************************************************/
int PulseF1() {
  char com1, data;
  int stat=0;
/*
   go to field 1 (positive signal only field) and set to Y
*/
  BNCIO(46);      // go to field
  BNCIO(1);       // select field 2
  BNCIO(0);       // 0 = yes; 1 = no
	
	return (stat);
}

/******************************************************************/
int PulseF2(int iduty) {
  char com1, data;
  int stat=0,ii,jj;
/*
  go to field 5 (voltage field) and set to millivolts
  do interger math to get % in characters
*/
  ii = iduty/10;           // tenth place of duty cycle  
  jj = iduty - (ii*10);    //  ones place of duty cycle 
	
  BNCIO(46);  // go to field
  BNCIO(2);   // select field 2
  BNCIO(ii);  // set 10's digit
  BNCIO(jj);  // set  1's digit
  BNCHz();    // acts as enter for percentage

  return (stat);
}

/******************************************************************/
int PulseF3(int trig) {
  char com1, data;
  int stat=0;
/*
  go to field 3 (trigger field) and set to continuous
*/
  BNCIO(46);      // go to field
  BNCIO(3);       // select field 2
  BNCIO(0);       // 0 = Trigger; 1 = Continuous

  return (stat);
}

/******************************************************************/
int PulseF4(double freq) {
  char com1, data;
  int    i8,i7,i6,i5,i4,i3,i2,i1,i0;
  double f8,f7,f6,f5,f4,f3,f2,f1;
  double xx,yy;
  int ii,jj,kk;
  int contrep;
  int stat=0;
/*
  go to field 4 (frequency field) 87,654,321
*/

  printf("frequency = %0.2lf\n",freq);

  BNCIO(46);      // go to field
  BNCIO(4);       // select field 4
  BNCIO(49);      // clear all digits
  
  xx = freq;
  printf("frequency = %0.2lf, %lf\n",freq,xx);
  for (kk=8; kk>-3; kk--){
    yy = xx / pow(10,kk);
    ii = (int) yy;
    xx = xx - ((double)ii * pow(10,kk));
    if (kk == -1) BNCIO(16);
    BNCIO(ii);      //  10^kk Hz
    printf("frequency = %0.2lf, xx= %lf, yy=%lf \n",freq, xx, yy);
  }

  BNCHz();        // enters Hz
	  
/*
   Pulse mode can only be given in 1 Hz steps.  I will guess that
   the fastest complete grow-in and decay cycle will always be greater
   than 100 ms or 10 Hz.  Presumably, this is adjustable.
*/
  if (freq <= 10.) {
    BNCIO(40);      // clear previous setting
    BNCIO(1);       // set to 1 Hz
    BNCHz();        // enters Hz

    BNCIO(24);      // go to arbitrary mode
    BNCIO(46);      // go to field
    BNCIO(2);       // select field 2
        
    contrep = 16000/freq; // see p 51 in manual
    xx = contrep;         // determine clock rep 
    printf("frequency = %0.2lf, %lf\n",freq,xx);
    for (kk=8; kk>-3; kk--){
      yy = xx / pow(10,kk);
      ii = (int) yy;
      xx = xx - ((double)ii * pow(10,kk));
      if (kk == -1) BNCIO(16);
      BNCIO(ii);      //  10^kk Hz
      printf("frequency = %0.2lf, xx= %lf, yy=%lf \n",freq, xx, yy);
    }
  BNCHz();        // enters Hz
  ArbF3();
 }
	return (stat);
}

/******************************************************************/
int PulseF5(char mvolt[]) {
  char com1, data;
  int stat=0, ii, jj;
/*
  go to field 5 (voltage field) and set to millivolts
*/
  printf("mvolt = %s\n",mvolt);

  BNCIO(46);             // go to field
  BNCIO(5);              // select field 5
 
  BNCIO(49);             // clear all digits

  ii = amplitude/1000;
  jj = amplitude - (ii*1000);
  BNCIO(ii);             // thousandth's place

  ii = jj/100;
  jj = jj - (ii*100);
  BNCIO(ii);       //   hundredth's place

  ii = jj/10;
  jj = jj - (ii*10);
  BNCIO(ii);       //  tenth's place

  ii = jj;

  BNCIO(ii);       // one's place
  
  BNCIO(12);             // millivolts selection
	
  return (stat);
}
/******************************************************************/
int ArbF3() {
  char com1, data;
  int stat=0, ii, jj;
/*
  go to field 5 (voltage field) and set to millivolts
*/
//  printf("mvolt = %s\n",mvolt);

  BNCIO(46);             // go to field
  BNCIO(3);              // select field 3
 
  BNCIO(49);             // clear all digits

  ii = amplitude/1000;
  jj = amplitude - (ii*1000);
  BNCIO(ii);             // thousandth's place

  ii = jj/100;
  jj = jj - (ii*100);
  BNCIO(ii);       //   hundredth's place

  ii = jj/10;
  jj = jj - (ii*10);
  BNCIO(ii);       //  tenth's place

  ii = jj;

  BNCIO(ii);       // one's place
  
  BNCIO(12);             // millivolts selection
	
  return (stat);
}

/******************************************************************/
/******************************************************************/
/******************************************************************/
/******************************************************************/
/******************************************************************/
/******************************************************************/
/******************************************************************/
int Manual(){
  int stat, command;
  char com1,data=0xff;

  printf("0 -  Number 0 or power measure\n");//    0x30          
  printf("1 -  Number 1 or AM\n");//               0x31
  printf("2 -  Number 2 or FM\n");//               0x32 
  printf("3 -  Number 3 or phase modulated\n");//  0x33 
  printf("4 -  Number 4 or sweep\n");//            0x34
  printf("5 -  Number 5 or FSK\n");//              0x35
  printf("6 -  Number 6 or burst\n");//            0x36
  printf("7 -  Number 7 or SSB\n");//              0x37
  printf("8 -  Number 8 or DTMF generator\n");//   0x38
  printf("9 -  Number 9 or DTMF detection\n");//   0x39

  printf("10 - MHz,   dBm \n");//                  0x7A = z 
  printf("11 - kHz,  Vp-p, Sec \n");//             0x79 = y 
  printf("12 -  Hz, mvp-p,  ms\n");//              0x78 = x 
  printf("13 - Offset \n");//                      0x6f = o 
  printf("14 - Trigger \n");//                     0x74 = t 
  printf("15 -  \n");//
  printf("16 - decimal point or *  \n");//         0x2E = . 
  printf("17 - - minus or # sign \n");//           0x2D = -
  printf("18 -  \n");
  printf("19 -  \n");

  printf("20 - Mode \n");//                        0x6D = m  
  printf("21 - Function \n");//                    0x76 = v 
  printf("22 - Sinewave \n");//                    0x62 = b  
  printf("23 - Pulse \n");//                       0x71 = q  
  printf("24 - Arbritary \n");//                   0x67 = g  
  printf("25 -  \n");//
  printf("26 -  \n");//
  printf("27 -  \n");//
  printf("28 -  \n");//
  printf("29 -  \n");//

  printf("30 - reset to sinewave \n");//                          0x6D = a 
  printf("31 - \n"); // Report hardware and software versions not implemented    0x76 = v 
  printf("32 - enable/disable front panel/knob (1/0) \n");//      0x6B = k 
  printf("33 - enable/disable LCD echo to terminal (1/0) \n");//  0x65 = e  
  printf("34 - \n");//  
  printf("35 - \n");// Help function through hardware not implemented    0x3F = ? 
  printf("36 - \n");// Help function through hardware not implemented    0x68 = h
  printf("37 - \n");// ^E test communication - returns ^C not implemented    0x05 = ^E  
  printf("38 - Echo LCD terminal (1 = on, 0 = off) \n");//        0x65 = e
  printf("39 - \n");//  

  printf("40 - Move field left \n");//                0x70 = p  
  printf("41 - Move field right \n");//               0x6E = n 
  printf("42 - Move digit up \n");//                  0x75 = u 
  printf("43 - Move digit down \n");//                0x64 = d 
  printf("44 - Move digit position left \n");//       0x6C = l  
  printf("45 - Move digit position right \n");//      0x72 = r  
  printf("46 - Field (couple with number 0-9) \n");// 0x66 = f
  printf("47 -  \n");//
  printf("48 - Store or recall  \n");//               0x73 =  
  printf("49 - Clear or other \n");//                 0x43 = 

  printf("\n");

  data=0xff;
  printf ("Enter the command ?\n");
  scanf ("%i", &command);

  switch (command) {
  case  0: BNC_0(); break;
  case  1: BNC_1(); break;
  case  2: BNC_2(); break;
  case  3: BNC_3(); break;
  case  4: BNC_4(); break;
  case  5: BNC_5(); break;
  case  6: BNC_6(); break;
  case  7: BNC_7(); break;
  case  8: BNC_8(); break;
  case  9: BNC_9(); break;
  case 10: BNCMHz(); break;
  case 11: BNCkHz(); break;
  case 12: BNCHz(); break;
  case 13: BNCOffset(); break;
  case 14: BNCTrigger(); break;
  case 16: BNCDot(); break;
  case 17: BNCMinus(); break;
  case 20: BNCMode(); break;
  case 21: BNCFunction(); break;
  case 22: BNCSine(); break;
  case 23: BNCPulse(); break;
  case 24: BNCArbitrary(); break;
  case 30: BNCResetSine(); break;
    //  case 31: data=0x76; break;
  case 32: BNCFrontOnOff(); break;
  case 33: BNCLCDEchoOnOff(); break;
    //  case 35: data=0x3F; break;
    //  case 36: data=0x68; break;
    //  case 37: data=0x05; break;
  case 38: BNCStatus(); break;
  case 40: BNCFieldLeft(); break;
  case 41: BNCFieldRight(); break;
  case 42: BNCDigitUp(); break;
  case 43: BNCDigitDown(); break;
  case 44: BNCDigitLeft(); break;
  case 45: BNCDigitRight(); break;
  case 46: BNCField(); break;
  case 48: printf("not implemented\n"); break;
  case 49: printf("not implemented\n"); break;
  default: printf("Invalid command .... returning ....\n"); break;
  }

  BNCStatus();
  return(stat);
}

/******************************************************************/
int BNCStatus(){

  int ii=0, n, stat=99;
  char com1, data, data110[110];

/*
  Report buffer consists of two lines of 40 characters sandwiched by
  a double quote, carriage return, and a linefeed sequence.  Do not know
  the order of these things:
   <cr> = 0x0d
   <lf> = 0x0a
    "   = 0x22

0         1         2         3         4
 1234567890123456789012345678901234567890
 Cursor: off                                ...<cr><lf> + 11 + <cr><lf> = 15
"Sinewave mode                           "  ...40 + 2 quotes + <cr><lf> = 44
" 1,000,000.00 Hz               -10.0 dBm"  ...40 + 2 quotes + <cr><lf> = 44

*/

  printf ("Reporting status...\n");
  BNCLCDEchoOn();      // Turn on LCD echo

	portread(fd,&data);         // read first character after initial call
  for (ii =0; ii < 106; ii++) {
    portread2(fd,&data);       // read next group of characters 
    printf("%c",data);
  }
  printf("\n characters read=%i\n",ii);

//  n= readport110(fd,data110);       // read report buffer 
//  printf("n =%i  = %s\n",n,data110);

  BNCLCDEchoOff();      // Turn off LCD echo

  return(stat);
}

/******************************************************************/
int BNCMode(){

  int ii, stat=99;
  char com1, data;

  printf ("Function (0-9) ? \n");
  printf ("0 - Power measure \n");
  printf ("1 - Am \n");
  printf ("2 - FM \n");
  printf ("3 - Phase modulated \n");
  printf ("4 - Sweep \n");
  printf ("5 - FSK \n");
  printf ("6 - Burst \n");
  printf ("7 - SSb \n");
  printf ("8 - DTMF generator \n");
  printf ("9 - DTMF detection \n");

  scanf ("%i", &ii);

  BNCIO(20);                // go to Mode

  switch(ii){
  case  0: BNC_0(); break;
  case  1: BNC_1(); break;
  case  2: BNC_2(); break;
  case  3: BNC_3(); break;
  case  4: BNC_4(); break;
  case  5: BNC_5(); break;
  case  6: BNC_6(); break;
  case  7: BNC_7(); break;
  case  8: BNC_8(); break;
  case  9: BNC_9(); break;
  default: BNC_0(); break;
  }

  return(stat);
}
/******************************************************************/

/******************************************************************/
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


