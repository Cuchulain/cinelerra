
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
#include "bcsignals.h"
#include "histogram.h"
#include "histogramconfig.h"
#include "histogramwindow.h"
#include "keys.h"
#include "language.h"


#include <unistd.h>
#include <string.h>

PLUGIN_THREAD_OBJECT(HistogramMain, HistogramThread, HistogramWindow)



HistogramWindow::HistogramWindow(HistogramMain *plugin, int x, int y)
 : BC_Window(plugin->gui_string, 
 	x,
	y,
	440, 
	480, 
	440, 
	480, 
	0, 
	1,
	1)
{
	this->plugin = plugin; 
}

HistogramWindow::~HistogramWindow()
{
}

#include "max_picon_png.h"
#include "mid_picon_png.h"
#include "min_picon_png.h"
static VFrame max_picon_image(max_picon_png);
static VFrame mid_picon_image(mid_picon_png);
static VFrame min_picon_image(min_picon_png);

int HistogramWindow::create_objects()
{
	int x = 10, y = 10, x1 = 10;
	BC_Title *title = 0;

	max_picon = new BC_Pixmap(this, &max_picon_image);
	mid_picon = new BC_Pixmap(this, &mid_picon_image);
	min_picon = new BC_Pixmap(this, &min_picon_image);
	add_subwindow(mode_v = new HistogramMode(plugin, 
		x, 
		y,
		HISTOGRAM_VALUE,
		_("Value")));
	x += 70;
	add_subwindow(mode_r = new HistogramMode(plugin, 
		x, 
		y,
		HISTOGRAM_RED,
		_("Red")));
	x += 70;
	add_subwindow(mode_g = new HistogramMode(plugin, 
		x, 
		y,
		HISTOGRAM_GREEN,
		_("Green")));
	x += 70;
	add_subwindow(mode_b = new HistogramMode(plugin, 
		x, 
		y,
		HISTOGRAM_BLUE,
		_("Blue")));
// 	x += 70;
// 	add_subwindow(mode_a = new HistogramMode(plugin, 
// 		x, 
// 		y,
// 		HISTOGRAM_ALPHA,
// 		_("Alpha")));

	x = x1;
	y += 30;
	add_subwindow(title = new BC_Title(x, y, _("Input X:")));
	x += title->get_w() + 10;
	input_x = new HistogramInputText(plugin,
		this,
		x,
		y,
		1);
	input_x->create_objects();

	x += input_x->get_w() + 10;
	add_subwindow(title = new BC_Title(x, y, _("Input Y:")));
	x += title->get_w() + 10;
	input_y = new HistogramInputText(plugin,
		this,
		x,
		y,
		0);
	input_y->create_objects();

	y += 30;
	x = x1;

	canvas_w = get_w() - x - x;
	canvas_h = get_h() - y - 170;
	title1_x = x;
	title2_x = x + (int)(canvas_w * -HIST_MIN_INPUT / FLOAT_RANGE);
	title3_x = x + (int)(canvas_w * (1.0 - HIST_MIN_INPUT) / FLOAT_RANGE);
	title4_x = x + (int)(canvas_w);
	add_subwindow(canvas = new HistogramCanvas(plugin,
		this,
		x, 
		y, 
		canvas_w, 
		canvas_h));
	draw_canvas_overlay();
	canvas->flash();

	y += canvas->get_h() + 1;
	add_subwindow(new BC_Title(title1_x, 
		y, 
		"-10%"));
	add_subwindow(new BC_Title(title2_x,
		y,
		"0%"));
	add_subwindow(new BC_Title(title3_x - get_text_width(MEDIUMFONT, "100"),
		y,
		"100%"));
	add_subwindow(new BC_Title(title4_x - get_text_width(MEDIUMFONT, "110"),
		y,
		"110%"));

	y += 20;
	add_subwindow(title = new BC_Title(x, y, _("Output min:")));
	x += title->get_w() + 10;
	output_min = new HistogramOutputText(plugin,
		this,
		x,
		y,
		&plugin->config.output_min[plugin->mode]);
	output_min->create_objects();
	x += output_min->get_w() + 10;
	add_subwindow(new BC_Title(x, y, _("Output Max:")));
	x += title->get_w() + 10;
	output_max = new HistogramOutputText(plugin,
		this,
		x,
		y,
		&plugin->config.output_max[plugin->mode]);
	output_max->create_objects();

	x = x1;
	y += 30;

	add_subwindow(output = new HistogramSlider(plugin, 
		this,
		x, 
		y, 
		get_w() - 20,
		30,
		0));
	output->update();
	y += 40;


	add_subwindow(automatic = new HistogramAuto(plugin, 
		x, 
		y));

	x += 120;
	add_subwindow(new HistogramReset(plugin, 
		x, 
		y));
	x += 100;
	add_subwindow(new BC_Title(x, y, _("Threshold:")));
	x += 100;
	threshold = new HistogramOutputText(plugin,
		this,
		x,
		y,
		&plugin->config.threshold);
	threshold->create_objects();
	x = x1;
	y += 40;	
	add_subwindow(split = new HistogramSplit(plugin, x, y));
	y += 6;
	x += 150;
	add_subwindow(new BC_Title(x,y, _("Interpolation:")));
	x += 120;
	add_subwindow(smoothModeChoser = new HistogramSmoothMode(plugin, this, x, y));
	smoothModeChoser->create_objects();

	show_window();

	return 0;
}

