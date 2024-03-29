
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

#include "bcsignals.h"
#include "clip.h"
#include "cwindowgui.h"
#include "defaulttheme.h"
#include "edl.h"
#include "edlsession.h"
#include "mainmenu.h"
#include "mainsession.h"
#include "mbuttons.h"
#include "meterpanel.h"
#include "mwindow.h"
#include "mwindowgui.h"
#include "new.h"
#include "patchbay.h"
#include "preferencesthread.h"
#include "recordgui.h"
#include "recordmonitor.h"
#include "setformat.h"
#include "statusbar.h"
#include "timebar.h"
#include "trackcanvas.h"
#include "vframe.h"
#include "vwindowgui.h"
#include "zoombar.h"




PluginClient* new_plugin(PluginServer *server)
{
	return new DefaultThemeMain(server);
}







DefaultThemeMain::DefaultThemeMain(PluginServer *server)
 : PluginTClient(server)
{
}

DefaultThemeMain::~DefaultThemeMain()
{
}

char* DefaultThemeMain::plugin_title()
{
	return "Blond";
}

Theme* DefaultThemeMain::new_theme()
{
	theme = new DefaultTheme;
	extern unsigned char _binary_defaulttheme_data_start[];
	theme->set_data(_binary_defaulttheme_data_start);
	return theme;
}








DefaultTheme::DefaultTheme()
 : Theme()
{
}

DefaultTheme::~DefaultTheme()
{
}

