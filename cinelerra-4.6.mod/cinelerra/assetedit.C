
/*
 * CINELERRA
 * Copyright (C) 2010 Adam Williams <broadcast at earthling dot net>
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

#include "asset.h"
#include "assetedit.h"
#include "awindow.h"
#include "awindowgui.h"
#include "bcprogressbox.h"
#include "bcsignals.h"
#include "bitspopup.h"
#include "cache.h"
#include "clip.h"
#include "cplayback.h"
#include "cwindow.h"
#include "edl.h"
#include "edlsession.h"
#include "file.h"
#include "filempeg.h"
#include "fileffmpeg.h"
#include "filesystem.h"
#include "format.inc"
#include "indexable.h"
#include "indexfile.h"
#include "indexstate.h"
#include "language.h"
#include "mainindexes.h"
#include "mwindow.h"
#include "mwindowgui.h"
#include "new.h"
#include "preferences.h"
#include "resizetrackthread.h"
#include "removefile.h"
#include "theme.h"
#include "transportque.h"

#include <string.h>



AssetEdit::AssetEdit(MWindow *mwindow)
 : BC_DialogThread()
{
	this->mwindow = mwindow;
	indexable = 0;
	window = 0;
	changed_params = new Asset;
	//set_synchronous(0);
}


AssetEdit::~AssetEdit()
{
	changed_params->remove_user();
}


void AssetEdit::edit_asset(Indexable *indexable)
{
	if(this->indexable)
	{
		BC_DialogThread::close_window();
	}

	this->indexable = indexable;
	this->indexable->add_user();

// Copy asset parameters into temporary storage for editing.
	if(indexable->is_asset)
	{
		changed_params->copy_from((Asset*)indexable, 0);
	}
	else
	{
		EDL *nested_edl = (EDL*)indexable;

        strcpy(changed_params->path, nested_edl->path);
		changed_params->sample_rate = nested_edl->session->sample_rate;
        changed_params->channels = nested_edl->session->audio_channels;

//printf("AssetEdit::edit_asset %d %f\n", __LINE__, nested_edl->session->frame_rate);
		changed_params->frame_rate = nested_edl->session->frame_rate;
        changed_params->width = nested_edl->session->output_w;
        changed_params->height = nested_edl->session->output_h;
	}

	BC_DialogThread::start();
}

void AssetEdit::handle_close_event(int result)
{
 	if(!result)
 	{
		int changed = 0;
		Asset *asset = 0;
		EDL *nested_edl = 0;

		if(indexable->is_asset)
		{
			asset = (Asset*)indexable;
			if(!changed_params->equivalent(*asset, 1, 1))
				changed = 1;
		}
		else
		{
			nested_edl = (EDL*)indexable;
			if(strcmp(changed_params->path, nested_edl->path) 
//                ||
//                changed_params->sample_rate != nested_edl->session->sample_rate ||
//				!EQUIV(changed_params->frame_rate, nested_edl->session->frame_rate)
                )
				changed = 1;
		}
//printf("AssetEdit::handle_close_event %d\n", __LINE__);

 		if(changed)
 		{
			mwindow->gui->lock_window();
//printf("AssetEdit::handle_close_event %d\n", __LINE__);

// Omit index status from copy since an index rebuild may have been
// happening when new_asset was created but not be happening anymore.
			if(asset)
			{
	    		mwindow->remove_asset_from_caches(asset);
//printf("AssetEdit::handle_close_event %d %f\n", __LINE__, asset->get_frame_rate());
				asset->copy_from(changed_params, 0);
//printf("AssetEdit::handle_close_event %d %d %d\n", __LINE__, changed_params->bits, asset->bits);
			}
			else
			{
				strcpy(nested_edl->path, changed_params->path);
// Other parameters can't be changed because they're defined in the other EDL
//                nested_edl->session->frame_rate = changed_params->frame_rate;
//                nested_edl->session->sample_rate = changed_params->sample_rate;
			}
//printf("AssetEdit::handle_close_event %d\n", __LINE__);

			mwindow->gui->update(0,
				2,
				0,
				0,
				0, 
				0,
				0);
//printf("AssetEdit::handle_close_event %d\n", __LINE__);

// Start index rebuilding
			if( (asset && asset->audio_data) || nested_edl)
			{
				char source_filename[BCTEXTLEN];
				char index_filename[BCTEXTLEN];
				IndexFile::get_index_filename(source_filename, 
					mwindow->preferences->index_directory,
					index_filename, 
					indexable->path);
				remove_file(index_filename);
				indexable->index_state->index_status = INDEX_NOTTESTED;
				mwindow->mainindexes->add_next_asset(0, indexable);
				mwindow->mainindexes->start_build();
			}
			mwindow->gui->unlock_window();
//printf("AssetEdit::handle_close_event %d\n", __LINE__);


			mwindow->awindow->gui->lock_window();
			mwindow->awindow->gui->update_assets();
			mwindow->awindow->gui->unlock_window();
//printf("AssetEdit::handle_close_event %d\n", __LINE__);

			mwindow->restart_brender();
			mwindow->sync_parameters(CHANGE_ALL);
//printf("AssetEdit::handle_close_event %d\n", __LINE__);
 		}
 	}

	this->indexable->remove_user();
	this->indexable = 0;
//printf("AssetEdit::handle_close_event %d\n", __LINE__);
}

BC_Window* AssetEdit::new_gui()
{
	window = new AssetEditWindow(mwindow, this);
	window->create_objects();
	return window;
}









AssetEditWindow::AssetEditWindow(MWindow *mwindow, AssetEdit *asset_edit)
 : BC_Window(PROGRAM_NAME ": Asset Info", 
 	mwindow->gui->get_abs_cursor_x(1) - 400 / 2, 
	mwindow->gui->get_abs_cursor_y(1) - 500 / 2, 
	400, 
	530,
	400,
	530,
	0,
	0,
	1)
{
	this->mwindow = mwindow;
	this->asset_edit = asset_edit;
	bitspopup = 0;
	path_text = 0;
	path_button = 0;
	hilo = 0;
	lohi = 0;
	allow_edits = 0;
	detail_thread = 0;
}





AssetEditWindow::~AssetEditWindow()
{
	lock_window("AssetEditWindow::~AssetEditWindow");
	if(bitspopup) delete bitspopup;
	if(detail_thread) delete detail_thread;
	unlock_window();
}




void AssetEditWindow::create_objects()
{
	int y = 10, x = 10, x1 = 10, x2 = 150;
	char string[BCTEXTLEN];
	int vmargin;
	FileSystem fs;
	BC_Title *title;
	Asset *asset = 0;
	EDL *nested_edl = 0;

	if(asset_edit->indexable->is_asset)
		asset = (Asset*)asset_edit->indexable;
	else
		nested_edl = (EDL*)asset_edit->indexable;

	if(asset && asset->format == FILE_PCM)
		allow_edits = 1;
	else
		allow_edits = 0;

	lock_window("AssetEditWindow::create_objects");
	if(allow_edits) 
		vmargin = 30;
	else
		vmargin = 20;

	add_subwindow(path_text = new AssetEditPathText(this, y));
	add_subwindow(path_button = new AssetEditPath(mwindow, 
		this, 
		path_text, 
		y, 
		asset_edit->indexable->path, 
		PROGRAM_NAME ": Asset path", _("Select a file for this asset:")));
	y += 30;

	if(asset)
	{
		add_subwindow(new BC_Title(x, y, _("File format:")));
		x = x2;
		add_subwindow(new BC_Title(x, y, File::formattostr(mwindow->plugindb, 
				asset->format), 
			MEDIUMFONT, 
			mwindow->theme->assetedit_color));
		x = x1;
		y += 20;

		int64_t bytes = fs.get_size(asset->path);
		add_subwindow(new BC_Title(x, y, _("Bytes:")));
		sprintf(string, "" _LD "", bytes);
		Units::punctuate(string);
	

		add_subwindow(new BC_Title(x2, y, string, MEDIUMFONT, mwindow->theme->assetedit_color));
		if(asset->format == FILE_MPEG || asset->format == FILE_FFMPEG)
		{
			detail_thread = new DetailAssetThread(mwindow);
			BC_GenericButton *detail = new DetailAssetButton(this, x2+120, y);
			add_subwindow(detail);
		}

		y += 20;
		x = x1;

		double length = 0.;
		y += 20;
		x = x1;

		if(asset->audio_length > 0)
			length = (double)asset->audio_length / asset->sample_rate;
		if(asset->video_length > 0)
			length = MAX(length, (double)asset->video_length / asset->frame_rate);
		int64_t bitrate;
		if(!EQUIV(length, 0))
			bitrate = (int64_t)(bytes * 8 / length);
		else
			bitrate = bytes;
		add_subwindow(new BC_Title(x, y, _("Bitrate (bits/sec):")));
		sprintf(string, "" _LD "", bitrate);

		Units::punctuate(string);
		add_subwindow(new BC_Title(x2, y, string, MEDIUMFONT, mwindow->theme->assetedit_color));

		y += 30;
		x = x1;
	}

	if((asset && asset->audio_data) || nested_edl)
	{
		add_subwindow(new BC_Bar(x, y, get_w() - x * 2));
		y += 5;

		add_subwindow(new BC_Title(x, y, _("Audio:"), LARGEFONT, RED));

		y += 30;

		if(asset)
		{
			if(asset->get_compression_text(1, 0))
			{
				add_subwindow(new BC_Title(x, y, _("Compression:")));
				x = x2;
				add_subwindow(new BC_Title(x, 
					y, 
					asset->get_compression_text(1, 0), 
					MEDIUMFONT, 
					mwindow->theme->assetedit_color));
				y += vmargin;
				x = x1;
			}
		}

		add_subwindow(new BC_Title(x, y, _("Channels:")));
		sprintf(string, "%d", asset_edit->changed_params->channels);

		x = x2;
		if(allow_edits)
		{
			BC_TumbleTextBox *textbox = new AssetEditChannels(this, 
				string, 
				x, 
				y);
			textbox->create_objects();
			y += vmargin;
		}
		else
		{
			add_subwindow(new BC_Title(x, y, string, MEDIUMFONT, mwindow->theme->assetedit_color));
			y += 20;
		}

		x = x1;
		add_subwindow(new BC_Title(x, y, _("Sample rate:")));
		sprintf(string, "%d", asset_edit->changed_params->sample_rate);

		x = x2;
		if(asset)
		{
			BC_TextBox *textbox;
			add_subwindow(textbox = new AssetEditRate(this, string, x, y));
			x += textbox->get_w();
			add_subwindow(new SampleRatePulldown(mwindow, textbox, x, y));
		}
		else
		{
			add_subwindow(new BC_Title(x, y, string, MEDIUMFONT, mwindow->theme->assetedit_color));
		}

		y += 30;
		x = x1;

		if(asset)
		{
			add_subwindow(new BC_Title(x, y, _("Bits:")));
			x = x2;
			if(allow_edits)
			{
				bitspopup = new BitsPopup(this, 
					x, 
					y, 
					&asset_edit->changed_params->bits, 
					1, 
					1, 
					1,
					0,
					1);
				bitspopup->create_objects();
			}
			else
				add_subwindow(new BC_Title(x, y, File::bitstostr(asset->bits), MEDIUMFONT, mwindow->theme->assetedit_color));


			x = x1;
			y += vmargin;
			add_subwindow(new BC_Title(x, y, _("Header length:")));
			sprintf(string, "%d", asset->header);

			x = x2;
			if(allow_edits)
				add_subwindow(new AssetEditHeader(this, string, x, y));
			else
				add_subwindow(new BC_Title(x, y, string, MEDIUMFONT, mwindow->theme->assetedit_color));

			y += vmargin;
			x = x1;

			add_subwindow(new BC_Title(x, y, _("Byte order:")));

			if(allow_edits)
			{
				x = x2;

				add_subwindow(lohi = new AssetEditByteOrderLOHI(this, 
					asset->byte_order, 
					x, 
					y));
				x += 70;
				add_subwindow(hilo = new AssetEditByteOrderHILO(this, 
					!asset->byte_order, 
					x, 
					y));
				y += vmargin;
			}
			else
			{
				x = x2;
				if(asset->byte_order)
					add_subwindow(new BC_Title(x, y, _("Lo-Hi"), MEDIUMFONT, mwindow->theme->assetedit_color));
				else
					add_subwindow(new BC_Title(x, y, _("Hi-Lo"), MEDIUMFONT, mwindow->theme->assetedit_color));
				y += vmargin;
			}


			x = x1;
			if(allow_edits)
			{
	//			add_subwindow(new BC_Title(x, y, _("Values are signed:")));
				add_subwindow(new AssetEditSigned(this, asset->signed_, x, y));
			}
			else
			{
				if(!asset->signed_ && asset->bits == 8)
					add_subwindow(new BC_Title(x, y, _("Values are unsigned")));
				else
					add_subwindow(new BC_Title(x, y, _("Values are signed")));
			}

			y += 30;
		}
	}

	x = x1;
	if((asset && asset->video_data) || nested_edl)
	{
		add_subwindow(new BC_Bar(x, y, get_w() - x * 2));
		y += 5;

		add_subwindow(new BC_Title(x, y, _("Video:"), LARGEFONT, RED));
		y += 30;
		x = x1;


		if(asset && asset->get_compression_text(0,1))
		{
			add_subwindow(new BC_Title(x, y, _("Compression:")));
			x = x2;
			add_subwindow(new BC_Title(x, 
				y, 
				asset->get_compression_text(0,1), 
				MEDIUMFONT, 
				mwindow->theme->assetedit_color));
			y += vmargin;
			x = x1;
		}

		add_subwindow(new BC_Title(x, y, _("Frame rate:")));
		x = x2;
		sprintf(string, "%.2f", asset_edit->changed_params->frame_rate);

//printf("AssetEditWindow::create_objects %d %f\n", __LINE__, asset_edit->changed_params->frame_rate);
		if(asset)
		{
			BC_TextBox *framerate;
			add_subwindow(framerate = new AssetEditFRate(this, string, x, y));
			x += 105;
			add_subwindow(new FrameRatePulldown(mwindow, framerate, x, y));
		}
		else
		{
			add_subwindow(new BC_Title(x, y, string, MEDIUMFONT, mwindow->theme->assetedit_color));
		}

		y += 30;
		x = x1;
		add_subwindow(new BC_Title(x, y, _("Width:")));
		x = x2;
		sprintf(string, "%d", asset_edit->changed_params->width);
		win_width = new BC_Title(x, y, string, MEDIUMFONT, mwindow->theme->assetedit_color);
		add_subwindow(win_width);

		y += vmargin;
		x = x1;
		add_subwindow(new BC_Title(x, y, _("Height:")));
		x = x2;
		sprintf(string, "%d", asset_edit->changed_params->height);
		win_height = new BC_Title(x, y, string, MEDIUMFONT, mwindow->theme->assetedit_color);
		add_subwindow(win_height);
		y += win_height->get_h() + 5;

		if(asset && (asset->format == FILE_MPEG || asset->format == FILE_FFMPEG ) )
		{
			y += 5;
			x = x1;
			add_subwindow(new BC_Title(x, y, _("Actual width:")));
			x = x2;
			sprintf(string, "%d", asset->actual_width);
			add_subwindow(new BC_Title(x, y, string, MEDIUMFONT, mwindow->theme->assetedit_color));

			BC_GenericButton *resize = new ResizeAssetButton(this, x+64, y);
			add_subwindow(resize);

			y += vmargin;
			x = x1;
			add_subwindow(new BC_Title(x, y, _("Actual height:")));
			x = x2;
			sprintf(string, "%d", asset->actual_height);
			title = new BC_Title(x, y, string, MEDIUMFONT, mwindow->theme->assetedit_color);
			add_subwindow(title);
			y += title->get_h() + 5;
		}
	}

	add_subwindow(new BC_OKButton(this));
	add_subwindow(new BC_CancelButton(this));
	show_window();
	unlock_window();
}

void AssetEditWindow::show_info_detail()
{
	detail_thread->start((Asset*)asset_edit->indexable);
}


AssetEditChannels::AssetEditChannels(AssetEditWindow *fwindow, 
	char *text, 
	int x,
	int y)
 : BC_TumbleTextBox(fwindow, 
		(int)atol(text),
		(int)1,
		(int)MAXCHANNELS,
		x, 
		y, 
		50)
{
	this->fwindow = fwindow;
}

int AssetEditChannels::handle_event()
{
	Asset *asset = (Asset*)fwindow->asset_edit->changed_params;
	asset->channels = atol(get_text());
	return 1;
}

AssetEditRate::AssetEditRate(AssetEditWindow *fwindow, char *text, int x, int y)
 : BC_TextBox(x, y, 100, 1, text)
{
	this->fwindow = fwindow;
}

int AssetEditRate::handle_event()
{
	Asset *asset = (Asset*)fwindow->asset_edit->changed_params;
	asset->sample_rate = atol(get_text());
	return 1;
}

AssetEditFRate::AssetEditFRate(AssetEditWindow *fwindow, char *text, int x, int y)
 : BC_TextBox(x, y, 100, 1, text)
{
	this->fwindow = fwindow;
}

int AssetEditFRate::handle_event()
{
	Asset *asset = (Asset*)fwindow->asset_edit->changed_params;
	asset->frame_rate = atof(get_text());
	return 1;
}

AssetEditHeader::AssetEditHeader(AssetEditWindow *fwindow, char *text, int x, int y)
 : BC_TextBox(x, y, 100, 1, text)
{
	this->fwindow = fwindow;
}

int AssetEditHeader::handle_event()
{
	Asset *asset = (Asset*)fwindow->asset_edit->changed_params;
	asset->header = atol(get_text());
	return 1;
}

AssetEditByteOrderLOHI::AssetEditByteOrderLOHI(AssetEditWindow *fwindow, 
	int value, 
	int x,
	int y)
 : BC_Radial(x, y, value, _("Lo-Hi"))
{
	this->fwindow = fwindow;
}

int AssetEditByteOrderLOHI::handle_event()
{
	Asset *asset = (Asset*)fwindow->asset_edit->changed_params;
	asset->byte_order = 1;
	fwindow->hilo->update(0);
	update(1);
	return 1;
}

AssetEditByteOrderHILO::AssetEditByteOrderHILO(AssetEditWindow *fwindow, 
	int value, 
	int x, 
	int y)
 : BC_Radial(x, y, value, _("Hi-Lo"))
{
	this->fwindow = fwindow;
}

int AssetEditByteOrderHILO::handle_event()
{
	Asset *asset = (Asset*)fwindow->asset_edit->changed_params;
	asset->byte_order = 0;
	fwindow->lohi->update(0);
	update(1);
	return 1;
}

AssetEditSigned::AssetEditSigned(AssetEditWindow *fwindow, 
	int value, 
	int x, 
	int y)
 : BC_CheckBox(x, y, value, _("Values are signed"))
{
	this->fwindow = fwindow;
}

int AssetEditSigned::handle_event()
{
	Asset *asset = (Asset*)fwindow->asset_edit->changed_params;
	asset->signed_ = get_value();
	return 1;
}







AssetEditPathText::AssetEditPathText(AssetEditWindow *fwindow, int y)
 : BC_TextBox(5, y, 300, 1, fwindow->asset_edit->changed_params->path) 
{
	this->fwindow = fwindow; 
}
AssetEditPathText::~AssetEditPathText() 
{
}
int AssetEditPathText::handle_event() 
{
	strcpy(fwindow->asset_edit->changed_params->path, get_text());
	return 1;
}

AssetEditPath::AssetEditPath(MWindow *mwindow, 
	AssetEditWindow *fwindow, 
	BC_TextBox *textbox, 
	int y, 
	const char *text, 
	const char *window_title, 
	const char *window_caption)
 : BrowseButton(mwindow, 
 	fwindow, 
	textbox, 
	310, 
	y, 
	text, 
	window_title, 
	window_caption, 
	0) 
{ 
	this->fwindow = fwindow; 
}
AssetEditPath::~AssetEditPath() {}






AssetEditFormat::AssetEditFormat(AssetEditWindow *fwindow, char* default_, int y)
 : FormatPopup(fwindow->mwindow->plugindb, 90, y)
{ 
	this->fwindow = fwindow; 
}
AssetEditFormat::~AssetEditFormat() 
{
}
int AssetEditFormat::handle_event()
{
	Asset *asset = (Asset*)fwindow->asset_edit->changed_params;
	asset->format = File::strtoformat(fwindow->mwindow->plugindb, 
		get_selection(0, 0)->get_text());
	return 1;
}




DetailAssetButton::DetailAssetButton(AssetEditWindow *fwindow, int x, int y)
 : BC_GenericButton(x, y, _("Detail"))
{
        this->fwindow = fwindow;
        set_underline(0);
}

DetailAssetButton::~DetailAssetButton()
{
}

int DetailAssetButton::handle_event()
{
	fwindow->show_info_detail();
        return 1;
}

DetailAssetWindow::DetailAssetWindow(MWindow *mwindow, Asset *asset)
 : BC_Window("Asset Detail",
        mwindow->gui->get_abs_cursor_x(1) - 600/2,
        mwindow->gui->get_abs_cursor_y(1) - 500/2,
        600, 500)
{
	this->mwindow = mwindow;
	this->asset = asset;
	asset->add_user();
	info[0] = 0;
	text = 0;
}

DetailAssetWindow::~DetailAssetWindow()
{
	asset->remove_user();
	delete text;
}

DetailAssetThread::DetailAssetThread(MWindow *mwindow)
 : Thread(1, 0, 0)
{
	this->mwindow = mwindow;
	dwindow = 0;
}

DetailAssetThread::~DetailAssetThread()
{
	if( Thread::running() ) {
		dwindow->lock_window("DetailAssetThread::~DetailAssetThread");
		dwindow->set_done(1);
		dwindow->unlock_window();
		Thread::join();
	}
}

void DetailAssetWindow::create_objects()
{
	int y = 10, x = 10;
	char file_name[BCTEXTLEN];
	int len = sizeof(info);
	strncpy(info,"no info available",len);
	if( !mwindow->preferences->get_asset_file_path(asset, file_name) ) {
		switch( asset->format ) {
		case FILE_MPEG:
			FileMPEG::get_info(asset->path, file_name, &info[0]);
			break;
		case FILE_FFMPEG:
			FileFFMPEG::get_info(asset->path, &info[0]);
			break;
		}
	}
	lock_window("DetailAssetThread::create_objects");
	text = new BC_ScrollTextBox(this, x, y, get_w()-32, 23, info, -len);
	text->create_objects();  text->set_text_row(0);
	add_subwindow(new BC_OKButton(this));
	show_window();
	unlock_window();
}

void DetailAssetThread::start(Asset *asset)
{
	if( !Thread::running() ) {
		delete dwindow;
		dwindow = new DetailAssetWindow(mwindow, asset);
		Thread::start();
	}
}

void DetailAssetThread::run()
{
// Get length from TOC
	dwindow->create_objects();
	dwindow->run_window();
	delete dwindow;
	dwindow = 0;
}


