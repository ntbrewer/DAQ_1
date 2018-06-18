#include <unistd.h>  /* UNIX standard function definitions */
#include <time.h>    /* Time definitions */
#include <gtk/gtk.h>
#include <cairo.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <ctype.h>
#include <time.h>
#include <math.h>    /* Math definitions */
#include <signal.h>  /* Signal interrupt definitions */

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
#include "../../include/labjackusb.h"
#include "../../include/kick-u3.h"

GtkWidget *window, *fixed;
GtkWidget *mtc9Label, *mtc10Label;
GtkWidget *mtcLabel[25], *mtcLOF[25]; // array is already a pointer so don't need the *

static gboolean time_handler(GtkWidget *widget);
static gboolean on_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data);
static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data);
void destroy( GtkWidget *widget, gpointer   data );
void kill_window(GtkWidget *widget, gpointer data);
void tapeMode(); 
int mmapSetup();

long int REFRESH = 100;   // update gtk graphics every 100 ms
int bkgCycle=18, bkg=20;
time_t curtime, time0;
long int beamOnPcent = 0, beamOffPcent = 0, beamPausePcent = 0;

/***********************************************************/
int main(int argc, char **argv){
  int mapKick=0;
/*
  Shared memory creation and attachment
  the segment number is stored in lnptr->pid
*/
  mapKick = mmapSetup();
  if (mapKick == -1) {
    printf(" Error on setting up memory map ... exiting \n");
    return 0;
  }
  time0 = time(NULL);
  
  gtk_init (&argc, &argv);                                                    // mandatory for gtk applications
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);                              // creates new window
  gtk_window_set_title (GTK_WINDOW (window),"CONTROLS STATUS");               // sets title of window
  gtk_container_set_border_width (GTK_CONTAINER(window),1);                   // sets border width

  fixed = gtk_fixed_new();                                                // create a container to hold future widgets
  gtk_container_add(GTK_CONTAINER(window), fixed);                        // I must determine position of all widgets

  g_signal_connect (window, "delete-event",G_CALLBACK (delete_event), NULL);        // handler to exit gtk
/*
  draw and expose-event redraws on any event including mouse movement.  They are usually needed in programs.
   But in this program I am renewing the widgets on a timer based on eitehr 50 or 100 ms
*/
  g_signal_connect (window, "draw", G_CALLBACK (on_expose_event), NULL);    // gtk3 - 
  //g_signal_connect (window, "expose-event", G_CALLBACK (on_expose_event), NULL); // gtk2 from cairo example  window -> frame1
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);           // from cairo example  window -> frame1

  gtk_window_set_default_size(GTK_WINDOW(window), 300, 500);                    // from cairo example  window -> frame1

/* track the number of cycles 
   - create the text
   - create the widget containing the text
   - define the location of the widget in the widget fixed
*/
  tapeMode();
/*
 Show all in window  .. believe the program loops between here and gtk_main every 100 ms
*/
  g_timeout_add(REFRESH, (GSourceFunc) time_handler, (gpointer) fixed);    // timer to refresh widget "fixed"
  gtk_widget_show_all (window);                                            // show windows before stating timer loop
  time_handler(fixed);
  gtk_main ();

 /*
  Ending the program so release the memory map and close file
*/

  if (munmap(mtcptr, sizeof (struct mtc*)) == -1) {
    perror("Error un-mapping the file");
  }
  close(mapKick);
  printf(" File closed and file unmapped \n");

  return 0;
}



/******************************************************************************/