void DefaultTheme::initialize()
{
	BC_Resources *resources = BC_WindowBase::get_resources();
	resources->generic_button_images = new_image_set(3, 
			"generic_up.png", 
			"generic_hi.png", 
			"generic_dn.png");
	resources->horizontal_slider_data = new_image_set(6,
			"hslider_fg_up.png",
			"hslider_fg_hi.png",
			"hslider_fg_dn.png",
			"hslider_bg_up.png",
			"hslider_bg_hi.png",
			"hslider_bg_dn.png");
	resources->progress_images = new_image_set(2,
			"progress_bg.png",
			"progress_hi.png");
	resources->tumble_data = new_image_set(4,
		"tumble_up.png",
		"tumble_hi.png",
		"tumble_botdn.png",
		"tumble_topdn.png");
	resources->listbox_button = new_image_set(4,
		"listbox_button_up.png",
		"listbox_button_hi.png",
		"listbox_button_dn.png",
		"listbox_button_hi.png");
	resources->listbox_column = new_image_set(3,
		"listbox_column_up.png",
		"listbox_column_hi.png",
		"listbox_column_dn.png");
	resources->pan_data = new_image_set(7,
			"pan_up.png", 
			"pan_hi.png", 
			"pan_popup.png", 
			"pan_channel.png", 
			"pan_stick.png", 
			"pan_channel_small.png", 
			"pan_stick_small.png");
	resources->pan_text_color = WHITE;

	resources->pot_images = new_image_set(3,
		"pot_up.png",
		"pot_hi.png",
		"pot_dn.png");

	resources->checkbox_images = new_image_set(5,
		"checkbox_up.png",
		"checkbox_uphi.png",
		"checkbox_checked.png",
		"checkbox_down.png",
		"checkbox_checkedhi.png");

	resources->radial_images = new_image_set(5,
		"radial_up.png",
		"radial_uphi.png",
		"radial_checked.png",
		"radial_down.png",
		"radial_checkedhi.png");

	resources->xmeter_images = new_image_set(7, 
		"xmeter_normal.png",
		"xmeter_green.png",
		"xmeter_red.png",
		"xmeter_yellow.png",
		"xmeter_white.png",
		"xmeter_over.png",
		"downmix51_2.png");
	resources->ymeter_images = new_image_set(7, 
		"ymeter_normal.png",
		"ymeter_green.png",
		"ymeter_red.png",
		"ymeter_yellow.png",
		"ymeter_white.png",
		"ymeter_over.png",
		"downmix51_2.png");

	resources->hscroll_data = new_image_set(10,
			"hscroll_center_up.png",
			"hscroll_center_hi.png",
			"hscroll_center_dn.png",
			"hscroll_bg.png",
			"hscroll_back_up.png",
			"hscroll_back_hi.png",
			"hscroll_back_dn.png",
			"hscroll_fwd_up.png",
			"hscroll_fwd_hi.png",
			"hscroll_fwd_dn.png");

	resources->vscroll_data = new_image_set(10,
			"vscroll_center_up.png",
			"vscroll_center_hi.png",
			"vscroll_center_dn.png",
			"vscroll_bg.png",
			"vscroll_back_up.png",
			"vscroll_back_hi.png",
			"vscroll_back_dn.png",
			"vscroll_fwd_up.png",
			"vscroll_fwd_hi.png",
			"vscroll_fwd_dn.png");

	resources->ok_images = new_button("ok.png", 
			"generic_up.png",
			"generic_hi.png",
			"generic_dn.png");

	resources->cancel_images = new_button("cancel.png", 
			"generic_up.png",
			"generic_hi.png",
			"generic_dn.png");


// Record windows
	rgui_batch = new_image("recordgui_batch.png");
	rgui_controls = new_image("recordgui_controls.png");
	rgui_list = new_image("recordgui_list.png");
	rmonitor_panel = new_image("recordmonitor_panel.png");
	rmonitor_meters = new_image("recordmonitor_meters.png");


// MWindow
	mbutton_left = new_image("mbutton_left.png");
	mbutton_right = new_image("mbutton_right.png");
	new_image("timebar_bg", "timebar_bg.png");
	new_image("timebar_brender", "timebar_brender.png");
	new_image("clock_bg", "mclock.png");
	new_image("patchbay_bg", "patchbay_bg.png");
	tracks_bg = new_image("tracks_bg.png");
	zoombar_left = new_image("zoombar_left.png");
	zoombar_right = new_image("zoombar_right.png");
	statusbar_left = new_image("statusbar_left.png");
	statusbar_right = new_image("statusbar_right.png");

// CWindow
	cpanel_bg = new_image("cpanel_bg.png");
	cbuttons_left = new_image("cbuttons_left.png");
	cbuttons_right = new_image("cbuttons_right.png");
	cmeter_bg = new_image("cmeter_bg.png");

// VWindow
	vbuttons_left = new_image("vbuttons_left.png");
	vbuttons_right = new_image("vbuttons_right.png");
	vmeter_bg = new_image("vmeter_bg.png");

	preferences_bg = new_image("preferences_bg.png");


	new_bg = new_image("new_bg.png");
	setformat_bg = new_image("setformat_bg2.png");


	timebar_view_data = new_image("timebar_view.png");

	setformat_w = 600;
	setformat_h = 560;
	setformat_x1 = 15;
	setformat_x2 = 100;

	setformat_x3 = 315;
	setformat_x4 = 415;
	setformat_y1 = 20;
	setformat_y2 = 85;
	setformat_y3 = 125;
	setformat_margin = 30;
	setformat_channels_x = 25;
	setformat_channels_y = 242;
	setformat_channels_w = 250;
	setformat_channels_h = 250;

	loadfile_pad = 70;
	browse_pad = 20;







	build_icons();
	build_bg_data();

	new_image_set("drawpatch_data", 5, "drawpatch_up.png", "drawpatch_hi.png", "drawpatch_checked.png", "drawpatch_dn.png", "drawpatch_checkedhi.png");
	new_image_set("expandpatch_data", 5, "expandpatch_up.png", "expandpatch_hi.png", "expandpatch_checked.png", "expandpatch_dn.png", "expandpatch_checkedhi.png");
	new_image_set("gangpatch_data", 5, "gangpatch_up.png", "gangpatch_hi.png", "gangpatch_checked.png", "gangpatch_dn.png", "gangpatch_checkedhi.png");
	new_image_set("mutepatch_data", 5, "mutepatch_up.png", "mutepatch_hi.png", "mutepatch_checked.png", "mutepatch_dn.png", "mutepatch_checkedhi.png");
	new_image_set("playpatch_data", 5, "playpatch_up.png", "playpatch_hi.png", "playpatch_checked.png", "playpatch_dn.png", "playpatch_checkedhi.png");
	new_image_set("recordpatch_data", 5, "recordpatch_up.png", "recordpatch_hi.png", "recordpatch_checked.png", "recordpatch_dn.png", "recordpatch_checkedhi.png");


	build_overlays();



	out_point = new_image_set(5,
		"out_up.png", 
		"out_hi.png", 
		"out_checked.png", 
		"out_dn.png", 
		"out_checkedhi.png");
	in_point = new_image_set(5,
		"in_up.png", 
		"in_hi.png", 
		"in_checked.png", 
		"in_dn.png", 
		"in_checkedhi.png");

	label_toggle = new_image_set(5,
		"labeltoggle_up.png", 
		"labeltoggle_uphi.png", 
		"label_checked.png", 
		"labeltoggle_dn.png", 
		"label_checkedhi.png");


	statusbar_cancel_data = new_image_set(3,
		"statusbar_cancel_up.png",
		"statusbar_cancel_hi.png",
		"statusbar_cancel_dn.png");


	VFrame *editpanel_up = new_image("editpanel_up.png");
	VFrame *editpanel_hi = new_image("editpanel_hi.png");
	VFrame *editpanel_dn = new_image("editpanel_dn.png");
	VFrame *editpanel_checked = new_image("editpanel_checked.png");
	VFrame *editpanel_checkedhi = new_image("editpanel_checkedhi.png");

	bottom_justify = new_button("bottom_justify.png", editpanel_up, editpanel_hi, editpanel_dn);
	center_justify = new_button("center_justify.png", editpanel_up, editpanel_hi, editpanel_dn);
	channel_data = new_button("channel.png", editpanel_up, editpanel_hi, editpanel_dn);
	new_button("copy.png", editpanel_up, editpanel_hi, editpanel_dn, "copy");
	new_button("cut.png", editpanel_up, editpanel_hi, editpanel_dn, "cut");
	new_button("fit.png", editpanel_up, editpanel_hi, editpanel_dn, "fit");
	new_button("fitautos.png", editpanel_up, editpanel_hi, editpanel_dn, "fitautos");
	new_button("inpoint.png", editpanel_up, editpanel_hi, editpanel_dn, "inbutton");
	new_button("label.png", editpanel_up, editpanel_hi, editpanel_dn, "labelbutton");
	left_justify = new_button("left_justify.png", editpanel_up, editpanel_hi, editpanel_dn);
	magnify_button_data = new_button("magnify.png", editpanel_up, editpanel_hi, editpanel_dn);
	middle_justify = new_button("middle_justify.png", editpanel_up, editpanel_hi, editpanel_dn);
	new_button("nextlabel.png", editpanel_up, editpanel_hi, editpanel_dn, "nextlabel");
	new_button("outpoint.png", editpanel_up, editpanel_hi, editpanel_dn, "outbutton");
	over_button = new_button("over.png", editpanel_up, editpanel_hi, editpanel_dn);
	overwrite_data = new_button("overwrite.png", editpanel_up, editpanel_hi, editpanel_dn);
	new_button("paste.png", editpanel_up, editpanel_hi, editpanel_dn, "paste");
	new_button("prevlabel.png", editpanel_up, editpanel_hi, editpanel_dn, "prevlabel");
	new_button("redo.png", editpanel_up, editpanel_hi, editpanel_dn, "redo");
	right_justify = new_button("right_justify.png", editpanel_up, editpanel_hi, editpanel_dn);
	splice_data = new_button("splice.png", editpanel_up, editpanel_hi, editpanel_dn);
	new_button("toclip.png", editpanel_up, editpanel_hi, editpanel_dn, "toclip");
	top_justify = new_button("top_justify.png", editpanel_up, editpanel_hi, editpanel_dn);
	new_button("undo.png", editpanel_up, editpanel_hi, editpanel_dn, "undo");
	wrench_data = new_button("wrench.png", editpanel_up, editpanel_hi, editpanel_dn);

// CWindow icons
	new_image("cwindow_inactive", "cwindow_inactive.png");
	new_image("cwindow_active", "cwindow_active.png");


	new_image_set("batch_render_start",
		3,
		"batchstart_up.png",
		"batchstart_hi.png",
		"batchstart_dn.png");
	new_image_set("batch_render_stop",
		3,
		"batchstop_up.png",
		"batchstop_hi.png",
		"batchstop_dn.png");
	new_image_set("batch_render_cancel",
		3,
		"batchcancel_up.png",
		"batchcancel_hi.png",
		"batchcancel_dn.png");

	new_toggle("arrow.png", editpanel_up, editpanel_hi, editpanel_checked, editpanel_dn, editpanel_checkedhi, "arrow");
	new_toggle("autokeyframe.png", editpanel_up, editpanel_hi, editpanel_checked, editpanel_dn, editpanel_checkedhi, "autokeyframe");
	camera_data = new_toggle("camera.png", editpanel_up, editpanel_hi, editpanel_checked, editpanel_dn, editpanel_checkedhi);
	crop_data = new_toggle("crop.png", editpanel_up, editpanel_hi, editpanel_checked, editpanel_dn, editpanel_checkedhi);
	new_toggle("ibeam.png", editpanel_up, editpanel_hi, editpanel_checked, editpanel_dn, editpanel_checkedhi, "ibeam");
	magnify_data = new_toggle("magnify.png", editpanel_up, editpanel_hi, editpanel_checked, editpanel_dn, editpanel_checkedhi);
	mask_data = new_toggle("mask.png", editpanel_up, editpanel_hi, editpanel_checked, editpanel_dn, editpanel_checkedhi);
	proj_data = new_toggle("projector.png", editpanel_up, editpanel_hi, editpanel_checked, editpanel_dn, editpanel_checkedhi);
	protect_data = new_toggle("protect.png", editpanel_up, editpanel_hi, editpanel_checked, editpanel_dn, editpanel_checkedhi);
	show_meters = new_toggle("show_meters.png", editpanel_up, editpanel_hi, editpanel_checked, editpanel_dn, editpanel_checkedhi);
	titlesafe_data = new_toggle("titlesafe.png", editpanel_up, editpanel_hi, editpanel_checked, editpanel_dn, editpanel_checkedhi);
	tool_data = new_toggle("toolwindow.png", editpanel_up, editpanel_hi, editpanel_checked, editpanel_dn, editpanel_checkedhi);
	new_toggle("eyedrop.png", 
		editpanel_up, 
		editpanel_hi, 
		editpanel_checked, 
		editpanel_dn, 
		editpanel_checkedhi, 
		"cwindow_eyedrop");




	static VFrame **transport_bg = new_image_set(3,
		"transportup.png", 
		"transporthi.png", 
		"transportdn.png");
	build_transport("end", get_image_data("end.png"), transport_bg, 2);
	build_transport("fastfwd", get_image_data("fastfwd.png"), transport_bg, 1);
	build_transport("fastrev", get_image_data("fastrev.png"), transport_bg, 1);
	build_transport("play", get_image_data("play.png"), transport_bg, 1);
	build_transport("framefwd", get_image_data("framefwd.png"), transport_bg, 1);
	build_transport("framerev", get_image_data("framerev.png"), transport_bg, 1);
	build_transport("pause", get_image_data("pause.png"), transport_bg, 1);
	build_transport("record", get_image_data("record.png"), transport_bg, 1);
	build_transport("singleframe", get_image_data("singleframe.png"), transport_bg, 1);
	build_transport("reverse", get_image_data("reverse.png"), transport_bg, 1);
	build_transport("rewind", get_image_data("rewind.png"), transport_bg, 0);
	build_transport("stop", get_image_data("stop.png"), transport_bg, 1);
	build_transport("stoprec", get_image_data("stoprec.png"), transport_bg, 2);
	flush_images();

	title_font = MEDIUMFONT_3D;
	title_color = WHITE;
	recordgui_fixed_color = YELLOW;
	recordgui_variable_color = RED;

	channel_position_color = MEYELLOW;
	resources->meter_title_w = 25;
}

