/*  Canon Inkjet Printer Driver for Linux
 *  Copyright CANON INC. 2001-2008
 *  All Rights Reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * NOTE:
 *  - As a special exception, this program is permissible to link with the
 *    libraries released as the binary modules.
 *  - If you write modifications of your own for these programs, it is your
 *    choice whether to permit this exception to apply to your modifications.
 *    If you do not wish that, delete this exception.
 */


/*
 * Initial main.c file generated by Glade. Edit as required.
 * Glade will not overwrite this file.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef ENABLE_NLS
#  include <libintl.h>
#endif

#include <gtk/gtk.h>
#ifdef	USE_LIB_GLADE
#	include <glade/glade.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <popt.h>

#include "callbacks.h"
#ifndef	USE_LIB_GLADE
#	include "interface.h"
#	include "support.h"
#endif


#define	DEFINE_GLOBALS
#include "bjuidefs.h"


void SetGtkResourceFile();
void InitOption(int argc, char* argv[]);
void FreeOption();
void CreateDialogs();
void DisposeDialogs();
void ConnectSignalHandlers();
void InitPrintingType();
void ShowModelDependWidgets();


int main(int argc, char *argv[])
{
	int ret = 1;

	g_main_window=NULL;	/* for UpdateMenuLink (check "first call") */
	g_quality_dialog = NULL;	/* for UpdateMenuLink (check "first call") */

	bindtextdomain(PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset( PACKAGE, "UTF-8" );
	textdomain(PACKAGE);

	gtk_set_locale();

	// Set gtk resource file.
	SetGtkResourceFile();

	gtk_init(&argc, &argv);

#ifdef	USE_LIB_GLADE
	// Initialize the glade library.
	glade_init();
#endif

	// Parse options.
	InitOption(argc, argv);

	// Set the directory name for storing keytext files.
	SetKeyTextDir(PACKAGE_DATA_DIR);

#ifndef	USE_LIB_GLADE
	// Set the directory name for storing xpm files.
	SetPixmapDir(PACKAGE_DATA_DIR);
#endif

	// Load fontset.
	//g_main_font = LoadFontSet();
	g_main_font = NULL;

	// Initialize signal control level.
	InitSignal();

	// Load the glade and keytext file.
	if( LoadResources() )
	{
		// Initialize databases.
		InitDataBase(g_model_name);

		// Initialize printint type table.
		InitPrintingType();

		// Create main dialog, and realize it.
		g_main_window = CreateMainWindow();
		gtk_widget_realize(UI_DIALOG(g_main_window)->window);

		// Create dialogs.
		CreateDialogs();

		// Connect signal handlers.
		ConnectSignalHandlers();

		// Update widgets.
		DisableSignal();
		UpdateWidgets(UI_DIALOG(g_main_window)->window, NULL);
		EnableSignal();

		// Set window title.
		snprintf(g_window_title, sizeof(g_window_title), "Canon %s", GetDispModelName());	//Ver.2.90(s)

		gtk_window_set_title(
			GTK_WINDOW(UI_DIALOG(g_main_window)->window), g_window_title);

		// Show widgets depend on model.
		ShowModelDependWidgets(g_main_window);

		gtk_widget_show(UI_DIALOG(g_main_window)->window);
		gtk_main();

		ret = 0;

		// Dispose dialogs.
		DisposeDialogs();

		// Dispose main dialog.
		DisposeDialog(UI_DIALOG(g_main_window));

		// Free databases.
		FreeDataBase();

		// Free resources.
		FreeResources();
	}
	// Free Gdk resource.
	if( g_main_font )
		gdk_font_unref(g_main_font);

	// Free option strings.
	FreeOption();

	return ret;
}

void SetGtkResourceFile()
{
	// Get gtk resources.
	const gchar* home_dir = NULL;
	gchar* rc_path = NULL;

	home_dir = g_get_home_dir();
	rc_path = g_strdup_printf("%s/.gtkrc", home_dir);
	gtk_rc_add_default_file(rc_path);
	g_free(rc_path);
}

#define	UI_OPT_SOCKET	1
#define	UI_OPT_MODEL	2
#define	UI_OPT_CUPS		4
#define	UI_OPT_INTVER	5

