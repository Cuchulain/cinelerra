
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

#if 0




#include "edl.h"
#include "keyframe.h"
#include "keys.h"
#include "language.h"
#include "localsession.h"
#include "mainsession.h"
#include "mainundo.h"
#include "mwindow.h"
#include "mwindowgui.h"
#include "plugin.h"
#include "presets.h"
#include "presetsgui.h"
#include "theme.h"
#include "trackcanvas.h"
#include "tracks.h"











PresetsThread::PresetsThread(MWindow *mwindow)
 : BC_DialogThread()
{
	this->mwindow = mwindow;
	plugin = 0;
	data = new ArrayList<BC_ListBoxItem*>;
	presets_db = new PresetsDB;
	plugin_title[0] = 0;
	window_title[0] = 0;
}

PresetsThread::~PresetsThread()
{
	delete data;
}

void PresetsThread::calculate_list()
{
	data->remove_all_objects();
	int total_presets = presets_db->get_total_presets(plugin_title);
	for(int i = 0; i < total_presets; i++)
	{
		data->append(new BC_ListBoxItem(presets_db->get_preset_title(
			plugin_title,
			i)));
	}
}


void PresetsThread::start_window(Plugin *plugin)
{
	if(!BC_DialogThread::is_running())
	{
		this->plugin = plugin;
		plugin->calculate_title(plugin_title, 0);
		sprintf(window_title, PROGRAM_NAME ": %s Presets", plugin_title);


// Calculate database
		presets_db->load();
		calculate_list();


		mwindow->gui->unlock_window();
		BC_DialogThread::start();
		mwindow->gui->lock_window("PresetsThread::start_window");
	}
}

BC_Window* PresetsThread::new_gui()
{
	mwindow->gui->lock_window("PresetsThread::new_gui");
	int x = mwindow->gui->get_abs_cursor_x(0) - 
		mwindow->session->plugindialog_w / 2;
	int y = mwindow->gui->get_abs_cursor_y(0) - 
		mwindow->session->plugindialog_h / 2;

	PresetsWindow *window = new PresetsWindow(mwindow, 
		this, 
		x, 
		y,
		window_title);

	window->create_objects();
	mwindow->gui->unlock_window();
	return window;
}

void PresetsThread::handle_done_event(int result)
{
// Apply the preset
	if(!result)
	{
		char *title = ((PresetsWindow*)get_gui())->title_text->get_text();
		apply_preset(title);
	}
}

void PresetsThread::handle_close_event(int result)
{
}

void PresetsThread::save_preset(char *title)
{
	get_gui()->unlock_window();
	mwindow->gui->lock_window("PresetsThread::save_preset");
	
// Test EDL for plugin existence
	if(!mwindow->edl->tracks->plugin_exists(plugin))
	{
		mwindow->gui->unlock_window();
		get_gui()->lock_window("PresetsThread::save_preset 2");
		return;
	}


// Get current plugin keyframe
	EDL *edl = mwindow->edl;
	Track *track = plugin->track;
	KeyFrame *keyframe = plugin->get_prev_keyframe(
			track->to_units(edl->local_session->get_selectionstart(1), 0), 
			PLAY_FORWARD);

// Send to database
	presets_db->save_preset(plugin_title, title, keyframe->get_data());

	mwindow->gui->unlock_window();
	get_gui()->lock_window("PresetsThread::save_preset 2");


// Update list
	calculate_list();
	((PresetsWindow*)get_gui())->list->update(data,
		0,
		0,
		1);
}

void PresetsThread::delete_preset(char *title)
{
	get_gui()->unlock_window();
	mwindow->gui->lock_window("PresetsThread::save_preset");
	
// Test EDL for plugin existence
	if(!mwindow->edl->tracks->plugin_exists(plugin))
	{
		mwindow->gui->unlock_window();
		get_gui()->lock_window("PresetsThread::delete_preset 1");
		return;
	}

	presets_db->delete_preset(plugin_title, title);
	
	mwindow->gui->unlock_window();
	get_gui()->lock_window("PresetsThread::delete_preset 2");


// Update list
	calculate_list();
	((PresetsWindow*)get_gui())->list->update(data,
		0,
		0,
		1);
}


void PresetsThread::apply_preset(char *title)
{
	if(presets_db->preset_exists(plugin_title, title))
	{
		get_gui()->unlock_window();
		mwindow->gui->lock_window("PresetsThread::apply_preset");

// Test EDL for plugin existence
		if(!mwindow->edl->tracks->plugin_exists(plugin))
		{
			mwindow->gui->unlock_window();
			get_gui()->lock_window("PresetsThread::delete_preset 1");
			return;
		}

		mwindow->undo->update_undo_before();
		KeyFrame *keyframe = plugin->get_keyframe();
		presets_db->load_preset(plugin_title, title, keyframe);
		mwindow->save_backup();
		mwindow->undo->update_undo_after(_("apply preset"), LOAD_AUTOMATION); 

		mwindow->update_plugin_guis();
		mwindow->gui->canvas->draw_overlays();
		mwindow->gui->canvas->flash();
		mwindow->sync_parameters(CHANGE_PARAMS);

		mwindow->gui->unlock_window();
		get_gui()->lock_window("PresetsThread::apply_preset");
	}
}





PresetsList::PresetsList(PresetsThread *thread,
	PresetsWindow *window,
	int x,
	int y,
	int w, 
	int h)
 : BC_ListBox(x, 
		y, 
		w, 
		h,
		LISTBOX_TEXT,
		thread->data)
{
	this->thread = thread;
	this->window = window;
}