#define CWINDOW_METER_MARGIN 5
#define VWINDOW_METER_MARGIN 5

void DefaultTheme::get_mwindow_sizes(MWindowGUI *gui, int w, int h)
{
	mbuttons_x = 0;
	mbuttons_y = gui->mainmenu->get_h();
	mbuttons_w = w;
	mbuttons_h = mbutton_left->get_h();
	mclock_x = 10;
	mclock_y = mbuttons_y + mbuttons_h + CWINDOW_METER_MARGIN;
	mclock_w = get_image("clock_bg")->get_w() - 40;
	mclock_h = get_image("clock_bg")->get_h();
	mtimebar_x = get_image("patchbay_bg")->get_w();
	mtimebar_y = mbuttons_y + mbuttons_h;
	mtimebar_w = w - mtimebar_x;
	mtimebar_h = get_image("timebar_bg")->get_h();
	mstatus_x = 0;
	mstatus_y = h - statusbar_left->get_h();
	mstatus_w = w;
	mstatus_h = statusbar_left->get_h();
	mstatus_message_x = 10;
	mstatus_message_y = 5;
	mstatus_progress_x = mstatus_w - statusbar_cancel_data[0]->get_w() - 240;
	mstatus_progress_y = mstatus_h - BC_WindowBase::get_resources()->progress_images[0]->get_h();
	mstatus_progress_w = 230;
	mstatus_cancel_x = mstatus_w - statusbar_cancel_data[0]->get_w();
	mstatus_cancel_y = mstatus_h - statusbar_cancel_data[0]->get_h();
	mzoom_x = 0;
	mzoom_y = mstatus_y - zoombar_left->get_h();
	mzoom_h = zoombar_left->get_h();
	mzoom_w = w;
	patchbay_x = 0;
	patchbay_y = mtimebar_y + mtimebar_h;
	patchbay_w = get_image("patchbay_bg")->get_w();
	patchbay_h = mzoom_y - patchbay_y;
	mcanvas_x = patchbay_x + patchbay_w;
	mcanvas_y = mtimebar_y + mtimebar_h;
	mcanvas_w = w - patchbay_w - BC_ScrollBar::get_span(SCROLL_VERT);
	mcanvas_h = patchbay_h;
	mhscroll_x = 0;
	mhscroll_y = mcanvas_y + mcanvas_h;
	mhscroll_w = w - BC_ScrollBar::get_span(SCROLL_VERT) - patchbay_w;
	mvscroll_x = mcanvas_x + mcanvas_w;
	mvscroll_y = mcanvas_y;
	mvscroll_h = mcanvas_h;
}

