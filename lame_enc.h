// LAME encoder wrapper
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

extern "C" {
	#include <lame.h>
}
#include <assert.h>
#include <stdio.h>
#include "platform.h"

class LameEnc
{
protected:
	lame_t lame;
	unsigned char *mp3buff;
	FILE *outf;
	int in_limit;

public:
	LameEnc()
	{
		lame = 0;
		mp3buff = 0;
		outf = 0;
	}

	~LameEnc()
	{
		close();
	}

	bool open(const char *fname, int num_channels, int sample_rate, int bits_per_sample, int data_byte_len)
	{

		assert(num_channels == 1 || num_channels == 2);
		close();

		lame = lame_init();
		if (!lame) {
			debuglog("lame: could not init\n");
			return false;
		}

		mp3buff = (unsigned char *)malloc(LAME_MAXMP3BUFFER);
		if (!mp3buff) {
			debuglog("lame: can allocate mp3 output buffer\n");
			close();
			return false;
		}

		outf = fopen(fname, "wb");
		if (!outf) {
			debuglog("lame: can't open mp3 output file\n");
			close();
			return false;
		}

		// set encoding to VBR, standard quality
		lame_set_VBR_q(lame, 2);
		lame_set_VBR(lame, vbr_default);

		// set input file parameters
		if (lame_set_num_channels(lame, num_channels) == -1) {
			debuglog("lame: unsupported number of channels\n");
			close();
			return false;
		}
		if (num_channels == 1) lame_set_mode(lame, MONO);
		if (lame_set_in_samplerate(lame, sample_rate) == -1) {
			debuglog("lame: unsupported sample rate\n");
			close();
			return false;
		}
		lame_set_num_samples(lame, data_byte_len / (num_channels * ((bits_per_sample + 7) / 8)));

		lame_set_write_id3tag_automatic(lame, 1);

		if (lame_init_params(lame) < 0) {
			debuglog("lame: param init error\n");
			close();
			return false;
		}

		// find max number of samples
		in_limit = lame_get_maximum_number_of_samples(lame, LAME_MAXMP3BUFFER);
		if (in_limit < 1)
			in_limit = 1;

		return true;
	}

	void close()
	{
		if (outf) {
			int n = lame_encode_flush(lame, mp3buff, LAME_MAXMP3BUFFER);
			if (n < 0 || fwrite(mp3buff, n, 1, outf) != 1)
				debuglog("lame: could not flush mp3 file\n");

			fclose(outf);
			outf = 0;
		}
		if (mp3buff) {
			free(mp3buff);
			mp3buff = 0;
		}
		if (lame) {
			lame_close(lame);
			lame = 0;
		}
	}

	// mp3 encode left and right channel with number of samples and write result to file
	bool write(const int* left, const int *right, int samples)
	{
		assert(lame && outf);

		int i = lame_encode_buffer_int(lame, left, right, samples, mp3buff, LAME_MAXMP3BUFFER);
		if (i < 0) {
			debuglog("lame: encoding failed num samples: %d\n", samples);
			return false;
		}
		return (fwrite(mp3buff, i, 1, outf) == 1);
	}

	int get_in_limit()
	{
		assert(lame);
		return in_limit;
	}
};
