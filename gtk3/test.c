/*  Compile With: 
*   gcc -Wall -o cursor1 `pkg-config --cflags --libs gtk+-3.0` cursor1.c
*/
#include <gtk/gtk.h> 

int main (int argc, char* argv[])
{
GtkWidget *window, *tview;
GtkTextBuffer *tbuf;

gtk_init (&argc, &argv);

/*-------- CSS ------------------------------------------------------------------------------------------------------------------------------------------*/
GtkCssProvider *provider = gtk_css_provider_new ();

gtk_css_provider_load_from_data (provider,
                                 "GtkTextView {\n"
                                 "color: blue;\n"
                                 "font: Serif 38;\n"
                                 "background-color: yellow;\n"
                                 "-GtkWidget-cursor-color: red;\n"
                                 "}\n"
                                 "GtkTextView:selected {\n"
                                 "background-color: black;\n"
                                 "color: green;\n"
                                 "}\n", -1, NULL);

GdkDisplay *display = gdk_display_get_default ();
GdkScreen *screen = gdk_display_get_default_screen (display);

gtk_style_context_add_provider_for_screen (screen,
                                           GTK_STYLE_PROVIDER (provider),
                                           GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

g_object_unref (provider);
/*--------------------------------------------------------------------------------------------------------------------------------------------------*/

window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

gtk_window_set_default_size (GTK_WINDOW(window),
                             500, 250);

g_signal_connect_swapped(G_OBJECT(window),
                         "destroy",
                         G_CALLBACK(gtk_main_quit),
                         NULL);

tbuf = gtk_text_buffer_new (NULL);

tview = gtk_text_view_new_with_buffer (tbuf);

gtk_container_add (GTK_CONTAINER(window),
                   tview);

gtk_widget_show_all (window);
gtk_main ();
return(0);
}