void DefaultTheme::get_cwindow_sizes(CWindowGUI *gui, int cwindow_controls)
{
	if(cwindow_controls)
	{
		ccomposite_x = 0;
		ccomposite_y = 5;
		ccomposite_w = cpanel_bg->get_w();
		ccomposite_h = mwindow->session->cwindow_h - cbuttons_left->get_h();
		cedit_x = 10;
		cedit_y = ccomposite_h + 17;
		ctransport_x = 10;
		ctransport_y = mwindow->session->cwindow_h - get_image_set("autokeyframe")[0]->get_h();
		ccanvas_x = ccomposite_x + ccomposite_w;
		ccanvas_y = 0;
		ccanvas_h = ccomposite_h;
		cstatus_x = 525;
		cstatus_y = mwindow->session->cwindow_h - 40;
		if(mwindow->edl->session->cwindow_meter)
		{
			cmeter_x = mwindow->session->cwindow_w - MeterPanel::get_meters_width(mwindow->edl->session->audio_channels, 
				mwindow->edl->session->cwindow_meter);
			ccanvas_w = cmeter_x - ccanvas_x - 5;
		}
		else
		{
			cmeter_x = mwindow->session->cwindow_w;
			ccanvas_w = cmeter_x - ccanvas_x;
		}
	}
	else
	{
		ccomposite_x = -cpanel_bg->get_w();
		ccomposite_y = 0;
		ccomposite_w = cpanel_bg->get_w();
		ccomposite_h = mwindow->session->cwindow_h - cbuttons_left->get_h();

		cedit_x = 10;
		cedit_y = mwindow->session->cwindow_h + 17;
		ctransport_x = 10;
		ctransport_y = cedit_y + 40;
		ccanvas_x = 0;
		ccanvas_y = 0;
		ccanvas_w = mwindow->session->cwindow_w;
		ccanvas_h = mwindow->session->cwindow_h;
		cmeter_x = mwindow->session->cwindow_w;
		cstatus_x = mwindow->session->cwindow_w;
		cstatus_y = mwindow->session->cwindow_h;
	}


	czoom_x = ctransport_x + PlayTransport::get_transport_width(mwindow) + 20;
	czoom_y = ctransport_y + 5;


	cmeter_y = 5;
	cmeter_h = mwindow->session->cwindow_h - cmeter_y;

	ctimebar_x = ccanvas_x;
	ctimebar_y = ccanvas_y + ccanvas_h;
	ctimebar_w = ccanvas_w;
	ctimebar_h = 16;


// Not used
	ctime_x = ctransport_x + PlayTransport::get_transport_width(mwindow);
	ctime_y = ctransport_y;
	cdest_x = czoom_x;
	cdest_y = czoom_y + 30;
}



