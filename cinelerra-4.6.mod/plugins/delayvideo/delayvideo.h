
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

#ifndef DELAYVIDEO_H
#define DELAYVIDEO_H



#include "bchash.inc"
#include "guicast.h"
#include "mutex.h"
#include "pluginvclient.h"
#include "vframe.inc"



class DelayVideo;





class DelayVideoConfig
{
public:
	DelayVideoConfig();
	
	int equivalent(DelayVideoConfig &that);
	void copy_from(DelayVideoConfig &that);
	void interpolate(DelayVideoConfig &prev, 
		DelayVideoConfig &next, 
		int64_t prev_frame, 
		int64_t next_frame, 
		int64_t current_frame);

	// kjb - match defined update() type of float instead of double.
	float length;
};

class DelayVideoWindow;

class DelayVideoSlider : public BC_TumbleTextBox
{
public:
	DelayVideoSlider(DelayVideoWindow *window,
		DelayVideo *plugin, 
		int x, 
		int y);
	
	int handle_event();
	
	DelayVideo *plugin;
	DelayVideoWindow *window;
	
};


class DelayVideoWindow : public PluginClientWindow
{
public:
	DelayVideoWindow(DelayVideo *plugin);
	~DelayVideoWindow();
	
	void create_objects();
	void update_gui();
	
	DelayVideo *plugin;
	DelayVideoSlider *slider;
};






class DelayVideo : public PluginVClient
{
public:
	DelayVideo(PluginServer *server);
	~DelayVideo();
	
	PLUGIN_CLASS_MEMBERS(DelayVideoConfig);
	int process_realtime(VFrame *input_ptr, VFrame *output_ptr);
	int is_realtime();
	void save_data(KeyFrame *keyframe);
	void read_data(KeyFrame *keyframe);
	void reset();
	void reconfigure();



	void update_gui();


	int need_reconfigure;
	int allocation;
	VFrame **buffer;
	VFrame *input, *output;
};



#endif
