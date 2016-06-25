/*
  Program test to control MTAS and all its signals
  Taken from mtas27.c developed for LabJack U3
  Let's see how easy it is to get it to run on U6
*/

#include "u6.h"
//#include "u3.h"
#include "labjackusb.h"
#include <unistd.h>  /* UNIX standard function definitions */
#include <time.h>    /* Time definitions */
#include <gtk/gtk.h>
#include <cairo.h>

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

#define INTERVAL 10
/*
#define kicker 12
#define tape   11
#define beam   9
#define light  10
#define daq    13
#define back   14
#define trig   8
*/
#define dac0   0
#define dac1   1
#define thermNum 4
#define tempRoom 0
#define tempLeft 1
#define tempRight 2
#define tempRef 3
#define celkelfah 0   // 0=celcius; 1=kelvin; 2=fahrenheit

#define down  0
#define up    1

long int ddxx=0;
/*
Make U3 stuff global
 */
HANDLE hU6;
u6CalibrationInfo caliInfo;
long int dio_init(long int dio);
long int dio_set(long int dio, long int ud);
long int dac_init(long int dac);
long int dac_set(long intdac, double vset);
long int adc_init(long int adc);
int tempInit();
double tempCKF(long int adc, long int ckf);
void tempRead();
void resetU6(uint8 res);

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

char *GetDate();
GtkWidget *spectra, *timeNow, *TLine;//, *stats;
cairo_t *cr, *crSpec, *crTime, *crTLine;//, crStat;

void  spectraDraw(GtkWidget *spectra);
long int plotTemp=0;

//static gboolean drawCairo (GtkWidget *widget, CairoContext *event, gpointer data);
static gboolean on_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data);
static gboolean time_handler(GtkWidget *widget);
static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data); 
void kill_window(GtkWidget *widget, gpointer data);
long int timeCurrent(cairo_t *cr);
void tempPlot(cairo_t *cr);      // plot temperatures
void tempPlot1(cairo_t *cr);      // plot temperatures
void tempPlot2(cairo_t *cr);      // plot temperatures
void timeLine(cairo_t *cr);      // plot cycle "animation"
int cairo_setup=0;
void timeCounter();

void mtasOpenTemp();
void menuControl();
void cycleSetup0 ();  // start of cycle...trigger and beam on
void cycleSetup1 ();  // end of cycle....beam off, light pulser, tape move, measurement
void cycleSetup2 ();  // same as 0 but with background signal set
void cycleSetup3 ();  // same as 1 but with background signal set
void cycleSetup4 ();  // take away mode...kicker on, tape move, kicker down, trigger, beam on
void cycleSetup5 ();  // kicker on, beam off, measurement on, 
void cycleSetup6 ();  // kicker on, beam off, measurement on, 
void cycleSetup7 ();  // kicker on, beam off, measurement on, 
void dioOff();
void mtcOn();
void kickOn();
void pauseSetup0 ();  // start of cycle...trigger and beam on
void pauseSetup1 ();  // end of cycle....beam off, light pulser, tape move, measurement
void pauseSetup2 ();  // same as 0 but with background signal set
void pauseSetup3 ();  // start of cycle...trigger and beam on
void pauseSetup4 ();  // end of cycle....beam off, light pulser, tape move, measurement
void pauseSetup5 ();  // same as 0 but with background signal set

GtkWidget *entryBeamOn, *entryBeamOff, *entryMTCMove, *entryLightPulser, *entryBeamPause;
GtkWidget *cyc_lab1, *cyc_lab2, *cyc_lab3, *cyc_lab4, *cyc_lab5;

GtkWidget *cyc_labtotal,*cyc_labdata,*cyc_labbkg,*cyc_labnow;

void dataEntry( GtkWidget *fixed);
void adjustBeamOn(GtkWidget *widget, GtkWidget *entryBeamOn);
void adjustBeamOff(GtkWidget *widget, GtkWidget *entryBeamOff);
void adjustMTCMove(GtkWidget *widget, GtkWidget *entryMTCMove);
void adjustLightPulser(GtkWidget *widget, GtkWidget *entryLightPulser);
void adjustBeamPause(GtkWidget *widget, GtkWidget *entryBeamPause);

uint8 tapeMove, litePulser;
double beamOn=2000, beamOff=2000, mtcMove=350, lightPulser=200, beamPause=0;

void setMode(GtkWidget *widget);
void setNormal(GtkWidget *widget);
void numberMoves(GtkWidget *widget);
//void setMode();
long int cycleOnOff=0;
void startCycles(GtkWidget *widget);
long int takeAway=1, bkg=0, bkgCycle=0, normal=0, cyc0=0, cyc1=0, cyc2=0, cyc3=0;
long int numCycles=0, numData=0, numBkg=0;
long int busylite,busymtc,numMTC;
GtkWidget *butCycle, *butKicker, *butMTC, *butMoves;

//gchar mode[25]="Normal mode\0";   // related to takeAway variable above normal=0, take away=1
void spectraDraw(GtkWidget *spectra);

void comboBkg();                  // works on bkg variable above
void comboBkg_selected(GtkWidget *widget, gpointer combo);
void cycleLabjack(uint8 cycle[], long int num);
void cycleSequence();
void cyclePause();
void kickerOn(GtkWidget *but);
long int kickerOnOff;

void moveMTC(GtkWidget *butMTC);

GtkWidget *window, *fixed;
gint XPLOT=800;
gint YPLOT=900;
gint YYPLOT=470; // (YPLOT-430)
long int REFRESH = 100;   // update gtk graphics every 100 ms
double tWidth=600.0, tHeight=100.0, tOffset=105.0, timedisplay=1500;      // choose time of spectrum to display, 1 day = 1440 min;

void alarm_wakeup (long int i);
struct itimerval tout_val;

long int cycle=0;
uint8 cycle0[24], cycle2[24];
uint8 cycle1[42], cycle3[42];
uint8 away0[42], away1[24];
uint8 away2[42], away3[24];
uint8 off[14], kick[14];
uint8 onMTC[24];
uint8 cycle4[32],cycle6[32];
uint8 pause0[24], pause1[14], pause2[42];
uint8 pause3[24], pause4[14], pause5[42];
uint8 pause6[32], pause9[32];
uint8 trig, beam, lite, mtc, kck, meas, back, mtc1;

long int time0=0, timeSave=0, timeDiff=0, timeDiffSave=0;
char time0Str[50]="\0";

long int t_conflict=0;   // flag to keep trying to read temperatures while cycling
long int maxchan=30240;   // allow 3 full weeks of running without resetting
int plotTempFlag =0;  // flag to restrict number of temperature plots per second.

struct thermometer {
  double deg[30240];
  double min;
  double max;
  double cal;
  double calx;
  long int unit;          // 0 = celcius; 1 = kelvin, 2=Fahrenheit
  double xx[30240];
  double yy[30240];
} therm[4];

  FILE *fileTherm;

/***********************************************************/ 
/***********************************************************/
int main(int argc, char **argv){
  long int localID=-1;
  long int error=0;
  GtkWidget *but, *but2;
  long int count=0;
  long int ii;

  //  long int ii=0,jj=0;

  //Open first found U3 over USB
  printf("opening usb .... ");
  //  localID = -1;
  while (count < 3) {
    if( (hU6 = openUSBConnection(localID)) == NULL){
      count++;
      printf("Opening failed; reseting and attempting %li of 2 more times \n",count);
      resetU6(0x01);                       // 0x00 = soft reset; 0x01 = reboot
      if (count > 2) return 0;
    } 
    else {
      count = 3;
    }
  }

  printf("opened usb\n");
  /*
  resetU3(0x01);                       // 0x00 = soft reset; 0x01 = reboot
  sleep(2);
  printf ("....U3 device reset \n");
  */
  //Get calibration information from UE9
  printf("getting calib .... ");
  error = getCalibrationInfo(hU6, &caliInfo);
  if(error != 0){
    printf("\n%li - %s\n",error, errormsg[error]);
    closeUSBConnection(hU6);
    return 0;
  } 
  printf("got calib \n");

/*
   Channels for each signal and hex value
     1 eio0 0x01 external trigger 
     2 eio1 0x02 beam signal for acquisition
     3 eio2 0x04 light pulser
     4 eio3 0x08 tape transport
     5 eio4 0x10 kicker 
       eio5    blocked due to the kicker
     7 eio6 0x40 background
     8 eio7 0x80 measurement 
     9 cio0 0x01 second MTC signal
  **** note that if you move these, then the write mask will change and a
   different array element will need to be used.
*/
  trig = 0x01;
  beam = 0x02;
  lite = 0x04;
  mtc  = 0x08;
  kck  = 0x10;
  back = 0x40;
  meas = 0x80;
  mtc1 = 0x01;

//  gstuff(&argc, &argv);

// Configure DIO5 for output and set output to low (USB, config, chan, up/down)

  for (ii=4; ii< 20; ii++){
    if (dio_init(ii) !=0) return 0;
  }
  /*
  if (dio_init(kicker) != 0) return 0;
  if (dio_init(tape) != 0) return 0;
  if (dio_init(beam) != 0) return 0;
  if (dio_init(light) != 0) return 0;
  if (dio_init(daq) != 0) return 0;
  if (dio_init(back) != 0) return 0;
  if (dio_init(trig) != 0) return 0;
  */
  //  if (temp_init(tempRoom) != 0) return 0;

  if (dac_init(dac0) != 0) return 0;
  if (dac_set(dac0,0.0) != 0) return 0;
  if (dac_set(dac0,5.0) != 0) return 0;

/*
    Set up thermometers, read start value, set up cairo plot parameters, and open
    and write first data to output file.  Purchased from LabJack.
    Using Universal Temperature Probe MODEL EI-1034 with silicon type LM34 sensor
    Typical room accuracy +/-0.4 F waterproof stainless steel tube.
    Wiring: Red +5V, Black = ground; White = signal (add 10 kOhm resistor in series for +25 foot length)
    Range is 0-300 F, add negative 5-15 V to ground to extend range below 0 F.
    See manual for extending the range to -50F
*/
  plotTempFlag = tempInit();
/*
long ehFeedback( HANDLE hDevice,
                 uint8 *inIOTypesDataBuff,    // data bytes
                 long inIOTypesDataSize,      // number of data bytes
                 uint8 *outErrorcode,
                 uint8 *outErrorFrame,
                 uint8 *outDataBuff,
                 long outDataSize);
*/
/*
   Load initial fixed cycle parameters channels for each signal and hex value

     1 eio0 0x01 external trigger 
     2 eio1 0x02 beam signal for acquisition
     3 eio2 0x04 light pulser
     4 eio3 0x08 tape transport
     5 eio4 0x10 kicker 
     6 eio5 0x20 measurement 
     7 eio6 0x40 background


*/
  cycleSetup0 ();  // normal mode regular cycle
  cycleSetup1 ();  // normal mode regular cycle
  cycleSetup2 ();  // normal mode background cycle
  cycleSetup3 ();  // normal mode background cycle
  cycleSetup4 ();  // take away mode regular cycle
  cycleSetup5 ();  // take away mode regular cycle
  cycleSetup6 ();  // take away mode background cycle
  cycleSetup7 ();  // take away mode background cycle
  dioOff();        // all DIO off
  kickOn();        // kicker on but all other DIO off
  mtcOn();       // move MTC once
  pauseSetup0() ;  // setup normal mode with pause in beam to wait for short-lived decay and/or control rate
  pauseSetup1() ;  // setup normal mode with pause in beam to wait for short-lived decay and/or control rate
  pauseSetup2() ;  // setup normal mode with pause in beam to wait for short-lived decay and/or control rate
  pauseSetup3() ;  // setup normal mode with pause in beam to wait for short-lived decay and/or control rate
  pauseSetup4() ;  // setup normal mode with pause in beam to wait for short-lived decay and/or control rate
  pauseSetup5() ;  // setup normal mode with pause in beam to wait for short-lived decay and/or control rate

  //  cycleSequence(); // set up cycles
  //  if (beamPause > 1) cyclePause();    // set up pause cycles
/* 
  kicker timer stuff
*/
/*
  tout_val.it_interval.tv_sec = 0;
  tout_val.it_interval.tv_usec = 0;
  //tout_val.it_value.tv_sec = 0; // set timer for "INTERVAL (10) seconds 
  tout_val.it_value.tv_sec = INTERVAL; // set timer for "INTERVAL (10) seconds 
  tout_val.it_value.tv_usec = 0;

  setitimer(ITIMER_REAL, &tout_val,0);   // start the timer

  signal(SIGALRM,alarm_wakeup);   // set the Alarm signal capture 
*/ 
/*
  GTK+ stuff  
*/
  //  g_thread_init(NULL);   // attempt to prevent interrupt collisions
  //  gdk_threads_init();
  
  gtk_init (&argc, &argv);                                                     // mandatory for gtk applications
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);                               // creates new window
  gtk_window_set_title (GTK_WINDOW (window),"U6-MTAS LABJACK");                   // sets title of window
  gtk_container_set_border_width (GTK_CONTAINER(window),1);                    // sets border width

  fixed = gtk_fixed_new();                                                     // create a container to hold future widgets
  gtk_container_add(GTK_CONTAINER(window), fixed);                             // I must determine position of all widgets

  g_signal_connect (window, "delete-event",G_CALLBACK (delete_event), NULL);        // handler to exit gtk
//  g_signal_connect (window, "event", NULL);    // from cairo example  window -> frame1
/*
  draw and expose-event redraws on any event including mouse movement.  They are usually needed in programs.
   But in this program I am renewing the widgets on a timer based on eitehr 50 or 100 ms
*/
  g_signal_connect (window, "draw", G_CALLBACK (on_expose_event), NULL);    // gtk3 - 
  //g_signal_connect (window, "expose-event", G_CALLBACK (on_expose_event), NULL);    // gtk2 from cairo example  window -> frame1
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);           // from cairo example  window -> frame1

  gtk_window_set_default_size(GTK_WINDOW(window), XPLOT, YPLOT);                    // from cairo example  window -> frame1
  //  gtk_widget_set_app_paintable(window, TRUE);                                       // from cario example  window -> frame1

  menuControl();
  dataEntry(fixed);

/*
 Mode selection....normal or take-away
*/
//  if (takeAway==1) strcpy(mode,);
  but = gtk_button_new_with_label("Normal Mode");
  gtk_fixed_put(GTK_FIXED(fixed),but, 50, 250);
  g_signal_connect(but, "clicked", G_CALLBACK(setMode), NULL);  

  if (takeAway == 1) {                                     // normal mode set initially
    but2 = gtk_button_new_with_label("Measure w/out Beam");
    gtk_fixed_put(GTK_FIXED(fixed),but2, 200, 250);
    g_signal_connect(but2, "clicked", G_CALLBACK(setNormal), NULL);  
  } 
/*
 1 or 2 tape movements
*/

  butMoves = gtk_button_new_with_label("MTC 1 move");
  gtk_fixed_put(GTK_FIXED(fixed),butMoves, 400, 250);
  g_signal_connect(butMoves, "clicked", G_CALLBACK(numberMoves), (gpointer) numMTC);  
/*
 Start/Stop cycling
*/

  butCycle = gtk_button_new_with_label("Cycling OFF");
  gtk_fixed_put(GTK_FIXED(fixed),butCycle, 400, 50);
  g_signal_connect(butCycle, "clicked", G_CALLBACK(startCycles), (gpointer) cycleOnOff);  
/*
 Single MTC move
*/

  butMTC = gtk_button_new_with_label("Move MTC");
  gtk_fixed_put(GTK_FIXED(fixed),butMTC, 700, 50);
  g_signal_connect(butMTC, "clicked", G_CALLBACK(moveMTC), NULL);   // add kicker                    
/*
 Start/Stop cycling
*/

  butKicker = gtk_button_new_with_label("Kicker OFF");
  gtk_fixed_put(GTK_FIXED(fixed),butKicker, 550, 50);
  g_signal_connect(butKicker, "clicked", G_CALLBACK(kickerOn), NULL);   // add kicker                    
/*
 Background selection.... every second, third, etc cycle will be tagged as background
 which means the kicker (beam) is on (off) for this cycle
*/
  comboBkg();

/*
 Set up the cairo windows and the surface upon which to draw. Drawing on a surface
which will get saved, cleared and then added to, will hopefully speed up the program
when updating large bits of code such as large histograms.
*/
  timeNow = gtk_drawing_area_new();                // an extra drawable for cairo graphics
  gtk_widget_set_size_request(timeNow,200,20);       // do not need a callback - probably
  gtk_fixed_put(GTK_FIXED(fixed),timeNow, 20,30);
  g_signal_connect (timeNow, "draw", G_CALLBACK (timeCurrent), crTime);    // gtk3 - sets the routine on draw
  /*
  crStat = gtk_drawing_area_new();                // an extra drawable for cairo graphics
  gtk_widget_set_size_request(crStat,200,150);       // do not need a callback - probably
  gtk_fixed_put(GTK_FIXED(fixed),crStat,300,150);
  g_signal_connect (crStat, "draw", G_CALLBACK (timeLine), crStat);    // gtk3 - sets the routine on draw
  */
  TLine = gtk_drawing_area_new();                // an extra drawable for cairo graphics
  gtk_widget_set_size_request(TLine,XPLOT-40,100);       // do not need a callback - probably
  gtk_fixed_put(GTK_FIXED(fixed),TLine,20,300);
  g_signal_connect (TLine, "draw", G_CALLBACK (timeLine), crTLine);    // gtk3 - sets the routine on draw

//  printf("Hi-main1 !\n");
 
  spectra = gtk_drawing_area_new();                // an extra drawable for cairo graphics
  gtk_widget_set_size_request(spectra,XPLOT,YPLOT-430);       // do not need a callback - probably
  gtk_fixed_put(GTK_FIXED(fixed),spectra, 0,430);
  g_signal_connect (spectra, "draw", G_CALLBACK (spectraDraw), NULL);    // gtk3 - sets the routine on draw

  //  GtkWidget* data = gtk_image_new_from_pixbuf(GdkPixbuf* spec_temps);

  //  printf("Hi-main2 !\n");