void DefaultTheme::get_recordgui_sizes(RecordGUI *gui, int w, int h)
{
	recordgui_status_x = 10;
	recordgui_status_y = 10;
	recordgui_status_x2 = 160;
	recordgui_batch_x = 310;
	recordgui_batch_y = 10;
	recordgui_batchcaption_x = recordgui_batch_x + 110;


	recordgui_transport_x = recordgui_batch_x;
	recordgui_transport_y = recordgui_batch_y + 150;

	recordgui_buttons_x = recordgui_batch_x - 50;
	recordgui_buttons_y = recordgui_transport_y + 40;
	recordgui_options_x = recordgui_buttons_x;
	recordgui_options_y = recordgui_buttons_y + 35;

	recordgui_batches_x = 10;
	recordgui_batches_y = 270;
	recordgui_batches_w = w - 20;
	recordgui_batches_h = h - recordgui_batches_y - 70;
	recordgui_loadmode_x = w / 2 - loadmode_w / 2;
	recordgui_loadmode_y = h - 60;

	recordgui_controls_x = 10;
	recordgui_controls_y = h - 40;
}



void DefaultTheme::get_vwindow_sizes(VWindowGUI *gui)
{
	vmeter_y = 5;
	vmeter_h = mwindow->session->vwindow_h - cmeter_y;
	vcanvas_x = 0;
	vcanvas_y = 0;
	vcanvas_h = mwindow->session->vwindow_h - vbuttons_left->get_h();

	if(mwindow->edl->session->vwindow_meter)
	{
		vmeter_x = mwindow->session->vwindow_w - 
			VWINDOW_METER_MARGIN - 
			MeterPanel::get_meters_width(mwindow->edl->session->audio_channels, 
			mwindow->edl->session->vwindow_meter);
		vcanvas_w = vmeter_x - vcanvas_x - VWINDOW_METER_MARGIN;
	}
	else
	{
		vmeter_x = mwindow->session->vwindow_w;
		vcanvas_w = mwindow->session->vwindow_w;
	}

	vtimebar_x = vcanvas_x;
	vtimebar_y = vcanvas_y + vcanvas_h;
	vtimebar_w = vcanvas_w;
	vtimebar_h = 16;

	vslider_x = 10;
	vslider_y = vtimebar_y + 25;
	vslider_w = vtimebar_w - vslider_x;
	vedit_x = 10;
	vedit_y = vslider_y + 17;
	vtransport_x = 10;
	vtransport_y = mwindow->session->vwindow_h - get_image_set("autokeyframe")[0]->get_h();
	vtime_x = 370;
	vtime_y = vedit_y + 10;
	vtime_w = 150;




	vzoom_x = vtime_x + 150;
	vzoom_y = vtime_y;
	vsource_x = vtime_x + 50;
	vsource_y = vtransport_y + 5;
}