static gboolean time_handler(GtkWidget *widget) {
  gchar txt[30]="\0";//,txt9[30]="\0";
  int jj=0;
  long int pCent = 0;

  if (mtcptr->com1 == 1) {                            // parameters were changed
    sprintf(txt,"Beam ON (%i ms)",mtcptr->bon.ms);
    gtk_label_set_text (GTK_LABEL (mtcLabel[1]),txt);
    sprintf(txt,"Beam Pause (%i ms)",mtcptr->pon.ms);
    gtk_label_set_text (GTK_LABEL (mtcLabel[2]),txt);
    sprintf(txt,"Laser (%i ms)",mtcptr->lon.ms);
    gtk_label_set_text (GTK_LABEL (mtcLabel[3]),txt);
    gtk_label_set_text (GTK_LABEL (mtcLabel[6]),txt);
    sprintf(txt,"MTC move (%i ms)",mtcptr->tmove.ms);
    gtk_label_set_text (GTK_LABEL (mtcLabel[4]),txt);
    gtk_label_set_text (GTK_LABEL (mtcLabel[7]),txt);
    gtk_label_set_text (GTK_LABEL (mtcLabel[8]),txt);
    sprintf(txt,"Beam OFF (measure) (%i ms)",mtcptr->boff.ms);
    gtk_label_set_text (GTK_LABEL (mtcLabel[5]),txt);
    usleep(300000);                         // sleep for 300 ms to allow any other kick-mon to update
    mtcptr->com1 = 0;                                 // labels were changed
  }                   
  if (mtcptr->com0 == 100)  gtk_main_quit();
  
  if (gtk_widget_get_window(widget) == NULL){
    printf("\n\n Returning FALSE from time_handler!\n");
    return FALSE;
  }

  if (!mtcptr->onoff){
    for (jj=0;jj<20;jj++){
      gtk_label_set_text (GTK_LABEL (mtcLOF[jj]),"  ");
      if (jj == 10) gtk_label_set_text (GTK_LABEL (mtcLOF[jj]),"=>");
    }
    if (mtcptr->gtkstat > 10 && mtcptr->gtkstat < 20)
        gtk_label_set_text (GTK_LABEL (mtcLOF[mtcptr->gtkstat]),"=>");
  }
  else {
    for (jj=0;jj<20;jj++){
      gtk_label_set_text (GTK_LABEL (mtcLOF[jj]),"  ");
      if (jj == 9) gtk_label_set_text (GTK_LABEL (mtcLOF[jj]),"=>");
    }
    if (mtcptr->onoff) gtk_label_set_text (GTK_LABEL (mtcLabel[9]),"Cycle ON");
    else  gtk_label_set_text (GTK_LABEL (mtcLabel[10]),"Cycle OFF");
    if (mtcptr->takeaway) gtk_label_set_text (GTK_LABEL (mtcLabel[0]),"Mode: TakeAway");
    if (mtcptr->normal) gtk_label_set_text (GTK_LABEL (mtcLabel[0]),"Mode: Normal");
    if (mtcptr->pause) gtk_label_set_text (GTK_LABEL (mtcLabel[0]),"Mode: Normal w/pause");
    if (mtcptr->gtkstat > 0 && mtcptr->gtkstat < 10) gtk_label_set_text (GTK_LABEL (mtcLOF[mtcptr->gtkstat]),"=>");
    
    if (mtcptr->beammeas) gtk_label_set_text (GTK_LABEL (mtcLabel[18]),"Beam w/measure");
    else gtk_label_set_text (GTK_LABEL (mtcLabel[18]),"Beam w/out measure");
    if (mtcptr->measbeam) gtk_label_set_text (GTK_LABEL (mtcLabel[19]),"Measure w/beam");
    else gtk_label_set_text (GTK_LABEL (mtcLabel[19]),"Measure w/out beam");
    if (mtcptr->background) gtk_label_set_text (GTK_LABEL (mtcLOF[16]),"=>");
    else gtk_label_set_text (GTK_LABEL (mtcLOF[15]),"=>");

  }

  sprintf(txt,"Data:Background ratio: %i", mtcptr->bkgRatio);
  gtk_label_set_text (GTK_LABEL (mtcLabel[14]),txt);
  sprintf(txt,"Data cycles: %i", mtcptr->cyclesData);
  gtk_label_set_text (GTK_LABEL (mtcLabel[15]),txt);
  sprintf(txt,"Background cycles: %i", mtcptr->cyclesBkg);
  gtk_label_set_text (GTK_LABEL (mtcLabel[16]),txt);
  sprintf(txt,"Total cycles: %i", (mtcptr->cyclesData + mtcptr->cyclesBkg));
  gtk_label_set_text (GTK_LABEL (mtcLabel[17]),txt);
  
  if (mtcptr->tapeFault) gtk_label_set_text (GTK_LABEL (mtcLabel[20]),"MTC FAULT ERROR");
  else gtk_label_set_text (GTK_LABEL (mtcLabel[20]),"mtc ok");
  if (mtcptr->tapeBreak) gtk_label_set_text (GTK_LABEL (mtcLabel[21]),"MTC TAPE BREAK ERROR");
  else gtk_label_set_text (GTK_LABEL (mtcLabel[21]),"mtc ok");

  if (mtcptr->onoff){
    if (mtcptr->gtkstat == 1 && mtcptr->bon.ms > 10000) pCent = (beamOnPcent++)*REFRESH*100 / mtcptr->bon.ms;
    else beamOnPcent = 0;
    if (mtcptr->gtkstat == 5 && mtcptr->boff.ms > 10000) pCent = (beamOffPcent++)*REFRESH*100 / mtcptr->boff.ms;
    else beamOffPcent =0;
    if (mtcptr->gtkstat == 2 && mtcptr->pon.ms > 10000) pCent = (beamPausePcent++)*REFRESH*100 /mtcptr->pon.ms;
    else beamPausePcent = 0;
  } else {
    pCent = 0;
    beamOnPcent = 0;
    beamOffPcent = 0;
    beamPausePcent=0;    
  }
  sprintf(txt,"Percent of cycle: %li %%",pCent );
  gtk_label_set_text (GTK_LABEL (mtcLabel[22]),txt);
  //  printf (" %li  %li   %li    %li \n",beamOnPcent, beamOffPcent, beamPausePcent, pCent);
  

  gtk_widget_queue_draw(fixed);
  
  return TRUE;
}

