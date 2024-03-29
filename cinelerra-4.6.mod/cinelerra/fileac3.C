
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
// work around (centos) for __STDC_CONSTANT_MACROS
#include <lzma.h>
#ifndef INT64_MAX
#   define INT64_MAX 9223372036854775807LL
#endif

extern "C"
{
#include "libavcodec/avcodec.h"
}

#include "asset.h"
#include "clip.h"
#include "fileac3.h"
#include "file.h"
#include "filempeg.h"
#include "language.h"
#include "mwindow.inc"
#include "mainerror.h"



#include <string.h>

FileAC3::FileAC3(Asset *asset, File *file)
 : FileBase(asset, file)
{
	reset_parameters();
	mpg_file = 0;
}

FileAC3::~FileAC3()
{
	if( mpg_file ) delete mpg_file;
	close_file();
}

int FileAC3::reset_parameters_derived()
{
	codec = 0;
	codec_context = 0;
	fd = 0;
	temp_raw = 0;
	temp_raw_size = 0;
	temp_raw_allocated = 0;
	temp_compressed = 0;
	compressed_allocated = 0;
	return 0;
}

void FileAC3::get_parameters(BC_WindowBase *parent_window,
		Asset *asset,
		BC_WindowBase* &format_window,
		int audio_options,
		int video_options)
{
	if(audio_options)
	{

		AC3ConfigAudio *window = new AC3ConfigAudio(parent_window, asset);
		format_window = window;
		window->create_objects();
		window->run_window();
		delete window;
	}
}

int FileAC3::check_sig()
{
// Libmpeg3 does this in FileMPEG.
	return 0;
}

int64_t FileAC3::get_channel_layout(int channels)
{
	switch( channels ) {
	case 1: return AV_CH_LAYOUT_MONO;
	case 2: return AV_CH_LAYOUT_STEREO;
	case 3: return AV_CH_LAYOUT_SURROUND;
	case 4: return AV_CH_LAYOUT_QUAD;
	case 5: return AV_CH_LAYOUT_5POINT0;
	case 6: return AV_CH_LAYOUT_5POINT1;
	case 8: return AV_CH_LAYOUT_7POINT1;
	}
	return 0;
}

int FileAC3::open_file(int rd, int wr)
{
	int result = 0;

	if(rd)
	{
		if( !mpg_file )
			mpg_file = new FileMPEG(file->asset, file);
		result = mpg_file->open_file(1, 0);
	}

	if( !result && wr )
	{
  		//avcodec_init();
		avcodec_register_all();
		codec = avcodec_find_encoder(CODEC_ID_AC3);
		if(!codec)
		{
			eprintf("FileAC3::open_file codec not found.\n");
			result = 1;
		}
		if( !result && !(fd = fopen(asset->path, "w")))
		{
			perror("FileAC3::open_file");
			result = 1;
		}
		if( !result ) {
			codec_context = avcodec_alloc_context3(codec);
			codec_context->bit_rate = asset->ac3_bitrate * 1000;
			codec_context->sample_rate = asset->sample_rate;
			codec_context->channels = asset->channels;
			codec_context->channel_layout =
				get_channel_layout(asset->channels);
			if(avcodec_open2(codec_context, codec, 0))
			{
				eprintf("FileAC3::open_file failed to open codec.\n");
				result = 1;
			}
		}
	}

	return 0;
}

int FileAC3::close_file()
{
	if( mpg_file )
	{
		delete mpg_file;
		mpg_file = 0;
	}
	if(codec_context)
	{
		avcodec_close(codec_context);
		free(codec_context);
		codec_context = 0;
		codec = 0;
	}
	if(fd)
	{
		fclose(fd);
		fd = 0;
	}
	if(temp_raw)
	{
		delete [] temp_raw;
		temp_raw = 0;
	}
	if(temp_compressed)
	{
		delete [] temp_compressed;
		temp_compressed = 0;
	}
	reset_parameters();
	FileBase::close_file();
	return 0;
}

int FileAC3::read_samples(double *buffer, int64_t len)
{
	return !mpg_file ? 0 : mpg_file->read_samples(buffer, len);
}

int FileAC3::get_index(char *index_path)
{
	return !mpg_file ? 1 : mpg_file->get_index(index_path);
}

// Channel conversion matrices because ffmpeg encodes a
// different channel order than liba52 decodes.
// Each row is an output channel.
// Each column is an input channel.
// static int channels5[] =
// {
// 	{ }
// };
//
// static int channels6[] =
// {
// 	{ }
// };


