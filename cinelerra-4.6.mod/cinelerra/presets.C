
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
#include "bcwindowbase.inc"
#include "cstrdup.h"
#include "filesystem.h"
#include "filexml.h"
#include "keyframe.h"
#include "messages.inc"
#include "mwindow.h"
#include "pluginserver.h"
#include "preferences.inc"
#include "presets.h"

#include <errno.h>
#include <string.h>
 
PresetsDB::PresetsDB()
{
}


void PresetsDB::clear()
{
	plugins.remove_all_objects();
}

void PresetsDB::load()
{
	clear();

	FileXML file;
	char path[BCTEXTLEN];
	char string[BCTEXTLEN];
	sprintf(path, "%s%s", BCASTDIR, PRESETS_FILE);
	FileSystem fs;
	fs.complete_path(path);
	file.read_from_file(path);
	int result = 0;

	do
	{
		result = file.read_tag();
		if(!result)
		{
			if(file.tag.title_is("PLUGIN"))
			{
				PresetsDBPlugin *plugin = 0;
				sprintf(string, "Unknown");
				const char *title = file.tag.get_property("TITLE", string);

// Search for existing plugin
				for(int i = 0; i < plugins.size(); i++)
				{
					if(!strcasecmp(plugins.get(i)->title, title))
					{
						plugin = plugins.get(i);
						break;
					}
				}

// Create new plugin
				if(!plugin)
				{
					plugin = new PresetsDBPlugin(title);
					plugins.append(plugin);
				}

				plugin->load(&file);
			}
		}
	}while(!result);
}

void PresetsDB::save()
{
	FileXML file;
	for(int i = 0; i < plugins.size(); i++)
	{
		PresetsDBPlugin *plugin = plugins.get(i);
		plugin->save(&file);
	}
	file.terminate_string();

	char path[BCTEXTLEN];
	sprintf(path, "%s%s", BCASTDIR, PRESETS_FILE);
	FileSystem fs;
	fs.complete_path(path);
	file.write_to_file(path);
}


int PresetsDB::get_total_presets(char *plugin_title)
{
	for(int i = 0; i < plugins.size(); i++)
	{
		PresetsDBPlugin *plugin = plugins.get(i);
		if(!strcasecmp(plugin->title, plugin_title))
		{
			return plugin->keyframes.size();
		}
	}

	return 0;
}

char* PresetsDB::get_preset_title(char *plugin_title, int number)
{
	for(int i = 0; i < plugins.size(); i++)
	{
		PresetsDBPlugin *plugin = plugins.get(i);
		if(!strcasecmp(plugin->title, plugin_title))
		{
			if(number < plugin->keyframes.size())
			{
				return plugin->keyframes.get(number)->title;
			}
			else
			{
				printf("PresetsDB::get_preset_title %d buffer overrun\n", __LINE__);
			}
			break;
		}
	}
	return 0;
}

char* PresetsDB::get_preset_data(char *plugin_title, int number)
{
	for(int i = 0; i < plugins.size(); i++)
	{
		PresetsDBPlugin *plugin = plugins.get(i);
		if(!strcasecmp(plugin->title, plugin_title))
		{
			if(number < plugin->keyframes.size())
			{
				return plugin->keyframes.get(number)->data;
			}
			else
			{
				printf("PresetsDB::get_preset_data %d buffer overrun\n", __LINE__);
			}
			break;
		}
	}
	return 0;
}

PresetsDBPlugin* PresetsDB::get_plugin(const char *plugin_title)
{
	for(int i = 0; i < plugins.size(); i++)
	{
		PresetsDBPlugin *plugin = plugins.get(i);
		if(!strcasecmp(plugin->title, plugin_title))
		{
			return plugin;
		}
	}
	return 0;
}

PresetsDBPlugin* PresetsDB::new_plugin(const char *plugin_title)
{
	PresetsDBPlugin *result = new PresetsDBPlugin(plugin_title);
	plugins.append(result);
	return result;
}


void PresetsDB::save_preset(const char *plugin_title, const char *preset_title, char *data)
{
	PresetsDBPlugin *plugin = get_plugin(plugin_title);
	if(!plugin) plugin = new_plugin(plugin_title);
	PresetsDBKeyframe *keyframe = plugin->get_keyframe(preset_title);
	if(!keyframe) keyframe = plugin->new_keyframe(preset_title);
	keyframe->set_data(data);
	save();

}


void PresetsDB::delete_preset(const char *plugin_title, const char *preset_title)
{
	PresetsDBPlugin *plugin = get_plugin(plugin_title);
	if(plugin)
	{
		plugin->delete_keyframe(preset_title);
	}
	save();
}