WINDOW_CLOSE_EVENT(HistogramWindow)

int HistogramWindow::keypress_event()
{
	int result = 0;
	if(get_keypress() == BACKSPACE ||
		get_keypress() == DELETE)
	{
		if(plugin->current_point >= 0)
		{
			HistogramPoint *current = 
				plugin->config.points[plugin->mode].get_item_number(plugin->current_point);
			delete current;
			plugin->current_point = -1;
			update_input();
			update_canvas();
			plugin->send_configure_change();
			result = 1;
		}
	}
	return result;
}

void HistogramWindow::update(int do_input)
{
	automatic->update(plugin->config.automatic);
	threshold->update(plugin->config.threshold);
	update_mode();

	if(do_input) update_input();
	update_output();
}

void HistogramWindow::update_input()
{
	input_x->update();
	input_y->update();
}

void HistogramWindow::update_output()
{
	output->update();
	output_min->update(plugin->config.output_min[plugin->mode]);
	output_max->update(plugin->config.output_max[plugin->mode]);
}

void HistogramWindow::update_mode()
{
	mode_v->update(plugin->mode == HISTOGRAM_VALUE ? 1 : 0);
	mode_r->update(plugin->mode == HISTOGRAM_RED ? 1 : 0);
	mode_g->update(plugin->mode == HISTOGRAM_GREEN ? 1 : 0);
	mode_b->update(plugin->mode == HISTOGRAM_BLUE ? 1 : 0);
	output_min->output = &plugin->config.output_min[plugin->mode];
	output_max->output = &plugin->config.output_max[plugin->mode];
}