void InitOption(int argc, char* argv[])
{
	static struct poptOption option_table[] =
	{
		{NULL,             's', POPT_ARG_STRING,  NULL,  UI_OPT_SOCKET },
		{"model",           0,  POPT_ARG_STRING,  NULL,  UI_OPT_MODEL  },
		{"cups",            0,  0,                NULL,  UI_OPT_CUPS   },
		{"internalversion", 0,  POPT_ARG_NONE,    NULL,  UI_OPT_INTVER },	/* Ver.2.70 */
		{NULL,              0,  0,                NULL,  0             }
	};

	poptContext context
		 = poptGetContext("printui", argc, (const char**)argv, option_table, 0);
	int rc;

	g_model_name = NULL;
	g_cups_mode = FALSE;

	while( (rc = poptGetNextOpt(context)) > 0 )
	{
		switch( rc )
		{
		case UI_OPT_SOCKET:
			g_socketname = g_strdup(poptGetOptArg(context));
			break;

		case UI_OPT_MODEL:
			g_model_name = g_strdup(poptGetOptArg(context));
			break;

		case UI_OPT_CUPS:
			g_cups_mode = TRUE;
			break;

		case UI_OPT_INTVER:
			g_message( "Internal Version : %s\n" ,INTERNAL_VERSION_STR );
			break;

		default:
			break;
		}
	}
	
	if( rc < -1 )
		g_warning("Unknown switch, %s\n",
			 poptBadOption(context, POPT_BADOPTION_NOALIAS));
}

void FreeOption()
{
	if( g_model_name != NULL )
		g_free(g_model_name);
	if( g_socketname != NULL )
		g_free(g_socketname);
}

void CreateDialogs()
{
	// Create quality dialog.
	g_quality_dialog = CreateQualityDialog(UI_DIALOG(g_main_window));

	// Create color adjustment dialog.
	g_color_dialog = CreateColorDialog(UI_DIALOG(g_main_window));
	g_color_dialog2 = CreateColorDialog2(UI_DIALOG(g_main_window));

	// Create user size dialog.
	g_user_size_dialog
		= CreateUserSizeDialog(UI_DIALOG(g_main_window), g_unit_inch);

	// Create version dialog.
	g_version_dialog = CreateVersionDialog(UI_DIALOG(g_main_window));

	// Create Media & Size dialog.
	g_mediasize_illegal_dialog
		= CreateMediaSizeDialog(UI_DIALOG(g_main_window),
			UI_MEDIASIZE_DLG_ILLEGAL);
	g_mediasize_recommend_dialog
		= CreateMediaSizeDialog(UI_DIALOG(g_main_window),
			UI_MEDIASIZE_DLG_RECOMMEND);
	g_mediasize_illegal_select_dialog
		= CreateMediaSizeDialog(UI_DIALOG(g_main_window),
			UI_MEDIASIZE_DLG_ILLEGAL_SELECT);

	// Create media type dialog.
	g_mediatype_dialog = CreateMediaTypeDialog(UI_DIALOG(g_main_window));

	// Create media border dialog.
	g_mediaborder_dialog = CreateMediaBorderDialog(UI_DIALOG(g_main_window));

}

void DisposeDialogs()
{
	// Dispose quality dialog.
	DisposeDialog((UIDialog*)g_quality_dialog);

	// Dispose color adjustment dialog.
	DisposeDialog((UIDialog*)g_color_dialog);

	// Dispose user size dialog.
	DisposeDialog((UIDialog*)g_user_size_dialog);

	// Dispose version dialog.
	DisposeDialog((UIDialog*)g_version_dialog);

	// Dispose Media & Size dialog.
	DisposeDialog((UIDialog*)g_mediasize_illegal_dialog);
	DisposeDialog((UIDialog*)g_mediasize_recommend_dialog);
	DisposeDialog((UIDialog*)g_mediasize_illegal_select_dialog);

	// Dispose media type dialog.
	DisposeMediaTypeDialog(g_mediatype_dialog);

	// Dispose media border dialog.
	DisposeMediaBorderDialog(g_mediaborder_dialog);

}