/****************************************************************************/
void destroy( GtkWidget *widget, gpointer   data ){
  gtk_main_quit();
  return;
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
//   long int xtime=0;

/*
  Usual routine which redraws widgets upon windo activity.  For this program, we redraw
  every 50 or 100 ms as we use the clock to determine the timing of the steps in the cycle.
*/


   return FALSE;
}
/******************************************************************************/
/* another callback */
static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) {

    gtk_main_quit ();
    return FALSE;
}
/****************************************************************************/

void tapeMode() {
  int jj=0;
  char mtcLon[25][3], mtcL[25][35];
 
/* track the number of cycles 
   - create the text
   - create the widget containing the text
   - define the location of the widget in the widget fixed
*/

  if (mtcptr->normal) sprintf(mtcL[0],"Mode: Normal");
  else sprintf(mtcL[0],"Mode: TakeAway");

  sprintf(mtcL[1],"Beam ON (%i ms)",mtcptr->bon.ms);
  sprintf(mtcL[2],"Beam Pause (%i ms)",mtcptr->pon.ms);
  sprintf(mtcL[3],"Laser (%i ms)",mtcptr->lon.ms);
  sprintf(mtcL[4],"MTC move (%i ms)",mtcptr->tmove.ms);
  sprintf(mtcL[5],"Beam OFF (measure) (%i ms)",mtcptr->boff.ms);
  sprintf(mtcL[6],"Laser (%i ms)",mtcptr->lon.ms);
  sprintf(mtcL[7],"MTC move (%i ms)",mtcptr->tmove.ms);
  sprintf(mtcL[8],"MTC move (%i ms)",mtcptr->tmove.ms);

  if (mtcptr->onoff) sprintf(mtcL[9],"Cycle ON");
  else sprintf(mtcL[9],"Cycle OFF");

  sprintf(mtcL[10],"Cycle OFF, Beam OFF");
  sprintf(mtcL[11],"Cycle OFF, Beam On, All LJ off");
  sprintf(mtcL[12],"Cycle OFF, MTC move");
  sprintf(mtcL[13],"Cycle OFF, Laser ON");
  sprintf(mtcL[14],"Data:Bkg  ratio  : %i",mtcptr->bkgRatio);
  sprintf(mtcL[15],"Data      cycles : %i",mtcptr->cyclesData);
  sprintf(mtcL[16],"Backgound cycles : %i",mtcptr->cyclesBkg);
  sprintf(mtcL[17],"Total     cycles : %i",(mtcptr->cyclesData + mtcptr->cyclesBkg));
  sprintf(mtcL[18],"Beam w/measure");
  sprintf(mtcL[19],"Mesaure w/beam ");
  sprintf(mtcL[20],"NO mtc fault  ");
  sprintf(mtcL[21],"NO mtc break ");
  sprintf(mtcL[22]," ");
  sprintf(mtcL[23]," ");
  sprintf(mtcL[24]," ");

  for (jj=0;jj<25;jj++){
    mtcLabel[jj] = gtk_label_new(mtcL[jj]);
    gtk_fixed_put(GTK_FIXED(fixed),mtcLabel[jj],25,20*(jj+1));
  }	   
  for (jj=0; jj<25;jj++){
    sprintf(mtcLon[jj],"  ");
    mtcLOF[jj] = gtk_label_new(mtcLon[jj]);
    gtk_fixed_put(GTK_FIXED(fixed),mtcLOF[jj],5,20*(jj+1));
  }
  
/*  */
  for (jj=0;jj<25;jj++){
    gtk_widget_show(mtcLOF[jj]);
    gtk_widget_show(mtcLabel[jj]);
  }

  return;
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
  fd = open(FILEPATH, O_RDWR, (mode_t)0600);
  if (fd == -1) {
    perror("Error opening file path for writing");
    exit(EXIT_FAILURE);
  }
        
  /* Now the file is ready to be mmapped.
   */
  mtcptr = (struct mtc_par*) mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (mtcptr == MAP_FAILED) {
    close(fd);
    perror("Error mmapping the file");
    exit(EXIT_FAILURE);
  }
  /* Don't forget to free the mmapped memory ... usually at end of main
   */
   
  return (fd);
}