void HistogramWindow::draw_canvas_overlay()
{
	canvas->set_color(0x00ff00);
	int y1;

// Calculate output curve
	plugin->tabulate_curve(plugin->mode, 0);

// Draw output line
	for(int i = 0; i < canvas_w; i++)
	{
		float input = (float)i / 
				canvas_w * 
				FLOAT_RANGE + 
				HIST_MIN_INPUT;
		float output = plugin->calculate_smooth(input, plugin->mode);

		int y2 = canvas_h - (int)(output * canvas_h);
		if(i > 0)
		{
			canvas->draw_line(i - 1, y1, i, y2);
		}
		y1 = y2;
	}

// Draw output points
	HistogramPoint *current = plugin->config.points[plugin->mode].first;
	int number = 0;
	while(current)
	{
		int x = (int)((current->x - HIST_MIN_INPUT) * canvas_w / FLOAT_RANGE);
		int y = (int)(canvas_h - current->y * canvas_h);
		if(number == plugin->current_point)
			canvas->draw_box(x - BOX_SIZE / 2, y - BOX_SIZE / 2, BOX_SIZE, BOX_SIZE);
		else
			canvas->draw_rectangle(x - BOX_SIZE / 2, y - BOX_SIZE / 2, BOX_SIZE, BOX_SIZE);
			
//Draw gradients
	if (plugin->config.smoothMode > HISTOGRAM_LINEAR)
	{	
		int x1,x2,y1,y2;
		canvas->set_color(0x0000ff);
		x2 = (int)((current->x + current->xoffset_right - HIST_MIN_INPUT) * canvas_w / FLOAT_RANGE);
		x1 = (int)((current->x + current->xoffset_left - HIST_MIN_INPUT) * canvas_w / FLOAT_RANGE);
		y2 = (int)(canvas_h - (current->y + current->xoffset_right * current->gradient) * canvas_h);
		y1 = (int)(canvas_h - (current->y + current->xoffset_left * current->gradient) * canvas_h);
/*		x2 = x + (title3_x - title2_x)/20;
		x1 = x - (title3_x - title2_x)/20;
		y1 = y + (int)(current->gradient * (float)(canvas_h)/20.0);
		y2 = y - (int)(current->gradient * (float)(canvas_h)/20.0);
//		int y2 = (int)(canvas_h - canvas_h * (current->y + current->gradient /10));*/
		canvas->draw_line(x1,y1,x2,y2);
		
		canvas->draw_circle(x1 - BOX_SIZE / 4, y1 - BOX_SIZE / 4, BOX_SIZE/2, BOX_SIZE/2);
		canvas->draw_circle(x2 - BOX_SIZE / 4, y2 - BOX_SIZE / 4, BOX_SIZE/2, BOX_SIZE/2);
		canvas->set_color(0x00ff00);
	}
				
		current = NEXT;
		number++;
	}


// Draw 0 and 100% lines.
	canvas->set_color(0xff0000);
	canvas->draw_line(title2_x - canvas->get_x(),
		0, 
		title2_x - canvas->get_x(), 
		canvas_h);
	canvas->draw_line(title3_x - canvas->get_x(), 
		0, 
		title3_x - canvas->get_x(), 
		canvas_h);
}

void HistogramWindow::update_canvas()
{
	int *accum = plugin->accum[plugin->mode];
	int accum_per_canvas_i = HISTOGRAM_SLOTS / canvas_w + 1;
	float accum_per_canvas_f = (float)HISTOGRAM_SLOTS / canvas_w;
	int normalize = 0;
	int max = 0;

	for(int i = 0; i < HISTOGRAM_SLOTS; i++)
	{
		if(accum && accum[i] > normalize) normalize = accum[i];
	}


	if(normalize)
	{
		for(int i = 0; i < canvas_w; i++)
		{
			int accum_start = (int)(accum_per_canvas_f * i);
			int accum_end = accum_start + accum_per_canvas_i;
			max = 0;
			for(int j = accum_start; j < accum_end; j++)
			{
				max = MAX(accum[j], max);
			}

//			max = max * canvas_h / normalize;
			max = (int)(log(max) / log(normalize) * canvas_h);

			canvas->set_color(0xffffff);
			canvas->draw_line(i, 0, i, canvas_h - max);
			canvas->set_color(0x000000);
			canvas->draw_line(i, canvas_h - max, i, canvas_h);
		}
	}
	else
	{
		canvas->set_color(0xffffff);
		canvas->draw_box(0, 0, canvas_w, canvas_h);
	}


	draw_canvas_overlay();
	canvas->flash();
}








HistogramCanvas::HistogramCanvas(HistogramMain *plugin,
	HistogramWindow *gui,
	int x,
	int y,
	int w,
	int h)
 : BC_SubWindow(x,
 	y,
	w,
	h,
	0xffffff)
{
	this->plugin = plugin;
	this->gui = gui;
}

