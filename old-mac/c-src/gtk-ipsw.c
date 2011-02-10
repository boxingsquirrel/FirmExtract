/* Test */
/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <plist/plist.h>
#include <gtk/gtk.h>
#include "file.h"
#include "ui.h"
#include "util_funcs.h"

#include "ipsw.h"

GtkWidget *window;
GtkWidget *file_choose;
GtkWidget *dir_choose;
GtkWidget *button1;

GtkWidget *fetch;
GtkWidget *fake_dev;
GtkWidget *fake_v;

GtkWidget *about_dlg;

char latest_plist[BUFSIZE];
char current_plist[BUFSIZE];
char img_plist[BUFSIZE];

char build_manifest[4096];

#define iPhone2G "iPhone1,1"

#define iPhone3G "iPhone1,2"

#define iPhone3GS "iPhone2,1"

#define iPhone4 "iPhone3,1"

#define ipt1g "iPod1,1"

#define ipt2g "iPod2,1"

#define ipt3g "iPod3,1"

#define iPad1G "iPad1,1"

static void destroy_win(GtkWidget *widget, gpointer data)
{
	if (widget==window)
	{
		gtk_main_quit();
	}
}

static void about_box()
{
	GtkBuilder *builder;
	GError* error = NULL;

	builder = gtk_builder_new ();
	if (!gtk_builder_add_from_file (builder, "res/about.xml", &error))
	{
		g_warning ("Couldn't load builder file: %s", error->message);
		g_error_free (error);
	}

	gtk_builder_connect_signals (builder, NULL);
	about_dlg = GTK_WIDGET (gtk_builder_get_object (builder, "aboutdialog1"));

	g_object_unref (builder);

	gtk_dialog_run (GTK_DIALOG(about_dlg));

	gtk_widget_destroy(about_dlg);
}

static void set_file(GtkWidget *widget, gpointer data)
{
	printf("Getting file...\n");
	ipsw=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_choose));
	printf("Selected file is %s\n", ipsw);
}
static void set_dir(GtkWidget *widget, gpointer data)
{
	printf("Getting directory...\n");
	out_dir=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dir_choose));
	printf("Selected directory is %s\n", out_dir);
}

static void toggle(GtkWidget *widget, gpointer data)
{
	gboolean real=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fetch));
	gboolean enable;

	if (real)
	{
		enable=FALSE;
	}

	else {
		enable=TRUE;
	}

		gtk_widget_set_sensitive(GTK_WIDGET(fake_dev), enable);
		gtk_widget_set_sensitive(GTK_WIDGET(fake_v), enable);
}

static void ipsw_extract_thread(GtkWidget *widget, gpointer data)
{
	gboolean real=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fetch));

	if (real)
	{
		ipsw_extract_all(0, NULL, NULL);
	}

	else {
		ipsw_extract_all(1, (char **)gtk_entry_get_text(GTK_ENTRY(fake_dev)), (char **)gtk_entry_get_text(GTK_ENTRY(fake_v)));
	}

	gtk_main_iteration();
}

int main(int argc, char *argv[])
{
	gtk_init (&argc, &argv);

	GtkBuilder *builder;
	GError* error = NULL;

	builder = gtk_builder_new ();
	char c[1024];
	getcwd(c, sizeof(c));
	char path[1024];
	snprintf(path, 1024, "%s/res/ui.xml", c);
	if (!gtk_builder_add_from_file (builder, (const char *)path, &error))
	{
		g_warning ("Couldn't load builder file: %s", error->message);
		g_error_free (error);
	}

	gtk_builder_connect_signals (builder, NULL);
	window = GTK_WIDGET (gtk_builder_get_object (builder, "window1"));
	file_choose=GTK_WIDGET(gtk_builder_get_object(builder, "file_choose"));
	dir_choose=GTK_WIDGET(gtk_builder_get_object(builder, "file_choose1"));
	button1=GTK_WIDGET(gtk_builder_get_object(builder, "button1"));
	fetch=GTK_WIDGET(gtk_builder_get_object(builder, "checkbutton1"));
	fake_dev=GTK_WIDGET(gtk_builder_get_object(builder, "entry1"));
	fake_v=GTK_WIDGET(gtk_builder_get_object(builder, "entry2"));

	g_signal_connect (G_OBJECT (file_choose), "file-set",
		      G_CALLBACK (set_file), NULL);
	g_signal_connect (G_OBJECT (dir_choose), "file-set",
		      G_CALLBACK (set_dir), NULL);
	g_signal_connect (G_OBJECT (button1), "released",
		      G_CALLBACK (ipsw_extract_thread), NULL);
	g_signal_connect (G_OBJECT (fetch), "toggled",
		      G_CALLBACK (toggle), NULL);
        g_signal_connect (G_OBJECT (window), "destroy",
		      G_CALLBACK (destroy_win), NULL);

	g_object_unref (builder);

	gtk_widget_show (window);

	//about_box();

	gtk_main ();

	return 0;
}

int read_in_plists()
{
	//r_file("/usr/local/share/iDeviceActivator/data/latest.plist");
	strcpy(latest_plist, data);

	//r_file("/usr/local/share/iDeviceActivator/data/img.plist");
	strcpy(img_plist, data);

	//r_file("/usr/local/share/iDeviceActivator/data/current.plist");
	strcpy(current_plist, data);

	return 0;
}*/