/*
 Show all in window
*/
  g_timeout_add(REFRESH, (GSourceFunc) time_handler, (gpointer) fixed);             // timer to refresh widget "fixed"
  gtk_widget_show_all (window);    // show windows before stating timer loop
  time_handler(fixed);
//  gdk_threads_enter();   // attempt to prevent interrupt collisions
  gtk_main ();
//  gdk_threads_leave();   // attempt to prevent interrupt collisions
/*
 Close USB connection and end program
*/
  fclose(fileTherm);

  closeUSBConnection(hU6);
  printf("USB closed\n");
  return 0;
}

/****************************************************************************/
void spectraDraw(GtkWidget *spectra){

  //   cr = gdk_cairo_create (gtk_widget_get_window(widget));   // gtk3
  //   printf("Hi-spectraDraw 1 !\n");

   /*  

   cr1 = gdk_cairo_create (gtk_widget_get_window(spectra)); 
   //   printf("Hi-spectraDraw 2 !\n");
    cairo_identity_matrix (cr1);                       // reset origin to upper left corner
    cairo_translate(cr1, 650, 10);   // move to start the rectangular graph
    cairo_set_source_rgb(cr1, 0, 0, 0);                // black
    cairo_rectangle(cr1, 0, 0, 50, 50);        // graph frame
    cairo_stroke_preserve(cr1);                        // includes the stroke
    cairo_set_source_rgb(cr1, 1.0, .0, 1.0);          // graph fill - white
    cairo_fill(cr1);                                   // fill graph with color
   */
  //    tempPlot1(cr1);
/*
    if (cairo_setup == 0 || cairo_setup == 1) {
      //      printf("Hi-0 = %li \n", cairo_setup) ;
  printf(" Yes I'm in here! \n");
*/

      if (timeDiff != 0) cairo_destroy(crSpec);    // only destroy and create after a read
      crSpec = gdk_cairo_create (gtk_widget_get_window(spectra));
      tempPlot1(crSpec);     // plotting routine for temperature spectra
      cairo_setup = 1;       // 1 = no new data to plot; 0 = thermometers read, plot data again
      /*
    } 
    else {
      //      printf("Hi-1 = %li \n", cairo_setup);
      crSpec = gdk_cairo_create (gtk_widget_get_window(spectra));
      tempPlot1(crSpec);
      //      tempPlot2(crSpec);
      //      tempPlot2(crSpec);
    }
      */
  return;
}
/****************************************************************************/
void comboBkg(){
  GtkWidget *combo, *label;
  gchar txt[30]="\0";
  long int ii;
/*
Now do combo box
*/
  //combo = gtk_combo_box_new_text();   //gtk2
  combo = gtk_combo_box_text_new();  //gtk3
  //    combo = gtk_combo_box_new();
  for (ii=0; ii< 21; ii++){
    sprintf(txt,"Data:Bkg :: %li : 1",ii);
    //gtk_combo_box_append_text(GTK_COMBO_BOX(combo), txt);  // gtk2
    gtk_combo_box_text_append_text((GtkComboBoxText*) combo, txt);  //gtk3
  }

  label=NULL;
  g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(comboBkg_selected), (gpointer) label);
  //  g_signal_connect(combo, "changed", G_CALLBACK(editlabel), NULL);//(gpointer) lab_edit);
  //  g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(dataEntry), fixed);

  gtk_fixed_put(GTK_FIXED(fixed),combo,550,250);


  return;
}
/****************************************************************************/
void comboBkg_selected(GtkWidget *widget, gpointer combo) { 
  int index=0;
  char tt[10]="\0", tt1[10]="\0", tt2[10]="\0", tt3[10]="\0",tt4[10]="\0",txt[30]="\0";
  gchar *text;

  //text =  gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget)); //gtk2
  text =  gtk_combo_box_text_get_active_text((GtkComboBoxText*) widget);   //gtk3
  //  gchar *text =  gtk_combo_box_text_get_active_text(Gtk_Combo_BoxText(widget));   //gtk3

  strcpy(txt,text);
  sscanf(text,"%s %s %s %s %s",tt,tt1,tt2,tt3,tt4);
  bkg=atol(tt2);

  g_free(text);

  //  gtk_widget_queue_draw_area(fixed,0,0,XPLOT,YPLOT);    // redraw entire window to prevent over writing

  return;
}

/****************************************************************************/
void kickerOn(GtkWidget *but){

  //  printf("kickeronoff =%li\n",kickerOnOff);
  if (kickerOnOff == 0){
    cycleOnOff = 0;
    kickerOnOff = 1;
    kickOn();
    cycleLabjack(kick,14);             
    gtk_button_set_label((GtkButton*) butKicker,"Kicker ON ");
    gtk_button_set_label((GtkButton*) butCycle,"Cycling OFF");  
  }
  else {
    cycleOnOff = 0;
    kickerOnOff = 0;
    dioOff();
    cycleLabjack(off,14);             
    gtk_button_set_label((GtkButton*) butKicker,"Kicker OFF");
    gtk_button_set_label((GtkButton*) butCycle,"Cycling OFF");  
  }

  return;
}

/****************************************************************************/
void moveMTC(GtkWidget *butMTC){
  
  kickerOnOff = 0;
  kickerOn(butKicker);   // put kicker on when tape is moving
  mtcOn();

  //  gtk_button_set_label((GtkButton*) butKicker,"Kicker ON");
  gtk_button_set_label((GtkButton*) butCycle,"Cycling OFF");  

  //  cycleLabjack(off,14);             
  //  gtk_button_set_label((GtkButton*) butMTC,"moving"); 
  //  printf ("mtc button should be changing\n");

  cycleLabjack(onMTC,24);             

  //  gtk_button_set_label((GtkButton*) butMTC,"MTC move"); 
  
  return;
}

/****************************************************************************/
void startCycles(GtkWidget *but1){
  gchar txt6[30]="\0",txt7[30]="\0",txt8[30]="\0";

  if (cycleOnOff == 0){
    cycleOnOff = 1;
    kickerOnOff=0; 
    cycleSequence();
    //    if (beamPause > 1) cyclePause;

    //    printf ("cyc 012 => %li %li %li\n",cyc0, cyc1, cyc2);
    if (beamPause < 1){
      if (takeAway == 1){
	cycleSetup0 ();  // normal mode regular cycle
	cycleSetup1 ();  // normal mode regular cycle
	cycleSetup2 ();  // normal mode background cycle
	cycleSetup3 ();  // normal mode background cycle
	cycleLabjack(cycle0,24);   // start the first non-background cycle             
      }
      else {
	cycleSetup4 ();  // take-away mode regular cycle
	cycleSetup5 ();  // take-away mode regular cycle
	cycleSetup6 ();  // take-away mode background cycle
	cycleSetup7 ();  // take-away mode background cycle
	cycleLabjack(away0,42);   // start the first non-background cycle             
      }
    } 
    else {
      //      printf("Setting up pause before starting - beamPause=%lf \n",beamPause);
      if (takeAway == 1){
	pauseSetup0 ();  // normal mode regular cycle
	pauseSetup1 ();  // normal mode regular cycle
	pauseSetup2 ();  // normal mode background cycle
	pauseSetup3 ();  // normal mode background cycle
	pauseSetup4 ();  // normal mode background cycle
	pauseSetup5 ();  // normal mode background cycle
	cycleLabjack(pause0,24);   // start the first non-background cycle             
      }
      /*
      else {
	cycleSetup4 ();  // take-away mode regular cycle
	cycleSetup5 ();  // take-away mode regular cycle
	cycleSetup6 ();  // take-away mode background cycle
	cycleSetup7 ();  // take-away mode background cycle
	cycleLabjack(away0,42);   // start the first non-background cycle             
      }
      */
    }

    gtk_button_set_label((GtkButton*) but1,"Cycling ON ");
    gtk_button_set_label((GtkButton*) butKicker,"Kicker OFF");

    numCycles=0;
    sprintf(txt6,"Total cycles: %li",numCycles);
    gtk_label_set_text (GTK_LABEL (cyc_labtotal), txt6);
    numData=0;
    sprintf(txt7,"Data cycles: %li",numData);
    gtk_label_set_text (GTK_LABEL (cyc_labdata), txt7);
    numBkg=0;
    sprintf(txt8,"Bkg cycles: %li",numBkg);
    gtk_label_set_text (GTK_LABEL (cyc_labbkg), txt8);
  
  }
  else {
    cycleOnOff = 0;
    kickerOnOff=0; 
    dioOff();

    cycleLabjack(off,14);             
    gtk_button_set_label((GtkButton*) but1,"Cycling OFF");
    gtk_button_set_label((GtkButton*) butKicker,"Kicker OFF");
  }

  return;
}

/****************************************************************************/
void numberMoves(GtkWidget *but){
  //  char s1=

  if (numMTC == 2){  //strcmp(mode,"Take Away Mode") == 0){
    numMTC = 1;
    gtk_button_set_label((GtkButton*) but,"MTC 1 move");
  }
  else {
    numMTC = 2;
    gtk_button_set_label((GtkButton*) but,"MTC 2 moves");
  }

  return;
}

/****************************************************************************/
void setMode(GtkWidget *but){
  //  char s1=

  if (takeAway == 0){  //strcmp(mode,"Take Away Mode") == 0){
    takeAway = 1;
    gtk_button_set_label((GtkButton*) but,"Normal Mode");
  }
  else {
    takeAway = 0;
    gtk_button_set_label((GtkButton*) but,"Take Away Mode");
  }

  return;
}
/****************************************************************************/
void setNormal(GtkWidget *but){
  //  char s1=

  if (normal == 0 && takeAway == 1){  //strcmp(mode,"Take Away Mode") == 0){
    normal = 1;
    gtk_button_set_label((GtkButton*) but,"Measure w/Beam");
  }
  else if (normal == 1 && takeAway == 1) {
    normal = 0;
    gtk_button_set_label((GtkButton*) but,"Measure w/out Beam");
  } else if (takeAway == 1) {
    normal = 0;
    gtk_button_set_label((GtkButton*) but,"Set Normal mode type?");
  }
  else {
    normal = 0;
    gtk_button_set_label((GtkButton*) but,"Not in Normal Mode");
  }

  return;
}
/****************************************************************************/
void adjustBeamOn( GtkWidget *widget, GtkWidget *entryBeamOn ){
  char txt[20]="\0";
/*
    Set the value for the duration of the grow in cycle. Actual kicker off duration
    may include light pulser + MTC move + other minor time delays
*/
//  double x1;

  beamOn = atof(gtk_entry_get_text(GTK_ENTRY(entryBeamOn)));      // change data string to double

  //    cycleSequence();
  //    if (beamPause > 1)cyclePause();

  //  sprintf(txt1,"<span foreground=\"green\">%.0f>/span>",beamOn);

  sprintf (txt,"%.0f",beamOn);
  gtk_label_set_text (GTK_LABEL (cyc_lab1), txt);

  //  gtk_widget_queue_draw_area(fixed,0,0,XPLOT,YPLOT);    // redraw entire window to prevent over writing

  return;
}
/****************************************************************************/
void adjustBeamOff( GtkWidget *widget, GtkWidget *entryBeamOff ){
  char txt[20]="\0";
/*
    Set the value for the duration of the decay out cycle. Actual kicker on duration
    may include light pulser + MTC move + other minor time delays
*/
//  double x1;

  beamOff = atof(gtk_entry_get_text(GTK_ENTRY(entryBeamOff)));      // change data string to double

  //  gtk_widget_queue_draw_area(fixed,0,0,XPLOT,YPLOT);    // redraw entire window to prevent over writing

  sprintf (txt,"%.0f",beamOff);
  gtk_label_set_text (GTK_LABEL (cyc_lab2), txt);

  //  cycleSequence();
  //    if (beamPause > 1) cyclePause();

  return;
}
/****************************************************************************/
void adjustBeamPause( GtkWidget *widget, GtkWidget *entryBeamPause ){
  char txt[20]="\0";
/*
    Set the value for the duration of the grow in cycle. Actual kicker off duration
    may include light pulser + MTC move + other minor time delays
*/
//  double x1;

  beamPause = atof(gtk_entry_get_text(GTK_ENTRY(entryBeamPause)));      // change data string to double

  //  cycleSequence();
  //    if (beamPause > 1) cyclePause();

  //  sprintf(txt1,"<span foreground=\"green\">%.0f>/span>",beamOn);

  sprintf (txt,"%.0f",beamPause);
  gtk_label_set_text (GTK_LABEL (cyc_lab5), txt);

  //  gtk_widget_queue_draw_area(fixed,0,0,XPLOT,YPLOT);    // redraw entire window to prevent over writing

  return;
}
/****************************************************************************/
void adjustMTCMove( GtkWidget *widget, GtkWidget *entryMTCMove ){
/*
    Set the value for the duration of the MTC movement
*/
  double x1;
  char txt[20]="\0";

  mtcMove = atof(gtk_entry_get_text(GTK_ENTRY(entryMTCMove)));      // change data string to double
  if (mtcMove < 4080 ) {
    x1 = mtcMove/16;        // convert to steps of 16 ms for labjack
    x1 = floor (x1);            // take the floor of the resulting fraction
    tapeMove = (uint8) x1;    // convert to uint8 for labjack
  } 
  else {
    mtcMove = 4080;         // max value for labjack
    tapeMove = 0xff;          // max value for labjack
  }

  mtcMove = (double) tapeMove * 16.000000;
  sprintf (txt,"%.0f",mtcMove);
  gtk_entry_set_text (GTK_ENTRY (entryMTCMove), txt);

  gtk_label_set_text (GTK_LABEL (cyc_lab3), txt);

  //  cycleSequence();
  //    if (beamPause > 1) cyclePause();

  mtcOn();

  //  gtk_widget_queue_draw_area(fixed,0,0,XPLOT,YPLOT);    // redraw entire window to prevent over writing

  return;
}

/****************************************************************************/

void adjustLightPulser( GtkWidget *widget, GtkWidget *entryLightPulser ){
/*
    Set the value for the duration of the light pulser measurement
*/
  double x1;
  char txt[20]="\0";

  lightPulser = atof(gtk_entry_get_text(GTK_ENTRY(entryLightPulser)));      // change data string to double
  if (lightPulser < 4080 ) {
    x1 = lightPulser/16;        // convert to steps of 16 ms for labjack
    x1 = floor (x1);            // take the floor of the resulting fraction
    litePulser = (uint8) x1;    // convert to uint8 for labjack
  } 
  else {
    lightPulser = 4080;         // max value for labjack
    litePulser = 0xff;          // max value for labjack
  }

  lightPulser = (double) litePulser * 16.000000;
  sprintf (txt,"%.0f",lightPulser);
  gtk_entry_set_text (GTK_ENTRY (entryLightPulser), txt);
  gtk_label_set_text (GTK_LABEL (cyc_lab4), txt);

  //  cycleSequence();
  //    if (beamPause > 1) cyclePause();

  //  gtk_widget_queue_draw_area(fixed,0,0,XPLOT,YPLOT);    // redraw entire window to prevent over writing

  return;
}

/****************************************************************************/

void dataEntry( GtkWidget *fixed){
  GtkWidget *box1;
  GtkWidget *table;
  GtkWidget *label1, *label2, *label3, *label4,*label5;
  gchar txt1[10]="\0", txt2[10]="\0", txt3[10]="\0", txt4[10]="\0";
  gchar txt5[10]="\0", txt6[30]="\0", txt7[30]="\0", txt8[30]="\0";
  gchar txt9[30]="\0";

/*
 Data entry fields
*/
//  box1 = gtk_vbox_new(FALSE, 0);
  box1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  gtk_box_set_homogeneous((GtkBox*)box1, TRUE);

  gtk_fixed_put(GTK_FIXED(fixed),box1,10,50);

  table = gtk_table_new(4, 2, FALSE);
  gtk_container_add(GTK_CONTAINER(box1), table);

  label1 = gtk_label_new("(ms) - Beam on");
  label2 = gtk_label_new("(ms) - Beam off");
  label3 = gtk_label_new("(ms) - MTC move");
  label4 = gtk_label_new("(ms) - Light pulser");
  label5 = gtk_label_new("(ms) - Beam pause");

  gtk_table_attach(GTK_TABLE(table), label1, 1, 2, 0, 1, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table), label2, 1, 2, 1, 2, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table), label3, 1, 2, 2, 3, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table), label4, 1, 2, 3, 4, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table), label5, 1, 2, 4, 5, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);

  entryBeamOn = gtk_entry_new();
  entryBeamOff = gtk_entry_new();
  entryMTCMove = gtk_entry_new();
  entryLightPulser = gtk_entry_new();
  entryBeamPause = gtk_entry_new();

  //  gtk_entry_set_max_length(GTK_ENTRY (entryBeamOn),8);  // does affect input window but # of characters

  //  sprintf(txt1,"<span foreground=\"red\">%.0f>/span>",beamOn);
  /**/

  sprintf(txt1,"%.0f",beamOn);
  sprintf(txt2,"%.0f",beamOff);
  sprintf(txt3,"%.0f",mtcMove);
  sprintf(txt4,"%.0f",lightPulser);
  sprintf(txt5,"%.0f",beamPause);

  cyc_lab1 = gtk_label_new(txt1);
  cyc_lab2 = gtk_label_new(txt2);
  cyc_lab3 = gtk_label_new(txt3);
  cyc_lab4 = gtk_label_new(txt4);
  cyc_lab5 = gtk_label_new(txt5);

  gtk_entry_set_text (GTK_ENTRY (entryBeamOn), txt1);
  gtk_entry_set_text (GTK_ENTRY (entryBeamOff), txt2);
  gtk_entry_set_text (GTK_ENTRY (entryMTCMove), txt3);
  gtk_entry_set_text (GTK_ENTRY (entryLightPulser), txt4);
  gtk_entry_set_text (GTK_ENTRY (entryBeamPause), txt5);

  /*
  gtk_table_attach(GTK_TABLE(table), cyc_lab1, 2, 3, 0, 1, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table), cyc_lab2, 2, 3, 1, 2, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table), cyc_lab3, 2, 3, 2, 3, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table), cyc_lab4, 2, 3, 3, 4, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
  */  
  /*  
  g_signal_connect(G_OBJECT(entryBeamOn), "activate", GTK_SIGNAL_FUNC(adjustBeamOn),entryBeamOn);
  g_signal_connect(G_OBJECT(entryBeamOff), "activate", GTK_SIGNAL_FUNC(adjustBeamOff),entryBeamOff);
  g_signal_connect(G_OBJECT(entryMTCMove), "activate", GTK_SIGNAL_FUNC(adjustMTCMove),entryMTCMove);
  g_signal_connect(G_OBJECT(entryLightPulser), "activate", GTK_SIGNAL_FUNC(adjustLightPulser),entryLightPulser);
  g_signal_connect(G_OBJECT(entryBeamPause), "activate", GTK_SIGNAL_FUNC(adjustBeamPause),entryBeamPause);
 */
  g_signal_connect(G_OBJECT(entryBeamOn), "activate", G_CALLBACK(adjustBeamOn),entryBeamOn);
  g_signal_connect(G_OBJECT(entryBeamOff), "activate", G_CALLBACK(adjustBeamOff),entryBeamOff);
  g_signal_connect(G_OBJECT(entryMTCMove), "activate", G_CALLBACK(adjustMTCMove),entryMTCMove);
  g_signal_connect(G_OBJECT(entryLightPulser), "activate", G_CALLBACK(adjustLightPulser),entryLightPulser);
  g_signal_connect(G_OBJECT(entryBeamPause), "activate", G_CALLBACK(adjustBeamPause),entryBeamPause);

  gtk_table_attach(GTK_TABLE(table), entryBeamOn,      0, 1, 0, 1, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table), entryBeamOff,     0, 1, 1, 2, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table), entryMTCMove,     0, 1, 2, 3, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table), entryLightPulser, 0, 1, 3, 4, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table), entryBeamPause,   0, 1, 4, 5, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);

  
  gtk_table_attach(GTK_TABLE(table), cyc_lab1, 2, 3, 0, 1, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table), cyc_lab2, 2, 3, 1, 2, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table), cyc_lab3, 2, 3, 2, 3, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table), cyc_lab4, 2, 3, 3, 4, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
  gtk_table_attach(GTK_TABLE(table), cyc_lab5, 2, 3, 4, 5, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);
  