int HistogramCanvas::button_press_event()
{
	int result = 0;
	if(is_event_win() && cursor_inside())
	{
		if(!plugin->dragging_point)
		{
			HistogramPoint *new_point = 0;
			gui->deactivate();
// Search for existing point under cursor
			HistogramPoint *current = plugin->config.points[plugin->mode].first;
			plugin->current_point = -1;
			int dragID = 0;
			while(current)
			{
				int x = (int)((current->x - HIST_MIN_INPUT) * gui->canvas_w / FLOAT_RANGE);
				int y = (int)(gui->canvas_h - current->y * gui->canvas_h);

/* Check for click on main point */
				if(get_cursor_x() >= x - BOX_SIZE / 2 &&
					get_cursor_y() >= y - BOX_SIZE / 2 &&
					get_cursor_x() < x + BOX_SIZE / 2 &&
					get_cursor_y() < y + BOX_SIZE / 2)
				{
					plugin->current_point = 
						plugin->config.points[plugin->mode].number_of(current);
					plugin->point_x_offset = get_cursor_x() - x;
					plugin->point_y_offset = get_cursor_y() - y;
					dragID = 1;
					break;
				}
				if (plugin->config.smoothMode == HISTOGRAM_LINEAR)
				  break;

				int xright = 
				  (int)((current->x + current->xoffset_right - HIST_MIN_INPUT) * gui->canvas_w / FLOAT_RANGE);
				int yright = 
				  (int)(gui->canvas_h - (current->y + current->xoffset_right * current->gradient) * 
				  gui->canvas_h);

/* Check for click on right handle */				
				if(get_cursor_x() >= xright - BOX_SIZE / 2 &&
				   get_cursor_y() >= yright - BOX_SIZE / 2 &&
				   get_cursor_x() <  xright + BOX_SIZE / 2 &&
				   get_cursor_y() <  yright + BOX_SIZE / 2)
				{
					plugin->current_point =
						plugin->config.points[plugin->mode].number_of(current);
					plugin->point_x_offset = get_cursor_x() - xright;
					plugin->point_y_offset = get_cursor_y() - yright;
					dragID = 2;
					break;
				}
				
/* Check for click on left handle */
				int xleft = 
				  (int)((current->x + current->xoffset_left - HIST_MIN_INPUT) * gui->canvas_w / FLOAT_RANGE);
				int yleft = 
				  (int)(gui->canvas_h - (current->y + current->xoffset_left * current->gradient) * 
				  gui->canvas_h);
				if(get_cursor_x() >= xleft - BOX_SIZE / 2 &&
				   get_cursor_y() >= yleft - BOX_SIZE / 2 &&
				   get_cursor_x() <  xleft + BOX_SIZE / 2 &&
				   get_cursor_y() <  yleft + BOX_SIZE / 2)
				{
					plugin->current_point =
						plugin->config.points[plugin->mode].number_of(current);
					plugin->point_x_offset = get_cursor_x() - xleft;
					plugin->point_y_offset = get_cursor_y() - yleft;
					dragID = 3;
					break;
				}


				current = NEXT;
			}

			if(plugin->current_point < 0)
			{
// Create new point under cursor
				float current_x = (float)get_cursor_x() * 
					FLOAT_RANGE / 
					get_w() + 
					HIST_MIN_INPUT;
				float current_y = 1.0 - 
					(float)get_cursor_y() / 
					get_h();
				new_point = 
					plugin->config.points[plugin->mode].insert(current_x, current_y);
				plugin->current_point = 
					plugin->config.points[plugin->mode].number_of(new_point);
				plugin->point_x_offset = 0;
				plugin->point_y_offset = 0;
				
// Default gradient		
// Get 2 points surrounding current position
				float x1,x2,y1,y2;

				HistogramPoint *current = plugin->config.points[plugin->mode].first;
				int done = 0;
				while(current && !done)
				{
					if(current->x > current_x)
					{
						x2 = current->x;
						y2 = current->y;
						done = 1;
					}
					else
						current = NEXT;
				}
		
				current = plugin->config.points[plugin->mode].last;
				done = 0;
				while(current && !done)
				{
					if(current->x <= current_x)
					{
						x1 = current->x;
						y1 = current->y;
						done = 1;
					}
					else
						current = PREVIOUS;
				}
				new_point->gradient = (y2 - y1) / (x2 - x1);
				dragID = 1;
						
			}
			

			plugin->dragging_point = dragID;
			result = 1;

			plugin->config.boundaries();
			gui->update_input();
			gui->update_canvas();
			if(new_point)
			{
				plugin->send_configure_change();
			}
		}
	}
	return result;
}

