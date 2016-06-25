#include <cairo.h>
#include <gtk/gtk.h>
#include <time.h>
#include <string.h>

// compiles under gtk3:  gcc -Wall `pkg-config --cflags --libs gtk+-3.0` -o test3 test3.c
static gboolean time_handler(GtkWidget *widget);
static gboolean on_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);
void kick(GtkWidget *kicker);
static char buffer[256];
static char stat[256];

time_t time0 = 0, curtime=0;
  GtkWidget *fixed;
  GtkWidget *window;

int main (int argc, char *argv[]){

  GtkWidget *darea;
  GtkWidget *kicker;
  GtkWidget *button;
  int ii = 0;

  gtk_init(&argc, &argv);

  if (ii == 0){
    ii=1;
    time0 = time(NULL);
  }


  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(window), 350, 400);

  //  darea = gtk_drawing_area_new();
  //  gtk_container_add(GTK_CONTAINER (window), darea);

  fixed = gtk_fixed_new();
  kick(fixed);
  gtk_container_add(GTK_CONTAINER (window), fixed);
  kick(fixed);
   gtk_fixed_put(GTK_FIXED(window),fixed,100,50);

  //  gtk_container_add(GTK_CONTAINER (fixed), kicker); 
  
  //  g_signal_connect(darea, "draw", G_CALLBACK(on_expose_event), NULL);
  g_signal_connect(fixed, "draw", G_CALLBACK(on_expose_event), NULL);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 170, 100);

  gtk_window_set_title(GTK_WINDOW(window), "Controls status");

  g_timeout_add(1000, (GSourceFunc) time_handler, (gpointer) window);
  gtk_widget_show_all(window);
  time_handler(window);

  gtk_main();

  return 0;
}

static gboolean on_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
  cairo_t *cr;

  //  cr = gdk_cairo_create(gtk_widget_get_window(widget));
  //  cairo_move_to(cr, 30, 30);
  //  cairo_show_text(cr, buffer);
  // cairo_show_text(cr, stat);

  //  cairo_destroy(cr);
  kick(fixed);
  return FALSE;
}

static gboolean time_handler(GtkWidget *widget) {
  int ii=0;
  time_t curtime;
  struct tm *loctime;

  if (gtk_widget_get_window(widget) == NULL) return FALSE;
  curtime = time(NULL);
  loctime = localtime(&curtime);
  strftime(buffer, 256, "%T", loctime);
  ii= (int) (curtime-time0);
  if ((ii > 5)  && (ii < 10)) strcpy(stat,"\nMTC on");
  else  strcpy(stat,"        ");
  gtk_widget_queue_draw(widget);
  return TRUE;
}


void kick(GtkWidget *kicker){
  //  GtkWidget *window;
  GtkWidget *kickU3;
  GtkWidget *takeAway;
  GtkWidget *normal;
  GtkWidget *pause;
  GtkWidget *beamON;
  GtkWidget *beamOFF;
  GtkWidget *liteON;
  GtkWidget *mtcON;
  GtkWidget *vbox;
  time_t curtime;
  int ii=0;
  
  kickU3 = gtk_label_new("Kick-u3 status");
  takeAway = gtk_label_new("TakeAway Mode");
  normal = gtk_label_new("Normal Mode");
  pause = gtk_label_new("Pause On");
  beamON = gtk_label_new("Beam On");
  beamOFF = gtk_label_new("Beam Off");
  liteON = gtk_label_new("Laser On");
  mtcON = gtk_label_new("MTC On");

  //  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
  //  gtk_container_add(GTK_CONTAINER(window), label);
  //  gtk_container_add(GTK_CONTAINER(window), takeAway);
  curtime=time(NULL);
  vbox = gtk_box_new(TRUE, 1);
  gtk_container_add(GTK_CONTAINER(kicker), vbox);
  //  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_box_pack_start(GTK_BOX(vbox), kickU3, TRUE, TRUE, 0);

  ii = (int)(curtime - time0);
  printf(" %i  %i   %i\n", ii,(int) curtime, (int)time0);
  gtk_box_pack_start(GTK_BOX(vbox), normal, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), pause, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), beamON, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), beamOFF, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), liteON, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), mtcON, TRUE, TRUE, 0);
  if (ii > 10) gtk_box_pack_start(GTK_BOX(vbox), takeAway, TRUE, TRUE, 0);
  else gtk_box_pack_start(GTK_BOX(vbox), kickU3, TRUE, TRUE, 0);
  gtk_widget_show(takeAway);

  return;

}
