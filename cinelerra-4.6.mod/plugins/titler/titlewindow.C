
/*
 * CINELERRA
 * Copyright (C) 1997-2014 Adam Williams <broadcast at earthling dot net>
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
#include "cstrdup.h"
#include "language.h"
#include "theme.h"
#include "titlewindow.h"
#include "bcfontentry.h"

#include <wchar.h>

static const int timeunit_formats[] =
{
	TIME_HMS,
	TIME_SECONDS,
	TIME_HMSF,
	TIME_SAMPLES,
	TIME_SAMPLES_HEX,
	TIME_FRAMES,
	TIME_FEET_FRAMES
};

TitleWindow::TitleWindow(TitleMain *client)
 : PluginClientWindow(client,
	client->config.window_w,
	client->config.window_h,
	100,
	100,
	1)
{
//printf("TitleWindow::TitleWindow %d %d %d\n", __LINE__, client->config.window_w, client->config.window_h);
	this->client = client;
	font_tumbler = 0;
	justify_title = 0;
	style_title = 0;
	size_title = 0;
	title_y = 0;
	bottom = 0;
	size = 0;
	loop = 0;
	title_x = 0;
	dropshadow = 0;
	motion = 0;
	dropshadow_title = 0;
	text = 0;
	timecode = 0;
	fade_in = 0;
	encoding_title = 0;
	x_title = 0;
	bold = 0;
	color_y = 0;
	speed = 0;
	center = 0;
	italic = 0;
	text_title = 0;
	motion_title = 0;
	fadeout_title = 0;
	font_title = 0;
	fadein_title = 0;
	fade_out = 0;
	color_button = 0;
	left = 0;
	speed_title = 0;
	top = 0;
	font = 0;
	right = 0;
	color_x = 0;
	color_y = 0;
	y_title = 0;
	color_thread = 0;
	mid = 0;
	encoding_title = 0;
	encoding = 0;
}

TitleWindow::~TitleWindow()
{
	for(int j = 0; j < fonts.size(); j++)
	{
// delete the pixmaps but not the vframes since they're static
		delete fonts.get(j)->get_icon();
	}

	sizes.remove_all_objects();
	delete timecode_format;
	delete color_thread;
#ifdef USE_OUTLINE
	delete color_stroke_thread;
#endif
	delete title_x;
	delete title_y;
}

void TitleWindow::create_objects()
{
	int x = 10, y = 10;
	int margin = client->get_theme()->widget_border;
	char string[BCTEXTLEN];

#define COLOR_W 50
#define COLOR_H 30
	client->build_previews(this);

	encodings.append(new BC_ListBoxItem("ISO8859-1"));
	encodings.append(new BC_ListBoxItem("ISO8859-2"));
	encodings.append(new BC_ListBoxItem("ISO8859-3"));
	encodings.append(new BC_ListBoxItem("ISO8859-4"));
	encodings.append(new BC_ListBoxItem("ISO8859-5"));
	encodings.append(new BC_ListBoxItem("ISO8859-6"));
	encodings.append(new BC_ListBoxItem("ISO8859-7"));
	encodings.append(new BC_ListBoxItem("ISO8859-8"));
	encodings.append(new BC_ListBoxItem("ISO8859-9"));
	encodings.append(new BC_ListBoxItem("ISO8859-10"));
	encodings.append(new BC_ListBoxItem("ISO8859-11"));
	encodings.append(new BC_ListBoxItem("ISO8859-12"));
	encodings.append(new BC_ListBoxItem("ISO8859-13"));
	encodings.append(new BC_ListBoxItem("ISO8859-14"));
	encodings.append(new BC_ListBoxItem("ISO8859-15"));
	encodings.append(new BC_ListBoxItem("KOI8"));
	encodings.append(new BC_ListBoxItem("UTF-8"));


	sizes.append(new BC_ListBoxItem("8"));
	sizes.append(new BC_ListBoxItem("9"));
	sizes.append(new BC_ListBoxItem("10"));
	sizes.append(new BC_ListBoxItem("11"));
	sizes.append(new BC_ListBoxItem("12"));
	sizes.append(new BC_ListBoxItem("13"));
	sizes.append(new BC_ListBoxItem("14"));
	sizes.append(new BC_ListBoxItem("16"));
	sizes.append(new BC_ListBoxItem("18"));
	sizes.append(new BC_ListBoxItem("20"));
	sizes.append(new BC_ListBoxItem("22"));
	sizes.append(new BC_ListBoxItem("24"));
	sizes.append(new BC_ListBoxItem("26"));
	sizes.append(new BC_ListBoxItem("28"));
	sizes.append(new BC_ListBoxItem("32"));
	sizes.append(new BC_ListBoxItem("36"));
	sizes.append(new BC_ListBoxItem("40"));
	sizes.append(new BC_ListBoxItem("48"));
	sizes.append(new BC_ListBoxItem("56"));
	sizes.append(new BC_ListBoxItem("64"));
	sizes.append(new BC_ListBoxItem("72"));
	sizes.append(new BC_ListBoxItem("100"));
	sizes.append(new BC_ListBoxItem("128"));
	sizes.append(new BC_ListBoxItem("256"));
	sizes.append(new BC_ListBoxItem("512"));
	sizes.append(new BC_ListBoxItem("1024"));

	paths.append(new BC_ListBoxItem(TitleMain::motion_to_text(NO_MOTION)));
	paths.append(new BC_ListBoxItem(TitleMain::motion_to_text(BOTTOM_TO_TOP)));
	paths.append(new BC_ListBoxItem(TitleMain::motion_to_text(TOP_TO_BOTTOM)));
	paths.append(new BC_ListBoxItem(TitleMain::motion_to_text(RIGHT_TO_LEFT)));
	paths.append(new BC_ListBoxItem(TitleMain::motion_to_text(LEFT_TO_RIGHT)));



// Construct font list
	ArrayList<BC_FontEntry*> *fontlist = get_resources()->fontlist;

	for(int i = 0; i < fontlist->size(); i++) {
		int exists = 0;
		for(int j = 0; j < fonts.size(); j++) {
			if(!strcasecmp(fonts.get(j)->get_text(),
				fontlist->get(i)->displayname)) {
				exists = 1;
				break;
			}
		}

		BC_ListBoxItem *item = 0;
		if(!exists) {
			fonts.append(item = new
				BC_ListBoxItem(fontlist->get(i)->displayname));
			if(!strcmp(client->config.font, item->get_text()))
				item->set_selected(1);
			if(fontlist->values[i]->image)
			{
				VFrame *vframe = fontlist->get(i)->image;
				BC_Pixmap *icon = new BC_Pixmap(this, vframe, PIXMAP_ALPHA);
				item->set_icon(icon);
				item->set_icon_vframe(vframe);
			}
		}
	}

// Sort font list
	int done = 0;
	while(!done) {
		done = 1;
		for(int i = 0; i < fonts.size() - 1; i++) {
			if(strcmp(fonts.values[i]->get_text(),
				fonts.values[i + 1]->get_text()) > 0) {
				BC_ListBoxItem *temp = fonts.values[i + 1];
				fonts.values[i + 1] = fonts.values[i];
				fonts.values[i] = temp;
				done = 0;
			}
		}
	}

	add_tool(font_title = new BC_Title(x, y, _("Font:")));
	font = new TitleFont(client, this, x, y + font_title->get_h());
	font->create_objects();
	x += font->get_w();
	add_subwindow(font_tumbler = new TitleFontTumble(client, this, x, y+10));
	x += font_tumbler->get_w() + margin;

	int x1 = x, y1 = y;
	add_tool(size_title = new BC_Title(x1, y1+10, _("Size:")));
	sprintf(string, "%d", client->config.size);
	x1 += size_title->get_w() + margin;
	size = new TitleSize(client, this, x1, y1+10, string);
	size->create_objects();
	int x2 = x1 + size->get_w(), y2 = y1 + size->get_h() + margin;
	add_subwindow(size_tumbler = new TitleSizeTumble(client, this, x2, y1+10));

	add_tool(pitch_title = new BC_Title(x-5, y2+10, _("Pitch:")));
	pitch = new TitlePitch(client, this, x1, y2+10, &client->config.line_pitch);
	pitch->create_objects();
	x = x2 + size_tumbler->get_w() + margin;

	add_tool(style_title = new BC_Title(x, y, _("Style:")));
	add_tool(italic = new TitleItalic(client, this, x, y + 20));
	add_tool(bold = new TitleBold(client, this, x, y + 50));
#ifdef USE_OUTLINE
	add_tool(stroke = new TitleStroke(client, this, x, y + 110));
#endif
	x += 90;
	add_tool(justify_title = new BC_Title(x, y, _("Justify:")));
	add_tool(left = new TitleLeft(client, this, x, y + 20));
	add_tool(center = new TitleCenter(client, this, x, y + 50));
	add_tool(right = new TitleRight(client, this, x, y + 80));

	x += 80;
	add_tool(top = new TitleTop(client, this, x, y + 20));
	add_tool(mid = new TitleMid(client, this, x, y + 50));
	add_tool(bottom= new TitleBottom(client, this, x, y + 80));

	y += 50;
	x = 10;

	add_tool(x_title = new BC_Title(x, y, _("X:")));
	title_x = new TitleX(client, this, x, y + 20);
	title_x->create_objects();
	x += 90;

	add_tool(y_title = new BC_Title(x, y, _("Y:")));
	title_y = new TitleY(client, this, x, y + 20);
	title_y->create_objects();
	x += 90;

	add_tool(motion_title = new BC_Title(x, y, _("Motion type:")));

	motion = new TitleMotion(client, this, x, y + 20);
	motion->create_objects();
	x += 150;

	add_tool(loop = new TitleLoop(client, x, y + 20));

	x = 10;
	y += 50;

	add_tool(dropshadow_title = new BC_Title(x, y, _("Drop shadow:")));
	dropshadow = new TitleDropShadow(client, this, x, y + 20);
	dropshadow->create_objects();
	x += 100;

	add_tool(fadein_title = new BC_Title(x, y, _("Fade in (sec):")));
	add_tool(fade_in = new TitleFade(client, this, &client->config.fade_in, x, y + 20));
	x += 100;

	add_tool(fadeout_title = new BC_Title(x, y, _("Fade out (sec):")));
	add_tool(fade_out = new TitleFade(client, this, &client->config.fade_out, x, y + 20));
	x += 110;

	add_tool(speed_title = new BC_Title(x, y, _("Speed:")));
	speed = new TitleSpeed(client, this, x, y + 20);
	speed->create_objects();
	x += 110;

	color_x = x;
	color_y = y + 20;
	x += COLOR_W + 5;
	add_tool(color_button = new TitleColorButton(client, this, x, y + 20, 0));
	x += color_button->get_w();
	color_thread = new TitleColorThread(client, this, 0);

	x = color_x;
	y += 50;

	outline_color_x = x;
	outline_color_y = y + 20;
	x += COLOR_W + 5;
	add_tool(outline_color_button = new TitleColorButton(client, this, x, y + 20, 1));
	x += outline_color_button->get_w();
	outline_color_thread = new TitleColorThread(client, this, 1);

	x = 10;
//	y += 50;

	add_tool(outline_title = new BC_Title(x, y, _("Outline:")));
	outline = new TitleOutline(client, this, x, y + outline_title->get_h() + margin);
	outline->create_objects();
	x += outline->get_w() + margin;

#ifndef X_HAVE_UTF8_STRING
	add_tool(encoding_title = new BC_Title(x, y + 3, _("Encoding:")));
	encoding = new TitleEncoding(client, this, x, y + encoding_title->get_h() + margin);
	encoding->create_objects();
	x += 100;
#endif

	y += outline_title->get_h() + margin;
	add_tool(timecode = new TitleTimecode(client, x, y));
	x += timecode->get_w() + margin;

	add_tool(timecode_format = new TitleTimecodeFormat(client, x, y,
		Units::print_time_format(client->config.timecode_format, string)));
	timecode_format->create_objects();
	x = 10;
	y += timecode_format->get_h() + margin;

	add_tool(text_title = new BC_Title(x, y + 3, _("Text:")));
	y += text_title->get_h() + margin;
	text = new TitleText(client, this, x, y, get_w() - x - 10, get_h() - y - 50);
	text->create_objects();
	update();

	show_window(1);
}

int TitleWindow::resize_event(int w, int h)
{
	client->config.window_w = w;
	client->config.window_h = h;

	clear_box(0, 0, w, h);
	font_title->reposition_window(font_title->get_x(), font_title->get_y());
	font->reposition_window(font->get_x(), font->get_y());
	font_tumbler->reposition_window(font_tumbler->get_x(), font_tumbler->get_y());
	x_title->reposition_window(x_title->get_x(), x_title->get_y());
	title_x->reposition_window(title_x->get_x(), title_x->get_y());
	y_title->reposition_window(y_title->get_x(), y_title->get_y());
	title_y->reposition_window(title_y->get_x(), title_y->get_y());
	style_title->reposition_window(style_title->get_x(), style_title->get_y());
	italic->reposition_window(italic->get_x(), italic->get_y());
	bold->reposition_window(bold->get_x(), bold->get_y());
#ifdef USE_OUTLINE
	stroke->reposition_window(stroke->get_x(), stroke->get_y());
#endif
	size_title->reposition_window(size_title->get_x(), size_title->get_y());
	size->reposition_window(size->get_x(), size->get_y());
	size_tumbler->reposition_window(size_title->get_x(), size_title->get_y());
	pitch_title->reposition_window(pitch_title->get_x(), pitch_title->get_y());
	pitch->reposition_window(size->get_x(), size->get_y());

#ifndef X_HAVE_UTF8_STRING
	encoding->reposition_window(encoding->get_x(), encoding->get_y());
#endif

	color_button->reposition_window(color_button->get_x(), color_button->get_y());
#ifdef USE_OUTLINE
	color_stroke_button->reposition_window(color_stroke_button->get_x(), color_stroke_button->get_y());
#endif
	outline_color_button->reposition_window(outline_color_button->get_x(), outline_color_button->get_y());
	motion_title->reposition_window(motion_title->get_x(), motion_title->get_y());
	motion->reposition_window(motion->get_x(), motion->get_y());
	loop->reposition_window(loop->get_x(), loop->get_y());
	dropshadow_title->reposition_window(dropshadow_title->get_x(), dropshadow_title->get_y());
	dropshadow->reposition_window(dropshadow->get_x(), dropshadow->get_y());
	fadein_title->reposition_window(fadein_title->get_x(), fadein_title->get_y());
	fade_in->reposition_window(fade_in->get_x(), fade_in->get_y());
	fadeout_title->reposition_window(fadeout_title->get_x(), fadeout_title->get_y());
	fade_out->reposition_window(fade_out->get_x(), fade_out->get_y());
	text_title->reposition_window(text_title->get_x(), text_title->get_y());
#ifdef USE_OUTLINE
	stroke_width->reposition_window(stroke_width->get_x(), stroke_width->get_y());
	strokewidth_title->reposition_window(strokewidth_title->get_x(), strokewidth_title->get_y());
#endif
	timecode->reposition_window(timecode->get_x(), timecode->get_y());

	text->reposition_window(text->get_x(),
		text->get_y(),
		w - text->get_x() - 10,
		BC_TextBox::pixels_to_rows(this, MEDIUMFONT, h - text->get_y() - 10));



	justify_title->reposition_window(justify_title->get_x(), justify_title->get_y());
	left->reposition_window(left->get_x(), left->get_y());
	center->reposition_window(center->get_x(), center->get_y());
	right->reposition_window(right->get_x(), right->get_y());
	top->reposition_window(top->get_x(), top->get_y());
	mid->reposition_window(mid->get_x(), mid->get_y());
	bottom->reposition_window(bottom->get_x(), bottom->get_y());
	speed_title->reposition_window(speed_title->get_x(), speed_title->get_y());
	speed->reposition_window(speed->get_x(), speed->get_y());
	update_color();
	flash();

	return 1;
}


void TitleWindow::previous_font()
{
	int current_font = font->get_number();
	current_font--;
	if(current_font < 0) current_font = fonts.total - 1;

	if(current_font < 0 || current_font >= fonts.total) return;

	for(int i = 0; i < fonts.total; i++)
	{
		fonts.values[i]->set_selected(i == current_font);
	}

	font->update(fonts.values[current_font]->get_text());
	strcpy(client->config.font, fonts.values[current_font]->get_text());
	client->send_configure_change();
}

void  TitleWindow::next_font()
{
	int current_font = font->get_number();
	current_font++;
	if(current_font >= fonts.total) current_font = 0;

	if(current_font < 0 || current_font >= fonts.total) return;

	for(int i = 0; i < fonts.total; i++)
	{
		fonts.values[i]->set_selected(i == current_font);
	}

	font->update(fonts.values[current_font]->get_text());
	strcpy(client->config.font, fonts.values[current_font]->get_text());
	client->send_configure_change();
}



void TitleWindow::update_color()
{
//printf("TitleWindow::update_color %x\n", client->config.color);
	set_color(client->config.color);
	draw_box(color_x, color_y, COLOR_W, COLOR_H);
	flash(color_x, color_y, COLOR_W, COLOR_H);
	set_color(client->config.outline_color);
	draw_box(outline_color_x, outline_color_y, COLOR_W, COLOR_H);
	set_color(BLACK);
	draw_rectangle(color_x, color_y, COLOR_W, COLOR_H);
	draw_rectangle(outline_color_x, outline_color_y, COLOR_W, COLOR_H);
	flash(outline_color_x, outline_color_y, COLOR_W, COLOR_H);
}

void TitleWindow::update_justification()
{
	left->update(client->config.hjustification == JUSTIFY_LEFT);
	center->update(client->config.hjustification == JUSTIFY_CENTER);
	right->update(client->config.hjustification == JUSTIFY_RIGHT);
	top->update(client->config.vjustification == JUSTIFY_TOP);
	mid->update(client->config.vjustification == JUSTIFY_MID);
	bottom->update(client->config.vjustification == JUSTIFY_BOTTOM);
}

void TitleWindow::update()
{
	title_x->update((int64_t)client->config.x);
	title_y->update((int64_t)client->config.y);
	italic->update(client->config.style & BC_FONT_ITALIC);
	bold->update(client->config.style & BC_FONT_BOLD);
#ifdef USE_OUTLINE
	stroke->update(client->config.style & BC_FONT_OUTLINE);
#endif
	size->update(client->config.size);
#ifndef X_HAVE_UTF8_STRING
	encoding->update(client->config.encoding);
#endif
	motion->update(TitleMain::motion_to_text(client->config.motion_strategy));
	loop->update(client->config.loop);
	dropshadow->update((float)client->config.dropshadow);
	fade_in->update((float)client->config.fade_in);
	fade_out->update((float)client->config.fade_out);
#ifdef USE_OUTLINE
	stroke_width->update((float)client->config.stroke_width);
#endif
	font->update(client->config.font);
	text->update(&client->config.wtext[0]);
	speed->update(client->config.pixels_per_second);
	outline->update((int64_t)client->config.outline_size);
	timecode->update(client->config.timecode);
	timecode_format->update(client->config.timecode_format);

	char string[BCTEXTLEN];
	for(int i = 0; i < lengthof(timeunit_formats); i++) {
		if(timeunit_formats[i] == client->config.timecode_format)
		{
			timecode_format->set_text(
				Units::print_time_format(timeunit_formats[i], string));
			break;
		}
	}
	update_justification();
	update_color();
}


TitleFontTumble::TitleFontTumble(TitleMain *client, TitleWindow *window, int x, int y)
 : BC_Tumbler(x, y)
{
	this->client = client;
	this->window = window;
}
int TitleFontTumble::handle_up_event()
{
	window->previous_font();
	return 1;
}

int TitleFontTumble::handle_down_event()
{
	window->next_font();
	return 1;
}



TitleSizeTumble::TitleSizeTumble(TitleMain *client, TitleWindow *window, int x, int y)
 : BC_Tumbler(x, y)
{
	this->client = client;
	this->window = window;
}

int TitleSizeTumble::handle_up_event()
{
	int current_index = -1;
	int current_difference = -1;
	for(int i = 0; i < window->sizes.size(); i++)
	{
		int size = atoi(window->sizes.get(i)->get_text());
		if(current_index < 0 ||
			abs(size - client->config.size) < current_difference)
		{
			current_index = i;
			current_difference = abs(size - client->config.size);
		}
	}

	current_index++;
	if(current_index >= window->sizes.size()) current_index = 0;


	client->config.size = atoi(window->sizes.get(current_index)->get_text());
	window->size->update(client->config.size);
	client->send_configure_change();
	return 1;
}

int TitleSizeTumble::handle_down_event()
{
	int current_index = -1;
	int current_difference = -1;
	for(int i = 0; i < window->sizes.size(); i++)
	{
		int size = atoi(window->sizes.get(i)->get_text());
		if(current_index < 0 ||
			abs(size - client->config.size) < current_difference)
		{
			current_index = i;
			current_difference = abs(size - client->config.size);
		}
	}

	current_index--;
	if(current_index < 0) current_index = window->sizes.size() - 1;


	client->config.size = atoi(window->sizes.get(current_index)->get_text());
	window->size->update(client->config.size);
	client->send_configure_change();
	return 1;
}

TitleBold::TitleBold(TitleMain *client, TitleWindow *window, int x, int y)
 : BC_CheckBox(x, y, client->config.style & BC_FONT_BOLD, _("Bold"))
{
	this->client = client;
	this->window = window;
}

int TitleBold::handle_event()
{
	client->config.style =
		(client->config.style & ~BC_FONT_BOLD) |
			(get_value() ? BC_FONT_BOLD : 0);
	client->send_configure_change();
	return 1;
}

TitleItalic::TitleItalic(TitleMain *client, TitleWindow *window, int x, int y)
 : BC_CheckBox(x, y, client->config.style & BC_FONT_ITALIC, _("Italic"))
{
	this->client = client;
	this->window = window;
}
int TitleItalic::handle_event()
{
	client->config.style =
		(client->config.style & ~BC_FONT_ITALIC) |
			(get_value() ? BC_FONT_ITALIC : 0);
	client->send_configure_change();
	return 1;
}



TitleSize::TitleSize(TitleMain *client, TitleWindow *window, int x, int y, char *text)
 : BC_PopupTextBox(window,
		&window->sizes,
		text,
		x,
		y,
		64,
		300)
{
	this->client = client;
	this->window = window;
}
TitleSize::~TitleSize()
{
}
int TitleSize::handle_event()
{
	client->config.size = atol(get_text());
//printf("TitleSize::handle_event 1 %s\n", get_text());
	client->send_configure_change();
	return 1;
}
void TitleSize::update(int size)
{
	char string[BCTEXTLEN];
	sprintf(string, "%d", size);
	BC_PopupTextBox::update(string);
}

TitlePitch::
TitlePitch(TitleMain *client, TitleWindow *window, int x, int y, int *value)
 : BC_TumbleTextBox(window, *value, 0, INT_MAX, x, y, 64)

{
	this->client = client;
	this->window = window;
	this->value = value;
}

TitlePitch::
~TitlePitch()
{
}

int TitlePitch::handle_event()
{
	*value = atof(get_text());
	client->send_configure_change();
	return 1;
}

TitleColorButton::TitleColorButton(TitleMain *client,
	TitleWindow *window,
	int x,
	int y,
	int is_outline)
 : BC_GenericButton(x, y, is_outline ? _("Outline color...") : _("Color..."))
{
	this->client = client;
	this->window = window;
	this->is_outline = is_outline;
}
int TitleColorButton::handle_event()
{
	if(is_outline)
		window->outline_color_thread->start_window(client->config.outline_color,
			client->config.outline_alpha);
	else
		window->color_thread->start_window(client->config.color,
			client->config.alpha);
	return 1;
}

TitleMotion::TitleMotion(TitleMain *client, TitleWindow *window, int x, int y)
 : BC_PopupTextBox(window,
		&window->paths,
		client->motion_to_text(client->config.motion_strategy),
		x,
		y,
		120,
		100)
{
	this->client = client;
	this->window = window;
}
int TitleMotion::handle_event()
{
	client->config.motion_strategy = client->text_to_motion(get_text());
	client->send_configure_change();
	return 1;
}

TitleLoop::TitleLoop(TitleMain *client, int x, int y)
 : BC_CheckBox(x, y, client->config.loop, _("Loop"))
{
	this->client = client;
}
int TitleLoop::handle_event()
{
	client->config.loop = get_value();
	client->send_configure_change();
	return 1;
}
TitleTimecode::TitleTimecode(TitleMain *client, int x, int y)
 : BC_CheckBox(x, y, client->config.timecode, _("Stamp timecode"))
{
	this->client = client;
}
int TitleTimecode::handle_event()
{
	client->config.timecode = get_value();
	client->send_configure_change();
	return 1;
}

TitleTimecodeFormat::TitleTimecodeFormat(TitleMain *client, int x, int y, const char *text)
 : BC_PopupMenu(x, y, 100, text, 1)
{
	this->client = client;
}

int TitleTimecodeFormat::handle_event()
{
	client->config.timecode_format = Units::text_to_format(get_text());
	client->send_configure_change();
	return 1;
}

void TitleTimecodeFormat::create_objects()
{
	char string[BCTEXTLEN];
	for(int i = 0; i < lengthof(timeunit_formats); i++) {
		add_item(new BC_MenuItem(
			Units::print_time_format(timeunit_formats[i], string)));
	}
}


int TitleTimecodeFormat::update(int timecode_format)
{
	char string[BCTEXTLEN];
	for(int i = 0; i < lengthof(timeunit_formats); i++) {
		if(timeunit_formats[i] == timecode_format)
		{
			set_text(Units::print_time_format(timeunit_formats[i], string));
			break;
		}
	}
	return 0;
}

TitleFade::TitleFade(TitleMain *client,
	TitleWindow *window,
	double *value,
	int x,
	int y)
 : BC_TextBox(x, y, 90, 1, (float)*value)
{
	this->client = client;
	this->window = window;
	this->value = value;
}

int TitleFade::handle_event()
{
	*value = atof(get_text());
	client->send_configure_change();
	return 1;
}

TitleFont::TitleFont(TitleMain *client, TitleWindow *window, int x, int y)
 : BC_PopupTextBox(window,
		&window->fonts,
		client->config.font,
		x,
		y,
		200,
		500,
		LISTBOX_ICON_LIST)
{
	this->client = client;
	this->window = window;
}
int TitleFont::handle_event()
{
	strcpy(client->config.font, get_text());
	client->send_configure_change();
	return 1;
}

TitleText::TitleText(TitleMain *client,
	TitleWindow *window, int x, int y, int w, int h)
 : BC_ScrollTextBox(window, x, y, w,
		BC_TextBox::pixels_to_rows(window, MEDIUMFONT, h),
		client->config.wtext, 8192)
{
	this->client = client;
	this->window = window;
//printf("TitleText::TitleText %s\n", client->config.text);
}

int TitleText::handle_event()
{
	int len =  sizeof(client->config.wtext) / sizeof(wchar_t);
	wcsncpy(client->config.wtext, get_wtext(), len);
	client->config.wtext[len-1] = 0;
	client->config.wlen = wcslen(client->config.wtext);
	client->send_configure_change();
	return 1;
}


TitleDropShadow::TitleDropShadow(TitleMain *client, TitleWindow *window, int x, int y)
 : BC_TumbleTextBox(window,
 	(int64_t)client->config.dropshadow,
	(int64_t)0,
	(int64_t)1000,
	x,
	y,
	70)
{
	this->client = client;
	this->window = window;
}
int TitleDropShadow::handle_event()
{
	client->config.dropshadow = atol(get_text());
	client->send_configure_change();
	return 1;
}


TitleOutline::TitleOutline(TitleMain *client, TitleWindow *window, int x, int y)
 : BC_TumbleTextBox(window,
 	(int64_t)client->config.outline_size,
	(int64_t)0,
	(int64_t)1000,
	x,
	y,
	70)
{
	this->client = client;
	this->window = window;
}
int TitleOutline::handle_event()
{
	client->config.outline_size = atol(get_text());
	client->send_configure_change();
	return 1;
}


TitleX::TitleX(TitleMain *client, TitleWindow *window, int x, int y)
 : BC_TumbleTextBox(window,
 	(int64_t)client->config.x,
	(int64_t)-2048,
	(int64_t)2048,
	x,
	y,
	60)
{
	this->client = client;
	this->window = window;
}
int TitleX::handle_event()
{
	client->config.x = atol(get_text());
	client->send_configure_change();
	return 1;
}

TitleY::TitleY(TitleMain *client, TitleWindow *window, int x, int y)
 : BC_TumbleTextBox(window,
 	(int64_t)client->config.y,
	(int64_t)-2048,
	(int64_t)2048,
	x,
	y,
	60)
{
	this->client = client;
	this->window = window;
}
int TitleY::handle_event()
{
	client->config.y = atol(get_text());
	client->send_configure_change();
	return 1;
}

TitleStrokeW::TitleStrokeW(TitleMain *client,
	TitleWindow *window,
	int x,
	int y)
 : BC_TumbleTextBox(window,
 	(float)client->config.stroke_width,
	(float)-2048,
	(float)2048,
	x,
	y,
	60)
{
	this->client = client;
	this->window = window;
}
int TitleStrokeW::handle_event()
{
	client->config.stroke_width = atof(get_text());
	client->send_configure_change();
	return 1;
}


TitleSpeed::TitleSpeed(TitleMain *client, TitleWindow *window, int x, int y)
 : BC_TumbleTextBox(window,
 	(float)client->config.pixels_per_second,
	(float)0,
	(float)1000,
	x,
	y,
	70)
{
	this->client = client;
}


int TitleSpeed::handle_event()
{
	client->config.pixels_per_second = atof(get_text());
	client->send_configure_change();
	return 1;
}







TitleLeft::TitleLeft(TitleMain *client, TitleWindow *window, int x, int y)
 : BC_Radial(x, y, client->config.hjustification == JUSTIFY_LEFT, _("Left"))
{
	this->client = client;
	this->window = window;
}
int TitleLeft::handle_event()
{
	client->config.hjustification = JUSTIFY_LEFT;
	window->update_justification();
	client->send_configure_change();
	return 1;
}

TitleCenter::TitleCenter(TitleMain *client, TitleWindow *window, int x, int y)
 : BC_Radial(x, y, client->config.hjustification == JUSTIFY_CENTER, _("Center"))
{
	this->client = client;
	this->window = window;
}
int TitleCenter::handle_event()
{
	client->config.hjustification = JUSTIFY_CENTER;
	window->update_justification();
	client->send_configure_change();
	return 1;
}

TitleRight::TitleRight(TitleMain *client, TitleWindow *window, int x, int y)
 : BC_Radial(x, y, client->config.hjustification == JUSTIFY_RIGHT, _("Right"))
{
	this->client = client;
	this->window = window;
}
int TitleRight::handle_event()
{
	client->config.hjustification = JUSTIFY_RIGHT;
	window->update_justification();
	client->send_configure_change();
	return 1;
}



TitleTop::TitleTop(TitleMain *client, TitleWindow *window, int x, int y)
 : BC_Radial(x, y, client->config.vjustification == JUSTIFY_TOP, _("Top"))
{
	this->client = client;
	this->window = window;
}
int TitleTop::handle_event()
{
	client->config.vjustification = JUSTIFY_TOP;
	window->update_justification();
	client->send_configure_change();
	return 1;
}

TitleMid::TitleMid(TitleMain *client, TitleWindow *window, int x, int y)
 : BC_Radial(x, y, client->config.vjustification == JUSTIFY_MID, _("Mid"))
{
	this->client = client;
	this->window = window;
}
int TitleMid::handle_event()
{
	client->config.vjustification = JUSTIFY_MID;
	window->update_justification();
	client->send_configure_change();
	return 1;
}

TitleBottom::TitleBottom(TitleMain *client, TitleWindow *window, int x, int y)
 : BC_Radial(x, y, client->config.vjustification == JUSTIFY_BOTTOM, _("Bottom"))
{
	this->client = client;
	this->window = window;
}
int TitleBottom::handle_event()
{
	client->config.vjustification = JUSTIFY_BOTTOM;
	window->update_justification();
	client->send_configure_change();
	return 1;
}



TitleColorThread::TitleColorThread(TitleMain *client, TitleWindow *window, int is_outline)
 : ColorThread(1)
{
	this->client = client;
	this->window = window;
	this->is_outline = is_outline;
}

int TitleColorThread::handle_new_color(int output, int alpha)
{
	if(is_outline)
	{
		client->config.outline_color = output;
		client->config.outline_alpha = alpha;
	}
	else
	{
		client->config.color = output;
		client->config.alpha = alpha;
	}

	window->lock_window("TitleColorThread::handle_new_color");
	window->update_color();
	window->flush();
	window->unlock_window();

	client->send_configure_change();


	return 1;
}
TitleColorStrokeThread::TitleColorStrokeThread(TitleMain *client, TitleWindow *window)
 : ColorThread()
{
	this->client = client;
	this->window = window;
}

int TitleColorStrokeThread::handle_event(int output)
{
	client->config.color_stroke = output;
	window->update_color();
	window->flush();
	client->send_configure_change();
	return 1;
}