int HistogramCanvas::cursor_motion_event()
{
	if(plugin->dragging_point)
	{
		HistogramPoint * current_point = plugin->config.points[plugin->mode].get_item_number(plugin->current_point);

	  	float current_x = 
			(float)(get_cursor_x() - plugin->point_x_offset) * 
			FLOAT_RANGE / 
			get_w() + 
			HIST_MIN_INPUT;
		float current_y = 1.0 - 
			(float)(get_cursor_y() - plugin->point_y_offset) / 
			get_h();

		switch(plugin->dragging_point)
		{
		  case 1: /* Main point dragged */  
			current_point->x = current_x;
			current_point->y = current_y;
			break;
		  case 2: /* Right control point dragged */
		        if (current_x - current_point->x > 0)
		  	{
		  	  current_point->xoffset_right = current_x - current_point->x;
		  	  current_point->gradient = (current_y - current_point->y) / (current_x - current_point->x);
		  	}
		  	break;
		  case 3: /* Left control point dragged */
		  	if (current_x - current_point->x < 0)
		  	{
		  	  current_point->xoffset_left = current_x - current_point->x;
		  	  current_point->gradient = (current_point->y - current_y) / (current_point->x - current_x);
		  	}
		  	break;
		}
		  
		plugin->config.boundaries();
		gui->update_input();
		gui->update_canvas();
		plugin->send_configure_change();
		return 1;
	}
	return 0;
}

int HistogramCanvas::button_release_event()
{
	if(plugin->dragging_point)
	{
// Test for out of order points to delete.
		HistogramPoint *current = 
			plugin->config.points[plugin->mode].get_item_number(plugin->current_point);
		HistogramPoint *prev = PREVIOUS;
		HistogramPoint *next = NEXT;

		if((prev && prev->x >= current->x) ||
			(next && next->x <= current->x))
		{
			delete current;
			plugin->current_point = -1;
			plugin->config.boundaries();
			gui->update_input();
			gui->update_canvas();
			plugin->send_configure_change();
		}

		plugin->dragging_point = 0;
	}
	return 0;
}







HistogramReset::HistogramReset(HistogramMain *plugin, 
	int x,
	int y)
 : BC_GenericButton(x, y, _("Reset"))
{
	this->plugin = plugin;
}
int HistogramReset::handle_event()
{
	plugin->config.reset(0);
	plugin->thread->window->update(1);
	plugin->thread->window->update_canvas();
	plugin->send_configure_change();
	return 1;
}









HistogramSlider::HistogramSlider(HistogramMain *plugin, 
	HistogramWindow *gui,
	int x, 
	int y, 
	int w,
	int h,
	int is_input)
 : BC_SubWindow(x, y, w, h)
{
	this->plugin = plugin;
	this->gui = gui;
	this->is_input = is_input;
	operation = NONE;
}

int HistogramSlider::input_to_pixel(float input)
{
	return (int)((input - HIST_MIN_INPUT) / FLOAT_RANGE * get_w());
}

int HistogramSlider::button_press_event()
{
	if(is_event_win() && cursor_inside())
	{
		int min;
		int max;
		int w = get_w();
		int h = get_h();
		int half_h = get_h() / 2;

		gui->deactivate();

		if(operation == NONE)
		{
			int x1 = input_to_pixel(plugin->config.output_min[plugin->mode]) - 
				gui->mid_picon->get_w() / 2;
			int x2 = x1 + gui->mid_picon->get_w();
			if(get_cursor_x() >= x1 && get_cursor_x() < x2 &&
				get_cursor_y() >= half_h && get_cursor_y() < h)
			{
				operation = DRAG_MIN_OUTPUT;
			}
		}

		if(operation == NONE)
		{
			int x1 = input_to_pixel(plugin->config.output_max[plugin->mode]) - 
				gui->mid_picon->get_w() / 2;
			int x2 = x1 + gui->mid_picon->get_w();
			if(get_cursor_x() >= x1 && get_cursor_x() < x2 &&
				get_cursor_y() >= half_h && get_cursor_y() < h)
			{
				operation = DRAG_MAX_OUTPUT;
			}
		}
		return 1;
	}
	return 0;
}

