
/*
 * CINELERRA
 * Copyright (C) 2008 Adam Williams <broadcast at earthling dot net>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */

#ifndef ASSETEDIT_H
#define ASSETEDIT_H

#include "asset.inc"
#include "awindow.inc"
#include "bcdialog.h"
#include "bitspopup.inc"
#include "browsebutton.h"
#include "formatpopup.h"
#include "guicast.h"
#include "indexable.inc"
#include "language.h"
#include "mwindow.h"
#include "resizetrackthread.inc"


class AssetEditByteOrderHILO;
class AssetEditByteOrderLOHI;
class AssetEditPath;
class AssetEditPathText;
class AssetEditWindow;

class DetailAssetWindow;
class DetailAssetThread;
class DetailAssetButton;

class AssetEdit : public BC_DialogThread
{
public:
	AssetEdit(MWindow *mwindow);
	~AssetEdit();
	
	void edit_asset(Indexable *indexable);
	int set_asset(Indexable *indexable);
	void handle_close_event(int result);
	BC_Window* new_gui();

	Indexable *indexable;
	MWindow *mwindow;
	AssetEditWindow *window;


// Changed parameters
	Asset *changed_params;
};



// Pcm is the only format users should be able to fix.
// All other formats display information about the file in read-only.

class AssetEditWindow : public BC_Window
{
public:
	AssetEditWindow(MWindow *mwindow, AssetEdit *asset_edit);
	~AssetEditWindow();

	void create_objects();
	AssetEditPathText *path_text;
	AssetEditPath *path_button;
	AssetEditByteOrderHILO *hilo;
	AssetEditByteOrderLOHI *lohi;
	BitsPopup *bitspopup;
	int allow_edits;
	MWindow *mwindow;
	AssetEdit *asset_edit;
	BC_Title *win_width;
	BC_Title *win_height;
	DetailAssetThread *detail_thread;
	void show_info_detail();
};


class AssetEditPath : public BrowseButton
{
public:
	AssetEditPath(MWindow *mwindow, 
		AssetEditWindow *fwindow, 
		BC_TextBox *textbox, 
		int y, 
		const char *text, 
		const char *window_title = _(PROGRAM_NAME " Path"), 
		const char *window_caption = _("Select a file"));
	~AssetEditPath();
	
	AssetEditWindow *fwindow;
};


class AssetEditPathText : public BC_TextBox
{
public:
	AssetEditPathText(AssetEditWindow *fwindow, int y);
	~AssetEditPathText();
	int handle_event();

	AssetEditWindow *fwindow;
};



class AssetEditFormat : public FormatPopup
{
public:
	AssetEditFormat(AssetEditWindow *fwindow, char* default_, int y);
	~AssetEditFormat();
	
	int handle_event();
	AssetEditWindow *fwindow;
};


class AssetEditChannels : public BC_TumbleTextBox
{
public:
	AssetEditChannels(AssetEditWindow *fwindow, char *text, int x, int y);
	
	int handle_event();
	
	AssetEditWindow *fwindow;
};

class AssetEditRate : public BC_TextBox
{
public:
	AssetEditRate(AssetEditWindow *fwindow, char *text, int x, int y);
	
	int handle_event();
	
	AssetEditWindow *fwindow;
};

class AssetEditFRate : public BC_TextBox
{
public:
	AssetEditFRate(AssetEditWindow *fwindow, char *text, int x, int y);
	
	int handle_event();
	
	AssetEditWindow *fwindow;
};

class AssetEditHeader : public BC_TextBox
{
public:
	AssetEditHeader(AssetEditWindow *fwindow, char *text, int x, int y);
	
	int handle_event();
	
	AssetEditWindow *fwindow;
};

class AssetEditByteOrderLOHI : public BC_Radial
{
public:
	AssetEditByteOrderLOHI(AssetEditWindow *fwindow, int value, int x, int y);
	
	int handle_event();
	
	AssetEditWindow *fwindow;
};

class AssetEditByteOrderHILO : public BC_Radial
{
public:
	AssetEditByteOrderHILO(AssetEditWindow *fwindow, int value, int x, int y);
	
	int handle_event();
	
	AssetEditWindow *fwindow;
};

class AssetEditSigned : public BC_CheckBox
{
public:
	AssetEditSigned(AssetEditWindow *fwindow, int value, int x, int y);
	
	int handle_event();
	
	AssetEditWindow *fwindow;
};

class DetailAssetWindow : public BC_Window
{
        MWindow *mwindow;
        Asset *asset;
	char info[65536];
	BC_ScrollTextBox *text;
public:
	DetailAssetWindow(MWindow *mwindow, Asset *asset);
	~DetailAssetWindow();
	void create_objects();
};

class DetailAssetThread : public Thread
{
	MWindow *mwindow;
	DetailAssetWindow *dwindow;
public:
	DetailAssetThread(MWindow *mwindow);
	~DetailAssetThread();

	void start(Asset *asset);
	void run();
};


class DetailAssetButton : public BC_GenericButton
{
public:
	DetailAssetButton(AssetEditWindow *fwindow, int x, int y);
	~DetailAssetButton();

	AssetEditWindow *fwindow;
	int handle_event();
};

#endif