void PresetsDB::load_preset(const char *plugin_title, const char *preset_title, KeyFrame *keyframe)
{
	PresetsDBPlugin *plugin = get_plugin(plugin_title);
	if(plugin)
	{
		plugin->load_preset(preset_title, keyframe);
	}
}

int PresetsDB::preset_exists(const char *plugin_title, const char *preset_title)
{
	PresetsDBPlugin *plugin = get_plugin(plugin_title);
	if(plugin)
	{
		return plugin->preset_exists(preset_title);
	}
	return 0;
}




PresetsDBKeyframe::PresetsDBKeyframe(const char *title)
{
	this->title = cstrdup(title);
	data = 0;
}

PresetsDBKeyframe::~PresetsDBKeyframe()
{
	delete [] title;
	delete [] data;
}

void PresetsDBKeyframe::set_data(char *data)
{
	delete [] this->data;
	this->data = cstrdup(data);
}



PresetsDBPlugin::PresetsDBPlugin(const char *title)
{
	this->title = cstrdup(title);
}

PresetsDBPlugin::~PresetsDBPlugin()
{
	keyframes.remove_all_objects();
	delete [] title;
}

void PresetsDBPlugin::load(FileXML *file)
{
	int result = 0;
	char string[BCTEXTLEN];

	do
	{
		result = file->read_tag();
		if(!result)
		{
			if(file->tag.title_is("/PLUGIN")) break;
			else
			if(file->tag.title_is("KEYFRAME"))
			{
				sprintf(string, "Unknown");
				const char *keyframe_title = file->tag.get_property("TITLE", string);
				PresetsDBKeyframe *keyframe = new PresetsDBKeyframe(keyframe_title);

				char data[MESSAGESIZE];
				int len = file->read_data_until("/KEYFRAME", data, MESSAGESIZE-1);
				data[len] = 0;
				keyframe->set_data(data);
				keyframes.append(keyframe);
		
			}
		}
	}while(!result);

	
}

void PresetsDBPlugin::save(FileXML *file)
{
	file->tag.set_title("PLUGIN");
	file->tag.set_property("TITLE", title);
	file->append_tag();
	file->append_newline();

	for(int j = 0; j < keyframes.size(); j++)
	{
		PresetsDBKeyframe *keyframe = keyframes.get(j);
		file->tag.set_title("KEYFRAME");
		file->tag.set_property("TITLE", keyframe->title);
		file->append_tag();
		file->append_text(keyframe->data);
		file->tag.set_title("/KEYFRAME");
		file->append_tag();
		file->append_newline();
	}

	file->tag.set_title("/PLUGIN");
	file->append_tag();
	file->append_newline();
}

PresetsDBKeyframe* PresetsDBPlugin::get_keyframe(const char *title)
{
	for(int i = 0; i < keyframes.size(); i++)
	{
		PresetsDBKeyframe *keyframe = keyframes.get(i);
		if(!strcasecmp(keyframe->title, title)) return keyframe;
	}
	return 0;
}

void PresetsDBPlugin::delete_keyframe(const char *title)
{
	for(int i = 0; i < keyframes.size(); i++)
	{
		PresetsDBKeyframe *keyframe = keyframes.get(i);
		if(!strcasecmp(keyframe->title, title)) 
		{
			keyframes.remove_object_number(i);
			return;
		}
	}
}


PresetsDBKeyframe* PresetsDBPlugin::new_keyframe(const char *title)
{
	PresetsDBKeyframe *keyframe = new PresetsDBKeyframe(title);
	keyframes.append(keyframe);
	return keyframe;
}

void PresetsDBPlugin::load_preset(const char *preset_title, KeyFrame *keyframe)
{
	PresetsDBKeyframe *src = get_keyframe(preset_title);
	if(src)
	{
		keyframe->set_data(src->data);
// Save as the plugin's default
// Need the path
//printf("PresetsDBPlugin::load_preset %d %s\n", __LINE__, title);
		PluginServer *server = MWindow::scan_plugindb(title, -1);
		if(!server)
		{
		}
		else
		{
			char path[BCTEXTLEN];
			server->get_defaults_path(path);
			FileSystem fs;
			fs.complete_path(path);

			FILE *fd = fopen(path, "w");
			if(fd)
			{
				if(!fwrite(src->data, strlen(src->data), 1, fd))
				{
					fprintf(stderr, "PresetsDBPlugin::load_preset %d \"%s\": %s\n",
						__LINE__,
						path,
						strerror(errno));
				}

				fclose(fd);
			}
			else
			{
				fprintf(stderr, "PresetsDBPlugin::load_preset %d \"%s\": %s\n",
					__LINE__,
					path,
					strerror(errno));
			}
		}
	}
}

int PresetsDBPlugin::preset_exists(const char *preset_title)
{
	PresetsDBKeyframe *src = get_keyframe(preset_title);
	if(src)
	{
		return 1;
	}
	return 0;
}