int HistogramSlider::button_release_event()
{
	if(operation != NONE)
	{
		operation = NONE;
		return 1;
	}
	return 0;
}

int HistogramSlider::cursor_motion_event()
{
	if(operation != NONE)
	{
		float value = (float)get_cursor_x() / get_w() * FLOAT_RANGE + HIST_MIN_INPUT;
		CLAMP(value, HIST_MIN_INPUT, HIST_MAX_INPUT);

		switch(operation)
		{
			case DRAG_MIN_OUTPUT:
				value = MIN(plugin->config.output_max[plugin->mode], value);
				plugin->config.output_min[plugin->mode] = value;
				break;
			case DRAG_MAX_OUTPUT:
				value = MAX(plugin->config.output_min[plugin->mode], value);
				plugin->config.output_max[plugin->mode] = value;
				break;
		}
	
		plugin->config.boundaries();
		gui->update_output();

		plugin->send_configure_change();
		return 1;
	}
	return 0;
}

void HistogramSlider::update()
{
	int w = get_w();
	int h = get_h();
	int half_h = get_h() / 2;
	int quarter_h = get_h() / 4;
	int mode = plugin->mode;
	int r = 0xff;
	int g = 0xff;
	int b = 0xff;

	clear_box(0, 0, w, h);

	switch(mode)
	{
		case HISTOGRAM_RED:
			g = b = 0x00;
			break;
		case HISTOGRAM_GREEN:
			r = b = 0x00;
			break;
		case HISTOGRAM_BLUE:
			r = g = 0x00;
			break;
	}

	for(int i = 0; i < w; i++)
	{
		int color = (int)(i * 0xff / w);
		set_color(((r * color / 0xff) << 16) | 
			((g * color / 0xff) << 8) | 
			(b * color / 0xff));

		draw_line(i, 0, i, half_h);
	}

	float min;
	float max;
	min = plugin->config.output_min[plugin->mode];
	max = plugin->config.output_max[plugin->mode];

	draw_pixmap(gui->min_picon,
		input_to_pixel(min) - gui->min_picon->get_w() / 2,
		half_h + 1);
	draw_pixmap(gui->max_picon,
		input_to_pixel(max) - gui->max_picon->get_w() / 2,
		half_h + 1);

	flash();
	flush();
}









HistogramAuto::HistogramAuto(HistogramMain *plugin, 
	int x, 
	int y)
 : BC_CheckBox(x, y, plugin->config.automatic, _("Automatic"))
{
	this->plugin = plugin;
}

int HistogramAuto::handle_event()
{
	plugin->config.automatic = get_value();
	plugin->send_configure_change();
	return 1;
}




HistogramSplit::HistogramSplit(HistogramMain *plugin, 
	int x, 
	int y)
 : BC_CheckBox(x, y, plugin->config.split, _("Split picture"))
{
	this->plugin = plugin;
}

int HistogramSplit::handle_event()
{
	plugin->config.split = get_value();
	plugin->send_configure_change();
	return 1;
}




HistogramMode::HistogramMode(HistogramMain *plugin, 
	int x, 
	int y,
	int value,
	char *text)
 : BC_Radial(x, y, plugin->mode == value, text)
{
	this->plugin = plugin;
	this->value = value;
}
int HistogramMode::handle_event()
{
	plugin->mode = value;
	plugin->current_point= -1;
	plugin->thread->window->update_canvas();
	plugin->thread->window->update_mode();
	plugin->thread->window->update_input();
	plugin->thread->window->update_canvas();
	plugin->thread->window->update_output();
	plugin->thread->window->output->update();
//	plugin->send_configure_change();
	return 1;
}









