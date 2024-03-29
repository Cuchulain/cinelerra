#include "funcprotos.h"
#include "quicktime.h"



void quicktime_delete_hdrl(quicktime_t *file, quicktime_hdrl_t *hdrl)
{
	int i;
	for(i = 0; i < file->moov.total_tracks; i++)
	{
		quicktime_delete_strl(hdrl->strl[i]);
	}
}


void quicktime_read_hdrl(quicktime_t *file,
	quicktime_hdrl_t *hdrl,
	quicktime_atom_t *parent_atom)
{
	quicktime_atom_t leaf_atom;
	char data[4];
	int current_track = 0;

//printf("quicktime_read_hdrl 1\n");
	do
	{
		quicktime_atom_read_header(file, &leaf_atom);

/* Got LIST */
		if(quicktime_atom_is(&leaf_atom, "LIST"))
		{
			data[0] = data[1] = data[2] = data[3] = 0;
			quicktime_read_data(file, data, 4);

/* Got strl */
			if(quicktime_match_32(data, "strl"))
			{
				quicktime_strl_t *strl =
					hdrl->strl[current_track++] =
					quicktime_new_strl();
				quicktime_read_strl(file, strl, &leaf_atom);
			}
		}

		quicktime_atom_skip(file, &leaf_atom);
	}while(quicktime_position(file) < parent_atom->end);

	quicktime_atom_skip(file, &leaf_atom);
}

void quicktime_init_hdrl(quicktime_t *file, quicktime_hdrl_t *hdrl)
{
	int i;
	quicktime_atom_t avih_atom;
	int current_track;

// LIST 'hdrl'
	quicktime_atom_write_header(file, &hdrl->atom, "LIST");
	quicktime_write_char32(file, "hdrl");


// avih
	quicktime_atom_write_header(file, &avih_atom, "avih");

	if(file->total_vtracks) {
		int d = quicktime_frame_rate_d(file, 0);
		int n = quicktime_frame_rate_n(file, 0);
   		quicktime_write_int32_le(file, (uint32_t)(1000000. * d / n));
	}
	else
		quicktime_write_int32_le(file, 0);

	hdrl->bitrate_offset = quicktime_position(file);
	quicktime_write_int32_le(file, 0); /* bitrate in bytes */
    quicktime_write_int32_le(file, 0); /* padding */
    quicktime_write_int32_le(file,
		AVI_TRUSTCKTYPE |
		AVI_HASINDEX |
		AVI_MUSTUSEINDEX |
		AVI_ISINTERLEAVED); /* flags */
	hdrl->frames_offset = quicktime_position(file);
    quicktime_write_int32_le(file, 0); /* nb frames, filled later */
    quicktime_write_int32_le(file, 0); /* initial frame */
    quicktime_write_int32_le(file, file->moov.total_tracks); /* nb streams */
    quicktime_write_int32_le(file, 0); /* suggested buffer size */

	if(file->total_vtracks)
    {
		quicktime_write_int32_le(file, file->vtracks[0].track->tkhd.track_width);
    	quicktime_write_int32_le(file, file->vtracks[0].track->tkhd.track_height);
	}
	else
    {
		quicktime_write_int32_le(file, 0);
    	quicktime_write_int32_le(file, 0);
	}
    quicktime_write_int32_le(file, 0); /* reserved */
    quicktime_write_int32_le(file, 0); /* reserved */
    quicktime_write_int32_le(file, 0); /* reserved */
    quicktime_write_int32_le(file, 0); /* reserved */

	quicktime_atom_write_footer(file, &avih_atom);


/* Write stream lists. */
/* Need the track maps to get the WAV ID for audio. */
	current_track = 0;
	for(i = 0; i < file->total_vtracks; i++)
	{
		quicktime_video_map_t *video_map = &file->vtracks[i];
		quicktime_trak_t *trak = video_map->track;
		quicktime_strl_t *strl =
			hdrl->strl[current_track++] =
			quicktime_new_strl();
		quicktime_init_strl(file,
			0,
			video_map,
			trak,
			strl);
	}

	for(i = 0; i < file->total_atracks; i++)
	{
		quicktime_audio_map_t *audio_map = &file->atracks[i];
		quicktime_trak_t *trak = audio_map->track;
		quicktime_strl_t *strl =
			hdrl->strl[current_track++] =
			quicktime_new_strl();
		quicktime_init_strl(file,
			audio_map,
			0,
			trak,
			strl);
	}

/*
 * for(i = 0; i < file->moov.total_tracks; i++)
 * {
 * printf("quicktime_init_hdrl 10 %d %p\n", i, file->riff[0]->hdrl.strl[i]->tag);
 * }
 */

/* ODML header */
	quicktime_init_odml(file, hdrl);
	quicktime_atom_write_footer(file, &hdrl->atom);
}


void quicktime_finalize_hdrl(quicktime_t *file, quicktime_hdrl_t *hdrl)
{
	int i;
	int64_t position = quicktime_position(file);
	int64_t total_frames = 0;
	double frame_rate = 0;

	for(i = 0; i < file->moov.total_tracks; i++)
	{
		quicktime_trak_t *trak = file->moov.trak[i];
		quicktime_strl_t *strl = hdrl->strl[i];

		if(trak->mdia.minf.is_video)
		{
			int length;
			quicktime_set_position(file, strl->length_offset);
			total_frames = length = quicktime_track_samples(file, trak);
			quicktime_write_int32_le(file, length);
			frame_rate = (double)trak->mdia.mdhd.time_scale /
				trak->mdia.minf.stbl.stts.table[0].sample_duration;
		}
		else
		if(trak->mdia.minf.is_audio)
		{
			int length, samples_per_chunk, wav_id, sample_size, sample_rate;
			quicktime_set_position(file, strl->length_offset);
			length = quicktime_track_samples(file, trak);
			quicktime_write_int32_le(file, length);

// dwScale and dwRate as per MSDN - // http://msdn.microsoft.com/library/default.asp?
//   url=/library/en-us/wcemultimedia5/html/wce50conAVIStreamHeaders.asp
			quicktime_set_position(file, strl->samples_per_chunk_offset);
			wav_id = ((quicktime_codec_t*)(file->atracks[0].codec))->wav_id;
			samples_per_chunk = wav_id == 1 ? 1 :
				quicktime_avg_chunk_samples(file, trak);
			quicktime_write_int32_le(file, samples_per_chunk);
			sample_rate = trak->mdia.minf.stbl.stsd.table[0].sample_rate;
			quicktime_write_int32_le(file, sample_rate);
			quicktime_set_position(file, strl->sample_size_offset);

// dwSampleSize as per MSDN
// as per http://www.virtualdub.org/blog/pivot/entry.php?id=27, many programs ignore this value
// sample_size in quicktime is in bits... so we must divide by 8
// FIXME: This has to be zero for _all_ VBR encodings, and other values are used for specific encodings
			sample_size = trak->mdia.minf.stbl.stsd.table[0].sample_size;
			quicktime_write_int32_le(file, wav_id != 1 ? sample_size :
				trak->mdia.minf.stbl.stsd.table[0].channels * sample_size / 8);
		}
	}

	if(total_frames)
	{
		quicktime_set_position(file, hdrl->bitrate_offset);
		quicktime_write_int32_le(file,
			file->total_length / (total_frames / frame_rate));
		quicktime_set_position(file, hdrl->frames_offset);
		quicktime_write_int32_le(file, total_frames);
	}

	quicktime_set_position(file, position);
}