void DefaultTheme::build_icons()
{
	mwindow_icon = new VFrame(get_image_data("heroine_icon.png"));
	vwindow_icon = new VFrame(get_image_data("heroine_icon.png"));
	cwindow_icon = new VFrame(get_image_data("heroine_icon.png"));
	awindow_icon = new VFrame(get_image_data("heroine_icon.png"));
	record_icon = new VFrame(get_image_data("heroine_icon.png"));
	clip_icon = new VFrame(get_image_data("clip_icon.png"));
}



void DefaultTheme::build_bg_data()
{
// Audio settings
	channel_bg_data = new VFrame(get_image_data("channel_bg.png"));
	channel_position_data = new VFrame(get_image_data("channel_position.png"));

// Track bitmaps
	new_image("resource1024", "resource1024.png");
	new_image("resource512", "resource512.png");
	new_image("resource256", "resource256.png");
	new_image("resource128", "resource128.png");
	new_image("resource64", "resource64.png");
	new_image("resource32", "resource32.png");
	plugin_bg_data = new VFrame(get_image_data("plugin_bg.png"));
	title_bg_data = new VFrame(get_image_data("title_bg.png"));
	vtimebar_bg_data = new VFrame(get_image_data("vwindow_timebar.png"));
}



void DefaultTheme::build_overlays()
{
	keyframe_data = new VFrame(get_image_data("keyframe3.png"));
	camerakeyframe_data = new VFrame(get_image_data("camerakeyframe.png"));
	maskkeyframe_data = new VFrame(get_image_data("maskkeyframe.png"));
	modekeyframe_data = new VFrame(get_image_data("modekeyframe.png"));
	pankeyframe_data = new VFrame(get_image_data("pankeyframe.png"));
	projectorkeyframe_data = new VFrame(get_image_data("projectorkeyframe.png"));
}









