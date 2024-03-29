
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

#include "bcdisplayinfo.h"
#include "clip.h"
#include "bchash.h"
#include "filesystem.h"
#include "filexml.h"
#include "language.h"
#include "leveleffect.h"
#include "picon_png.h"
#include "samples.h"
#include "units.h"
#include "vframe.h"

#include <errno.h>
#include <math.h>
#include <string.h>
#include <unistd.h>










REGISTER_PLUGIN(SoundLevelEffect)









SoundLevelConfig::SoundLevelConfig()
{
	duration = 1.0;
}

void SoundLevelConfig::copy_from(SoundLevelConfig &that)
{
	duration = that.duration;
}

int SoundLevelConfig::equivalent(SoundLevelConfig &that)
{
	return EQUIV(duration, that.duration);
}

void SoundLevelConfig::interpolate(SoundLevelConfig &prev, 
	SoundLevelConfig &next, 
	int64_t prev_frame, 
	int64_t next_frame, 
	int64_t current_frame)
{
	duration = prev.duration;
}















SoundLevelDuration::SoundLevelDuration(SoundLevelEffect *plugin, int x, int y)
 : BC_FSlider(x, y, 0, 180, 180, 0.0, 10.0, plugin->config.duration)
{
	this->plugin = plugin;
	set_precision(0.1);
}

int SoundLevelDuration::handle_event()
{
	plugin->config.duration = get_value();
	plugin->send_configure_change();
	return 1;
}



SoundLevelWindow::SoundLevelWindow(SoundLevelEffect *plugin)
 : PluginClientWindow(plugin, 
	350, 
	120, 
	350, 
	120,
	0)
{
	this->plugin = plugin;
}

void SoundLevelWindow::create_objects()
{
//printf("SoundLevelWindow::create_objects 1\n");
	int x = 10, y = 10;


	add_subwindow(new BC_Title(x, y, _("Duration (seconds):")));
	add_subwindow(duration = new SoundLevelDuration(plugin, x + 150, y));
	y += 35;
	add_subwindow(new BC_Title(x, y, _("Max soundlevel (dB):")));
	add_subwindow(soundlevel_max = new BC_Title(x + 150, y, "0.0"));
	y += 35;
	add_subwindow(new BC_Title(x, y, _("RMS soundlevel (dB):")));
	add_subwindow(soundlevel_rms = new BC_Title(x + 150, y, "0.0"));

	show_window();
	flush();
//printf("SoundLevelWindow::create_objects 2\n");
}





























SoundLevelEffect::SoundLevelEffect(PluginServer *server)
 : PluginAClient(server)
{
	
	reset();
}

SoundLevelEffect::~SoundLevelEffect()
{
	
}

NEW_PICON_MACRO(SoundLevelEffect)

LOAD_CONFIGURATION_MACRO(SoundLevelEffect, SoundLevelConfig)

NEW_WINDOW_MACRO(SoundLevelEffect, SoundLevelWindow)



void SoundLevelEffect::reset()
{
	rms_accum = 0;
	max_accum = 0;
	accum_size = 0;
}

const char* SoundLevelEffect::plugin_title() { return N_("SoundLevel"); }
int SoundLevelEffect::is_realtime() { return 1; }


void SoundLevelEffect::read_data(KeyFrame *keyframe)
{
	FileXML input;
	input.set_shared_input(keyframe->get_data(), strlen(keyframe->get_data()));

	int result = 0;
	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("SOUNDLEVEL"))
			{
				config.duration = input.tag.get_property("DURATION", config.duration);
			}
		}
	}
}

void SoundLevelEffect::save_data(KeyFrame *keyframe)
{
	FileXML output;
	output.set_shared_output(keyframe->get_data(), MESSAGESIZE);

	output.tag.set_title("SOUNDLEVEL");
	output.tag.set_property("DURATION", config.duration);
	output.append_tag();
	output.append_newline();

	output.terminate_string();
}


void SoundLevelEffect::update_gui()
{
//printf("SoundLevelEffect::update_gui 1\n");
	if(thread)
	{
		load_configuration();
		thread->window->lock_window();
		((SoundLevelWindow*)thread->window)->duration->update(config.duration);
		thread->window->unlock_window();
	}
//printf("SoundLevelEffect::update_gui 2\n");
}

int SoundLevelEffect::process_realtime(int64_t size, Samples *input_ptr, Samples *output_ptr)
{
	load_configuration();

	accum_size += size;
	double *input_samples = input_ptr->get_data();
	for(int i = 0; i < size; i++)
	{
		double value = fabs(input_samples[i]);
		if(value > max_accum) max_accum = value;
		rms_accum += value * value;
	}

	if(accum_size > config.duration * PluginAClient::project_sample_rate)
	{
//printf("SoundLevelEffect::process_realtime 1 %f %d\n", rms_accum, accum_size);
		rms_accum = sqrt(rms_accum / accum_size);
		double arg[2];
		arg[0] = max_accum;
		arg[1] = rms_accum;
		send_render_gui(arg, 2);
		rms_accum = 0;
		max_accum = 0;
		accum_size = 0;
	}
	return 0;
}

void SoundLevelEffect::render_gui(void *data, int size)
{
	if(thread)
	{
		thread->window->lock_window();
		char string[BCTEXTLEN];
		double *arg = (double*)data;
		sprintf(string, "%.2f", DB::todb(arg[0]));
		((SoundLevelWindow*)thread->window)->soundlevel_max->update(string);
		sprintf(string, "%.2f", DB::todb(arg[1]));
		((SoundLevelWindow*)thread->window)->soundlevel_rms->update(string);
		thread->window->flush();
		thread->window->unlock_window();
	}
}