/* track the number of cycles */

  sprintf(txt6,"Total cycles: %li",numCycles);
  cyc_labtotal = gtk_label_new(txt6);
  gtk_fixed_put(GTK_FIXED(fixed),cyc_labtotal,400,120);

  sprintf(txt7,"Data cycles: %li",numData);
  cyc_labdata = gtk_label_new(txt7);
  gtk_fixed_put(GTK_FIXED(fixed),cyc_labdata,400,140);


  sprintf(txt8,"Bkg cycles: %li",numBkg);
  cyc_labbkg = gtk_label_new(txt8);
  gtk_fixed_put(GTK_FIXED(fixed),cyc_labbkg,400,160);

  if (bkgCycle == bkg) sprintf(txt9,"Current cycle: BKG");
  else sprintf(txt9,"Current cycle: DATA %li", bkgCycle);
  cyc_labnow = gtk_label_new(txt9);
  gtk_fixed_put(GTK_FIXED(fixed),cyc_labnow,400,180);

/* show widgets */
  gtk_widget_show(table);

  gtk_widget_show(label1);
  gtk_widget_show(label2);
  gtk_widget_show(label3);
  gtk_widget_show(label4);

  gtk_widget_show(entryBeamOn);
  gtk_widget_show(entryBeamOff);
  gtk_widget_show(entryMTCMove);
  gtk_widget_show(entryLightPulser);
  /*  */
  gtk_widget_show(cyc_lab1);
  gtk_widget_show(cyc_lab2);
  gtk_widget_show(cyc_lab3);
  gtk_widget_show(cyc_lab4);
  /*  */

  gtk_widget_show(cyc_labtotal);
  gtk_widget_show(cyc_labdata);
  gtk_widget_show(cyc_labbkg);
  gtk_widget_show(cyc_labnow);


  return;
}
/******************************************************************************/
void cyclePause(){
/*
  Calculates the times when labjack cycles are issued.
  This is for normal sequence w/out beam
*/
  double t1=0.,t2=0., t3=0.;

  busylite = lightPulser/REFRESH;     // calculates delays in labjack where other calls to labjack should NOT be done
  busymtc = mtcMove/REFRESH;

  if (takeAway == 1) {
    t1 = beamOn;                          // pause0 array
    t2 = t1 + beamPause + lightPulser;         // pause1 array
    t3 = t2 + mtcMove + beamOff;          // pause2 array

    cyc3 = (long int)t1;
    cyc1 = (long int)t2;
    cyc2 = (long int)t3;

    cyc3 = cyc3/REFRESH;        // send array pause1 to labjack
    cyc1 = cyc1/REFRESH;        // send array pause2 to labjack
    cyc2 = cyc2/REFRESH;        // send array pause0 to labjack
    cyc0 = 0;

  }
  //  printf ("0- %lf 1-%lf 2-%lf  \n",t1,t2,t3);
  //  printf ("0- %li 1- %li 2- %li  \n",cyc3,cyc1,cyc2);
  return;
}

/******************************************************************************/
void cycleSequence(){
/*
  Calculates the times when labjack cycles are issued.
  This is for normal sequence w/out beam
*/
  double t2=0., t3=0.;

  busylite = lightPulser/REFRESH;     // calculates delays in labjack where other calls to labjack should NOT be done
  busymtc = mtcMove/REFRESH;

  if (takeAway == 1) {
    t2 = beamOn + lightPulser;            // cycle0 array
    t3 = t2 + mtcMove + beamOff;          // cycle1 array
    cyc1 = (long int)t2;
    cyc2 = (long int)t3;
    cyc1 = cyc1/REFRESH;        // send array cycle1 to labjack
    cyc2 = cyc2/REFRESH;        // send array cycle0 to labjack
    cyc0 = 0;
  } 
  else {
    t2 = lightPulser + mtcMove + beamOn;   //  in away0 array
    t3 = t2 + beamOff;                    //  in away1 array
    cyc1 = (long int)t2;
    cyc2 = (long int)t3;
    cyc1 = cyc1/REFRESH;        // send array away1 to labjack
    cyc2 = cyc2/REFRESH;        // send array away0 to labjack
    cyc0 = 0;

  }
  cyc3=0;

  //  printf ("in cycleSequence = beamPause = %lf  \n",beamPause);
  if (beamPause > 1) cyclePause();

  return;
}

/***********************************************************/
void resetU6(uint8 res){
/*
  Resets U3
*/
  uint8 resetIn[4], resetOut[4];
  long int ii=0, error=0;

  for (ii=0; ii<4; ii++){
    resetIn[ii]=0;
    resetOut[ii]=0;
  }
  resetIn[1] = 0x99;
  resetIn[2] = res;
  resetIn[3] = 0x00;
  resetIn[0] = normalChecksum8(resetIn,4);

  if( (error = LJUSB_BulkWrite(hU6, U6_PIPE_EP1_OUT, resetIn, 4)) < 4){
    LJUSB_BulkRead(hU6, U6_PIPE_EP2_IN, resetOut, 4); 
    printf("U6 Reset error: %s\n", errormsg[(int)resetOut[3]]);
    closeUSBConnection(hU6);
    return;
  }
  printf ("....U6 device reset \n");
  return;
}
/***********************************************************/
long int dio_init(long int dio){
/*
  sets digital i/o line dio to state 0= lo
  configuration command is 1
*/
  long int error=0;

  //  if((error = eDO(hU3, 1, dio, down)) != 0) {
  if((error = eDO(hU6, dio, down)) != 0) {
    printf("%s\n", errormsg[error]);
    closeUSBConnection(hU6);
    return error;
  }
  return error;
}

/***********************************************************/
long int dio_set(long int dio, long int ud){
/*
  sets diogital i/o line dio to state ud (0= lo, 1 = hi)
  assumes line has already been configured and initialized
*/
  long int error=0;

  //  if((error = eDO(hU3, 0, dio, ud)) != 0) {
  if((error = eDO(hU6, dio, ud)) != 0) {
    printf("%s\n", errormsg[error]);
    closeUSBConnection(hU6);
    return error;
  }
  return error;
}
/***********************************************************/
long int dac_init(long int dac){
/*
  sets digital-to-analog line dac to voltage vset = 0
  configuration command is 1
   timers must be set and 48MHz provides smallest ripple/noise: LJ_tc48MHZ
*/
  long int error=0;
  long int alngEnableTimers[2] = {1, 1};  //Enable Timer0-Timer1
  long int alngTimerModes[2] = {LJ_tmFREQOUT, LJ_tmPWM16};  //Set timer modes  - u3.h has definitions
  double adblTimerValues[2] = {10, 32767};  //Set PWM16 duty-cycles to 50%
  long int alngEnableCounters[2] = {0, 0};  //Enable Counter0
  double volts=0.0;

  // set timer 0 to 4, ie., FIO4
  //  if((error = eTCConfig(hU3, alngEnableTimers, alngEnableCounters, 4, LJ_tc48MHZ, 0, alngTimerModes, adblTimerValues, 0, 0)) != 0){
  if((error = eTCConfig(hU6, alngEnableTimers, alngEnableCounters, 4, LJ_tc48MHZ, 0, alngTimerModes, adblTimerValues, 0, 0)) != 0){
    printf("%s\n", errormsg[error]);
    closeUSBConnection(hU6);
    return error;
  }

  //  if((error = eDAC(hU3, &caliInfo, 1, dac, volts, 0, 0, 0)) != 0) {
  if((error = eDAC(hU6, &caliInfo, dac, volts, 0, 0, 0)) != 0) {
    printf("%s\n", errormsg[error]);
    closeUSBConnection(hU6);
    return error;
  }

  return error;
}
/***********************************************************/
long int dac_set(long int dac, double vset){
/*
  sets digital-to-analog line dac to voltage vset
  assumes line has already been configured and initialized
*/
  long int error=0;

  //  if((error = eDAC(hU3, &caliInfo, 0, dac, vset, 0, 0, 0)) != 0) {
  if((error = eDAC(hU6, &caliInfo, dac, vset, 0, 0, 0)) != 0) {
    printf("%s\n", errormsg[error]);
    closeUSBConnection(hU6);
    return error;
  }

  return error;
}
/***********************************************************/
long int adc_init(long int adc){
/*
  initializes ADC for temperature with command:

long eAIN(HANDLE Handle, 
          u6CalibrationInfo *CalibrationInfo, 
//          long ConfigIO,    // =1 configure and initialize; =0 after 
//          long *DAC1Enable,  //  =1 (TRUE); used on for hardware rev 1.20-1.21 and we have 1.30+
          long ChannelP, 
          long ChannelN,    // 31 for single ended values, other eAIN for differential
          double *Voltage,  // ADC value read
          long Range, 
          long Resolution, 
          long Settling, 
          long Binary, 
          long Reserved1, long Reserved2)
 */
  //long int involts=0;
  double involts=0.0;
  long int error=0; 
  double DACEnable=1; // Always enabled so set it to TRUE anyway
  //  long int DACEnable=1; // Always enabled so set it to TRUE anyway

  error = eAIN(hU6, &caliInfo, adc, 0, &involts, 0, 0, 0, 0, 0, 0);
  if (error != 0) {
    printf("%li - %s\n", error, errormsg[error]);
    closeUSBConnection(hU6);
    return 0;
  }

return 0;
}

/***********************************************************/
double tempCKF(long int adc, long int ckf){
/*
  reads temperature with command and returns in ckf usits:
     fck = 0 - Celcius
           1 - Kelvin
           2 - Farenheit

long eAIN(HANDLE Handle, 
          u3CalibrationInfo *CalibrationInfo, 
          long ConfigIO,    // =1 configure and initialize; =0 after 
          long *DAC1Enable, // =1 TRUE...always true except for hardware rev 1.20, 1.21
          long ChannelP, 
          long ChannelN,    // 31 for single ended values, other eAIN for differential
          double *Voltage,  // ADC value read
          long Range, 
          long Resolution, 
          long Settling, 
          long Binary, 
          long Reserved1, long Reserved2)
 */
  double involts=0.0, deg=0.0, degC=0.0, degK=0.0, degF=0.0, u3degK=0.0;
  long int error=0;   //0, DAC1Enable=1;
  double ohms=0.0, degRTD=273.15, amps=0.004167;  //I=V/R=4.92/1080 = 4.555 mA ; 200 - 50 uA

  // LJ_rgAUTO
  // LJ_rgBIP10V
  // LJ_rgBIP1V
  // LJ_rgBIPP1V
  // LJ_rgBIPP01V

  error = eAIN(hU6, &caliInfo, adc, 0, &involts, LJ_rgBIP1V, 8, 0, 0, 0, 0);
  //  error = eAIN(hU3, &caliInfo, 0, &DAC1Enable, adc, 31, &involts, 0, 0, 0, 0, 0, 0);
  if (error != 0){
    printf("%li - %s\n", error, errormsg[error]);
    closeUSBConnection(hU6);
    return 0;
  }

  ckf =3;       // voltage value in mV for RTDs

  degF = involts*100.0;
  degK = (55.56*involts) + 255.37;
  degC = degK - 273.15;

  deg = degC;
  if (ckf == 1) deg = degK;
  if (ckf == 2) deg = degF;
  if (ckf == 3) {
    ohms =  involts / amps;
    degRTD = 2.453687 * ohms + 27.781;
    deg = degRTD;  // voltage value in mV
  }
  return (deg);
}

/***********************************************************/
int tempInit(){
/*
    Set up thermometers
    Using Universal Temperature Probe MODEL EI-1034 with silicon type LM34 sensor
    Purchased from LabJack.
    Typical room accuracy +/-0.4 F waterproof stainless steel tube.
    Wiring: Red +5V, Black = ground; White = signal (add 10 kOhm resistor in series for +25 foot length)
    Range is 0-300 F, add negative 5-15 V to ground to extend range below 0 F.
    See manual for extending the range to -50F
*/
  long int ii=0, jj=0;

  for (ii=0; ii<thermNum; ii++){
    therm[ii].unit = celkelfah;
    if (adc_init(ii) != 0) return;
    for (jj=0; jj< maxchan; jj++){
      therm[ii].deg[jj]= -450;       // an impossible temp to measure
    }
    therm[ii].deg[0]= tempCKF(ii,therm[ii].unit);                    // read initial temperature
    therm[ii].min = floor(therm[ii].deg[0]-2.0);                     // determine plotting range (low)
    therm[ii].max = floor(therm[ii].deg[0]+3.0);                     // determine plotting range (high)
    therm[ii].cal = tHeight/(therm[ii].max - therm[ii].min);         // determine plotting range in cairo terms
    therm[ii].calx = tWidth/timedisplay;                             // choose time of spectrum to display
    for (jj=0; jj< maxchan; jj++){                                     // puts every temperature point at therm[ii].min
      therm[ii].yy[jj]= ( (double)YYPLOT - 50 - (tOffset*ii));        // tOffset = tHeight + 5
      therm[ii].xx[jj]= 50. + (therm[ii].calx * jj);                     // puts every time point as if one long spectrum
    }
    therm[ii].yy[0] -= (therm[ii].deg[0] - therm[ii].min) * therm[ii].cal;  // calculate y plot position (lower temp has higher yy)

  }

/* 
   Open and write data to file recording time 0.
*/
  if(thermNum > 0) {
    mtasOpenTemp();
    fprintf(fileTherm,"%s\n",GetDate());
  }

  fprintf(fileTherm,"     0 ");
  for (ii=0; ii<thermNum;ii++){
    fprintf(fileTherm,"% 6.3f ",therm[ii].deg[0]);
  }
  fprintf(fileTherm,"\n");

  fflush(fileTherm);                       // make sure data saved to disk even if program crashes.

  return (1);
}

/***********************************************************/
void tempRead(){
/*
    Read temperature and record in output file
*/
  long int ii=0;
  int flag=0;

  if (timeDiff < 0 || timeDiff > maxchan-1) printf("timeDiff out of range!\n");
  for (ii=0; ii<thermNum; ii++){
    if (therm[ii].deg[timeDiff] < -400) {    // checks that temp for this time has not already be read
      therm[ii].deg[timeDiff] = tempCKF(ii,therm[ii].unit);
      if (therm[ii].deg[timeDiff] < therm[ii].min) therm[ii].min = floor (therm[ii].deg[timeDiff]);
      if (therm[ii].deg[timeDiff] > therm[ii].max) therm[ii].max = floor (therm[ii].deg[timeDiff] + 1.0);
      therm[ii].yy[timeDiff] -= (therm[ii].deg[timeDiff] - therm[ii].min) * therm[ii].cal;  // calculate y plot position
      flag = 1;
    }
  }
/*
  Record thermometer data to log file
*/
  if (flag == 1){
    fprintf(fileTherm,"%6li ", timeDiff);
    for (ii=0; ii<thermNum;ii++){
      fprintf(fileTherm,"% 6.3f ",therm[ii].deg[timeDiff]);
    }
    fprintf(fileTherm,"\n");
    
    fflush(fileTherm);                       // make sure data saved to disk even if program crashes.
    flag = 0;
  }

  gtk_widget_queue_draw(spectra);          // redraw and update spectra widget
  //  cairo_setup = 0;  // if I'm writing then i redraw and update spectra widget
  //  spectraDraw(spectra);   // redraw and update spectra widget

  return ;
}

/***********************************************************/
void alarm_wakeup (long int i) {
  //  time_t curtime;
  //  GtkWidget *widget;
  /*
attempt to reload MTAS timers
   */

  //  tout_val.it_value.tv_sec = INTERVAL; // set timer for "INTERVAL (10) seconds 

  //  setitimer(ITIMER_REAL, &tout_val,0);   // this starts the timer which will issue the alarm signal....needed to restart

  //  signal(SIGALRM,alarm_wakeup);   // set the Alarm signal capture ... may not be needed 

  //  degC= temp_ckf(0,0);

  //  printf("degC = %.1f \n", tempCKF(0,0) );


  return;
 }