int PresetsList::selection_changed()
{
	window->title_text->update(
		thread->data->get(get_selection_number(0, 0))->get_text());
	return 0;
}

int PresetsList::handle_event()
{
	window->set_done(0);
	return 0;
}










PresetsText::PresetsText(PresetsThread *thread,
	PresetsWindow *window,
	int x,
	int y,
	int w)
 : BC_TextBox(x, 
	y, 
	w, 
	1, 
	"")
{
	this->thread = thread;
	this->window = window;
}

int PresetsText::handle_event()
{
	return 0;
}

















PresetsDelete::PresetsDelete(PresetsThread *thread,
	PresetsWindow *window,
	int x,
	int y)
 : BC_GenericButton(x, y, _("Delete"))
{
	this->thread = thread;
	this->window = window;
}

int PresetsDelete::handle_event()
{
	thread->delete_preset(window->title_text->get_text());
	return 1;
}







PresetsSave::PresetsSave(PresetsThread *thread,
	PresetsWindow *window,
	int x,
	int y)
: BC_GenericButton(x, y, _("Save"))
{
	this->thread = thread;
	this->window = window;
}

int PresetsSave::handle_event()
{
	thread->save_preset(window->title_text->get_text());
	return 1;
}








PresetsApply::PresetsApply(PresetsThread *thread,
	PresetsWindow *window,
	int x,
	int y)
 : BC_GenericButton(x, y, _("Apply"))
{
	this->thread = thread;
	this->window = window;
}

int PresetsApply::handle_event()
{
	thread->apply_preset(window->title_text->get_text());
	return 1;
}



PresetsOK::PresetsOK(PresetsThread *thread,
	PresetsWindow *window)
 : BC_OKButton(window)
{
	this->thread = thread;
	this->window = window;
}

int PresetsOK::keypress_event()
{
	if(get_keypress() == RETURN)
	{
printf("PresetsOK::keypress_event %d\n", __LINE__);
		if(thread->presets_db->preset_exists(thread->plugin_title, 
			window->title_text->get_text()))
		{
printf("PresetsOK::keypress_event %d\n", __LINE__);
			window->set_done(0);
			return 1;
		}
		else
		{
printf("PresetsOK::keypress_event %d\n", __LINE__);
			thread->save_preset(window->title_text->get_text());
			return 1;
		}
	}
	return 0;
}










PresetsWindow::PresetsWindow(MWindow *mwindow,
	PresetsThread *thread,
	int x,
	int y,
	char *title_string)
 : BC_Window(title_string, 
 	x,
	y,
	mwindow->session->presetdialog_w, 
	mwindow->session->presetdialog_h, 
	320, 
	240,
	1,
	0,
	1)
{
	this->mwindow = mwindow;
	this->thread = thread;
}

void PresetsWindow::create_objects()
{
	Theme *theme = mwindow->theme;

	lock_window("PresetsWindow::create_objects");
	theme->get_presetdialog_sizes(this);
	
	add_subwindow(title1 = new BC_Title(theme->presets_list_x,
		theme->presets_list_y - BC_Title::calculate_h(this, "P") - theme->widget_border,
		_("Saved presets:")));
	add_subwindow(list = new PresetsList(thread,
		this,
		theme->presets_list_x,
		theme->presets_list_y,
		theme->presets_list_w, 
		theme->presets_list_h));
	add_subwindow(title2 = new BC_Title(theme->presets_text_x,
		theme->presets_text_y - BC_Title::calculate_h(this, "P") - theme->widget_border,
		_("Preset title:")));
	add_subwindow(title_text = new PresetsText(thread,
		this,
		theme->presets_text_x,
		theme->presets_text_y,
		theme->presets_text_w));
	add_subwindow(delete_button = new PresetsDelete(thread,
		this,
		theme->presets_delete_x,
		theme->presets_delete_y));
	add_subwindow(save_button = new PresetsSave(thread,
		this,
		theme->presets_save_x,
		theme->presets_save_y));
	add_subwindow(apply_button = new PresetsApply(thread,
		this,
		theme->presets_apply_x,
		theme->presets_apply_y));

	add_subwindow(new PresetsOK(thread, this));
	add_subwindow(new BC_CancelButton(this));

	show_window();
	unlock_window();
}

int PresetsWindow::resize_event(int w, int h)
{
	Theme *theme = mwindow->theme;
	mwindow->session->presetdialog_w = w;
	mwindow->session->presetdialog_h = h;
	theme->get_presetdialog_sizes(this);

	title1->reposition_window(theme->presets_list_x,
		theme->presets_list_y - BC_Title::calculate_h(this, "P") - theme->widget_border);
	title2->reposition_window(theme->presets_text_x,
		theme->presets_text_y - BC_Title::calculate_h(this, "P") - theme->widget_border);
	list->reposition_window(theme->presets_list_x,
		theme->presets_list_y,
		theme->presets_list_w, 
		theme->presets_list_h);
	title_text->reposition_window(theme->presets_text_x,
		theme->presets_text_y,
		theme->presets_text_w);
	delete_button->reposition_window(theme->presets_delete_x,
		theme->presets_delete_y);
	save_button->reposition_window(theme->presets_save_x,
		theme->presets_save_y);
	apply_button->reposition_window(theme->presets_apply_x,
		theme->presets_apply_y);
	return 0;
}






#endif