int FileAC3::write_samples(double **buffer, int64_t len)
{
// Convert buffer to encoder format
	if(temp_raw_size + len > temp_raw_allocated)
	{
		int new_allocated = temp_raw_size + len;
		int16_t *new_raw = new int16_t[new_allocated * asset->channels];
		if(temp_raw)
		{
			memcpy(new_raw,
				temp_raw,
				sizeof(int16_t) * temp_raw_size * asset->channels);
			delete [] temp_raw;
		}
		temp_raw = new_raw;
		temp_raw_allocated = new_allocated;
	}

// Allocate compressed data buffer
	if(temp_raw_allocated * asset->channels * 2 > compressed_allocated)
	{
		compressed_allocated = temp_raw_allocated * asset->channels * 2;
		delete [] temp_compressed;
		temp_compressed = new unsigned char[compressed_allocated];
	}

// Append buffer to temp raw
	int16_t *out_ptr = temp_raw + temp_raw_size * asset->channels;
	for(int i = 0; i < len; i++)
	{
		for(int j = 0; j < asset->channels; j++)
		{
			int sample = (int)(buffer[j][i] * 32767);
			CLAMP(sample, -32768, 32767);
			*out_ptr++ = sample;
		}
	}
	temp_raw_size += len;
	
	AVCodecContext *&avctx = codec_context;
	int frame_size = avctx->frame_size;
	int output_size = 0, cur_sample = 0, ret = 0;
	for(cur_sample = 0; !ret &&
		cur_sample + frame_size <= temp_raw_size;
		cur_sample += frame_size)
	{
		AVPacket avpkt;
		av_init_packet(&avpkt);
		avpkt.data = temp_compressed + output_size;
		avpkt.size = compressed_allocated - output_size;
		AVFrame *frame = av_frame_alloc();
		frame->nb_samples = frame_size;
		const uint8_t *samples = (uint8_t *)temp_raw +
			cur_sample * sizeof(int16_t) * asset->channels;
		int samples_sz = av_samples_get_buffer_size(NULL, avctx->channels,
			frame_size, avctx->sample_fmt, 1);
		int ret = avcodec_fill_audio_frame(frame, avctx->channels,
			avctx->sample_fmt, samples, samples_sz, 1);
		if( ret >= 0 ) {
			frame->pts = avctx->sample_rate && avctx->time_base.num ?
				file->get_audio_position() : AV_NOPTS_VALUE ;
			int got_packet = 0;
			ret = avcodec_encode_audio2(avctx, &avpkt, frame, &got_packet);
			if( !ret ) {
				if(got_packet && avctx->coded_frame) {
					avctx->coded_frame->pts = avpkt.pts;
					avctx->coded_frame->key_frame = !!(avpkt.flags & AV_PKT_FLAG_KEY);
				}
			}
		}
		av_packet_free_side_data(&avpkt);
		output_size += avpkt.size;
		if(frame->extended_data != frame->data)
			av_freep(&frame->extended_data);
		av_frame_free(&frame);
	}

// Shift buffer back
	memcpy(temp_raw,
		temp_raw + cur_sample * asset->channels,
		(temp_raw_size - cur_sample) * sizeof(int16_t) * asset->channels);
	temp_raw_size -= cur_sample;

	int bytes_written = fwrite(temp_compressed, 1, output_size, fd);
	if(bytes_written < output_size)
	{
		eprintf("Error while writing samples. \n%m\n");
		return 1;
	}
	return 0;
}







AC3ConfigAudio::AC3ConfigAudio(BC_WindowBase *parent_window,
	Asset *asset)
 : BC_Window(PROGRAM_NAME ": Audio Compression",
 	parent_window->get_abs_cursor_x(1),
 	parent_window->get_abs_cursor_y(1),
	500,
	BC_OKButton::calculate_h() + 100,
	500,
	BC_OKButton::calculate_h() + 100,
	0,
	0,
	1)
{
	this->parent_window = parent_window;
	this->asset = asset;
}

void AC3ConfigAudio::create_objects()
{
	int x = 10, y = 10;
	int x1 = 150;
	lock_window("AC3ConfigAudio::create_objects");
	add_tool(new BC_Title(x, y, "Bitrate (kbps):"));
	AC3ConfigAudioBitrate *bitrate;
	add_tool(bitrate =
		new AC3ConfigAudioBitrate(this,
			x1,
			y));
	bitrate->create_objects();

	add_subwindow(new BC_OKButton(this));
	show_window(1);
	unlock_window();
}

int AC3ConfigAudio::close_event()
{
	set_done(0);
	return 1;
}






AC3ConfigAudioBitrate::AC3ConfigAudioBitrate(AC3ConfigAudio *gui,
	int x,
	int y)
 : BC_PopupMenu(x,
 	y,
	150,
	AC3ConfigAudioBitrate::bitrate_to_string(gui->string, gui->asset->ac3_bitrate))
{
	this->gui = gui;
}

char* AC3ConfigAudioBitrate::bitrate_to_string(char *string, int bitrate)
{
	sprintf(string, "%d", bitrate);
	return string;
}

void AC3ConfigAudioBitrate::create_objects()
{
	add_item(new BC_MenuItem("32"));
	add_item(new BC_MenuItem("40"));
	add_item(new BC_MenuItem("48"));
	add_item(new BC_MenuItem("56"));
	add_item(new BC_MenuItem("64"));
	add_item(new BC_MenuItem("80"));
	add_item(new BC_MenuItem("96"));
	add_item(new BC_MenuItem("112"));
	add_item(new BC_MenuItem("128"));
	add_item(new BC_MenuItem("160"));
	add_item(new BC_MenuItem("192"));
	add_item(new BC_MenuItem("224"));
	add_item(new BC_MenuItem("256"));
	add_item(new BC_MenuItem("320"));
	add_item(new BC_MenuItem("384"));
	add_item(new BC_MenuItem("448"));
	add_item(new BC_MenuItem("512"));
	add_item(new BC_MenuItem("576"));
	add_item(new BC_MenuItem("640"));
}

int AC3ConfigAudioBitrate::handle_event()
{
	gui->asset->ac3_bitrate = atol(get_text());
	return 1;
}