/***********************************************************/
/*********************************************************************************/
void cycleSetup0 (){
/*
   Start of cycle: (normal cycle: beam off during measurements; light Pulser on at end of beam on)
      cycle starts at 0 and end at (beamOn + lightPulser) (assumes 16 uS external trigger negligible)
new = old
old
     up   => external trigger
     down => measurement on, kicker on
     wait => 0 => minimum possible which is ~16-17 us
     up   => beam on
     down => external trigger
*/
//  uint8 cycle0[24];

  cycle0 [0] = (uint8)(0x00);   // checksum will populate
  cycle0 [1] = (uint8)(0xf8);   // fixed command for feeback
  cycle0 [2] = (uint8)(0x09);   // number of data words (number of i*2 words in the array - 3)=> 24/2-3=9
  cycle0 [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  cycle0 [4] = (uint8)(0x00);   // checksum will populate
  cycle0 [5] = (uint8)(0x00);   // checksum will populate
  cycle0 [6] = (uint8)(0x00);   // echo 0 

  cycle0 [7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle0 [8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle0 [9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  cycle0[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  cycle0[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle0[12] = trig;  //(uint8)(0x01);   // trigger up, all others down
  cycle0[13] = (uint8)(0x00);   // upper 4 bits undefined
  
  cycle0[14] = (uint8)(0x05);   // IOType #2 => 5 WaitShort
  cycle0[15] = (uint8)(0x05);   // number * 128 uS, multiplier of 0 should be 16-17 us delay

  cycle0[16] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle0[17] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle0[18] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  cycle0[19] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  cycle0[20] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle0[21] = beam;  //(uint8)(0x02);   // beam on, trigger down
  cycle0[22] = (uint8)(0x00);   // upper 4 bits undefined

  cycle0[23] = (uint8)(0x00);   // extra 0 for aligned integers

  extendedChecksum(cycle0,24);  // checksum load array elements 0, 4, and 5

  cycle4 [0] = (uint8)(0x00);   // checksum will populate
  cycle4 [1] = (uint8)(0xf8);   // fixed command for feeback
  cycle4 [2] = (uint8)(0x0d);   // number of data words (number of i*2 words in the array - 3)=> 24/2-3=9
  cycle4 [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  cycle4 [4] = (uint8)(0x00);   // checksum will populate
  cycle4 [5] = (uint8)(0x00);   // checksum will populate
  cycle4 [6] = (uint8)(0x00);   // echo 0 

  cycle4[7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle4[8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle4[9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  cycle4[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  cycle4[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle4[12] = kck + mtc;    //(uint8)(0x18);   // kicker on, tape move
  cycle4[13] = mtc1;   // upper 4 bits undefined

  cycle4[14] = (uint8)(0x06);   // IOType #4 => 6 WaitLong ... tape movement
  cycle4[15] = tapeMove;         // number * 16 ms, 23*16 = 368 ms


  cycle4 [16] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle4 [17] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle4 [18] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  cycle4[19] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  cycle4[20] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle4[21] = trig;  //(uint8)(0x01);   // trigger up, all others down
  cycle4[22] = (uint8)(0x00);   // upper 4 bits undefined
  
  cycle4[23] = (uint8)(0x05);   // IOType #2 => 5 WaitShort
  cycle4[24] = (uint8)(0x05);   // number * 128 uS, multiplier of 0 should be 16-17 us delay

  cycle4[25] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle4[26] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle4[27] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  cycle4[28] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  cycle4[29] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle4[30] = beam;  //(uint8)(0x02);   // beam on, trigger down
  cycle4[31] = (uint8)(0x00);   // upper 4 bits undefined


  extendedChecksum(cycle4,32);  // checksum load array elements 0, 4, and 5


  return;
}

/*
   Channels for each signal and hex value
     1 eio0 0x01 external trigger 
     2 eio1 0x02 beam signal for acquisition
     3 eio2 0x04 light pulser
     4 eio3 0x08 tape transport
     5 eio4 0x10 kicker 
     6 eio5 0x20 measurement 
     7 eio6 0x40 background
*/

/*********************************************************************************/
void cycleSetup1 (){
/*
   Start of cycle: (normal cycle: beam off during measurements; light Pulser on at end of beam on; starts at lightpulser)
      cycle starts at (beamOn + lightPulser) and ends at (beamOn+lightPulser + beamOff+tape Transport) 
      (assumes 16 uS for beam to go away negligible)
new = old
new
     up   => beam, light pulser (0x06)  
     down => all others
     wait => litePulser
     up   => kicker, tape transport (0x10)
     down => beam, light pulser, all others
     wait => short to get beam gone
     up   => kicker, tape transport (0x18)
     down => all others
     wait => tapeMove
     up   => kicker, measurement (0x30)
     down => all others
old
     up   => kicker (0x01)
     down => beam 
     wait => 0 => minimum possible which is ~16-17 us
     up   => light pulser, kicker (0x14)
     down => no change
     wait => ~208 ms (0x0d)
     up   => tape transport, kicker (0x18)
     down => light pulser
     wait => ~368 ms (0x17) 
     up   => measurement, kicker (0x30)
     down => tape transport

*/
//  uint8 cycle1[42];

  cycle1 [0] = (uint8)(0x00);   // checksum will populate
  cycle1 [1] = (uint8)(0xf8);   // fixed command for feeback
  cycle1 [2] = (uint8)(0x12);   // number of data words (number of i*2 words in the array - 3) 
  cycle1 [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  cycle1 [4] = (uint8)(0x00);   // checksum will populate
  cycle1 [5] = (uint8)(0x00);   // checksum will populate
  cycle1 [6] = (uint8)(0x00);   // echo 0 

  cycle1 [7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle1 [8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle1 [9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  cycle1[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  cycle1[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle1[12] = beam + lite;  //(uint8)(0x06);   // beam, light pulser
  cycle1[13] = (uint8)(0x00);   // upper 4 bits undefined
  
  cycle1[14] = (uint8)(0x06);   // IOType #2 => 6 LongShort...litePulser
  cycle1[15] = litePulser;   // number * 16 ms, multiplier of 0 should be 16-17 us delay

  cycle1[16] = (uint8)(0x1B);   // IOType #3 => 27 PortWriteState
  cycle1[17] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle1[18] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  cycle1[19] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  cycle1[20] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle1[21] = kck;          //(uint8)(0x10);   // kicker on all others off
  cycle1[22] = (uint8)(0x00);   // upper 4 bits undefined

  cycle1[23] = (uint8)(0x05);   // IOType #4 => 5 WaitShort ... time for beam to go before tape move
  cycle1[24] = (uint8)(0x00);   // number * 16 us

  cycle1[25] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle1[26] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle1[27] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  cycle1[28] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  cycle1[29] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle1[30] = kck + mtc;    //(uint8)(0x18);   // kicker on, tape move
  cycle1[31] = mtc1;   // upper 4 bits undefined
  //  cycle1[31] = (uint8)(0x00);   // upper 4 bits undefined

  cycle1[32] = (uint8)(0x06);   // IOType #4 => 6 WaitLong ... tape movement
  cycle1[33] = tapeMove;         // number * 16 ms, 23*16 = 368 ms

  cycle1[34] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle1[35] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle1[36] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  cycle1[37] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  cycle1[38] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle1[39] = kck + meas;   //(uint8)(0x30);   // kicker on, beam measurement
  cycle1[40] = (uint8)(0x00);   // upper 4 bits undefined

  cycle1[41] = (uint8)(0x00);   // extra 0 for aligned integers
  if (normal == 1){
    cycle1[39] = meas;       //(uint8)(0x20);   // beam measurement, kicker off

  }

/* old
  cycle1 [0] = (uint8)(0x00);   // checksum will populate
  cycle1 [1] = (uint8)(0xf8);   // fixed command for feeback
  cycle1 [2] = (uint8)(0x12);   // number of data words (number of i*2 words in the array - 3) 
  cycle1 [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  cycle1 [4] = (uint8)(0x00);   // checksum will populate
  cycle1 [5] = (uint8)(0x00);   // checksum will populate
  cycle1 [6] = (uint8)(0x00);   // echo 0 

  cycle1 [7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle1 [8] = (uint8)(0x00);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle1 [9] = (uint8)(0x7f);   // start of "high current" outs EIO/CIO
  cycle1[10] = (uint8)(0x00);   // upper 4 bits undefined..only 20 channels
  cycle1[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle1[12] = (uint8)(0x10);   // kicker (0x01)
  cycle1[13] = (uint8)(0x00);   // upper 4 bits undefined
  
  cycle1[14] = (uint8)(0x05);   // IOType #2 => 5 WaitShort...beam going away
  cycle1[15] = (uint8)(0x01);   // number * 128 us, multiplier of 0 should be 16-17 us delay

  cycle1[16] = (uint8)(0x1B);   // IOType #3 => 27 PortWriteState
  cycle1[17] = (uint8)(0x00);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle1[18] = (uint8)(0x7f);   // start of "high current" outs EIO/CIO  
  cycle1[19] = (uint8)(0x00);   // upper 4 bits undefined..only 20 channels
  cycle1[20] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle1[21] = (uint8)(0x14);   // light pulser, kicker (0x14)
  cycle1[22] = (uint8)(0x00);   // upper 4 bits undefined

  cycle1[23] = litePulser;      // IOType #4 => 6 WaitLong ...light pulser measurement
  cycle1[24] = (uint8)(0x0d);   // number * 16 ms, 13*16 = 208 ms

  cycle1[25] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle1[26] = (uint8)(0x00);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle1[27] = (uint8)(0x7f);   // start of "high current" outs EIO/CIO  
  cycle1[28] = (uint8)(0x00);   // upper 4 bits undefined..only 20 channels
  cycle1[29] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle1[30] = (uint8)(0x18);   // kicker on, light off, tape move
  cycle1[31] = (uint8)(0x00);   // upper 4 bits undefined

  cycle1[32] = tapeMove     ;   // IOType #4 => 6 WaitLong ... tape movement
  cycle1[33] = (uint8)(0x17);   // number * 16 ms, 23*16 = 368 ms

  cycle1[34] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle1[35] = (uint8)(0x00);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle1[36] = (uint8)(0x7f);   // start of "high current" outs EIO/CIO  
  cycle1[37] = (uint8)(0x00);   // upper 4 bits undefined..only 20 channels
  cycle1[38] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle1[39] = (uint8)(0x30);   // kicker on, light off, tape move
  cycle1[40] = (uint8)(0x00);   // upper 4 bits undefined

  cycle1[41] = (uint8)(0x00);   // extra 0 for aligned integers
*/
  extendedChecksum(cycle1,42);  // checksum load array elements 0, 4, and 5

  return;
}

/*********************************************************************************/
void cycleSetup2 (){
/*
   Start of cycle:(normal cycle: beam off during measurements; light Pulser on at end of beam on)
new = old
old:
     up   => external trigger, background (0x41)
     down => measurement on, kicker on
     wait => 0 => minimum possible which is ~16-17 us
     up   => beam on and background (0x42)
     down => external trigger
*/
//  uint8 cycle2[24];

  cycle2 [0] = (uint8)(0x00);   // checksum will populate
  cycle2 [1] = (uint8)(0xf8);   // fixed command for feeback
  cycle2 [2] = (uint8)(0x09);   // number of data words (number of i*2 words in the array - 3)=> 24/2-3=9
  cycle2 [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  cycle2 [4] = (uint8)(0x00);   // checksum will populate
  cycle2 [5] = (uint8)(0x00);   // checksum will populate
  cycle2 [6] = (uint8)(0x00);   // echo 0 

  cycle2 [7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle2 [8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle2 [9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  cycle2[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  cycle2[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle2[12] = back + trig + kck; //(uint8)(0x51);   // trigger, kicker, and background up, all others down
  cycle2[13] = (uint8)(0x00);   // upper 4 bits undefined
  
  cycle2[14] = (uint8)(0x05);   // IOType #2 => 5 WaitShort
  cycle2[15] = (uint8)(0x00);   // number * 128 uS, multiplier of 0 should be 16-17 us delay

  cycle2[16] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle2[17] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle2[18] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  cycle2[19] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  cycle2[20] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle2[21] = back +beam + kck;  //(uint8)(0x52);   // beam on, kicker on, background , trigger down 
  cycle2[22] = (uint8)(0x00);   // upper 4 bits undefined

  cycle2[23] = (uint8)(0x00);   // extra 0 for aligned integers

  extendedChecksum(cycle2,24);  // checksum load array elements 0, 4, and 5


  cycle6 [0] = (uint8)(0x00);   // checksum will populate
  cycle6 [1] = (uint8)(0xf8);   // fixed command for feeback
  cycle6 [2] = (uint8)(0x0d);   // number of data words (number of i*2 words in the array - 3)=> 24/2-3=9
  cycle6 [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  cycle6 [4] = (uint8)(0x00);   // checksum will populate
  cycle6 [5] = (uint8)(0x00);   // checksum will populate
  cycle6 [6] = (uint8)(0x00);   // echo 0 

  cycle6[7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle6[8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle6[9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  cycle6[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  cycle6[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle6[12] = back + mtc + kck;    //(uint8)(0x18);   // kicker on, tape move
  cycle6[13] = mtc1;   // upper 4 bits undefined

  cycle6[14] = (uint8)(0x06);   // IOType #4 => 6 WaitLong ... tape movement
  cycle6[15] = tapeMove;         // number * 16 ms, 23*16 = 368 ms


  cycle6 [16] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle6 [17] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle6 [18] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  cycle6[19] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  cycle6[20] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle6[21] = back + trig + kck;  //(uint8)(0x01);   // trigger up, all others down
  cycle6[22] = (uint8)(0x00);   // upper 4 bits undefined
  
  cycle6[23] = (uint8)(0x05);   // IOType #2 => 5 WaitShort
  cycle6[24] = (uint8)(0x05);   // number * 128 uS, multiplier of 0 should be 16-17 us delay

  cycle6[25] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle6[26] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle6[27] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  cycle6[28] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  cycle6[29] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle6[30] = back + beam + kck;  //(uint8)(0x02);   // beam on, trigger down
  cycle6[31] = (uint8)(0x00);   // upper 4 bits undefined


  extendedChecksum(cycle6,32);  // checksum load array elements 0, 4, and 5


  return;
}

/*********************************************************************************/
void cycleSetup3 (){
/*
   Start of cycle:(normal cycle: beam off during measurements; light Pulser on at end of beam on; starts at lightpulser)
new
     up   => beam, light pulser (0x06)  
     down => all others
     wait => litePulser
     up   => kicker, tape transport (0x18)
     down => beam, light pulser, all others
     wait => tapeMove
     up   => kicker, measurement (0x30)

old
     up   => kicker, background (0x50)
     down => beam 
     wait => 0 => minimum possible which is ~16-17 us
     up   => light pulser, kicker, background (0x54)
     down => no change
     wait => ~200 ms
     up   => tape transport, kicker, background (0x58)
     down => light pulser
     wait => ~350 ms
     up   => measurement, kicker, background (0x70)
     down => tape transport

*/
//  uint8 cycle3[42];

  cycle3 [0] = (uint8)(0x00);   // checksum will populate
  cycle3 [1] = (uint8)(0xf8);   // fixed command for feeback
  cycle3 [2] = (uint8)(0x12);   // number of data words (number of i*2 words in the array - 3) 
  cycle3 [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  cycle3 [4] = (uint8)(0x00);   // checksum will populate
  cycle3 [5] = (uint8)(0x00);   // checksum will populate
  cycle3 [6] = (uint8)(0x00);   // echo 0 

  cycle3 [7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle3 [8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle3 [9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  cycle3[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  cycle3[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle3[12] = back + lite +beam + kck; //(uint8)(0x56);   // beam, light pulser, kicker, background
  cycle3[13] = (uint8)(0x00);   // upper 4 bits undefined
  
  cycle3[14] = (uint8)(0x06);   // IOType #2 => 6 LongShort...litePulser
  cycle3[15] = litePulser   ;   // number * 16 ms, multiplier of 0 should be 16-17 us delay

  cycle3[16] = (uint8)(0x1B);   // IOType #3 => 27 PortWriteState
  cycle3[17] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle3[18] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  cycle3[19] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  cycle3[20] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle3[21] = back + kck; //(uint8)(0x50);   // kicker and background on all others off
  cycle3[22] = (uint8)(0x00);   // upper 4 bits undefined

  cycle3[23] = (uint8)(0x05);   // IOType #4 => 5 WaitShort ... time for beam to go before tape move
  cycle3[24] = (uint8)(0x00);   // number * 16 us

  cycle3[25] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle3[26] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle3[27] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  cycle3[28] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  cycle3[29] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle3[30] = back + mtc + kck; //(uint8)(0x58);   // kicker, tape move, background
  cycle3[31] = mtc1;           // upper 4 bits undefined
  //  cycle3[31] = (uint8)(0x00);   // upper 4 bits undefined

  cycle3[32] = (uint8)(0x06);   // IOType #4 => 6 WaitLong ... tape movement
  cycle3[33] = tapeMove;         // number * 16 ms, 23*16 = 368 ms

  cycle3[34] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle3[35] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle3[36] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  cycle3[37] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  cycle3[38] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle3[39] = back + meas + kck; //(uint8)(0x70);   // kicker on, beam measurement, background
  cycle3[40] = (uint8)(0x00);   // upper 4 bits undefined

  cycle3[41] = (uint8)(0x00);   // extra 0 for aligned integers
  if (normal == 1){
    cycle3[39] = back + meas + kck; //(uint8)(0x70);   // background, beam measurement, kicker off

  }


/*
  cycle3 [0] = (uint8)(0x00);   // checksum will populate
  cycle3 [1] = (uint8)(0xf8);   // fixed command for feeback
  cycle3 [2] = (uint8)(0x12);   // number of data words (number of i*2 words in the array - 3) 
  cycle3 [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  cycle3 [4] = (uint8)(0x00);   // checksum will populate
  cycle3 [5] = (uint8)(0x00);   // checksum will populate
  cycle3 [6] = (uint8)(0x00);   // echo 0 

  cycle3 [7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle3 [8] = (uint8)(0x00);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle3 [9] = (uint8)(0x7f);   // start of "high current" outs EIO/CIO
  cycle3[10] = (uint8)(0x00);   // upper 4 bits undefined..only 20 channels
  cycle3[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle3[12] = (uint8)(0x50);   // trigger and background up, all others down
  cycle3[13] = (uint8)(0x00);   // upper 4 bits undefined
  
  cycle3[14] = (uint8)(0x05);   // IOType #2 => 5 WaitShort...beam going away
  cycle3[15] = (uint8)(0x01);   // number * 128 us, multiplier of 0 should be 16-17 us delay

  cycle3[16] = (uint8)(0x1B);   // IOType #3 => 27 PortWriteState
  cycle3[17] = (uint8)(0x00);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle3[18] = (uint8)(0x7f);   // start of "high current" outs EIO/CIO  
  cycle3[19] = (uint8)(0x00);   // upper 4 bits undefined..only 20 channels
  cycle3[20] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle3[21] = (uint8)(0x54);   // beam on, trigger down
  cycle3[22] = (uint8)(0x00);   // upper 4 bits undefined

  cycle3[23] = litePulser;      // IOType #4 => 6 WaitLong ...light pulser measurement
  cycle3[24] = (uint8)(0x0d);   // number * 16 ms, 13*16 = 208 ms

  cycle3[25] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle3[26] = (uint8)(0x00);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle3[27] = (uint8)(0x7f);   // start of "high current" outs EIO/CIO  
  cycle3[28] = (uint8)(0x00);   // upper 4 bits undefined..only 20 channels
  cycle3[29] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle3[30] = (uint8)(0x58);   // kicker on, light off, tape move
  cycle3[31] = (uint8)(0x00);   // upper 4 bits undefined

  cycle3[32] = tapeMove     ;   // IOType #4 => 6 WaitLong ... tape movement
  cycle3[33] = (uint8)(0x17);   // number * 16 ms, 23*16 = 368 ms

  cycle3[34] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  cycle3[35] = (uint8)(0x00);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  cycle3[36] = (uint8)(0x7f);   // start of "high current" outs EIO/CIO  
  cycle3[37] = (uint8)(0x00);   // upper 4 bits undefined..only 20 channels
  cycle3[38] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  cycle3[39] = (uint8)(0x70);   // kicker, background, measurement on, light off, tape move
  cycle3[40] = (uint8)(0x00);   // upper 4 bits undefined

  cycle3[41] = (uint8)(0x00);   // extra 0 for aligned integers
*/
  extendedChecksum(cycle3,42);  // checksum load array elements 0, 4, and 5

  return;
}

/*********************************************************************************/
void dioOff (){
/*
   Start of cycle:
     up   => external trigger, background (0x41)
     down => measurement on, kicker on
     wait => 0 => minimum possible which is ~16-17 us
     up   => beam on and background (0x42)
     down => external trigger
*/
//  uint8 off[24];

  off [0] = (uint8)(0x00);   // checksum will populate
  off [1] = (uint8)(0xf8);   // fixed command for feeback
  off [2] = (uint8)(0x04);   // number of data words (number of i*2 words in the array - 3)=> 24/2-3=9
  off [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  off [4] = (uint8)(0x00);   // checksum will populate
  off [5] = (uint8)(0x00);   // checksum will populate
  off [6] = (uint8)(0x00);   // echo 0 

  off [7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  off [8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  off [9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  off[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  off[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  off[12] = (uint8)(0x00);   // trigger and background up, all others down
  off[13] = (uint8)(0x00);   // upper 4 bits undefined
  
  extendedChecksum(off,14);  // checksum load array elements 0, 4, and 5

  return;
}

/*********************************************************************************/
void mtcOn (){
/*
   Start of cycle:
     up   => external trigger, background (0x41)
     down => measurement on, kicker on
     wait => 0 => minimum possible which is ~16-17 us
     up   => beam on and background (0x42)
     down => external trigger
*/
//  uint8 off[24];
/*
  trig = 0x01;
  beam = 0x02;
  lite = 0x04;
  mtc  = 0x08;
  kck  = 0x10;
  meas = 0x20;
  back = 0x40;
*/

  onMTC[0] = (uint8)(0x00);   // checksum will populate
  onMTC[1] = (uint8)(0xf8);   // fixed command for feeback
  onMTC[2] = (uint8)(0x09);   // number of data words (number of i*2 words in the array - 3)=> 24/2-3=9
  onMTC[3] = (uint8)(0x00);   // extended command number ... for feedback =0
  onMTC[4] = (uint8)(0x00);   // checksum will populate
  onMTC[5] = (uint8)(0x00);   // checksum will populate
  onMTC[6] = (uint8)(0x00);   // echo 0 

  onMTC[7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  onMTC[8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  onMTC[9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  onMTC[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  onMTC[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  onMTC[12] = mtc + kck;             // mtc up, everything else down
  onMTC[13] = mtc1;   // upper 4 bits undefined

  onMTC[14] = (uint8)(0x06);   // IOType #4 => 6 WaitLong ... tape movement
  onMTC[15] = tapeMove;         // number * 16 ms, 23*16 = 368 ms

  onMTC[16] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  onMTC[17] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  onMTC[18] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  onMTC[19] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  onMTC[20] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  onMTC[21] = kck;             // everything off after single tape move
  onMTC[22] = (uint8)(0x00);   // upper 4 bits undefined
  onMTC[23] = (uint8)(0x00);  // extra 0's to complete word

  extendedChecksum(onMTC,24);  // checksum load array elements 0, 4, and 5

  return;
}
/*********************************************************************************/
void kickOn (){
/*
   Start of cycle:
     up   => external trigger, background (0x41)
     down => measurement on, kicker on
     wait => 0 => minimum possible which is ~16-17 us
     up   => beam on and background (0x42)
     down => external trigger
*/
//  uint8 off[24];
/*
  trig = 0x01;
  beam = 0x02;
  lite = 0x04;
  mtc  = 0x08;
  kck  = 0x10;
  meas = 0x20;
  back = 0x40;
*/

  kick [0] = (uint8)(0x00);   // checksum will populate
  kick [1] = (uint8)(0xf8);   // fixed command for feeback
  kick [2] = (uint8)(0x04);   // number of data words (number of i*2 words in the array - 3)=> 24/2-3=9
  kick [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  kick [4] = (uint8)(0x00);   // checksum will populate
  kick [5] = (uint8)(0x00);   // checksum will populate
  kick [6] = (uint8)(0x00);   // echo 0 

  kick [7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  kick [8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  kick [9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  kick[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  kick[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  kick[12] = kck;              // kicker up, everything else down
  kick[13] = (uint8)(0x00);   // upper 4 bits undefined
  
  extendedChecksum(kick,14);  // checksum load array elements 0, 4, and 5

  return;
}
/*********************************************************************************/
void cycleSetup4 (){
/*
   Start of cycle: (take-away cycle: light Pulser on at end of beam off; starts at lightpulser)
      cycle starts at 0 and end at (beamOn + lightPulser) (assumes 16 uS external trigger negligible)
*/

  away0 [0] = (uint8)(0x00);   // checksum will populate
  away0 [1] = (uint8)(0xf8);   // fixed command for feeback
  away0 [2] = (uint8)(0x12);   // number of data words (number of i*2 words in the array - 3)=> 21-3=18 = 0x12
  away0 [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  away0 [4] = (uint8)(0x00);   // checksum will populate
  away0 [5] = (uint8)(0x00);   // checksum will populate
  away0 [6] = (uint8)(0x00);   // echo 0 

  away0 [7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  away0 [8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  away0 [9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  away0[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  away0[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  away0[12] = kck + lite;  //(uint8)(0x14);   // kicker, light pulser
  away0[13] = (uint8)(0x00);   // upper 4 bits undefined
  
  away0[14] = (uint8)(0x06);   // IOType #2 => 6 LongShort...litePulser
  away0[15] = litePulser;      // number * 16 ms, multiplier of 0 should be 16-17 us delay

  away0[16] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  away0[17] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  away0[18] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  away0[19] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  away0[20] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  away0[21] = kck + mtc; //(uint8)(0x18);   // kicker on, tape on, all others down
  away0[22] = mtc1;   // upper 4 bits undefined

  away0[23] = (uint8)(0x06);   // IOType #4 => 6 WaitLong ... tape movement
  away0[24] = tapeMove;         // number * 16 ms, 23*16 = 368 ms

  away0[25] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  away0[26] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  away0[27] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  away0[28] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  away0[29] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  away0[30] = trig;   //(uint8)(0x01);   // trigger up, all others down
  away0[31] = (uint8)(0x00);   // upper 4 bits undefined
  
  away0[32] = (uint8)(0x05);   // IOType #2 => 5 WaitShort
  away0[33] = (uint8)(0x00);   // number * 128 uS, multiplier of 0 should be 16-17 us delay

  away0[34] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  away0[35] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  away0[36] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  away0[37] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  away0[38] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  away0[39] = meas + beam;  //(uint8)(0x22);   // beam on, measurement on, all others down  ... grow in cycle
  away0[40] = (uint8)(0x00);   // upper 4 bits undefined

  away0[41] = (uint8)(0x00);   // extra 0's to complete word

  extendedChecksum(away0,42);  // checksum load array elements 0, 4, and 5

  return;
}
/*
   Channels for each signal and hex value
     1 eio0 0x01 external trigger 
     2 eio1 0x02 beam signal for acquisition
     3 eio2 0x04 light pulser
     4 eio3 0x08 tape transport
     5 eio4 0x10 kicker 
     6 eio5 0x20 measurement 
     7 eio6 0x40 background
*/
/******************************************************************************/

void cycleSetup5 (){
/*
   Start of cycle: (take-away cycle: light Pulser on at end of beam off; starts at lightpulser)
      cycle starts at (beamOn + lightPulser) and ends at (beamOn+lightPulser + beamOff+tape Transport) 
      (assumes 16 uS for beam to go away negligible)
new = old
new
     up   => beam, light pulser (0x06)  
     down => all others
     wait => litePulser
     up   => kicker, tape transport (0x10)
     down => beam, light pulser, all others
     wait => short to get beam gone
     up   => kicker, tape transport (0x18)
     down => all others
     wait => tapeMove
     up   => kicker, measurement (0x30)
     down => all others
*/
  away1 [0] = (uint8)(0x00);   // checksum will populate
  away1 [1] = (uint8)(0xf8);   // fixed command for feeback
  away1 [2] = (uint8)(0x09);   // number of data words (number of i*2 words in the array - 3) 
  away1 [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  away1 [4] = (uint8)(0x00);   // checksum will populate
  away1 [5] = (uint8)(0x00);   // checksum will populate
  away1 [6] = (uint8)(0x00);   // echo 0 

  away1 [7] = (uint8)(0x1B);   // IOType #3 => 27 PortWriteState
  away1 [8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  away1 [9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  away1[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  away1[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  away1[12] = meas + kck;   //(uint8)(0x30);   // kicker and measurement on, all others off
  away1[13] = (uint8)(0x00);   // upper 4 bits undefined

  away1[14] = (uint8)(0x05);   // IOType #4 => 5 WaitShort ... time for beam to go before decay out
  away1[15] = (uint8)(0x00);   // number * 16 us

  away1[16] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  away1[17] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  away1[18] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  away1[19] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  away1[20] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  away1[21] = meas + kck;   //(uint8)(0x30);   // measurement on, kicker on
  away1[22] = (uint8)(0x00);   // upper 4 bits undefined

  away1[23] = (uint8)(0x00);   // added 0's to finish word

  extendedChecksum(away1,24);  // checksum load array elements 0, 4, and 5

  return;
}
/*********************************************************************************/
void cycleSetup6 (){
/*
   Start of cycle: (take-away background cycle: light Pulser on at end of beam off; starts at lightpulser)
      cycle starts at 0 and end at (beamOn + lightPulser) (assumes 16 uS external trigger negligible)
*/
  away2 [0] = (uint8)(0x00);   // checksum will populate
  away2 [1] = (uint8)(0xf8);   // fixed command for feeback
  away2 [2] = (uint8)(0x12);   // number of data words (number of i*2 words in the array - 3)=> 21-3=18 = 0x12
  away2 [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  away2 [4] = (uint8)(0x00);   // checksum will populate
  away2 [5] = (uint8)(0x00);   // checksum will populate
  away2 [6] = (uint8)(0x00);   // echo 0 

  away2 [7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  away2 [8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  away2 [9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  away2[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  away2[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  away2[12] = kck + lite;   //(uint8)(0x54);   // kicker, light pulser
  away2[13] = (uint8)(0x00);   // upper 4 bits undefined
  
  away2[14] = (uint8)(0x06);   // IOType #2 => 6 LongShort...litePulser
  away2[15] = litePulser;      // number * 16 ms, multiplier of 0 should be 16-17 us delay

  away2[16] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  away2[17] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  away2[18] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  away2[19] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  away2[20] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  away2[21] = kck + mtc;   //(uint8)(0x58);   // kicker on, tape on, all others down
  away2[22] = mtc1;   // upper 4 bits undefined

  away2[23] = (uint8)(0x06);   // IOType #4 => 6 WaitLong ... tape movement
  away2[24] = tapeMove;         // number * 16 ms, 23*16 = 368 ms

  away2[25] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  away2[26] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  away2[27] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  away2[28] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  away2[29] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  away2[30] = back + kck + trig;            //(uint8)(0x51);   // trigger up with kicker and background up, all others down
  away2[31] = (uint8)(0x00);   // upper 4 bits undefined
  
  away2[32] = (uint8)(0x05);   // IOType #2 => 5 WaitShort
  away2[33] = (uint8)(0x00);   // number * 128 uS, multiplier of 0 should be 16-17 us delay

  away2[34] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  away2[35] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  away2[36] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  away2[37] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  away2[38] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  away2[39] = back + meas + kck + beam;  //(uint8)(0x72);   // beam on, measurement on with kicker on and background, all others down  ... grow in cycle (background)
  away2[40] = (uint8)(0x00);   // upper 4 bits undefined

  away2[41] = (uint8)(0x00);   // extra 0's to complete word

  extendedChecksum(away2,42);  // checksum load array elements 0, 4, and 5

  return;
}
/*
   Channels for each signal and hex value
     1 eio0 0x01 external trigger 
     2 eio1 0x02 beam signal for acquisition
     3 eio2 0x04 light pulser
     4 eio3 0x08 tape transport
     5 eio4 0x10 kicker 
     6 eio5 0x20 measurement 
     7 eio6 0x40 background
*/
/******************************************************************************/

void cycleSetup7 (){
/*
   Start of cycle: (take-away background cycle: light Pulser on at end of beam off; starts at lightpulser)
      cycle starts at (beamOn + lightPulser) and ends at (beamOn+lightPulser + beamOff+tape Transport) 
      (assumes 16 uS for beam to go away negligible)
new = old
new
     up   => beam, light pulser (0x06)  
     down => all others
     wait => litePulser
     up   => kicker, tape transport (0x10)
     down => beam, light pulser, all others
     wait => short to get beam gone
     up   => kicker, tape transport (0x18)
     down => all others
     wait => tapeMove
     up   => kicker, measurement (0x30)
     down => all others
*/
  away3 [0] = (uint8)(0x00);   // checksum will populate
  away3 [1] = (uint8)(0xf8);   // fixed command for feeback
  away3 [2] = (uint8)(0x09);   // number of data words (number of i*2 words in the array - 3) 
  away3 [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  away3 [4] = (uint8)(0x00);   // checksum will populate
  away3 [5] = (uint8)(0x00);   // checksum will populate
  away3 [6] = (uint8)(0x00);   // echo 0 

  away3 [7] = (uint8)(0x1B);   // IOType #3 => 27 PortWriteState
  away3 [8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  away3 [9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  away3[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  away3[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  away3[12] = back + kck;  //(uint8)(0x50);   // kicker, background on, all others off
  away3[13] = (uint8)(0x00);   // upper 4 bits undefined

  away3[14] = (uint8)(0x05);   // IOType #4 => 5 WaitShort ... time for beam to go before decay out
  away3[15] = (uint8)(0x00);   // number * 16 us

  away3[16] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  away3[17] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  away3[18] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  away3[19] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  away3[20] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  away3[21] = back + meas + kck;  //(uint8)(0x70);   // measurement on, kicker on, background on
  away3[22] = (uint8)(0x00);   // upper 4 bits undefined

  away3[23] = (uint8)(0x00);   // added 0's to finish word

  extendedChecksum(away3,24);  // checksum load array elements 0, 4, and 5

  return;
}
/*********************************************************************************/
void pauseSetup0 (){
/*
   Start of cycle:
     up   => external trigger, background (0x41)
     down => measurement on, kicker on
     wait => 0 => minimum possible which is ~16-17 us
     up   => beam on and background (0x42)
     down => external trigger
*/
//  uint8 off[24];
/*
  trig = 0x01;
  beam = 0x02;
  lite = 0x04;
  mtc  = 0x08;
  kck  = 0x10;
  meas = 0x20;
  back = 0x40;
*/

  pause0 [0] = (uint8)(0x00);   // checksum will populate
  pause0 [1] = (uint8)(0xf8);   // fixed command for feeback
  pause0 [2] = (uint8)(0x09);   // number of data words (number of i*2 words in the array - 3)=> 24/2-3=9
  pause0 [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  pause0 [4] = (uint8)(0x00);   // checksum will populate
  pause0 [5] = (uint8)(0x00);   // checksum will populate
  pause0 [6] = (uint8)(0x00);   // echo 0 

  pause0 [7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  pause0 [8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  pause0 [9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  pause0[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  pause0[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  pause0[12] = trig;            //(uint8)(0x01);   // trigger up, all others down
  pause0[13] = (uint8)(0x00);   // upper 4 bits undefined
  
  pause0[14] = (uint8)(0x05);   // IOType #2 => 5 WaitShort
  pause0[15] = (uint8)(0x05);   // number * 128 uS, multiplier of 0 should be 16-17 us delay

  pause0[16] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  pause0[17] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  pause0[18] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  pause0[19] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  pause0[20] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  pause0[21] = beam;  //(uint8)(0x02);   // beam on, trigger down
  pause0[22] = (uint8)(0x00);   // upper 4 bits undefined

  pause0[23] = (uint8)(0x00);   // extra 0 for aligned integers

  extendedChecksum(pause0,24);  // checksum load array elements 0, 4, and 5

  // double tape move

  pause6 [0] = (uint8)(0x00);   // checksum will populate
  pause6 [1] = (uint8)(0xf8);   // fixed command for feeback
  pause6 [2] = (uint8)(0x0d);   // number of data words (number of i*2 words in the array - 3)=> 24/2-3=9
  pause6 [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  pause6 [4] = (uint8)(0x00);   // checksum will populate
  pause6 [5] = (uint8)(0x00);   // checksum will populate
  pause6 [6] = (uint8)(0x00);   // echo 0 

  pause6[7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  pause6[8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  pause6[9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  pause6[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  pause6[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  pause6[12] = kck + mtc;    //(uint8)(0x18);   // kicker on, tape move
  pause6[13] = mtc1;   // upper 4 bits undefined

  pause6[14] = (uint8)(0x06);   // IOType #4 => 6 WaitLong ... tape movement
  pause6[15] = tapeMove;         // number * 16 ms, 23*16 = 368 ms


  pause6 [16] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  pause6 [17] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  pause6 [18] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  pause6[19] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  pause6[20] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  pause6[21] = trig;  //(uint8)(0x01);   // trigger up, all others down
  pause6[22] = (uint8)(0x00);   // upper 4 bits undefined
  
  pause6[23] = (uint8)(0x05);   // IOType #2 => 5 WaitShort
  pause6[24] = (uint8)(0x05);   // number * 128 uS, multiplier of 0 should be 16-17 us delay

  pause6[25] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  pause6[26] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  pause6[27] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  pause6[28] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  pause6[29] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  pause6[30] = beam;  //(uint8)(0x02);   // beam on, trigger down
  pause6[31] = (uint8)(0x00);   // upper 4 bits undefined

  extendedChecksum(pause6,32);  // checksum load array elements 0, 4, and 5

  return;
}
/*********************************************************************************/
void pauseSetup1 (){
/*
   Start of cycle:
     up   => external trigger, background (0x41)
     down => measurement on, kicker on
     wait => 0 => minimum possible which is ~16-17 us
     up   => beam on and background (0x42)
     down => external trigger
*/
//  uint8 off[24];
/*
  trig = 0x01;
  beam = 0x02;
  lite = 0x04;
  mtc  = 0x08;
  kck  = 0x10;
  meas = 0x20;
  back = 0x40;
*/
  pause1 [0] = (uint8)(0x00);   // checksum will populate
  pause1 [1] = (uint8)(0xf8);   // fixed command for feeback
  pause1 [2] = (uint8)(0x04);   // number of data words (number of i*2 words in the array - 3)=> 24/2-3=9
  pause1 [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  pause1 [4] = (uint8)(0x00);   // checksum will populate
  pause1 [5] = (uint8)(0x00);   // checksum will populate
  pause1 [6] = (uint8)(0x00);   // echo 0 

  pause1[7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  pause1[8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  pause1[9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  pause1[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  pause1[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  pause1[12] = kck + lite;             //(uint8)(0x18);   // kicker on, tape move
  pause1[13] = (uint8)(0x00);   // upper 4 bits undefined

// lite is borrowed here to indicate to pixie that kck is set
   
  extendedChecksum(pause1,14);  // checksum load array elements 0, 4, and 5

  return;
}
/*********************************************************************************/
void pauseSetup2 (){
/*
   Start of cycle:
     up   => external trigger, background (0x41)
     down => measurement on, kicker on
     wait => 0 => minimum possible which is ~16-17 us
     up   => beam on and background (0x42)
     down => external trigger
*/
//  uint8 off[24];
/*
  trig = 0x01;
  beam = 0x02;
  lite = 0x04;
  mtc  = 0x08;
  kck  = 0x10;
  meas = 0x20;
  back = 0x40;
*/

  pause2 [0] = (uint8)(0x00);   // checksum will populate
  pause2 [1] = (uint8)(0xf8);   // fixed command for feeback
  pause2 [2] = (uint8)(0x12);   // number of data words (number of i*2 words in the array - 3) 
  pause2 [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  pause2 [4] = (uint8)(0x00);   // checksum will populate
  pause2 [5] = (uint8)(0x00);   // checksum will populate
  pause2 [6] = (uint8)(0x00);   // echo 0 

  pause2 [7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  pause2 [8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  pause2 [9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  pause2[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  pause2[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  pause2[12] = kck + lite;      //(uint8)(0x06);   // kck, light pulser
  pause2[13] = (uint8)(0x00);   // upper 4 bits undefined
  
  pause2[14] = (uint8)(0x06);   // IOType #2 => 6 LongShort...litePulser
  pause2[15] = litePulser;   // number * 16 ms, multiplier of 0 should be 16-17 us delay

  pause2[16] = (uint8)(0x1B);   // IOType #3 => 27 PortWriteState
  pause2[17] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  pause2[18] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  pause2[19] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  pause2[20] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  pause2[21] = kck;          //(uint8)(0x10);   // kicker on all others off
  pause2[22] = (uint8)(0x00);   // upper 4 bits undefined

  pause2[23] = (uint8)(0x05);   // IOType #4 => 5 WaitShort ... time for beam to go before tape move
  pause2[24] = (uint8)(0x00);   // number * 16 us

  pause2[25] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  pause2[26] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  pause2[27] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  pause2[28] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  pause2[29] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  pause2[30] = kck + mtc;    //(uint8)(0x18);   // kicker on, tape move
  pause2[31] = mtc1;   // upper 4 bits undefined
  //  pause2[31] = (uint8)(0x00);   // upper 4 bits undefined

  pause2[32] = (uint8)(0x06);   // IOType #4 => 6 WaitLong ... tape movement
  pause2[33] = tapeMove;         // number * 16 ms, 23*16 = 368 ms

  pause2[34] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  pause2[35] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  pause2[36] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  pause2[37] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  pause2[38] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  pause2[39] = kck + meas;   //(uint8)(0x30);   // kicker on, beam measurement
  pause2[40] = (uint8)(0x00);   // upper 4 bits undefined

  pause2[41] = (uint8)(0x00);   // extra 0 for aligned integers
  if (normal == 1){
    pause2[39] = meas;       //(uint8)(0x20);   // beam measurement, kicker off

  }
 
  extendedChecksum(pause2,42);  // checksum load array elements 0, 4, and 5

  return;
}
/*********************************************************************************/
void pauseSetup3 (){
/*
   Start of cycle:
     up   => external trigger, background (0x41)
     down => measurement on, kicker on
     wait => 0 => minimum possible which is ~16-17 us
     up   => beam on and background (0x42)
     down => external trigger
*/
//  uint8 off[24];
/*
  trig = 0x01;
  beam = 0x02;
  lite = 0x04;
  mtc  = 0x08;
  kck  = 0x10;
  meas = 0x20;
  back = 0x40;
*/

  pause3 [0] = (uint8)(0x00);   // checksum will populate
  pause3 [1] = (uint8)(0xf8);   // fixed command for feeback
  pause3 [2] = (uint8)(0x09);   // number of data words (number of i*2 words in the array - 3)=> 24/2-3=9
  pause3 [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  pause3 [4] = (uint8)(0x00);   // checksum will populate
  pause3 [5] = (uint8)(0x00);   // checksum will populate
  pause3 [6] = (uint8)(0x00);   // echo 0 

  pause3 [7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  pause3 [8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  pause3 [9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  pause3[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  pause3[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  pause3[12] = back + kck + trig;            //(uint8)(0x01);   // trigger up, all others down
  pause3[13] = (uint8)(0x00);   // upper 4 bits undefined
  
  pause3[14] = (uint8)(0x05);   // IOType #2 => 5 WaitShort
  pause3[15] = (uint8)(0x05);   // number * 128 uS, multiplier of 0 should be 16-17 us delay

  pause3[16] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  pause3[17] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  pause3[18] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  pause3[19] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  pause3[20] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  pause3[21] = back + kck + beam;  //(uint8)(0x02);   // beam on, trigger down
  pause3[22] = (uint8)(0x00);   // upper 4 bits undefined

  pause3[23] = (uint8)(0x00);   // extra 0 for aligned integers

  extendedChecksum(pause3,24);  // checksum load array elements 0, 4, and 5

  // double tape move

  pause9 [0] = (uint8)(0x00);   // checksum will populate
  pause9 [1] = (uint8)(0xf8);   // fixed command for feeback
  pause9 [2] = (uint8)(0x0d);   // number of data words (number of i*2 words in the array - 3)=> 24/2-3=9
  pause9 [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  pause9 [4] = (uint8)(0x00);   // checksum will populate
  pause9 [5] = (uint8)(0x00);   // checksum will populate
  pause9 [6] = (uint8)(0x00);   // echo 0 

  pause9[7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  pause9[8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  pause9[9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  pause9[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  pause9[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  pause9[12] = back + kck + mtc;    //(uint8)(0x18);   // kicker on, tape move
  pause9[13] = mtc1;   // upper 4 bits undefined

  pause9[14] = (uint8)(0x06);   // IOType #4 => 6 WaitLong ... tape movement
  pause9[15] = tapeMove;         // number * 16 ms, 23*16 = 368 ms


  pause9 [16] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  pause9 [17] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  pause9 [18] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  pause9[19] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  pause9[20] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  pause9[21] = back + kck + trig;  //(uint8)(0x01);   // trigger up, all others down
  pause9[22] = (uint8)(0x00);   // upper 4 bits undefined
  
  pause9[23] = (uint8)(0x05);   // IOType #2 => 5 WaitShort
  pause9[24] = (uint8)(0x05);   // number * 128 uS, multiplier of 0 should be 16-17 us delay

  pause9[25] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  pause9[26] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  pause9[27] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  pause9[28] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  pause9[29] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  pause9[30] = back + kck + beam;  //(uint8)(0x02);   // beam on, trigger down
  pause9[31] = (uint8)(0x00);   // upper 4 bits undefined

  extendedChecksum(pause9,32);  // checksum load array elements 0, 4, and 5

  return;
}


/*********************************************************************************/
void pauseSetup4 (){
/*
   Start of cycle:
     up   => external trigger, background (0x41)
     down => measurement on, kicker on
     wait => 0 => minimum possible which is ~16-17 us
     up   => beam on and background (0x42)
     down => external trigger
*/
//  uint8 off[24];
/*
  trig = 0x01;
  beam = 0x02;
  lite = 0x04;
  mtc  = 0x08;
  kck  = 0x10;
  meas = 0x20;
  back = 0x40;
*/
  pause4 [0] = (uint8)(0x00);   // checksum will populate
  pause4 [1] = (uint8)(0xf8);   // fixed command for feeback
  pause4 [2] = (uint8)(0x04);   // number of data words (number of i*2 words in the array - 3)=> 24/2-3=9
  pause4 [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  pause4 [4] = (uint8)(0x00);   // checksum will populate
  pause4 [5] = (uint8)(0x00);   // checksum will populate
  pause4 [6] = (uint8)(0x00);   // echo 0 

  pause4[7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  pause4[8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  pause4[9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  pause4[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  pause4[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  pause4[12] = back + kck + lite;             //(uint8)(0x18);   // kicker on, tape move
  pause4[13] = (uint8)(0x00);   // upper 4 bits undefined

// lite is borrowed here to indicate to pixie that kck is set
   
  extendedChecksum(pause4,14);  // checksum load array elements 0, 4, and 5

  return;
}
/*********************************************************************************/
void pauseSetup5 (){
/*
   Start of cycle:
     up   => external trigger, background (0x41)
     down => measurement on, kicker on
     wait => 0 => minimum possible which is ~16-17 us
     up   => beam on and background (0x42)
     down => external trigger
*/
//  uint8 off[24];
/*
  trig = 0x01;
  beam = 0x02;
  lite = 0x04;
  mtc  = 0x08;
  kck  = 0x10;
  meas = 0x20;
  back = 0x40;
*/

  pause5 [0] = (uint8)(0x00);   // checksum will populate
  pause5 [1] = (uint8)(0xf8);   // fixed command for feeback
  pause5 [2] = (uint8)(0x12);   // number of data words (number of i*2 words in the array - 3) 
  pause5 [3] = (uint8)(0x00);   // extended command number ... for feedback =0
  pause5 [4] = (uint8)(0x00);   // checksum will populate
  pause5 [5] = (uint8)(0x00);   // checksum will populate
  pause5 [6] = (uint8)(0x00);   // echo 0 

  pause5 [7] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  pause5 [8] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  pause5 [9] = (uint8)(0xff);   // start of "high current" outs EIO/CIO
  pause5[10] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  pause5[11] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  pause5[12] = back + kck + lite;      //(uint8)(0x06);   // kck, light pulser
  pause5[13] = (uint8)(0x00);   // upper 4 bits undefined
  
  pause5[14] = (uint8)(0x06);   // IOType #2 => 6 LongShort...litePulser
  pause5[15] = litePulser;   // number * 16 ms, multiplier of 0 should be 16-17 us delay

  pause5[16] = (uint8)(0x1B);   // IOType #3 => 27 PortWriteState
  pause5[17] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  pause5[18] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  pause5[19] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  pause5[20] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  pause5[21] = back + kck;          //(uint8)(0x10);   // kicker on all others off
  pause5[22] = (uint8)(0x00);   // upper 4 bits undefined

  pause5[23] = (uint8)(0x05);   // IOType #4 => 5 WaitShort ... time for beam to go before tape move
  pause5[24] = (uint8)(0x00);   // number * 16 us

  pause5[25] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  pause5[26] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  pause5[27] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  pause5[28] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  pause5[29] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  pause5[30] = back + kck + mtc;    //(uint8)(0x18);   // kicker on, tape move
  pause5[31] = mtc1;   // upper 4 bits undefined
  //  pause5[31] = (uint8)(0x00);   // upper 4 bits undefined

  pause5[32] = (uint8)(0x06);   // IOType #4 => 6 WaitLong ... tape movement
  pause5[33] = tapeMove;         // number * 16 ms, 23*16 = 368 ms

  pause5[34] = (uint8)(0x1B);   // IOType #1 => 27 PortWriteState
  pause5[35] = (uint8)(0xf0);   // 3 bytes of write mask (first 4 bits are fixed analog so ignored)
  pause5[36] = (uint8)(0xff);   // start of "high current" outs EIO/CIO  
  pause5[37] = (uint8)(0x0f);   // upper 4 bits undefined..only 20 channels
  pause5[38] = (uint8)(0x00);   // 3 bytes of state, first 4 bits are analog
  pause5[39] =  back + kck + meas;   //(uint8)(0x30);   // kicker on, beam measurement
  pause5[40] = (uint8)(0x00);   // upper 4 bits undefined

  pause5[41] = (uint8)(0x00);   // extra 0 for aligned integers
  if (normal == 1){
    pause5[39] = back + meas;       //(uint8)(0x20);   // beam measurement, kicker off

  }
 
  extendedChecksum(pause5,42);  // checksum load array elements 0, 4, and 5

  return;
}

/******************************************************************************/
void cycleLabjack(uint8 cycle[], long int num){
  long int ii;
  long int error=0;
  //  uint8 cycleRead[100];
  BYTE cycleRead[100];
  long int timeout;
  unsigned long int count;

  //  sleep(1);
  //  printf("cycle %li  \n ",ddxx++);      
  count = num;
  //  sleep(1);

    //  error = LJUSB_BulkWrite(hU3, U3_PIPE_EP1_OUT, cycle, num);
  error = LJUSB_Write(hU6, cycle, count);
  if (error < num){
    if (error == 0) printf("Feedback setup error : write failed\n");
    else printf("Feedback setup error : did not write all of the buffer\n");
    //    return;
  }
  /*
  sleep(1);
  printf ("     %li bytes written in cycle %li  ",error,ddxx);  
  sleep(1);
  timeout = 0;
  while (error < 0){
    error = LJUSB_Read(hU3, cycleRead, count);
  //    error = LJUSB_BulkRead(hU3, U3_PIPE_EP2_IN, cycleRead, num);
    if (timeout++ > 10) break;           // added to allow 10 seconds to finish bulkread
  }
  sleep(1);
  printf ("      %li bytes read end cycle %li\n",error,ddxx++);
  sleep(1);
  */
    /*
  if(error < 10) {
    printf(" Receiving... \n");
    for (ii=0; ii<num; ii++){
      printf(" %li => %x %li \n",ii,cycleRead[ii], error);      
    }
  }
    */  
  return;
}

/****************************************************************************/
void timeLine(cairo_t *cr){   //,double xxx, double yyy) {
  double t0,t1,t2,t3;
  double tred,tyellow,tgreen,xxx;
  //  double xmin=50, xmax=700, ytic=10, ytext=300, yaxis=320;
  double xmin=50, xmax=700, ytic=10, ytext=10, yaxis=30;
  //double xmid=300;

  //  xmid = xmin + ((xmax-xmin)/2.0) ;

  crTLine = gdk_cairo_create (gtk_widget_get_window(TLine));

  cairo_translate(crTLine, 10, 10);    // shift to account for button frame   

  t0 = (double) cyc0;
  t1 = (double) cyc1;
  t2 = (double) cyc2;
  t3 = (double) cyc3;
  if (t2 > 0) {
    tred = xmin+xmax*(t3/t2);
    tyellow = xmin+xmax*(t1/t2);
    if (beamPause < 1) tred = tyellow;

    tgreen= xmax;
    xxx = xmin+(xmax-xmin)*(t0/t2);
  }

  //  cairo_identity_matrix (crTLine);                     // reset origin to upper left corner
  cairo_move_to(crTLine, xmin-30, ytext);                 // shift to account for button frame   
  cairo_select_font_face(crTLine, "Monaco", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(crTLine, 10);
  cairo_set_source_rgb(crTLine, 0, 0, 0);
  cairo_show_text(crTLine, "Cycle start");  

  cairo_move_to(crTLine, xmax-30, ytext);                 // shift to account for button frame   
  cairo_select_font_face(crTLine, "Monaco", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(crTLine, 10);
  cairo_set_source_rgb(crTLine, 0, 0, 0);
  cairo_show_text(crTLine, "Cycle end");  

  cairo_set_line_width(crTLine,1.0);
  cairo_set_source_rgb(crTLine, 0, 0, 0);
  cairo_move_to(crTLine, xmin, yaxis);                 // shift to account for button frame   
  cairo_line_to(crTLine,xmax,yaxis);
  cairo_stroke(crTLine);                                                     // draw line

  cairo_move_to(crTLine, xmin, yaxis-ytic);                 // shift to account for button frame   
  cairo_line_to(crTLine,xmin,yaxis+ytic);
  cairo_stroke(crTLine);                                                     // draw line
  cairo_move_to(crTLine, tred, yaxis-ytic);                 // shift to account for button frame   
  cairo_line_to(crTLine,tred,yaxis+ytic);
  cairo_stroke(crTLine);                                                     // draw line
  cairo_move_to(crTLine, tyellow, yaxis-ytic);                 // shift to account for button frame   
  cairo_line_to(crTLine,tyellow,yaxis+ytic);
  cairo_stroke(crTLine);                                                     // draw line
  cairo_move_to(crTLine, xmax, yaxis-ytic);                 // shift to account for button frame   
  cairo_line_to(crTLine,xmax,yaxis+ytic);
  cairo_stroke(crTLine);                                                     // draw line

  if (beamPause > 1){
    if (xxx >= 0.0  && xxx < tred)   cairo_set_source_rgb(crTLine, 1.0, 0.0, 0.0);
    else if (xxx >= tred  && xxx < tyellow)   cairo_set_source_rgb(crTLine, 1.0, 1.0, 0.0);
    else if (xxx >= tyellow  && xxx < tgreen)   cairo_set_source_rgb(crTLine, 0.0, 1.0, 0.0);
    else cairo_set_source_rgb(crTLine, 0.0, 0.0, 0.0);      // 

  } else {
    if (xxx >= 0.0  && xxx < tred)   cairo_set_source_rgb(crTLine, 1.0, 0.0, 0.0);
    //    else if (xxx >= tred  && xxx < tyellow)   cairo_set_source_rgb(crTLine, 1.0, 1.0, 0.0);
    else if (xxx >= tyellow  && xxx < tgreen)   cairo_set_source_rgb(crTLine, 0.0, 1.0, 0.0);
    else cairo_set_source_rgb(crTLine, 0.0, 0.0, 0.0);      // 
  }

  cairo_arc(crTLine,xxx,yaxis, 10.0, 0, 2*M_PI);    // put not on axis
  cairo_fill(crTLine);

  cairo_destroy(crTLine);
  return;
}

/******************************************************************************/

static gboolean time_handler(GtkWidget *widget) {
  //  static char buffer[256]="\0";
  //  time_t curtime=-1;
  //  struct tm *loctime;
  gchar txt6[30]="\0",txt7[30]="\0",txt8[30]="\0",txt9[30]="\0";
  long int flag=0;

  //  if (timeDiff > FFG) printf("In time_handler  ");

  //  if (widget->window == NULL){          // see below for gtk2 -> gtk3 migration (set or get)
   if (gtk_widget_get_window(widget) == NULL){
     printf("\n\n Returning FALSE from time_handler!\n");
     return FALSE;
  }

   //   gtk_widget_queue_draw(widget);
   timeCounter();                    // links reading the temps to the time_handler rather than the expose-event
   gtk_widget_queue_draw(timeNow);
   gtk_widget_queue_draw(TLine);
   //   if (cairo_setup == 0) gtk_widget_queue_draw(spectra);  // 0 means new data added  widget update moved to readTemp
/*
  gtk_widget_queue_draw_area(fixed,0,0,XPLOT,YPLOT);   // redraw window prevents over writing
  gtk_widget_queue_draw_area(spectra,0,0,XPLOT,YPLOT); 
*/
/*
   Attempt to get tape handling
  cyc0 = 0 => counter
  cyc1 = beamOn + lightPulser
  cyc2 = complete cycle
  bkg = number of cycles for background measurement
  bkgCycle = running count until bkg 

change labels once at beginning of each cycle
*/

  if (cyc0 == 0 && cycleOnOff == 1){
    if (bkgCycle == bkg) flag = 1;
    if (bkgCycle == 0) flag = 0; 
    if (flag == 0){
      sprintf(txt9,"Current cycle: DATA %li", bkgCycle+1);
      sprintf(txt7,"Data cycles: %li",++numData);
      gtk_label_set_text (GTK_LABEL (cyc_labdata), txt7);
    }
    else if (flag == 1){
      sprintf(txt9,"Current cycle: BKG");
      sprintf(txt8,"Bkg cycles: %li",++numBkg);
      gtk_label_set_text (GTK_LABEL (cyc_labbkg), txt8);
    }
    gtk_label_set_text (GTK_LABEL (cyc_labnow), txt9);

    sprintf(txt6,"Total cycles: %li",++numCycles);
    gtk_label_set_text (GTK_LABEL (cyc_labtotal), txt6);

  }

  if (beamPause < 1){
    if (cycleOnOff == 1) {
      if (cyc0 == cyc1) {                              // middle cycle
	if (bkgCycle == bkg){
	  if (takeAway == 1) cycleLabjack(cycle3,42);                     // bkg cycle
	  else cycleLabjack(away3,24);
	  bkgCycle = 0;                                // reset bkg counter
	} 
	else {
	  if (takeAway == 1) cycleLabjack(cycle1,42);  // regular cycle
	  else cycleLabjack(away1,24);
	  bkgCycle++;                                  // increment bkg cycle
	}
	cyc0++;
      }
      else if (cyc0 == cyc2) {                         // end of cycle so restart sequence
	if (numMTC == 1) {                                    // numMTC=2
	  if (bkgCycle == bkg) {
	    if (takeAway == 1) cycleLabjack(cycle2,24);                     // bkg cycle
	    else cycleLabjack(away2,42);
	  }
	  else { 
	    if (takeAway == 1) cycleLabjack(cycle0,24);  // regular cycle
	    else cycleLabjack(away0,42);
	  }
	}
	else {                                     // numMTC=2
	  if (bkgCycle == bkg) {
	    if (takeAway == 1) cycleLabjack(cycle6,32);                     // bkg cycle
	    else cycleLabjack(away2,42);
	  }
	  else { 
	    if (takeAway == 1) cycleLabjack(cycle4,32);  // regular cycle
	    else cycleLabjack(away0,42);
	  }
	}
	cyc0=0;
      }
      else {
	cyc0++;
      }
    } 
    else {
      cyc0 = 0;
      bkgCycle=0;
    }
  } else {                // beam pause if statement

    if (cycleOnOff == 1) {
      if (cyc0 == cyc1) {                              // middle cycle
	if (bkgCycle == bkg){
	  if (takeAway == 1) cycleLabjack(pause5,42);                     // bkg cycle
	  bkgCycle = 0;                                // reset bkg counter
	} 
	else {
	  if (takeAway == 1) cycleLabjack(pause2,42);  // regular cycle

	  bkgCycle++;                                  // increment bkg cycle
	}
	cyc0++;
      }
      else if (cyc0 == cyc2) {                         // end of cycle so restart sequence
	if (numMTC == 1) {                                    // numMTC=2
	  if (bkgCycle == bkg) {
	    if (takeAway == 1) cycleLabjack(pause3,24);                     // bkg cycle
	  }
	  else { 
	    if (takeAway == 1) cycleLabjack(pause0,24);  // regular cycle
	  }  
	}
	else {                                     // numMTC=2
	  if (bkgCycle == bkg) {
	    if (takeAway == 1) cycleLabjack(pause9,32);                     // bkg cycle
	  }
	  else { 
	    if (takeAway == 1) cycleLabjack(pause6,32);  // regular cycle
	  }
	}
	cyc0=0;
      }
      else if (cyc0 == cyc3) { 

	if (bkgCycle == bkg){
	  if (takeAway == 1) cycleLabjack(pause4,14);                     // bkg cycle
	} 
	else {
	  if (takeAway == 1) cycleLabjack(pause1,14);  // regular cycle
	}
	cyc0++;
      }
      else {
	cyc0++;
      }
    } 
    else {
      cyc0 = 0;
      bkgCycle=0;
    }
  }

  return TRUE;
}

/****************************************************************************/
void timeCounter(){
  //   long int xtime=0;
/*
  cyc0 = counter
  cyc1 = 
  cyc2 = 
  cyc3 = 
*/
/*
   xtime=plotTemp;

   Every minute plotTemp changes to indicate time to read thermometers
   but that should not be done close to when a buffer will be sent to the LabJack

   printf("plotTemp -> %li\n",plotTemp);  
*/
  //  if ((cycleOnOff == 0) && timeCurrent(cr) == 1) {

  if ((cycleOnOff == 0) && (plotTemp == 1) ) {
    tempRead();
  }  
  if (beamPause < 1){
    //    if (timeCurrent(cr) == 1 || (t_conflict == 1)){
    if (plotTemp == 1 || (t_conflict == 1)){
      if (numMTC == 1) {
	if ( ( cyc0 > 2             && (cyc0 < (cyc1-3)) ) || 
	     ( ( cyc0 > (cyc1 + busymtc + busylite + 2 )) && (cyc0 < (cyc2-3)) ) ) {
	  tempRead();         // get current time and if = 1, read temperatures (each minute)
	  t_conflict=0;
	} 
	else {
	  t_conflict=1;
	}
      }
      else {
	if ( (   cyc0 > busymtc  + 2                      && (cyc0 < (cyc1-3)) ) || 
	     ( ( cyc0 > (cyc1 + busymtc + busylite + 2 )) && (cyc0 < (cyc2-3)) ) ) {
	  tempRead();         // get current time and if = 1, read temperatures (each minute)
	  t_conflict=0;
	} 
	else {
	  t_conflict=1;
	}
      }
    }
  }                             // Read temperatures but not when the multi-command arrays are being processes
  else {
    //    if (timeCurrent(cr) == 1 || (t_conflict == 1)){
    if (plotTemp == 1 || (t_conflict == 1)){
      if (numMTC == 1) {
	if ( ( cyc0 > 2              && (cyc0 < (cyc3-3)) ) || 
	     ( ( cyc0 > (cyc3 + 2 )) && (cyc0 < (cyc1-3)) ) || 
	     ( ( cyc0 > (cyc1 + busymtc + busylite + 2 )) && (cyc0 < (cyc2-3)) ) ) {
	  tempRead();         // get current time and if = 1, read temperatures (each minute)
	  t_conflict=0;
	} 
	else {
	  t_conflict=1;
	}
      }
      else {
	if ( (   cyc0 > busymtc  + 2                      && (cyc0 < (cyc3-3)) ) || 
	     ( ( cyc0 > (cyc3 + 2 ))                      && (cyc0 < (cyc1-3)) ) || 
	     ( ( cyc0 > (cyc1 + busymtc + busylite + 2 )) && (cyc0 < (cyc2-3)) ) ) {
	  tempRead();         // get current time and if = 1, read temperatures (each minute)
	  t_conflict=0;
	} 
	else {
	  t_conflict=1;
	}
      }
    }
  }
  return;
}


/****************************************************************************/
void destroy( GtkWidget *widget, gpointer   data ){
  gtk_main_quit();
  return;
}

/****************************************************************************/
/* another callback */
static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) {

    gtk_main_quit ();
    return FALSE;
}
/******************************************************************************/
void kill_window(GtkWidget *widget, gpointer data) {

  //  if (svgmon != 0) svgmon=0; // set the flag to kill monitoring loops
  gtk_widget_destroy(data);
  return;
}

/******************************************************************************/
static gboolean on_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data) {
//static gboolean drawCairo (GtkWidget *widget, CairoContext *event, gpointer data) {
//  cairo_t *cr;
   long int xtime=0;

/*
  Usual routine which redraws widgets upon windo activity.  For this program, we redraw
  every 50 or 100 ms as we use the clock to determine the timing of the steps in the cycle.
*/

// cr = gdk_cairo_create (widget->window);      // see below for gtk2 -> gtk3 migration (set or get)
//   cr = gdk_cairo_create (gtk_widget_get_window(widget));   // gtk3
   /*   
  char xxtime[50]="\0";

  cairo_translate(cr, 420, 150);    // shift to account for button frame   
  cairo_select_font_face(cr, "Monaco", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 20);
  cairo_set_source_rgb(cr, 0, 0, 0);
  strcpy(xxtime,GetDate()); 
  cairo_show_text(cr, xxtime);            // update current time


  //   cairo_paint(cr);
      // cairo_set_source_surface(cr, cr1,0,0);          // this assigns the surface to cr at positions 0,0

  //   cr = cairo_create (cr_surf);
  //   cairo_set_source_surface (cr, surface, 700, 800);
   //    cr = gdk_cairo_create ((GdkWindow*) widget);
  //  xtime = timeCurrent(cr);
//!@#$%^

   xtime=1;//plotTemp;
   printf("xtime -> %li\n",xtime);
  //  if ((cycleOnOff == 0) && timeCurrent(cr) == 1) {
  if ((cycleOnOff == 0) && (xtime == 1) ) {
    tempRead();
  }  
  if (beamPause < 1){
    //    if (timeCurrent(cr) == 1 || (t_conflict == 1)){
    if (xtime == 1 || (t_conflict == 1)){
      if (numMTC == 1) {
	if ( ( cyc0 > 2             && (cyc0 < (cyc1-3)) ) || 
	     ( ( cyc0 > (cyc1 + busymtc + busylite + 2 )) && (cyc0 < (cyc2-2)) ) ) {
	  tempRead();         // get current time and if = 1, read temperatures (each minute)
	  t_conflict=0;
	} 
	else {
	  t_conflict=1;
	}
      }
      else {
	if ( (   cyc0 > busymtc  + 2                      && (cyc0 < (cyc1-3)) ) || 
	     ( ( cyc0 > (cyc1 + busymtc + busylite + 2 )) && (cyc0 < (cyc2-2)) ) ) {
	  tempRead();         // get current time and if = 1, read temperatures (each minute)
	  t_conflict=0;
	} 
	else {
	  t_conflict=1;
	}
      }
    }
  }                             // Read temperatures but not when the multi-command arrays are being processes
  else {
    //    if (timeCurrent(cr) == 1 || (t_conflict == 1)){
    if (xtime == 1 || (t_conflict == 1)){
      if (numMTC == 1) {
	if ( ( cyc0 > 2              && (cyc0 < (cyc3-3)) ) || 
	     ( ( cyc0 > (cyc3 + 2 )) && (cyc0 < (cyc1-3)) ) || 
	     ( ( cyc0 > (cyc1 + busymtc + busylite + 2 )) && (cyc0 < (cyc2-2)) ) ) {
	  tempRead();         // get current time and if = 1, read temperatures (each minute)
	  t_conflict=0;
	} 
	else {
	  t_conflict=1;
	}
      }
      else {
	if ( (   cyc0 > busymtc  + 2                      && (cyc0 < (cyc3-3)) ) || 
	     ( ( cyc0 > (cyc3 + 2 ))                      && (cyc0 < (cyc1-3)) ) || 
	     ( ( cyc0 > (cyc1 + busymtc + busylite + 2 )) && (cyc0 < (cyc2-2)) ) ) {
	  tempRead();         // get current time and if = 1, read temperatures (each minute)
	  t_conflict=0;
	} 
	else {
	  t_conflict=1;
	}
      }
    }
  }
   */
  //  if (t_conflict == 0) {
  //    printf (" plotTempFlag = %i,  - plot temps\n",plotTempFlag);

  //  if (plotTempFlag == 1) {
  //    tempPlot(cr);
  //    tempPlot(cr);      // coment back in if returning to drawing only on fixed.

  
  //  spectraDraw(spectra);  // THIS IS A MANUAL UPDATE
  //  cairo_setup = 1;

    //   cairo_show_page(cr_surf);
    //   cairo_surface_finish(cr_surf);
  //   cairo_surface_flush(cr_surf);
  //  cairo_surface_destroy(cr_surf);
    //    cairo_save(cr1);
    //    cairo_restore(cr1);
    // cairo_destroy(cr);
    //    plotTempFlag=0;
    //  }
    //    plotTemp = 0;
    //  }
  //  timeLine(crTLine);  // THIS IS A MANUAL UPDATE

  //  cairo_destroy(cr);

  return FALSE;
   }

/******************************************************************************/
long int timeCurrent(cairo_t *crTime){
  time_t curtime = -1;
  double temp=0.0;
  long int mm=60;   // record temps every 60 seconds
  char xxtime[50]="\0", yytime[50]="\0";
  long int nn=0;

  crTime = gdk_cairo_create (gtk_widget_get_window(timeNow));

  cairo_translate(crTime, 10, 10);    // shift to account for button frame   
  //  cairo_translate(crTime, 20, 50);    // shift to account for button frame   
  cairo_select_font_face(crTime, "Monaco", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(crTime, 10);
  cairo_set_source_rgb(crTime, 0, 0, 0);
  strcpy(yytime,GetDate());       
  nn = strlen(yytime);                   //2 lines to remove null character at end
  strncpy(xxtime,yytime,nn-1);
  cairo_show_text(crTime, xxtime);            // update current time

  //  cairo_identity_matrix (crTime);      // reset origin to upper left corner
  cairo_destroy(crTime);

  while (curtime < 0){
    curtime = time(NULL);
  }
  plotTemp = 0;

  if (time0 == 0) {
    time0 = curtime;
    timeSave = time0;
    timeDiff = 0;
    //    sprintf(time0Str,"%s",GetDate());          // record zero time in bottom left corner
    sprintf(time0Str,"%s",xxtime);          // record zero time in bottom left corner
    plotTemp = 1;
    return 1;
  }
  if (curtime >= (timeSave + mm)){                  // every minute return 1 to read temperatures
    timeDiff =  (long int) ((curtime - time0)/ mm);  
    timeSave = (timeDiff * mm) + time0;  //curtime;  // base it on time0 so that latencies do not propogate
    plotTemp = 1;

    return 1;
  }

  return 0;
}

/****************************************************************************/
void menuControl(){
  GtkWidget *menubar, *menubox,*filemenu, *file, *quit;// *file, *save, *quit;

  //  if (timeDiff > FFG) printf("In menuControl ");
  //  menubox = gtk_vbox_new(FALSE, 0);
  menubox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_fixed_put(GTK_FIXED(fixed),menubox,0,0);
  //  gtk_container_add(GTK_CONTAINER(window), menubox);

  menubar = gtk_menu_bar_new();                                    // create menu bar
  filemenu = gtk_menu_new();                                       // create file menu
  file = gtk_menu_item_new_with_label("File");                     // create  menu item
  quit = gtk_menu_item_new_with_label("Quit");                     // create  menu items

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);                        // implement a file  menu
  gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);                           // add quit to the file menu
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);                            // add "file" file menu to menu bar

  g_signal_connect(G_OBJECT(quit), "activate", G_CALLBACK(gtk_main_quit), NULL);

/*
Menu bar finished now load it in the menubox
*/
  gtk_box_pack_start(GTK_BOX(menubox), menubar, FALSE, FALSE, 3);                 // add file menu to menu bar

  //  if (timeDiff > FFG) printf(".... Out menuControl\n");
  return;
}

/****************************************************************************/
void tempPlot(cairo_t *cr){   //,double xxx, double yyy) {
  double xxx=0.0,yyy=0.0, xlab=0.0;// offset;
  long int ii=0, jj=0, jjlo=0, xbin=0;
  char txt[10]="\0", tt[10]="\0";

/*
 Record time 0 as date/time on the canvasBegin plot of temperature
*/
//  if (timeDiff > FFG) printf("In tempPlot  ");
  cairo_identity_matrix (cr);                     // reset origin to upper left corner
  cairo_move_to(cr, 5, YPLOT-10);                 // shift to account for button frame   
  cairo_select_font_face(cr, "Monaco", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 10);
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_show_text(cr, time0Str);  
/*
 Begin plot of temperature
*/
  jjlo=0;
  xbin= timeDiff/1500;   // interger division
  jjlo = xbin*1500;

  for (ii=0; ii<thermNum; ii++){
    cairo_identity_matrix (cr);                       // reset origin to upper left corner
    cairo_translate(cr, 50, YPLOT - 150 - (ii*tOffset));   // move to start the rectangular graph
    cairo_set_source_rgb(cr, 0, 0, 0);                // black
    cairo_rectangle(cr, 0, 0, tWidth,tHeight);        // graph frame
    cairo_stroke_preserve(cr);                        // includes the stroke
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);          // graph fill - white
    cairo_fill(cr);                                   // fill graph with color
  }
/*
   Record current temperature
*/
  cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);      // graph fill
  cairo_set_line_width(cr,1.0);

  for (ii=0; ii<thermNum; ii++){
    cairo_select_font_face(cr, "Monaco", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 10);
    cairo_set_source_rgb(cr, 0, 0, 0);
    sprintf(txt,"%.1lf",therm[ii].deg[timeDiff]);

    cairo_identity_matrix (cr);      // reset origin to upper left corner
    cairo_move_to(cr,55, (double)YPLOT - 140 - (tOffset * (double)ii ));  // 90 units away from bottom of graph
    cairo_show_text(cr, txt);  
/*
   plot data as small circles...no lines connecting them
*/
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);      // 
 
    for (jj=jjlo; jj<= timeDiff; jj++){
      //      printf ("in tempPlot: %lf %lf\n",therm[ii].xx[jj], therm[ii].yy[jj]);
      //           if (jj == 0){
      //       cairo_arc(cr,therm[ii].xx[jj], therm[ii].yy[jj], 3.0, 0, 2*M_PI);    // put not on axis
	//	cairo_fill(cr);
      //            }
      //        else {
	//	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);      // 
	cairo_arc(cr,therm[ii].xx[jj]-xbin*(1500*therm[ii].calx), therm[ii].yy[jj], 1.0, 0, 2*M_PI);    // put not on axis
	//	cairo_fill(cr);
	//		      }
      cairo_fill(cr);

    }
// !@#$%^& testing below
 
/*
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);      // 
	cairo_arc(cr,therm[ii].xx[timeDiff]-xbin*(1500*therm[ii].calx), therm[ii].yy[timeDiff], 1.0, 0, 2*M_PI);    // put not on axis
	cairo_fill(cr);
*/ 
    cairo_select_font_face(cr, "Monaco", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 10);
    cairo_set_source_rgb(cr, 0, 0, 0);
/*
   plot Y ticks and labels
*/
    for (jj=0; jj<5; jj++){
      cairo_identity_matrix (cr);      // reset origin to upper left corner
      sprintf(tt,"%.0lf",therm[ii].min+jj);    // make tick label
      yyy=(double)YPLOT - 50 - tHeight/5*(double)jj - tOffset*(double)ii;   // 50 from bottom, tick pos on graph, tOffset*ii = next graph
      cairo_move_to(cr,25,yyy+3);
      cairo_show_text(cr, tt); 
      cairo_move_to(cr,45,yyy);                // draw ticks
      cairo_line_to(cr,55,yyy);
      cairo_stroke(cr);                                                     // draw line
    }

    cairo_move_to(cr,15,(double)YPLOT-50-((double)ii*tOffset)-20);
    cairo_rotate(cr, -M_PI/2);
    if (ii == 0) cairo_show_text(cr, "Temp (C)"); 
    if (ii == 1) cairo_show_text(cr, "Temp (C)"); 
    if (ii == 2) cairo_show_text(cr, "Temp (C)"); 
    if (ii == 3) cairo_show_text(cr, "Temp (C)"); 

    cairo_identity_matrix (cr);      // reset origin to upper left corner
    cairo_move_to(cr,(tWidth+60),(double)YPLOT-50-((double)ii*tOffset)-tOffset/2);
    //    cairo_rotate(cr, M_PI/2);
    if (ii == 0) cairo_show_text(cr, "Room"); 
    if (ii == 1) cairo_show_text(cr, "Upstream"); 
    if (ii == 2) cairo_show_text(cr, "Downstream Right"); 
    if (ii == 3) cairo_show_text(cr, "Downstream Left"); 

/*
   plot X ticks and labels
*/

    if (ii == 0){
      for (jj=0; jj<11; jj++){
	xlab = jjlo + (timedisplay/10*(double)jj);
	sprintf(tt,"%.0lf",xlab);                                           // make tick label
	cairo_identity_matrix (cr);      // reset origin to upper left corner
	if (strlen(tt) == 1) cairo_move_to(cr,47 + tWidth/10*jj, YPLOT-30); 
	if (strlen(tt) == 2) cairo_move_to(cr,44 + tWidth/10*jj, YPLOT-30); 
	if (strlen(tt) == 3) cairo_move_to(cr,40 + tWidth/10*jj, YPLOT-30); 
	if (strlen(tt) == 4) cairo_move_to(cr,35 + tWidth/10*jj, YPLOT-30); 
	if (strlen(tt) == 5) cairo_move_to(cr,32 + tWidth/10*jj, YPLOT-30); 
	cairo_show_text(cr, tt); 

	xxx= 50 + tWidth/10*jj;
	yyy = (double)YPLOT-45-((double)ii*tHeight) - tOffset*(double)ii;
	cairo_move_to(cr,xxx,yyy);                // draw ticks
	cairo_line_to(cr,xxx,yyy-10);
	cairo_stroke(cr);                                                     // draw line
      }
    } else {
      for (jj=0; jj<11; jj++){
	xxx= 50 + tWidth/10*jj;
	yyy = (double)YPLOT-45-((double)ii*tOffset);
	cairo_identity_matrix (cr);      // reset origin to upper left corner
	cairo_move_to(cr,xxx,yyy);                // draw ticks
	cairo_line_to(cr,xxx,yyy-10);
	cairo_stroke(cr);                                                     // draw line
      }

    }
  }   

  cairo_identity_matrix (cr);      // reset origin to upper left corner

  //  if (timeDiff > FFG) printf(".... Out tempPlot\n");
  return;
} 

/*
   cairo coordinates: graph starts at (50,400) so origin is (50,800), top Y axis (50.400), top X axis (650,800)

   so 

*/
  
/******************************************************************************/
void mtasOpenTemp(){
  long int ii=0, jj=0, kk=0;
  char file_name[200]="therm-", tt[50]="\0";
  //  int c,n,a,index;
  //  char *ind;
  //  FILE *fileTherm;


  //  if (timeDiff > FFG) printf("In OpenTemp");
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
  //  printf("%s \n",tt);
  //  if (strcmp(field.name[2],"ofile") == 0) strcat(file_name,field.value[2]);
  if (jj > 0) strcat(file_name,tt);
  strcat(file_name,".log\0");

  //  printf("%s\n",file_name);                  

 if (( fileTherm = fopen (file_name,"a+") ) == NULL){
   printf ("*** File on disk could not be opened \n");
   exit (EXIT_FAILURE);
 }
  printf("Temperature logfile opened: %s\n",file_name);             
 // 

  //  if (timeDiff > FFG) printf(".... Out OpenTemp\n");
  return;
}
/******************************************************************************/

/******************************************************************************/


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


/****************************************************************************/
void tempPlot1(cairo_t *crSpec){   //,double xxx, double yyy) {
  double xxx=0.0,yyy=0.0, xlab=0.0;// offset;
  long int ii=0, jj=0, jjlo=0, xbin=0;
  char txt[10]="\0", tt[10]="\0";
  //  double YYPLOT;

  //  printf("Hi-tempPlot1 = %li\n", cairo_setup);
/*
 Record time 0 as date/time on the canvasBegin plot of temperature

  if (cairo_setup == 1) {
    tempPlot2(cr1);
    return;
  }
  cairo_setup = 1;
*/
//  if (timeDiff > FFG) printf("In tempPlot  ");

//  YYPLOT = YPLOT - 430;

  cairo_identity_matrix (crSpec);                     // reset origin to upper left corner
  cairo_move_to(crSpec, 5, YYPLOT-10);                 // shift to account for button frame   
  cairo_select_font_face(crSpec, "Monaco", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(crSpec, 10);
  cairo_set_source_rgb(crSpec, 0, 0, 0);
  cairo_show_text(crSpec, time0Str);  
/*
 Begin plot of temperature
*/
  jjlo=0;
  xbin= timeDiff/1500;   // interger division
  jjlo = xbin*1500;

  //  YYPLOT = YPLOT - 430;

  for (ii=0; ii<thermNum; ii++){
    cairo_identity_matrix (crSpec);                       // reset origin to upper left corner
    cairo_translate(crSpec, 50, YYPLOT - 150 - (ii*tOffset));   // move to start the rectangular graph
    cairo_set_source_rgb(crSpec, 0, 0, 0);                // black
    cairo_rectangle(crSpec, 0, 0, tWidth,tHeight);        // graph frame
    cairo_stroke_preserve(crSpec);                        // includes the stroke
    cairo_set_source_rgb(crSpec, 1.0, 1.0, 1.0);          // graph fill - white
    cairo_fill(crSpec);                                   // fill graph with color
  }
/*
   Record current temperature
*/
  cairo_set_source_rgb(crSpec, 0.0, 0.0, 0.0);      // graph fill
  cairo_set_line_width(crSpec,1.0);

  for (ii=0; ii<thermNum; ii++){
    cairo_select_font_face(crSpec, "Monaco", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(crSpec, 10);
    cairo_set_source_rgb(crSpec, 0, 0, 0);
    sprintf(txt,"%.1lf",therm[ii].deg[timeDiff]);

    cairo_identity_matrix (crSpec);      // reset origin to upper left corner
    cairo_move_to(crSpec,55, (double)YYPLOT - 140 - (tOffset * (double)ii ));  // 90 units away from bottom of graph
    cairo_show_text(crSpec, txt);  
/*
   plot data as small circles...no lines connecting them
*/
    cairo_set_source_rgb(crSpec, 0.0, 0.0, 0.0);      // 
 
       for (jj=jjlo; jj<= timeDiff; jj++){
      cairo_arc(crSpec,therm[ii].xx[jj]-xbin*(1500*therm[ii].calx), therm[ii].yy[jj], 2.0, 0, 2*M_PI);    // put not on axis
            cairo_fill(crSpec);
         }

// !@#$%^& testing below
 
/*
	cairo_set_source_rgb(crSpec, 0.0, 0.0, 0.0);      // 
	cairo_arc(crSpec,therm[ii].xx[timeDiff]-xbin*(1500*therm[ii].calx), therm[ii].yy[timeDiff], 1.0, 0, 2*M_PI);    // put not on axis
	cairo_fill(crSpec);
*/ 
    cairo_select_font_face(crSpec, "Monaco", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(crSpec, 10);
    cairo_set_source_rgb(crSpec, 0, 0, 0);
/*
   plot Y ticks and labels
*/
    for (jj=0; jj<5; jj++){
      cairo_identity_matrix (crSpec);      // reset origin to upper left corner
      sprintf(tt,"%.0lf",therm[ii].min+jj);    // make tick label
      yyy=(double)YYPLOT - 50 - tHeight/5*(double)jj - tOffset*(double)ii;   // 50 from bottom, tick pos on graph, tOffset*ii = next graph
      cairo_move_to(crSpec,25,yyy+3);
      cairo_show_text(crSpec, tt); 
      cairo_move_to(crSpec,45,yyy);                // draw ticks
      cairo_line_to(crSpec,55,yyy);
      cairo_stroke(crSpec);                                                     // draw line
    }

    cairo_move_to(crSpec,15,(double)YYPLOT-50-((double)ii*tOffset)-20);
    cairo_rotate(crSpec, -M_PI/2);
    if (ii == 0) cairo_show_text(crSpec, "Temp (C)"); 
    if (ii == 1) cairo_show_text(crSpec, "Temp (C)"); 
    if (ii == 2) cairo_show_text(crSpec, "Temp (C)"); 
    if (ii == 3) cairo_show_text(crSpec, "Temp (C)"); 

    cairo_identity_matrix (crSpec);      // reset origin to upper left corner
    cairo_move_to(crSpec,(tWidth+60),(double)YYPLOT-50-((double)ii*tOffset)-tOffset/2);
    //    cairo_rotate(crSpec, M_PI/2);
    if (ii == 0) cairo_show_text(crSpec, "Room"); 
    if (ii == 1) cairo_show_text(crSpec, "Upstream"); 
    if (ii == 2) cairo_show_text(crSpec, "Downstream Right"); 
    if (ii == 3) cairo_show_text(crSpec, "Downstream Left"); 

/*
   plot X ticks and labels
*/

    if (ii == 0){
      for (jj=0; jj<11; jj++){
	xlab = jjlo + (timedisplay/10*(double)jj);
	sprintf(tt,"%.0lf",xlab);                                           // make tick label
	cairo_identity_matrix (crSpec);      // reset origin to upper left corner
	if (strlen(tt) == 1) cairo_move_to(crSpec,47 + tWidth/10*jj, YYPLOT-30); 
	if (strlen(tt) == 2) cairo_move_to(crSpec,44 + tWidth/10*jj, YYPLOT-30); 
	if (strlen(tt) == 3) cairo_move_to(crSpec,40 + tWidth/10*jj, YYPLOT-30); 
	if (strlen(tt) == 4) cairo_move_to(crSpec,35 + tWidth/10*jj, YYPLOT-30); 
	if (strlen(tt) == 5) cairo_move_to(crSpec,32 + tWidth/10*jj, YYPLOT-30); 
	cairo_show_text(crSpec, tt); 

	xxx= 50 + tWidth/10*jj;
	yyy = (double)YYPLOT-45-((double)ii*tHeight) - tOffset*(double)ii;
	cairo_move_to(crSpec,xxx,yyy);                // draw ticks
	cairo_line_to(crSpec,xxx,yyy-10);
	cairo_stroke(crSpec);                                                     // draw line
      }
    } else {
      for (jj=0; jj<11; jj++){
	xxx= 50 + tWidth/10*jj;
	yyy = (double)YYPLOT-45-((double)ii*tOffset);
	cairo_identity_matrix (crSpec);      // reset origin to upper left corner
	cairo_move_to(crSpec,xxx,yyy);                // draw ticks
	cairo_line_to(crSpec,xxx,yyy-10);
	cairo_stroke(crSpec);                                                     // draw line
      }

    }
  }   

  //  cairo_identity_matrix (crSpec);      // reset origin to upper left corner
  //  cairo_destroy(crSpec);
  //  if (timeDiff > FFG) printf(".... Out tempPlot\n");
  return;
} 

/*
   cairo coordinates: graph starts at (50,400) so origin is (50,800), top Y axis (50.400), top X axis (650,800)

   so 

*/
  
void tempPlot2(cairo_t *crSpec){   //,double xxx, double yyy) {
  long int ii=0, xbin=0;
  //  double xxx=0.0,yyy=0.0, xlab=0.0;// offset;
  //  long int ii=0, jj=0, jjlo=0, xbin=0;
   char txt[10]="\0", tt[10]="\0";
/*
   plot data as small circles...no lines connecting them
*/
  xbin= timeDiff/1500;   // interger division
  cairo_identity_matrix (crSpec);      // reset origin to upper left corner

  for (ii=0; ii<thermNum; ii++){

    cairo_select_font_face(crSpec, "Monaco", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(crSpec, 10);
    cairo_set_source_rgb(crSpec, 0, 0, 0);
    sprintf(txt,"%.1lf",therm[ii].deg[timeDiff]);

    cairo_identity_matrix (crSpec);      // reset origin to upper left corner
    cairo_move_to(crSpec,55, (double)YYPLOT - 140 - (tOffset * (double)ii ));  // 90 units away from bottom of graph
    cairo_show_text(crSpec, txt);  
/*
   plot data as small circles...no lines connecting them
*/
    cairo_set_source_rgb(crSpec, 0.0, 0.0, 0.0);      // 
 

    cairo_set_source_rgb(crSpec, 0.0, 0.0, 0.0);      // 
    cairo_arc(crSpec,therm[ii].xx[timeDiff]-xbin*(1500*therm[ii].calx), therm[ii].yy[timeDiff], 3.0, 0, 2*M_PI);    // put not on axis
    cairo_fill(crSpec);
  }
  //  cairo_destroy(crSpec);
  return;

}
