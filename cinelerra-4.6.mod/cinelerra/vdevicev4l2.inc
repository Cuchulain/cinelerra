
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

#ifndef VDEVICEV4L2_INC
#define VDEVICEV4L2_INC


// Short delay is necessary whenn capturing a lousy source.
//#define V4L2_BUFFER_TIMEOUT 250000

// Long delay is necessary to avoid losing synchronization due to spurrious
// resets.
#define V4L2_BUFFER_TIMEOUT 10000000

class VDeviceV4L2;
class VDeviceV4L2Thread;
class VDeviceV4L2Put;


#endif
