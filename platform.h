// platform specific definitions
// currently supports Windows and Linux
//
// Copyright(C) 2017. Nebojsa Sumrak <nsumrak@yahoo.com>
//
//   This program is free software; you can redistribute it and / or modify
//	 it under the terms of the GNU General Public License as published by
//	 the Free Software Foundation; either version 2 of the License, or
//	 (at your option) any later version.
//
//	 This program is distributed in the hope that it will be useful,
//	 but WITHOUT ANY WARRANTY; without even the implied warranty of
//	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//	 GNU General Public License for more details.
//
//	 You should have received a copy of the GNU General Public License along
//	 with this program; if not, write to the Free Software Foundation, Inc.,
//	 51 Franklin Street, Fifth Floor, Boston, MA 02110 - 1301 USA.

#pragma once

#ifdef _WIN32

#include <windows.h>

inline int getNumberOfCores()
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
}

#define SLASH '\\'

#define PACKED
typedef long int32_t;
typedef short int16_t;

#ifdef _DEBUG
#define debuglog printf
#else
#define debuglog(...)
#endif

#define strcasecmp stricmp
inline void sleep(int sec) { ::Sleep(sec * 1000); }

#else // Linux

#include <unistd.h>

#ifdef DEBUG
#define debuglog printf
#else
#define debuglog(...)
#endif

#define SLASH '/'
#define MAX_PATH 256

#define PACKED __attribute__((packed))

inline int getNumberOfCores()
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

#ifdef BYTE_ORDER
	// In gcc compiler detect the byte order automatically
	#if BYTE_ORDER == BIG_ENDIAN
		// big-endian platform.
		#error Only little-endian (eg. Intel) architecture is currently supported
	#endif
#endif

#endif
