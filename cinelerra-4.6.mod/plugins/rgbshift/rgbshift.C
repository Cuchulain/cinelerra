
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
#include "filexml.h"
#include "guicast.h"
#include "language.h"
#include "picon_png.h"
#include "cicolors.h"
#include "pluginvclient.h"
#include "vframe.h"

#include <stdint.h>
#include <string.h>


class RGBShiftEffect;


class RGBShiftConfig
{
public:
	RGBShiftConfig();

	void copy_from(RGBShiftConfig &src);
	int equivalent(RGBShiftConfig &src);
	void interpolate(RGBShiftConfig &prev, 
		RGBShiftConfig &next, 
		long prev_frame, 
		long next_frame, 
		long current_frame);

	int r_dx, r_dy, g_dx, g_dy, b_dx, b_dy;
};

class RGBShiftLevel : public BC_ISlider
{
public:
	RGBShiftLevel(RGBShiftEffect *plugin, int *output, int x, int y);
	int handle_event();
	RGBShiftEffect *plugin;
	int *output;
};

class RGBShiftWindow : public PluginClientWindow
{
public:
	RGBShiftWindow(RGBShiftEffect *plugin);
	void create_objects();
	RGBShiftLevel *r_dx, *r_dy, *g_dx, *g_dy, *b_dx, *b_dy;
	RGBShiftEffect *plugin;
};



class RGBShiftEffect : public PluginVClient
{
	VFrame *temp_frame;
public:
	RGBShiftEffect(PluginServer *server);
	~RGBShiftEffect();


	PLUGIN_CLASS_MEMBERS(RGBShiftConfig)
	int process_realtime(VFrame *input, VFrame *output);
	int is_realtime();
	void save_data(KeyFrame *keyframe);
	void read_data(KeyFrame *keyframe);
	void update_gui();
};





REGISTER_PLUGIN(RGBShiftEffect)







RGBShiftConfig::RGBShiftConfig()
{
	r_dx = r_dy = 0;
	g_dx = g_dy = 0;
	b_dx = b_dy = 0;
}

void RGBShiftConfig::copy_from(RGBShiftConfig &src)
{
	r_dx = src.r_dx;  r_dy = src.r_dy;
	g_dx = src.g_dx;  g_dy = src.g_dy;
	b_dx = src.b_dx;  b_dy = src.b_dy;
}

int RGBShiftConfig::equivalent(RGBShiftConfig &src)
{
	return EQUIV(r_dx, src.r_dx) && EQUIV(g_dx, src.g_dx) && EQUIV(b_dx, src.b_dx) &&
		EQUIV(r_dy, src.r_dy) && EQUIV(g_dy, src.g_dy) && EQUIV(b_dy, src.b_dy);
}

void RGBShiftConfig::interpolate(RGBShiftConfig &prev, 
	RGBShiftConfig &next, 
	long prev_frame, 
	long next_frame, 
	long current_frame)
{
	double next_scale = (double)(current_frame - prev_frame) / (next_frame - prev_frame);
	double prev_scale = (double)(next_frame - current_frame) / (next_frame - prev_frame);

	r_dx = prev.r_dx * prev_scale + next.r_dx * next_scale;
	r_dy = prev.r_dy * prev_scale + next.r_dy * next_scale;
	g_dx = prev.g_dx * prev_scale + next.g_dx * next_scale;
	g_dy = prev.g_dy * prev_scale + next.g_dy * next_scale;
	b_dx = prev.b_dx * prev_scale + next.b_dx * next_scale;
	b_dy = prev.b_dy * prev_scale + next.b_dy * next_scale;
}






#define MAXVALUE 100

RGBShiftLevel::RGBShiftLevel(RGBShiftEffect *plugin, int *output, int x, int y)
 : BC_ISlider(x, y, 0, 200, 200, -MAXVALUE, MAXVALUE, *output)
{
	this->plugin = plugin;
	this->output = output;
}

int RGBShiftLevel::handle_event()
{
	*output = get_value();
	plugin->send_configure_change();
	return 1;
}


RGBShiftWindow::RGBShiftWindow(RGBShiftEffect *plugin)
 : PluginClientWindow(plugin, 300, 200, 300, 200, 0)
{
	this->plugin = plugin;
}

