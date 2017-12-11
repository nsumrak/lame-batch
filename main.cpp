// main.cpp - code demonstrating multi-threaded encoding using libLAME
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


// pthread-w32 VS 2015 config
#define HAVE_STRUCT_TIMESPEC

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include "thread_signal.h"
#include "platform.h"
#include "wavreader.h"
#include "lame_enc.h"

// number of samples read in every chunk of data
#define NUM_SAMPLES_CHUNK	1152

// set this to true if you want to recurse subdirectories
const bool gc_recursive = false;

ThreadSignal g_ts(getNumberOfCores());


// read mono stream
int read_mono(WavReader &wr, int *left, int *right, int snum)
{
	int sread;
	for (sread = 0; sread < snum && wr.readsample(left[sread]); sread++);
	memset(right, 0, sizeof(int)*sread);
	return sread;
}

// read stereo stream
int read_stereo(WavReader &wr, int *left, int *right, int snum)
{
	int sread;
	for (sread = 0; sread < snum && wr.readsample(left[sread]) && wr.readsample(right[sread]); sread++);
	return sread;
}

// encoding thread
void *encode_file_thread(void *vfn)
{
	if (!vfn) {
		g_ts.signal_end();
		return 0;
	}

	WavReader wr;
	LameEnc le;
	int left[NUM_SAMPLES_CHUNK], right[NUM_SAMPLES_CHUNK];
	char *fn = (char *)vfn;

	if (!wr.open(fn))
		debuglog("thread file [%s] is not valid WAV\n", fn); 
	else {
		strcpy(fn + strlen(fn) - 3, "mp3");
		if (!le.open(fn, wr.get_num_channels(), wr.get_sample_rate(), wr.get_bits_per_sample(), wr.get_data_byte_len()))
			debuglog("thread: error opening encoder\n");
		else {
			debuglog("thread open [%s]\n", fn);

			int snum = le.get_in_limit();
			if (snum > NUM_SAMPLES_CHUNK) snum = NUM_SAMPLES_CHUNK;

			// main encoding loop
			while (!wr.is_eof()) {
				int sread = (wr.get_num_channels() == 1 ? read_mono(wr, left, right, snum) : read_stereo(wr, left, right, snum));
				le.write(left, right, sread);
			}

			le.close();
			debuglog("thread [%s] finished!\n", fn);
		}
		wr.close();
	}

	free(vfn);
	g_ts.signal_end();
	return 0;
}

enum FType {
	FT_OTHER, FT_REGULAR, FT_DIRECTORY
};

// helper to check whether file is regular or directory (or other)
static FType get_file_type(const char *f)
{
	struct stat fs;
	if (stat(f, &fs)) return FT_OTHER;
	if (S_ISDIR(fs.st_mode)) return FT_DIRECTORY;
	if (S_ISREG(fs.st_mode)) return FT_REGULAR;
	return FT_OTHER;
}

// open wav file and launch encoding thread
bool openfile(const char *fname)
{
	int slen = strlen(fname);
	// convert only wav files
	if (slen > 4 && !strcasecmp(fname + slen - 4, ".wav")) {
		pthread_t th;
		g_ts.wait_for_signal();
		g_ts.new_thread();
		int rc = pthread_create(&th, NULL, encode_file_thread, strdup(fname));
		if (rc) {
			debuglog("ERROR: return code from pthread_create() is %d\n", rc);
			return false;
		}
	}
	return true;
}

// process input path
bool processfile(char *fname)
{
	switch (get_file_type(fname)) {

		case FT_DIRECTORY:
		{
			// open directory
			debuglog("opening directory %s\n", fname);
			int slen = strlen(fname);
			if (fname[slen - 1] != SLASH) {
				fname[slen++] = SLASH;
				fname[slen] = 0;
			}

			struct dirent *de;
			DIR *d = opendir(fname);
			if (!d) return false;
			while ((de = readdir(d)) != NULL) {
				if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) continue;
				// check weather new path would fit to the buffer
				if (strlen(de->d_name) + slen + 1 > MAX_PATH) {
					debuglog("ERROR: could not open %s in dir %s, path too long\n", de->d_name, fname);
					continue;
				}
				strcat(fname, de->d_name);
				if (gc_recursive)
					processfile(fname);
				else {
					if (get_file_type(fname) == FT_REGULAR)
						openfile(fname);
				}
				fname[slen] = 0;
			}
			closedir(d);
			break;
		}

		case FT_REGULAR:
			openfile(fname);
			break;

		default:
			return false;

	}
	return true;
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		debuglog("USAGE: %s <path>\n", argv[0]);
		exit(-2);
	}

	char fname[MAX_PATH];
	strcpy(fname, argv[1]);
	if (!processfile(fname)) {
		debuglog("failed to open file %s!\n",argv[1]);
	}

	debuglog("main: waiting to finish\n");
	g_ts.wait_for_finish();
	debuglog("main: finished\n");

	return 0;
}