HistogramOutputText::HistogramOutputText(HistogramMain *plugin,
	HistogramWindow *gui,
	int x,
	int y,
	float *output)
 : BC_TumbleTextBox(gui, 
		output ? (float)*output : 0.0,
		(float)HIST_MIN_INPUT,
		(float)HIST_MAX_INPUT,
		x, 
		y, 
		60)
{
	this->plugin = plugin;
	this->output = output;
	set_precision(DIGITS);
	set_increment(PRECISION);
}


int HistogramOutputText::handle_event()
{
	if(output)
	{
		*output = atof(get_text());
	}

	plugin->thread->window->output->update();
	plugin->send_configure_change();
	return 1;
}








HistogramInputText::HistogramInputText(HistogramMain *plugin,
	HistogramWindow *gui,
	int x,
	int y,
	int do_x)
 : BC_TumbleTextBox(gui, 
		0.0,
		(float)HIST_MIN_INPUT,
		(float)HIST_MAX_INPUT,
		x, 
		y, 
		60)
{
	this->do_x = do_x;
	this->plugin = plugin;
	this->gui = gui;
	set_precision(DIGITS);
	set_increment(PRECISION);
}


int HistogramInputText::handle_event()
{
	if(plugin->current_point >= 0 &&
		plugin->current_point < plugin->config.points[plugin->mode].total())
	{
		HistogramPoint *point = 
			plugin->config.points[plugin->mode].get_item_number(
				plugin->current_point);

		if(point)
		{
			if(do_x) 
				point->x = atof(get_text());
			else
				point->y = atof(get_text());

			plugin->config.boundaries();
			gui->update_canvas();

			plugin->thread->window->output->update();
			plugin->send_configure_change();
		}
	}
	return 1;
}

void HistogramInputText::update()
{
	if(plugin->current_point >= 0 &&
		plugin->current_point < plugin->config.points[plugin->mode].total())
	{
		HistogramPoint *point = 

			plugin->config.points[plugin->mode].get_item_number(
				plugin->current_point);

		if(point)
		{
			if(do_x)
				BC_TumbleTextBox::update(point->x);
			else
				BC_TumbleTextBox::update(point->y);
		}
		else
		{
			BC_TumbleTextBox::update((float)0.0);
		}
	}
	else
	{
		BC_TumbleTextBox::update((float)0.0);
	}
	
}


HistogramSmoothMode::HistogramSmoothMode(HistogramMain*plugin, 
	HistogramWindow *gui, 
	int x, 
	int y)
 : BC_PopupMenu(x, y, 120, to_text(plugin->config.smoothMode), 1)
{
	this->plugin = plugin;
	this->gui = gui;
}
void HistogramSmoothMode::create_objects()
{
	add_item(new BC_MenuItem(to_text(HISTOGRAM_LINEAR)));
	add_item(new BC_MenuItem(to_text(HISTOGRAM_POLYNOMINAL)));
	add_item(new BC_MenuItem(to_text(HISTOGRAM_BEZIER)));
}

char* HistogramSmoothMode::to_text(int mode)
{
	switch(mode)
	{
		case HISTOGRAM_LINEAR:
			return _("Linear");
		case HISTOGRAM_POLYNOMINAL:
			return _("Polynominal");
		case HISTOGRAM_BEZIER:
			return _("Bezier");
	}
}
int HistogramSmoothMode::from_text(char *text)
{
	if(!strcmp(text, to_text(HISTOGRAM_LINEAR))) 
		return HISTOGRAM_LINEAR;
	if(!strcmp(text, to_text(HISTOGRAM_POLYNOMINAL))) 
		return HISTOGRAM_POLYNOMINAL;
	if(!strcmp(text, to_text(HISTOGRAM_BEZIER))) 
		return HISTOGRAM_BEZIER;
}

int HistogramSmoothMode::handle_event()
{
	plugin->config.smoothMode = from_text(get_text());
	gui->update_canvas();
	plugin->send_configure_change();
}