void RGBShiftWindow::create_objects()
{
	int x = 10, y = 10, x1 = 50;
	add_subwindow(new BC_Title(x, y, _("R_dx:")));
	add_subwindow(r_dx = new RGBShiftLevel(plugin, &plugin->config.r_dx, x1, y));
	y += 30;
	add_subwindow(new BC_Title(x, y, _("R_dy:")));
	add_subwindow(r_dy = new RGBShiftLevel(plugin, &plugin->config.r_dy, x1, y));
	y += 30;
	add_subwindow(new BC_Title(x, y, _("G_dx:")));
	add_subwindow(g_dx = new RGBShiftLevel(plugin, &plugin->config.g_dx, x1, y));
	y += 30;
	add_subwindow(new BC_Title(x, y, _("G_dy:")));
	add_subwindow(g_dy = new RGBShiftLevel(plugin, &plugin->config.g_dy, x1, y));
	y += 30;
	add_subwindow(new BC_Title(x, y, _("B_dx:")));
	add_subwindow(b_dx = new RGBShiftLevel(plugin, &plugin->config.b_dx, x1, y));
	y += 30;
	add_subwindow(new BC_Title(x, y, _("B_dy:")));
	add_subwindow(b_dy = new RGBShiftLevel(plugin, &plugin->config.b_dy, x1, y));

	show_window();
	flush();
}









RGBShiftEffect::RGBShiftEffect(PluginServer *server)
 : PluginVClient(server)
{
	temp_frame = 0;
}
RGBShiftEffect::~RGBShiftEffect()
{
	delete temp_frame;
}

const char* RGBShiftEffect::plugin_title() { return N_("RGBShift"); }
int RGBShiftEffect::is_realtime() { return 1; } 


NEW_PICON_MACRO(RGBShiftEffect)
NEW_WINDOW_MACRO(RGBShiftEffect, RGBShiftWindow)
LOAD_CONFIGURATION_MACRO(RGBShiftEffect, RGBShiftConfig)

void RGBShiftEffect::update_gui()
{
	if(thread)
	{
		RGBShiftWindow *yuv_wdw = (RGBShiftWindow*)thread->window;
		yuv_wdw->lock_window("RGBShiftEffect::update_gui");
		load_configuration();
		yuv_wdw->r_dx->update(config.r_dx);
		yuv_wdw->r_dy->update(config.r_dy);
		yuv_wdw->g_dx->update(config.g_dx);
		yuv_wdw->g_dy->update(config.g_dy);
		yuv_wdw->b_dx->update(config.b_dx);
		yuv_wdw->b_dy->update(config.b_dy);
		yuv_wdw->unlock_window();
	}
}

void RGBShiftEffect::save_data(KeyFrame *keyframe)
{
	FileXML output;
	output.set_shared_output(keyframe->get_data(), MESSAGESIZE);
	output.tag.set_title("RGBSHIFT");
	output.tag.set_property("R_DX", config.r_dx);
	output.tag.set_property("R_DY", config.r_dy);
	output.tag.set_property("G_DX", config.g_dx);
	output.tag.set_property("G_DY", config.g_dy);
	output.tag.set_property("B_DX", config.b_dx);
	output.tag.set_property("B_DY", config.b_dy);
	output.append_tag();
	output.terminate_string();
}

void RGBShiftEffect::read_data(KeyFrame *keyframe)
{
	FileXML input;
	input.set_shared_input(keyframe->get_data(), strlen(keyframe->get_data()));
	while(!input.read_tag())
	{
		if(input.tag.title_is("RGBSHIFT"))
		{
			config.r_dx = input.tag.get_property("R_DX", config.r_dx);
			config.r_dy = input.tag.get_property("R_DY", config.r_dy);
			config.g_dx = input.tag.get_property("G_DX", config.g_dx);
			config.g_dy = input.tag.get_property("G_DY", config.g_dy);
			config.b_dx = input.tag.get_property("B_DX", config.b_dx);
			config.b_dy = input.tag.get_property("B_DY", config.b_dy);
		}
	}
}


static YUV yuv_static;

#define RGB_MACRO(type, temp_type, components) \
{ \
	for(int i = 0; i < h; i++) { \
		int ri = i + config.r_dy, gi = i + config.g_dy, bi = i + config.b_dy; \
		type *in_r = ri >= 0 && ri < h ? (type *)frame->get_rows()[ri] : 0; \
		type *in_g = gi >= 0 && gi < h ? (type *)frame->get_rows()[gi] : 0; \
		type *in_b = bi >= 0 && bi < h ? (type *)frame->get_rows()[bi] : 0; \
		type *out_row = (type *)output->get_rows()[i]; \
		for(int j = 0; j < w; j++) { \
			int rj = j + config.r_dx, gj = j + config.g_dx, bj = j + config.b_dx; \
			type *rp = in_r && rj >= 0 && rj < w ? in_r + rj*components: 0; \
			type *gp = in_g && gj >= 0 && gj < w ? in_g + gj*components: 0; \
			type *bp = in_b && bj >= 0 && bj < w ? in_b + bj*components: 0; \
			out_row[0] = rp ? rp[0] : 0; \
			out_row[1] = gp ? gp[1] : 0; \
			out_row[2] = bp ? bp[2] : 0; \
			out_row += components; \
		} \
	} \
}

