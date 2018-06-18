//Author: LabJack
//April 7, 2008
//This examples demonstrates how to read from analog inputs (AIN) and digital inputs(FIO),
//set analog outputs (DAC) and digital outputs (FIO), and how to configure and enable
//timers and counters and read input timers and counters values using the "easy" functions.

#include "../include/u3-err-msg.h"
#include "../include/u3.h"
#include "../include/labjackusb.h"
#include "../include/kick-u3.h"
#include "../include/u3.h"
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
/*
char *errormsg[] =
   {
     "no msg code",   //0
	"SCRATCH_WRT_FAIL",   //1	
	"SCRATCH_ERASE_FAIL",   //2	
	"DATA_BUFFER_OVERFLOW",   //3	
	"ADC0_BUFFER_OVERFLOW",   //4
	"FUNCTION_INVALID",   //5	
	"SWDT_TIME_INVALID",   //6	
	"XBR_CONFIG_ERROR",   //7	
        "no msg code",   //8
        "no msg code",   //9
        "no msg code",   //10
        "no msg code",   //11
        "no msg code",   //12
        "no msg code",   //13
        "no msg code",   //14
        "no msg code",   //15
	"FLASH_WRITE_FAIL",   //	16
	"FLASH_ERASE_FAIL",   //	17
	"FLASH_JMP_FAIL",   //	18
	"FLASH_PSP_TIMEOUT",   //19	
	"FLASH_ABORT_RECEIVED",   //20	
	"FLASH_PAGE_MISMATCH",   //21	
	"FLASH_BLOCK_MISMATCH",   //22	
	"FLASH_PAGE_NOT_IN_CODE_AREA",   //23	
	"MEM_ILLEGAL_ADDRESS",   //24	
	"FLASH_LOCKED",   //	25
	"INVALID_BLOCK",   //	26
	"FLASH_ILLEGAL_PAGE",   //27	
	"FLASH_TOO_MANY_BYTES",   //28	
	"FLASH_INVALID_STRING_NUM",   //29	
        "no msg code",   //30
        "no msg code",   //31
        "no msg code",   //32
        "no msg code",   //33
        "no msg code",   //34
        "no msg code",   //35
        "no msg code",   //36
        "no msg code",   //37
        "no msg code",   //38
        "no msg code",   //39
	"SHT1x_COMM_TIME_OUT",   //40	
	"SHT1x_NO_ACK",   //	41
	"SHT1x_CRC_FAILED",   //	42
	"SHT1X_TOO_MANY_W_BYTES",   //43	
	"SHT1X_TOO_MANY_R_BYTES",   //44	
	"SHT1X_INVALID_MODE",   //45	
	"SHT1X_INVALID_LINE",   //46
        "no msg code",   //47	
	"STREAM_IS_ACTIVE",   //	48
	"STREAM_TABLE_INVALID",   //	49
	"STREAM_CONFIG_INVALID",   //	50
	"STREAM_BAD_TRIGGER_SOURCE",   //51	
	"STREAM_NOT_RUNNING",   //	52
	"STREAM_INVALID_TRIGGER",   //	53
	"STREAM_ADC0_BUFFER_OVERFLOW",   //54	
	"STREAM_SCAN_OVERLAP",   //	55
	"STREAM_SAMPLE_NUM_INVALID",   //56	
	"STREAM_BIPOLAR_GAIN_INVALID",   //57	
	"STREAM_SCAN_RATE_INVALID",   //	58
	"STREAM_AUTORECOVER_ACTIVE",   //59	
	"STREAM_AUTORECOVER_REPORT",   //60
        "no msg code",   //61	
        "no msg code",   //62	
	"STREAM_AUTORECOVER_OVERFLOW",   //63	
	"TIMER_INVALID_MODE",   //	64
	"TIMER_QUADRATURE_AB_ERROR",   //65	
	"TIMER_QUAD_PULSE_SEQUENCE",   //66	
	"TIMER_BAD_CLOCK_SOURCE",   //67
	"TIMER_STREAM_ACTIVE",   //68	
	"TIMER_PWMSTOP_MODULE_ERROR",   //69	
	"TIMER_SEQUENCE_ERROR",   //70	
	"TIMER_LINE_SEQUENCE_ERROR",   //71	
	"TIMER_SHARING_ERROR",   //72	
        "no msg code",   //73
        "no msg code",   //74
        "no msg code",   //75
        "no msg code",   //76
        "no msg code",   //77
        "no msg code",   //78
        "no msg code",   //79
	"EXT_OSC_NOT_STABLE",   //80	
	"INVALID_POWER_SETTING",   //81	
	"PLL_NOT_LOCKED",   //82	
        "no msg code",   //83
        "no msg code",   //84
        "no msg code",   //85
        "no msg code",   //86
        "no msg code",   //87
        "no msg code",   //88
        "no msg code",   //89
        "no msg code",   //90
        "no msg code",   //91
        "no msg code",   //92
        "no msg code",   //93
        "no msg code",   //94
        "no msg code",   //95
	"INVALID_PIN",   //96	
	"PIN_CONFIGURED_FOR_ANALOG",   //97	
	"PIN_CONFIGURED_FOR_DIGITAL",   //98	
	"IOTYPE_SYNCH_ERROR",   //99	
	"INVALID_OFFSET",   //100
	"IOTYPE_NOT_VALID",   //101	
	"TC_PIN_OFFSET_MUST_BE_4-8",   //102	
        "no msg code",   //103
        "no msg code",   //104
        "no msg code",   //105
        "no msg code",   //106
        "no msg code",   //107
        "no msg code",   //108
        "no msg code",   //109
        "no msg code",   //110
        "no msg code",   //111
	"UART_TIMEOUT",   //112	
	"UART_NOT_CONNECTED",   //113	
	"UART_NOT_ENABLED",   //114	
   }; 
*/
int main(int argc, char **argv)
{
    HANDLE hDevice;
    u3CalibrationInfo caliInfo;
    int localID;
    long error;
    long ioval=0;
    //    long DAC1Enable, error;

    //    int ii=0,jj=0;
    //    double volts, involts, driver;
    //    char errstr[256]="\0";

    //Open first found U3 over USB
    printf("opening usb \n");
    localID = -1;
    if( (hDevice = openUSBConnection(localID)) == NULL){
      return 0;
    }
      //        goto done;
    printf("opened usb...and reseting \n");
    //   driver=GetDriverVersion();
    //    ResetLabJack(hDevice);

    //  sleep(3);

    //Get calibration information from UE9
    printf("getting calib \n");
    if(getCalibrationInfo(hDevice, &caliInfo) < 0){
      printf("Received an error code less than 0 \n");
      closeUSBConnection(hDevice);
      return 0;
    }
    printf("got calib \n");

long eDO( HANDLE Handle,
          long ConfigIO,
          long Channel,
          long State);

     error = eDO(hDevice,1, 3, 0);
     error = eDO(hDevice,1, 4, 0);
     error = eDO(hDevice,1, 5, 0);
    //Enable and configure 1 output timer and 1 input timer, and enable counter0
    printf("\nCalling eTCConfig to enable and configure 1 output timer (Timer0) and 1 input timer (Timer1), and enable counter0\n");
    //long alngEnableTimers[2] = {1, 1};  //Enable Timer0-Timer1
    //long alngTimerModes[2] = {LJ_tmPWM48,LJ_tmPWM48};  //Set timer modes
    //    double adblTimerValues[2] = {30000, 58000};  //Set PWM8 duty-cycles to 75%
    //long alngEnableCounters[2] = {0, 0};  //Enable Counter0
    //    if((error = eTCConfig(hDevice, alngEnableTimers, alngEnableCounters, 4, LJ_tc48MHZ, 0, alngTimerModes, adblTimerValues, 0, 0)) != 0){
    //int error;

  long alngEnableTimers[2] = {1, 1};  //Enable Timer0-Timer1
  //  long alngTimerModes[2] = {LJ_tmPWM8, LJ_tmRISINGEDGES32};  //Set timer modes    LJ_tmTIMERSTOP
  //  long alngTimerModes[2] = {LJ_tmPWM8, LJ_tmTIMERSTOP};  //Set timer modes    
    long alngTimerModes[2] = {LJ_tmFREQOUT, LJ_tmTIMERSTOP};  //Set timer modes    
  //  double adblTimerValues[2] = {16384, 0};  //Set PWM8 duty-cycles to 75%
  // double adblTimerValues[2] = {32000, 200};  //Set PWM8 duty-cycles to x%, count 200 pulses   // works for PWM8
  double adblTimerValues[2] = {250, 200};  //Set frequency to 48MHz//96/250/2 = 1 kHz, count 200 pulses
  //  long alngEnableCounters[2] = {0, 1};  //Enable Counter0-Counter1
  long alngEnableCounters[2] = {0, 0};  //Enable Counter0-Counter1
  //double volts=0.0;
  
  printf ("timers enable = %li    %li \n",alngEnableTimers[0], alngEnableTimers[1]);  // enable timers
  printf ("counters enable = %li    %li \n",alngEnableCounters[0], alngEnableCounters[1]);  // enable counters
  //   printf ("timer modes = %i    %i \n",LJ_tmPWM8, LJ_tmTIMERSTOP);  // 4 = pin offset..ie where do timers start...works for PWM8
    printf ("timer modes = %i    %i \n",LJ_tmFREQOUT, LJ_tmTIMERSTOP);  // 4 = pin offset..ie where do timers start
  printf ("freq mode = %i     \n",LJ_tc48MHZ_DIV);  // 4 = pin offset..ie where do timers start
  printf ("timers values = %lf    %lf \n",adblTimerValues[0], adblTimerValues[1]);  // enable timers

  // This is needed to define the signal out before configuring the timers for timer-timer-stop applications (MTAS laser test)
  long configIO = 1;
  long DIV=96;      // divisor for frequency....
  long tcStart = 5;   // start timers and counters on channel FIO4
  printf ("divisor = %li     \n",DIV);  // 4 = pin offset..ie where do timers start
  if( (error = eDI(hDevice, configIO, tcStart, &ioval)) != 0 ){  // 1 is needed on first read to configure channel; set to 0 isf known to be configured correctly
    printf("%s\n", errormsg[error]);
    closeUSBConnection(hDevice);
    return error;
  }
  printf("FIO4 state = %ld\n", ioval);

  printf("Calling eDAC to set DAC0 to 2.1 V\n");
  if( (error = eDAC(hDevice, &caliInfo, configIO, 0, 2.1, 0, 0, 0)) != 0 ){
    printf("%s\n", errormsg[error]);
    closeUSBConnection(hDevice);
    return error;
  }
 
  configIO = 0;    /// now its configured
  // end of pre-timer set-up
  // now configure the timers
  int jj = 2;
  while (jj != 0){ 
  if((error = eTCConfig(hDevice, alngEnableTimers, alngEnableCounters, tcStart, LJ_tc48MHZ_DIV, DIV, alngTimerModes, adblTimerValues, 0, 0)) != 0){
    printf("%s\n", errormsg[error]);         // 48 MHz / 96 * value (250) = 0.5 MHz * 250 = 
    closeUSBConnection(hDevice);
    return error;
  }

      printf("\nWaiting for 2 second...\n");
    sleep(2);
    jj--;
	   
  }

  long aReadTimers[2] = {1,1};
  long aUpdateResetTimers[2] = {1,1};
  long aReadCounters[2] = {0,0};
  long aResetCounters[2] = {0,0};
  //  double adblTimerValues = {0,0};
  double aCounterValues[2] = {0,0};
  printf("\n trying eTCValues ...\n");
  if((error = eTCValues(hDevice, aReadTimers, aUpdateResetTimers, aReadCounters, aResetCounters, adblTimerValues, aCounterValues, 0, 0)) != 0){
    printf("%s\n", errormsg[error]);         // 48 MHz / 96 * value (250) = 0.5 MHz * 250 = 
    closeUSBConnection(hDevice);
    return error;
  }

  printf("\n ending eTCValues ...\n");

  /*
long eTCValues( HANDLE Handle,
                long *aReadTimers,
                long *aUpdateResetTimers,
                long *aReadCounters,
                long *aResetCounters,
                double *aTimerValues,
                double *aCounterValues,
                long Reserved1,
                long Reserved2);
*//*  
   printf("\n trying feedback...\n");
  uint8 c[12];

  c[0]  = (uint8)(0x00);
  c[1]  = (uint8)(0xf8);
  c[2]  = (uint8)(0x03);
  c[3]  = (uint8)(0x00);
  c[4]  = (uint8)(0x00);
  c[5]  = (uint8)(0x00);
  c[6]  = (uint8)(0x00);
                           // do stoptimer first
  c[7]  = (uint8)(0x2C);   // 42 = 2A  ;  44 = 2C
  c[8]  = (uint8)(0x01);   // update - reset bit
  c[9]  = (uint8)(0xC8);   // 200 = C8  ;  250 = FA
  c[10] = (uint8)(0x00);
                           // do timer out second
  c[11] = (uint8)(0x2A);   // 42 = 2A  ;  44 = 2C
  c[12] = (uint8)(0x01);   // update - reset bit
  c[13] = (uint8)(0xFA);   // 200 = C8  ;  250 = FA
  c[14] = (uint8)(0x00);
  
  //  c[11] = (uint8)(0x2C);   // 42 = 2A  ;  44 = 2C
  c[15] = (uint8)(0x00);

  extendedChecksum(c,12);  // checksum load array elements 0, 4, and 5
  
  unsigned long int count = 12;
  error = LJUSB_Write(hDevice, c, count);  // sleep was needed when all bits were not set!!!  not sure why
  printf ("%li \n", error);
  if (error < 12){                      // found at ANL VANDLE run 2015
    if (error == 0) printf("Feedback setup error : write failed\n");
    else printf("Feedback setup error : did not write all of the buffer\n");
  }

   printf("\n ending feedback...\n");

*/
 
  /*
    if((error = eTCConfig(hDevice, alngEnableTimers, alngEnableCounters, 8, LJ_tc1MHZ_DIV, 0, alngTimerModes, adblTimerValues, 0, 0)) != 0){
      printf("Received an error code of %ld \n", error);
      //           ErrorToString(error,errstr);
      printf("error string: %s\n",errstr);
      closeUSBConnection(hDevice);
      return 0;
    }
  */

    //Read and reset the input timer (Timer1), read and reset Counter0, and update the
    //value (duty-cycle) of the output timer (Timer0)
    //   printf("\nCalling eTCValues to read and reset the input Timer1 and Counter0, and update the value (duty-cycle) of the output Timer0\n");
    //    long alngReadTimers[2] = {1, 1};  //Read Timer1
    //    long alngUpdateResetTimers[2] = {1, 200};  //Update timer0
    //    long alngReadCounters[2] = {1, 0};  //Read Counter0
    //    long alngResetCounters[2] = {0, 0};  //Reset no Counters
    //    double adblCounterValues[2] = {0, 0};
    //    adblTimerValues[0] = 32768;  //Change Timer0 duty-cycle to 50%
    //    if((error = eTCValues(hDevice, alngReadTimers, alngUpdateResetTimers, alngReadCounters, alngResetCounters, adblTimerValues, adblCounterValues, 0, 0)) != 0){
    //          printf("Received an error code of %ld\n", error);
    //          closeUSBConnection(hDevice);
    //          return 0;
	  //        }

    //    printf("\nWaiting for 10 second...\n");
    //    sleep(10);

    
    //    printf("Timer1 value = %.0f\n", adblTimerValues[1]);
    //    printf("Counter0 value = %.0f\n", adblCounterValues[0]);

    printf("ending....\n");

/*
 Close USB connection and end program
*/
    closeUSBConnection(hDevice);
    return 0;
}