void DefaultTheme::draw_rwindow_bg(RecordGUI *gui)
{
	int y;
	int margin = 50;
	int margin2 = 80;
	gui->draw_9segment(recordgui_batch_x - margin,
		0,
		mwindow->session->rwindow_w - recordgui_status_x + margin,
		recordgui_buttons_y,
		rgui_batch);
	gui->draw_3segmenth(recordgui_options_x - margin2,
		recordgui_buttons_y - 5,
		mwindow->session->rwindow_w - recordgui_options_x + margin2,
		rgui_controls);
	y = recordgui_buttons_y - 5 + rgui_controls->get_h();
	gui->draw_9segment(0,
		y,
		mwindow->session->rwindow_w,
		mwindow->session->rwindow_h - y,
		rgui_list);
}

void DefaultTheme::draw_rmonitor_bg(RecordMonitorGUI *gui)
{
	int margin = 45;
	int panel_w = 300;
	int x = rmonitor_meter_x - margin;
	int w = mwindow->session->rmonitor_w - x;
	if(w < rmonitor_meters->get_w()) w = rmonitor_meters->get_w();
	gui->clear_box(0, 
		0, 
		mwindow->session->rmonitor_w, 
		mwindow->session->rmonitor_h);
	gui->draw_9segment(x,
		0,
		w,
		mwindow->session->rmonitor_h,
		rmonitor_meters);
}