#define YUV_MACRO(type, temp_type, components) \
{ \
	for(int i = 0; i < h; i++) { \
		int ri = i + config.r_dy, gi = i + config.g_dy, bi = i + config.b_dy; \
		uint8_t *in_r = ri >= 0 && ri < h ? (uint8_t *)frame->get_rows()[ri] : 0; \
		uint8_t *in_g = gi >= 0 && gi < h ? (uint8_t *)frame->get_rows()[gi] : 0; \
		uint8_t *in_b = bi >= 0 && bi < h ? (uint8_t *)frame->get_rows()[bi] : 0; \
		type *out_row = (type *)output->get_rows()[i]; \
		for(int j = 0; j < w; j++) { \
			int rj = j + config.r_dx, gj = j + config.g_dx, bj = j + config.b_dx; \
			uint8_t *rp = in_r && rj >= 0 && rj < w ? in_r + rj*3: 0; \
			uint8_t *gp = in_g && gj >= 0 && gj < w ? in_g + gj*3: 0; \
			uint8_t *bp = in_b && bj >= 0 && bj < w ? in_b + bj*3: 0; \
			temp_type y, u, v; \
			temp_type r = rp ? rp[0] : 0; \
			temp_type g = gp ? gp[1] : 0; \
			temp_type b = bp ? bp[2] : 0; \
			if( sizeof(type) == 4 ) \
				yuv_static.rgb_to_yuv_f(r, g, b, y, u, v); \
			else if( sizeof(type) == 2 ) \
				yuv_static.rgb_to_yuv_16(r, g, b, y, u, v); \
			else \
				yuv_static.rgb_to_yuv_8(r, g, b, y, u, v); \
			out_row[0] = y; \
			out_row[1] = u; \
			out_row[2] = v; \
			out_row += components; \
		} \
	} \
}

int RGBShiftEffect::process_realtime(VFrame *input, VFrame *output)
{
	load_configuration();

	if( EQUIV(config.r_dx, 0) && EQUIV(config.g_dx, 0) && EQUIV(config.b_dx, 0) &&
		EQUIV(config.r_dy, 0) && EQUIV(config.g_dy, 0) && EQUIV(config.b_dy, 0) ) {
		if(input->get_rows()[0] != output->get_rows()[0])
			output->copy_from(input);
		return 0;
	}

	int w = input->get_w(), h = input->get_h();
	int color_model = input->get_color_model();
	int is_yuv = BC_CModels::is_yuv(color_model);
	if( is_yuv ) color_model = BC_RGB888;
	VFrame *frame = input;
	if( input->get_rows()[0] == output->get_rows()[0] || !is_yuv ) {
		if( temp_frame && ( temp_frame->get_color_model() != color_model ||
			temp_frame->get_w() != w || temp_frame->get_h() != h ) ) {
			delete temp_frame;  temp_frame = 0;
		}
		if( !temp_frame )
			temp_frame = new VFrame(w, h, color_model);
		frame = temp_frame;
		if( color_model != input->get_color_model() )
			BC_CModels::transfer(frame->get_rows(), input->get_rows(),
				frame->get_y(), frame->get_u(), frame->get_v(),
				input->get_y(), input->get_u(), input->get_v(),
				0, 0, input->get_w(), input->get_h(),
				0, 0, frame->get_w(), frame->get_h(),
				input->get_color_model(), frame->get_color_model(), 0,
				input->get_bytes_per_line(), w);
		else
			frame->copy_from(input);
	}

	switch( input->get_color_model() ) {
	case BC_YUV888:       YUV_MACRO(unsigned char, int, 3);  break;
	case BC_YUV161616:    YUV_MACRO(uint16_t, int, 3);       break;
	case BC_YUVA8888:     YUV_MACRO(unsigned char, int, 4);  break;
	case BC_YUVA16161616: YUV_MACRO(uint16_t, int, 4);       break;
	case BC_RGB_FLOAT:    RGB_MACRO(float, float, 3);        break;
	case BC_RGB888:       RGB_MACRO(unsigned char, int, 3);  break;
	case BC_RGB161616:    RGB_MACRO(uint16_t, int, 3);       break;
	case BC_RGBA_FLOAT:   RGB_MACRO(float, float, 4);        break;
	case BC_RGBA8888:     RGB_MACRO(unsigned char, int, 4);  break;
	case BC_RGBA16161616: RGB_MACRO(uint16_t, int, 4);       break;
	}

	return 0;
}