/*

  uint8 c[16];

  c[0]  = (uint8)(0x00);
  c[1]  = (uint8)(0xf8);
  c[2]  = (uint8)(0x00);
  c[3]  = (uint8)(0x00);
  c[4]  = (uint8)(0x00);
  c[5]  = (uint8)(0x00);
  c[6]  = (uint8)(0x00);

  c[7]  = (uint8)(0x2A);   // 42 = 2A  ;  44 = 2C
  c[8]  = (uint8)(0x00);   // update - reset bit
  c[9]  = (uint8)(0xC8);   // 200 = C8  ;  250 = FA
  c[10] = (uint8)(0x00);

  c[11] = (uint8)(0x2C);   // 42 = 2A  ;  44 = 2C
  c[12] = (uint8)(0x00);   // update - reset bit
  c[13] = (uint8)(0xFA);   // 200 = C8  ;  250 = FA
  c[14] = (uint8)(0x00);
  c[15] = (uint8)(0x00);


  extendedChecksum(c,16);  // checksum load array elements 0, 4, and 5
  count = 16;
  error = LJUSB_Write(hDevice, c, count);  // sleep was needed when all bits were not set!!!  not sure why
  if (error < num){                      // found at ANL VANDLE run 2015
    if (error == 0) printf("Feedback setup error : write failed\n");
    else printf("Feedback setup error : did not write all of the buffer\n");
  }
 
 */