void DefaultTheme::draw_mwindow_bg(MWindowGUI *gui)
{
// Button bar
	gui->draw_3segmenth(mbuttons_x, 
		mbuttons_y, 
		750, 
		mbutton_left);
	gui->draw_3segmenth(mbuttons_x + 750, 
		mbuttons_y, 
		mbuttons_w - 500, 
		mbutton_right);

// Clock
	gui->draw_3segmenth(0, 
		mbuttons_y + mbutton_left->get_h(),
		get_image("patchbay_bg")->get_w(), 
		get_image("clock_bg"));

// Patchbay
	gui->draw_3segmentv(patchbay_x, 
		patchbay_y, 
		patchbay_h + 20, 
		get_image("patchbay_bg"));

// Track canvas
	gui->draw_9segment(mcanvas_x, 
		mcanvas_y, 
		mcanvas_w, 
		patchbay_h + 20, 
		tracks_bg);

// Timebar
	gui->draw_3segmenth(mtimebar_x, 
		mtimebar_y, 
		mtimebar_w, 
		get_image("timebar_bg"));

// Zoombar
	int zoombar_center = 710;
	gui->draw_3segmenth(mzoom_x, 
		mzoom_y,
		zoombar_center, 
		zoombar_left);
	if(mzoom_w > zoombar_center)
		gui->draw_3segmenth(mzoom_x + zoombar_center, 
			mzoom_y, 
			mzoom_w - zoombar_center, 
			zoombar_right);

// Status
	gui->draw_3segmenth(mstatus_x, 
		mstatus_y,
		zoombar_center, 
		statusbar_left);

	if(mstatus_w > zoombar_center)
		gui->draw_3segmenth(mstatus_x + zoombar_center, 
			mstatus_y,
			mstatus_w - zoombar_center, 
			statusbar_right);
}

void DefaultTheme::draw_cwindow_bg(CWindowGUI *gui)
{
	const int button_division = 530;
	gui->draw_3segmentv(0, 0, ccomposite_h, cpanel_bg);
	gui->draw_3segmenth(0, ccomposite_h, button_division, cbuttons_left);
	if(mwindow->edl->session->cwindow_meter)
	{
		gui->draw_3segmenth(button_division, 
			ccomposite_h, 
			cmeter_x - CWINDOW_METER_MARGIN - button_division, 
			cbuttons_right);
		gui->draw_9segment(cmeter_x - CWINDOW_METER_MARGIN, 
			0, 
			mwindow->session->cwindow_w - cmeter_x + CWINDOW_METER_MARGIN, 
			mwindow->session->cwindow_h, 
			cmeter_bg);
	}
	else
	{
		gui->draw_3segmenth(button_division, 
			ccomposite_h, 
			cmeter_x - CWINDOW_METER_MARGIN - button_division + 100, 
			cbuttons_right);
	}
}

void DefaultTheme::draw_vwindow_bg(VWindowGUI *gui)
{
	const int button_division = 400;
	gui->draw_3segmenth(0, vcanvas_h, button_division, vbuttons_left);
	if(mwindow->edl->session->vwindow_meter)
	{
		gui->draw_3segmenth(button_division, 
			vcanvas_h, 
			vmeter_x - VWINDOW_METER_MARGIN - button_division, 
			vbuttons_right);
		gui->draw_9segment(vmeter_x - VWINDOW_METER_MARGIN,
			0,
			mwindow->session->vwindow_w - vmeter_x + VWINDOW_METER_MARGIN, 
			mwindow->session->vwindow_h, 
			vmeter_bg);
	}
	else
	{
		gui->draw_3segmenth(button_division, 
			vcanvas_h, 
			vmeter_x - VWINDOW_METER_MARGIN - button_division + 100, 
			vbuttons_right);
	}
}

void DefaultTheme::get_preferences_sizes()
{
}


void DefaultTheme::draw_preferences_bg(PreferencesWindow *gui)
{
	gui->draw_9segment(0, 0, gui->get_w(), gui->get_h() - 40, preferences_bg);
}

void DefaultTheme::get_new_sizes(NewWindow *gui)
{
}

void DefaultTheme::draw_new_bg(NewWindow *gui)
{
	gui->draw_vframe(new_bg, 0, 0);
}

void DefaultTheme::draw_setformat_bg(SetFormatWindow *gui)
{
	gui->draw_vframe(setformat_bg, 0, 0);
}






